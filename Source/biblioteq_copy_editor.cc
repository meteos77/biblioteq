/*
** Copyright (c) 2006 - present, Alexis Megas.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from BiblioteQ without specific prior written permission.
**
** BIBLIOTEQ IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** BIBLIOTEQ, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "biblioteq.h"
#include "biblioteq_copy_editor.h"

#include <QScrollBar>
#include <QSettings>
#include <QTimer>
#include <QtMath>

biblioteq_copy_editor::biblioteq_copy_editor
(QWidget *parent, biblioteq *biblioteq):QDialog(parent)
{
  m_bitem = nullptr;
  m_parent = parent;
  m_quantity = 1;
  m_showForLending = false;
  m_speedy = false;
  m_spinbox = nullptr;
  qmain = biblioteq;
}

biblioteq_copy_editor::biblioteq_copy_editor
(QWidget *parent,
 biblioteq *biblioteq,
 biblioteq_item *bitemArg,
 const bool showForLendingArg,
 const int quantityArg,
 const QString &ioidArg,
 QSpinBox *spinboxArg,
 const QFont &font,
 const QString &itemTypeArg,
 const QString &uniqueIdArg,
 const bool speedy):QDialog(parent)
{
  m_bitem = bitemArg;
  m_cb.setupUi(this);
  m_cb.deleteButton->setVisible(false);
  m_cb.information->setVisible(showForLendingArg);
  m_cb.table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  m_ioid = ioidArg;
  m_itemType = itemTypeArg;
  m_parent = parent;
  m_quantity = quantityArg >= 1 ?
    quantityArg :
    biblioteq_misc_functions::quantity(biblioteq->getDB(), m_ioid, m_itemType);
  m_showForLending = showForLendingArg;
  m_speedy = speedy;
  m_spinbox = spinboxArg;
  m_uniqueIdArg = uniqueIdArg;
  prepareIcons();
  qmain = biblioteq;
  setGlobalFonts(font);
  setWindowModality(Qt::ApplicationModal);

  if(!uniqueIdArg.trimmed().isEmpty())
    setWindowTitle(tr("BiblioteQ: Copy Browser (%1)").arg(uniqueIdArg));
  else
    setWindowTitle(tr("BiblioteQ: Copy Browser"));
}

biblioteq_copy_editor::~biblioteq_copy_editor()
{
  clearCopiesList();
}

QString biblioteq_copy_editor::saveCopies(void)
{
  QSqlQuery query(qmain->getDB());
  QString lastError("");
  copy_class *copy = nullptr;
  int i = 0;

  query.prepare
    (QString("DELETE FROM %1_copy_info WHERE item_oid = ?").
     arg(m_itemType.toLower().remove(" ")));
  query.addBindValue(m_ioid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to purge copy data."),
		      query.lastError().text(),
		      __FILE__,
		      __LINE__);
      return query.lastError().text();
    }
  else
    {
      QApplication::restoreOverrideCursor();

      QProgressDialog progress(this);

      progress.setCancelButton(nullptr);
      progress.setLabelText(tr("Saving the copy data..."));
      progress.setMaximum(m_copies.size());
      progress.setMinimum(0);
      progress.setMinimumWidth
	(qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	       progress.sizeHint().width()));
      progress.setModal(true);
      progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
      progress.show();
      progress.repaint();
      QApplication::processEvents();

      int max = 0;

      for(i = 0; i < m_copies.size(); i++)
	max = qMax(m_copies.at(i)->m_copynumber.toInt(), max);

      for(i = 0; i < m_copies.size(); i++)
	{
	  copy = m_copies.at(i);

	  if(!copy)
	    goto continue_label;

	  if(qmain->getDB().driverName() != "QSQLITE")
	    query.prepare(QString("INSERT INTO %1_copy_info "
				  "(item_oid, "
				  "copy_number, "
				  "copyid, "
				  "status, "
				  "notes) "
				  "VALUES "
				  "(?, ?, ?, ?, ?)").arg
			  (m_itemType.toLower().remove(" ")));
	  else
	    query.prepare(QString("INSERT INTO %1_copy_info "
				  "(item_oid, "
				  "copy_number, "
				  "copyid, "
				  "myoid, "
				  "status, "
				  "notes) "
				  "VALUES (?, ?, ?, ?, ?, ?)").
			  arg(m_itemType.toLower().remove(" ")));

	  query.addBindValue(copy->m_itemoid);

	  if(copy->m_copynumber.isEmpty())
	    {
	      max += 1;
	      query.addBindValue(max);
	    }
	  else
	    query.addBindValue(copy->m_copynumber.toInt());

	  query.addBindValue(copy->m_copyid);

	  if(qmain->getDB().driverName() == "QSQLITE")
	    {
	      QString errorstr("");
	      qint64 value = 0;

	      value = biblioteq_misc_functions::getSqliteUniqueId
		(qmain->getDB(), errorstr);

	      if(errorstr.isEmpty())
		query.addBindValue(value);
	      else
		{
		  lastError = errorstr;
		  qmain->addError(tr("Database Error"),
				  tr("Unable to generate a unique integer."),
				  errorstr);
		}
	    }

	  query.addBindValue(copy->m_status);
	  query.addBindValue(copy->m_notes);

	  if(!query.exec())
	    {
	      if(lastError.isEmpty())
		lastError = query.lastError().text();

	      qmain->addError(tr("Database Error"),
			      tr("Unable to create copy data."),
			      query.lastError().text(),
			      __FILE__,
			      __LINE__);
	    }

	continue_label:

	  if(i + 1 <= progress.maximum())
	    progress.setValue(i + 1);

	  progress.repaint();
	  QApplication::processEvents();
	}

      progress.close();
    }

  return lastError;
}

void biblioteq_copy_editor::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  m_cb.retranslateUi(this);
	  break;
	}
      default:
	break;
      }

  QDialog::changeEvent(event);
}

void biblioteq_copy_editor::clearCopiesList(void)
{
  for(int i = 0; i < m_copies.size(); i++)
    delete m_copies.at(i);

  m_copies.clear();
}

void biblioteq_copy_editor::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event);
  slotCloseCopyEditor();
}

void biblioteq_copy_editor::keyPressEvent(QKeyEvent *event)
{
  if(event && event->key() == Qt::Key_Escape)
    slotCloseCopyEditor();

  QDialog::keyPressEvent(event);
}

void biblioteq_copy_editor::populateCopiesEditor(void)
{
  QComboBox *comboBox = nullptr;
  QSqlQuery query(qmain->getDB());
  QString str = "";
  QStringList list;
  QTableWidgetItem *item = nullptr;
  auto terminate = false;
  int i = 0;
  int j = 0;
  int row = 0;

  m_cb.deleteButton->setVisible(false);
  m_cb.dueDateFrame->setVisible(m_showForLending);

  if(!m_showForLending)
    {
      disconnect(m_cb.deleteButton, SIGNAL(clicked(void)));
      disconnect(m_cb.saveButton, SIGNAL(clicked(void)));
      connect(m_cb.deleteButton,
	      SIGNAL(clicked(void)),
	      this,
	      SLOT(slotDeleteCopy(void)));
      connect(m_cb.saveButton,
	      SIGNAL(clicked(void)),
	      this,
	      SLOT(slotSaveCopies(void)));
      m_cb.saveButton->setText(tr("&Save"));
    }
  else
    {
      /*
      ** Set the minimum date to duedate.
      */

      QString errorstr1("");
      QString errorstr2("");
      auto const memberid
	(biblioteq_misc_functions::
	 getColumnString(qmain->getBB().table,
			 qmain->getBB().table->currentRow(),
			 qmain->getBBColumnIndexes().indexOf("Member ID")));
      auto duedate = QDate::currentDate();
      int maximumReserved = 0;
      qint64 totalReserved = 0;

      QApplication::setOverrideCursor(Qt::WaitCursor);
      duedate = duedate.addDays
	(biblioteq_misc_functions::
	 getMinimumDays(qmain->getDB(), m_itemType, errorstr1));
      maximumReserved = biblioteq_misc_functions::maximumReserved
	(qmain->getDB(), memberid, m_itemType);

      if(m_itemType == "Book")
	totalReserved = biblioteq_misc_functions::getItemsReservedCounts
	  (qmain->getDB(), memberid, errorstr2).value("numbooks");
      else
	{
	  auto type(QString("num%1s").arg(m_itemType.toLower()));

	  type = type.remove(' ').remove('_');
	  totalReserved = biblioteq_misc_functions::getItemsReservedCounts
	    (qmain->getDB(), memberid, errorstr2).value(type);
	}

      QApplication::restoreOverrideCursor();

      if(!errorstr1.isEmpty())
	qmain->addError(tr("Database Error"),
			tr("Unable to retrieve the minimum number of days."),
			errorstr1,
			__FILE__,
			__LINE__);

      m_cb.dueDate->setMinimumDate(duedate);
      m_cb.information->setText
	(tr("Maximum %1s Reserved %2 | Total %1s Reserved %3").
	 arg(m_itemType).arg(maximumReserved).arg(totalReserved));
      m_cb.saveButton->setText(tr("&Reserve"));
      disconnect(m_cb.saveButton, SIGNAL(clicked(void)));
      connect(m_cb.saveButton,
	      SIGNAL(clicked(void)),
	      this,
	      SLOT(slotCheckoutCopy(void)));
    }

  disconnect(m_cb.cancelButton, SIGNAL(clicked(void)));
  connect(m_cb.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotCloseCopyEditor(void)));
  list.append(tr("Title"));
  list.append(tr("Barcode"));
  list.append(tr("Availability"));
  list.append(tr("Status"));
  list.append(tr("Notes"));
  list.append("ITEM_OID");
  list.append("Copy Number");
  m_columnHeaderIndexes.clear();
  m_columnHeaderIndexes.append("Title");
  m_columnHeaderIndexes.append("Barcode");
  m_columnHeaderIndexes.append("Availability");
  m_columnHeaderIndexes.append("Status");
  m_columnHeaderIndexes.append("Notes");
  m_columnHeaderIndexes.append("ITEM_OID");
  m_columnHeaderIndexes.append("Copy Number");
  m_cb.table->horizontalScrollBar()->setValue(0);
  m_cb.table->setColumnCount(0);
  m_cb.table->setColumnCount(list.size());
  m_cb.table->setHorizontalHeaderLabels(list);
  m_cb.table->setRowCount(0);
  m_cb.table->setRowCount(m_quantity);
  m_cb.table->scrollToTop();

  /*
  ** Hide the Copy Number and ITEM_OID columns.
  */

  m_cb.table->setColumnHidden(m_cb.table->columnCount() - 1, true);
  m_cb.table->setColumnHidden(m_cb.table->columnCount() - 2, true);
  updateGeometry();
#ifdef Q_OS_ANDROID
  showMaximized();
#else
  show();
#endif
  QApplication::processEvents();

  QProgressDialog progress1(this);
  QProgressDialog progress2(this);

  progress1.setLabelText(tr("Constructing objects..."));
  progress1.setMaximum(m_quantity);
  progress1.setMinimum(0);
  progress1.setMinimumWidth
    (qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	   progress1.sizeHint().width()));
  progress1.setModal(true);
  progress1.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress1.show();
  progress1.repaint();
  QApplication::processEvents();

  for(i = 0; i < m_quantity && !progress1.wasCanceled(); i++)
    for(j = 0; j < m_cb.table->columnCount(); j++)
      {
	if(j == static_cast<int> (Columns::STATUS))
	  {
	    comboBox = new QComboBox();
	    comboBox->addItem(tr("Available"));
	    comboBox->addItem(tr("Deleted"));
	    comboBox->addItem(tr("Expired"));
	    comboBox->addItem(tr("Lost"));
	    comboBox->addItem(tr("Not Available"));
	    comboBox->addItem(tr("Recalled"));
	    comboBox->addItem(biblioteq::s_unknown);
	    comboBox->setEnabled(!m_showForLending);
	    biblioteq_misc_functions::sortCombinationBox(comboBox);
	    goto done_label;
	  }
	else
	  comboBox = nullptr;

	item = new QTableWidgetItem();

	if(m_showForLending)
	  item->setFlags(Qt::NoItemFlags);
	else if(j == static_cast<int> (Columns::BARCODE) ||
		j == static_cast<int> (Columns::NOTES))
	  item->setFlags
	    (Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	else
	  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

	if(j == static_cast<int> (Columns::AVAILABILITY))
	  {
	    if(m_showForLending)
	      item->setText("0");
	    else
	      item->setText("1");
	  }
	else if(j == static_cast<int> (Columns::BARCODE))
	  {
	    if(!m_uniqueIdArg.isEmpty())
	      item->setText(m_uniqueIdArg + "-" + QString::number(i + 1));
	    else
	      item->setText("");
	  }
	else if(j == static_cast<int> (Columns::MYOID))
	  item->setText(m_ioid);
	else
	  item->setText("");

	m_cb.table->setItem(i, j, item);

      done_label:

	if(comboBox)
	  m_cb.table->setCellWidget(i, j, comboBox);

	if(i + 1 <= progress1.maximum())
	  progress1.setValue(i + 1);

	progress1.repaint();
	QApplication::processEvents();
      }

  progress1.close();
  m_cb.table->setRowCount(i); // Support cancellation.

  if(m_itemType == "Grey Literature")
    {
      query.prepare("SELECT grey_literature.document_title, "
		    "'1', "
		    "1, "
		    "'', "
		    "0, "
		    "1 "
		    "FROM "
		    "grey_literature WHERE grey_literature.myoid = ?");
      query.addBindValue(m_ioid);
    }
  else
    {
      query.prepare(QString("SELECT %1.title, "
			    "%1_copy_info.copyid, "
			    "(1 - COUNT(item_borrower.copyid)), "
			    "%1_copy_info.status, "
			    "%1_copy_info.notes, "
			    "%1_copy_info.item_oid, "
			    "%1_copy_info.copy_number "
			    "FROM "
			    "%1, "
			    "%1_copy_info LEFT JOIN item_borrower ON "
			    "%1_copy_info.copyid = item_borrower.copyid AND "
			    "%1_copy_info.item_oid = "
			    "item_borrower.item_oid AND "
			    "item_borrower.type = ? "
			    "WHERE %1_copy_info.item_oid = ? AND "
			    "%1.myoid = ? "
			    "GROUP BY %1.title, "
			    "%1_copy_info.copyid, "
			    "%1_copy_info.status, "
			    "%1_copy_info.notes, "
			    "%1_copy_info.item_oid, "
			    "%1_copy_info.copy_number "
			    "ORDER BY %1_copy_info.copy_number").
		    arg(m_itemType.toLower().remove(" ")));
      query.addBindValue(m_itemType);
      query.addBindValue(m_ioid);
      query.addBindValue(m_ioid);
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    qmain->addError(tr("Database Error"),
		    tr("Unable to retrieve copy data."),
		    query.lastError().text(),
		    __FILE__,
		    __LINE__);

  QApplication::restoreOverrideCursor();
  progress2.setLabelText(tr("Retrieving copy information..."));
  progress2.setMinimum(0);
  progress2.setMinimumWidth
    (qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	   progress2.sizeHint().width()));
  progress2.setModal(true);
  progress2.setWindowTitle(tr("BiblioteQ: Progress Dialog"));

  /*
  ** SQLite does not support query.size().
  */

  if(qmain->getDB().driverName() == "QSQLITE")
    {
      if(query.lastError().isValid())
	progress2.setMaximum(0);
      else
	progress2.setMaximum
	  (biblioteq_misc_functions::sqliteQuerySize(query.lastQuery(),
						     query.boundValues(),
						     qmain->getDB(),
						     __FILE__,
						     __LINE__,
						     qmain));
    }
  else
    progress2.setMaximum(query.size());

  progress2.show();
  progress2.repaint();
  QApplication::processEvents();
  i = -1;

  while(i++, !progress2.wasCanceled() && query.next())
    {
      if(query.isValid())
	{
	  row = i;

	  for(j = 0; j < m_cb.table->columnCount(); j++)
	    if(m_cb.table->cellWidget(row, j) != nullptr)
	      {
		str = query.value(j).toString().trimmed();

		auto comboBox = qobject_cast<QComboBox *>
		  (m_cb.table->cellWidget(row, j));

		if(comboBox)
		  {
		    if(comboBox->findText(str) >= 0)
		      comboBox->setCurrentIndex(comboBox->findText(str));
		    else
		      comboBox->setCurrentIndex
			(comboBox->findText(biblioteq::s_unknown));
		  }
	      }
	    else if(m_cb.table->item(row, j) != nullptr)
	      {
		str = query.value(j).toString().trimmed();

		if(query.value(static_cast<int> (Columns::AVAILABILITY)).
		   toString().trimmed() == "0")
		  m_cb.table->item(row, j)->setFlags(Qt::NoItemFlags);
		else if(m_showForLending)
		  {
		    m_cb.table->item(row, j)->setFlags
		      (Qt::ItemIsEnabled | Qt::ItemIsSelectable);

		    if(m_cb.table->currentRow() == -1)
		      m_cb.table->selectRow(row);
		  }
		else if(j == static_cast<int> (Columns::BARCODE) ||
			j == static_cast<int> (Columns::NOTES))
		  m_cb.table->item(row, j)->setFlags
		    (Qt::ItemIsEditable |
		     Qt::ItemIsEnabled |
		     Qt::ItemIsSelectable);

		if(j == static_cast<int> (Columns::AVAILABILITY))
		  m_cb.table->item(row, j)->setText
		    (query.value(j).toString().trimmed());
		else
		  {
		    if(i == 0 && j == static_cast<int> (Columns::TITLE))
		      m_cb.table->item(row, j)->setText(str);
		    else if(j != static_cast<int> (Columns::TITLE))
		      m_cb.table->item(row, j)->setText(str);
		  }
	      }
	    else
	      terminate = true;
	}

      if(i + 1 <= progress2.maximum())
	progress2.setValue(i + 1);

      progress2.repaint();
      QApplication::processEvents();

      if(terminate)
	break; // Out of resources?
    }

  progress2.close();
  m_cb.table->resizeColumnsToContents();
  m_cb.table->resizeRowsToContents();

  if(m_speedy)
    QTimer::singleShot(250, this, SLOT(slotCheckoutCopy(void)));
}

void biblioteq_copy_editor::prepareIcons(void)
{
  QSettings setting;
  auto const index = setting.value
    ("otheroptions/display_icon_set_index", 0).toInt();

  if(index == 1)
    {
      // System.

      m_cb.cancelButton->setIcon
	(QIcon::fromTheme("window-close", QIcon(":/16x16/cancel.png")));
      m_cb.deleteButton->setIcon
	(QIcon::fromTheme("edit-delete", QIcon(":/16x16/eraser.png")));
      m_cb.saveButton->setIcon
	(QIcon::fromTheme("dialog-ok", QIcon(":/16x16/ok.png")));
    }
  else
    {
      // Faenza.

      m_cb.cancelButton->setIcon(QIcon(":/16x16/cancel.png"));
      m_cb.deleteButton->setIcon(QIcon(":/16x16/eraser.png"));
      m_cb.saveButton->setIcon(QIcon(":/16x16/ok.png"));
    }
}

void biblioteq_copy_editor::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach(auto widget, findChildren<QWidget *> ())
    {
      widget->setFont(font);
      widget->update();
    }

  update();
}

void biblioteq_copy_editor::slotCheckoutCopy(void)
{
  QDate now(QDate::currentDate());
  QSqlQuery query(qmain->getDB());
  QString copyid = "";
  QString copynumber = "";
  QString errorstr = "";
  QString memberid = "";
  auto available = false;
  auto const checkedout = now.toString(biblioteq::s_databaseDateFormat);
  auto const copyrow = m_cb.table->currentRow();
  auto const duedate = m_cb.dueDate->date().toString
    (biblioteq::s_databaseDateFormat);
  auto const memberrow = qmain->getBB().table->currentRow();
  auto dnt = true;

  if(copyrow < 0 ||
     m_cb.table->item(copyrow, static_cast<int> (Columns::BARCODE)) == nullptr)
    {
      QMessageBox::critical(this,
			    tr("BiblioteQ: User Error"),
			    tr("Please select a copy to reserve."));
      QApplication::processEvents();
      return;
    }
  else if((m_cb.table->item(copyrow, static_cast<int> (Columns::BARCODE))->
	   flags() & Qt::ItemIsEnabled) == 0)
    {
      QMessageBox::critical(this,
			    tr("BiblioteQ: User Error"),
			    tr("It appears that the copy you have selected "
			       "does not exist or is not available."));
      QApplication::processEvents();
      return;
    }
  else if(m_cb.dueDate->date() <= now)
    {
      QMessageBox::critical(this,
			    tr("BiblioteQ: User Error"),
			    tr("Please select a future Due Date."));
      QApplication::processEvents();
      return;
    }

  copyid = biblioteq_misc_functions::getColumnString
    (m_cb.table, copyrow, m_columnHeaderIndexes.indexOf("Barcode"));
  copynumber = biblioteq_misc_functions::getColumnString
    (m_cb.table, copyrow, m_columnHeaderIndexes.indexOf("Copy Number"));
  memberid = biblioteq_misc_functions::getColumnString
    (qmain->getBB().table,
     memberrow,
     qmain->getBBColumnIndexes().indexOf("Member ID"));
  QApplication::setOverrideCursor(Qt::WaitCursor);
  available = biblioteq_misc_functions::isCopyAvailable
    (qmain->getDB(), m_ioid, copyid, m_itemType, errorstr);
  QApplication::restoreOverrideCursor();

  if(!available && errorstr.length() > 0)
    {
      qmain->addError(tr("Database Error"),
		      tr("Unable to determine the selected copy's "
			 "availability."),
		      errorstr,
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to determine the selected copy's "
			       "availability."));
      QApplication::processEvents();
      return;
    }
  else if(!available)
    {
      QMessageBox::critical(this,
			    tr("BiblioteQ: User Error"),
			    tr("The copy that you have selected does "
			       "not exist or is not available."));
      QApplication::processEvents();
      return;
    }

  query.prepare("INSERT INTO item_borrower "
		"(item_oid, "
		"memberid, "
		"reserved_date, "
		"duedate, "
		"copyid, "
		"copy_number, "
		"reserved_by, "
		"type) "
		"VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
  query.addBindValue(m_ioid);
  query.addBindValue(memberid);
  query.addBindValue(checkedout);
  query.addBindValue(duedate);
  query.addBindValue(copyid);
  query.addBindValue(copynumber);
  query.addBindValue(qmain->getAdminID());
  query.addBindValue(m_itemType);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to create an item_borrower record."),
		      query.lastError().text(),
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to create an item_borrower record."));
      QApplication::processEvents();
      return;
    }
  else
    {
      /*
      ** Delete the request.
      */

      QSqlQuery query(qmain->getDB());

      query.prepare
	("DELETE FROM item_request WHERE "
	 "item_oid = ? AND memberid = ? and type = ?");
      query.addBindValue(m_ioid);
      query.addBindValue(memberid);
      query.addBindValue(m_itemType);
      query.exec();
    }

  /*
  ** Record the reservation in the member's history table.
  */

  dnt = biblioteq_misc_functions::dnt(qmain->getDB(), memberid, errorstr);

  if(!dnt)
    {
      query.prepare("INSERT INTO member_history "
		    "(memberid, "
		    "item_oid, "
		    "copyid, "
		    "reserved_date, "
		    "duedate, "
		    "returned_date, "
		    "reserved_by, "
		    "type) "
		    "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
      query.addBindValue(memberid);
      query.addBindValue(m_ioid);
      query.addBindValue(copyid);
      query.addBindValue(checkedout);
      query.addBindValue(duedate);
      query.addBindValue(QString("N/A"));
      query.addBindValue(qmain->getAdminID());
      query.addBindValue(m_itemType);

      if(!query.exec())
	qmain->addError(tr("Database Error"),
			tr("Unable to create a member_history entry."),
			query.lastError().text(),
			__FILE__,
			__LINE__);
    }

  /*
  ** Update the Reserved Items count on the Members Browser.
  */

  qmain->updateMembersBrowser();
  QApplication::restoreOverrideCursor();

  /*
  ** Update the Availability and Total Reserved columns.
  */

  QApplication::setOverrideCursor(Qt::WaitCursor);

  auto const availability = biblioteq_misc_functions::getAvailability
    (m_ioid, qmain->getDB(), m_itemType, errorstr);
  auto const reserved = biblioteq_misc_functions::getTotalReserved
    (qmain->getDB(), m_itemType, m_ioid);

  QApplication::restoreOverrideCursor();

  if(!availability.isEmpty())
    biblioteq_misc_functions::updateColumn
      (qmain->getUI().table,
       m_bitem->getRow(),
       qmain->getUI().table->columnNumber("Availability"),
       availability);

  if(qmain->availabilityColors())
    {
      QColor color(Qt::white);

      if(availability.toInt() > 0)
	color = qmain->availabilityColor(m_itemType);

      biblioteq_misc_functions::updateColumnColor
	(qmain->getUI().table,
	 m_bitem->getRow(),
	 qmain->getUI().table->columnNumber("Availability"),
	 color);
    }

  if(!reserved.isEmpty())
    biblioteq_misc_functions::updateColumn
      (qmain->getUI().table,
       m_bitem->getRow(),
       qmain->getUI().table->columnNumber("Total Reserved"),
       reserved);

  slotCloseCopyEditor();

  /*
  ** Update the main window's summary panel, if necessary.
  */

  if(m_bitem &&
     m_ioid ==
     biblioteq_misc_functions::getColumnString(qmain->getUI().table,
					       m_bitem->getRow(),
					       "MYOID") &&
     m_itemType ==
     biblioteq_misc_functions::getColumnString(qmain->getUI().table,
					       m_bitem->getRow(),
					       qmain->getUI().table->
					       columnNumber("Type")))
    qmain->slotDisplaySummary();
}

void biblioteq_copy_editor::slotCloseCopyEditor(void)
{
  clearCopiesList();
  deleteLater();
}

void biblioteq_copy_editor::slotDeleteCopy(void)
{
  QString copyid = "";
  QString errorstr = "";
  auto const row = m_cb.table->currentRow();
  auto isCheckedOut = false;

  if(row < 0)
    {
      QMessageBox::critical(this,
			    tr("BiblioteQ: User Error"),
			    tr("Please select the copy that you intend to "
			       "delete."));
      QApplication::processEvents();
      return;
    }
  else if(m_cb.table->rowCount() == 1)
    {
      QMessageBox::critical(this,
			    tr("BiblioteQ: User Error"),
			    tr("You must have at least one copy."));
      QApplication::processEvents();
      return;
    }

  copyid = biblioteq_misc_functions::getColumnString
    (m_cb.table, row, m_columnHeaderIndexes.indexOf("Barcode"));
  QApplication::setOverrideCursor(Qt::WaitCursor);
  isCheckedOut = biblioteq_misc_functions::isCopyCheckedOut(qmain->getDB(),
							    copyid,
							    m_ioid,
							    m_itemType,
							    errorstr);
  QApplication::restoreOverrideCursor();

  if(isCheckedOut)
    {
      if(m_cb.table->item(row, static_cast<int> (Columns::BARCODE)) != nullptr)
	m_cb.table->item(row, static_cast<int> (Columns::BARCODE))->setFlags
	  (Qt::NoItemFlags);

      if(m_cb.table->item(row, static_cast<int> (Columns::AVAILABILITY)) !=
	 nullptr)
	{
	  m_cb.table->item(row, static_cast<int> (Columns::AVAILABILITY))->
	    setFlags(Qt::NoItemFlags);
	  m_cb.table->item(row, static_cast<int> (Columns::AVAILABILITY))->
	    setText("0");
	}

      QMessageBox::critical(this,
			    tr("BiblioteQ: User Error"),
			    tr("It appears that the copy you selected to "
			       "delete is reserved."));
      QApplication::processEvents();
      return;
    }
  else if(errorstr.length() > 0)
    {
      qmain->addError(tr("Database Error"),
		      tr("Unable to determine the reservation "
			 "status of the selected copy."),
		      errorstr,
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to determine the reservation status "
			       "of the selected copy."));
      QApplication::processEvents();
      return;
    }
  else
    m_cb.deleteButton->animate(2500);

  m_cb.table->removeRow(m_cb.table->currentRow());
}

void biblioteq_copy_editor::slotSaveCopies(void)
{
  QComboBox *comboBox = nullptr;
  QString errormsg = "";
  QString errorstr = "";
  QStringList duplicates;
  QTableWidgetItem *item1 = nullptr;
  QTableWidgetItem *item2 = nullptr;
  QTableWidgetItem *item3 = nullptr;
  QTableWidgetItem *item4 = nullptr;
  copy_class *copy = nullptr;
  int i = 0;

  m_cb.table->setFocus();

  for(i = 0; i < m_cb.table->rowCount(); i++)
    if(m_cb.table->item(i, static_cast<int> (Columns::BARCODE)) != nullptr &&
       m_cb.table->item(i, static_cast<int> (Columns::BARCODE))->
       text().trimmed().isEmpty())
      {
	errormsg = tr("Row number ") +
	  QString::number(i + 1) +
	  tr(" contains an empty Barcode.");
	QMessageBox::critical(this, tr("BiblioteQ: User Error"), errormsg);
	QApplication::processEvents();
	return;
      }
    else if(m_cb.table->item(i, static_cast<int> (Columns::BARCODE)) != nullptr)
      {
	if(duplicates.
	   contains(m_cb.table->item(i, static_cast<int> (Columns::BARCODE))->
		    text()))
	  {
	    errormsg = tr("Row number ") +
	      QString::number(i + 1) +
	      tr(" contains a duplicate Barcode.");
	    QMessageBox::critical(this, tr("BiblioteQ: User Error"), errormsg);
	    QApplication::processEvents();
	    return;
	  }
	else
	  duplicates.append
	    (m_cb.table->item(i, static_cast<int> (Columns::BARCODE))->text());
      }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  clearCopiesList();

  if(!qmain->getDB().transaction())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to create a database transaction."),
		      qmain->getDB().lastError().text(),
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to create a database transaction."));
      QApplication::processEvents();
      return;
    }

  QApplication::restoreOverrideCursor();

  for(i = 0; i < m_cb.table->rowCount(); i++)
    {
      comboBox = qobject_cast<QComboBox *>
	(m_cb.table->cellWidget(i, static_cast<int> (Columns::STATUS)));
      item1 = m_cb.table->item(i, static_cast<int> (Columns::BARCODE));
      item2 = m_cb.table->item(i, static_cast<int> (Columns::COPY_NUMBER));
      item3 = m_cb.table->item(i, static_cast<int> (Columns::MYOID));
      item4 = m_cb.table->item(i, static_cast<int> (Columns::NOTES));

      if(comboBox == nullptr ||
	 item1 == nullptr ||
	 item2 == nullptr ||
	 item3 == nullptr ||
	 item4 == nullptr)
	continue;

      copy = new copy_class(item1->text().trimmed(),            // Copy ID
			    item2->text(),                      // Copy Number
			    item3->text(),                      // Item OID
			    item4->text(),                      // Notes
			    comboBox->currentText().trimmed()); // Status
      m_copies.append(copy);
    }

  if(saveCopies().isEmpty())
    {
      QApplication::setOverrideCursor(Qt::WaitCursor);
      biblioteq_misc_functions::saveQuantity
	(qmain->getDB(), m_ioid, m_copies.size(), m_itemType, errorstr);
      QApplication::restoreOverrideCursor();

      if(!errorstr.isEmpty())
	qmain->addError(tr("Database Error"),
			tr("Unable to save the item's quantity."),
			errorstr,
			__FILE__,
			__LINE__);
      else
	goto success_label;
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  clearCopiesList();

  if(!qmain->getDB().rollback())
    qmain->addError(tr("Database Error"),
		    tr("Rollback failure."),
		    qmain->getDB().lastError().text(),
		    __FILE__,
		    __LINE__);

  QApplication::restoreOverrideCursor();
  QMessageBox::critical(this,
			tr("BiblioteQ: Database Error"),
			tr("Unable to save the copy data."));
  QApplication::processEvents();
  return;

 success_label:

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().commit())
    {
      clearCopiesList();
      qmain->addError(tr("Database Error"),
		      tr("Commit failure."),
		      qmain->getDB().lastError().text(),
		      __FILE__,
		      __LINE__);
      qmain->getDB().rollback();
      QApplication::restoreOverrideCursor();
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to commit the copy data."));
      QApplication::processEvents();
      return;
    }
  else
    m_cb.saveButton->animate(2500);

  auto const availability = biblioteq_misc_functions::getAvailability
    (m_ioid, qmain->getDB(), m_itemType, errorstr);
  auto const reserved = biblioteq_misc_functions::getTotalReserved
    (qmain->getDB(), m_itemType, m_ioid);

  QApplication::restoreOverrideCursor();

  if(!availability.isEmpty())
    biblioteq_misc_functions::updateColumn
      (qmain->getUI().table,
       m_bitem->getRow(),
       qmain->getUI().table->columnNumber("Availability"),
       availability);

  if(qmain->availabilityColors())
    {
      QColor color(Qt::white);

      if(availability.toInt() > 0)
	color = qmain->availabilityColor(m_itemType);

      biblioteq_misc_functions::updateColumnColor
	(qmain->getUI().table,
	 m_bitem->getRow(),
	 qmain->getUI().table->columnNumber("Availability"),
	 color);
    }

  biblioteq_misc_functions::updateColumn
    (qmain->getUI().table,
     m_bitem->getRow(),
     qmain->getUI().table->columnNumber("Quantity"),
     QString::number(m_copies.size()));

  if(!reserved.isEmpty())
    biblioteq_misc_functions::updateColumn
      (qmain->getUI().table,
       m_bitem->getRow(),
       qmain->getUI().table->columnNumber("Total Reserved"),
       reserved);

  if(m_bitem)
    {
      m_bitem->setOldQ(m_copies.size());
      m_bitem->updateQuantity(m_copies.size());
    }

  if(m_spinbox)
    m_spinbox->setValue(m_copies.size());

  clearCopiesList();
}
