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
#include "biblioteq_borrowers_editor.h"
#ifdef BIBLIOTEQ_QT_PDF_SUPPORTED
#include "biblioteq_documentationwindow.h"
#endif
#include "biblioteq_filesize_table_item.h"
#include "biblioteq_grey_literature.h"
#include "biblioteq_pdfreader.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QSettings>
#include <QShortcut>
#include <QSqlField>
#include <QSqlRecord>
#include <QUuid>
#include <QtMath>

biblioteq_grey_literature::biblioteq_grey_literature(biblioteq *parentArg,
						     const QString &oidArg,
						     const QModelIndex &index):
  QMainWindow(), biblioteq_item(index)
{
  qmain = parentArg;

  auto menu = new QMenu(this);

  m_duplicate = false;
  m_isQueryEnabled = false;
  m_oid = oidArg;
  m_parentWid = parentArg;
  m_ui.setupUi(this);
  setQMain(this);
  m_ui.resetButton->setMenu(menu);
  connect(m_ui.attach_files,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAttachFiles(void)));
  connect(m_ui.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotCancel(void)));
  connect(m_ui.delete_files,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotDeleteFiles(void)));
  connect(m_ui.export_files,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotExportFiles(void)));
  connect(m_ui.files,
	  SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
	  this,
	  SLOT(slotFilesDoubleClicked(QTableWidgetItem *)));
  connect(m_ui.okButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotGo(void)));
  connect(m_ui.printButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotPrint(void)));
  connect(m_ui.resetButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(m_ui.showUserButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotShowUsers(void)));
  connect(menu->addAction(tr("Reset Title")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset ID")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Date")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Authors")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Clients")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Code-A")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Code-B")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Job Number")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Notes")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Location")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Status")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Type")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(qmain,
	  SIGNAL(fontChanged(const QFont &)),
	  this,
	  SLOT(setGlobalFonts(const QFont &)));
  connect(qmain,
	  SIGNAL(otherOptionsSaved(void)),
	  this,
	  SLOT(slotPrepareIcons(void)));
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S),
		this,
		SLOT(slotGo(void)));
#else
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S),
		this,
		SLOT(slotGo(void)));
#endif
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_ui.files->setColumnHidden(static_cast<int> (Columns::MYOID), true);
  m_ui.files->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

  QString errorstr("");

  m_ui.date->setDisplayFormat(qmain->publicationDateFormat("greyliterature"));
  m_ui.date_enabled->setVisible(false);
  m_ui.location->addItems
    (biblioteq_misc_functions::getLocations(qmain->getDB(),
					    "Grey Literature",
					    errorstr));
  qmain->addError
    (tr("Database Error"),
     tr("Unable to retrieve the grey literature locations."),
     errorstr,
     __FILE__,
     __LINE__);
  m_ui.type->addItems
    (biblioteq_misc_functions::
     getGreyLiteratureTypes(qmain->getDB(), errorstr));
  qmain->addError
    (tr("Database Error"),
     tr("Unable to retrieve the grey literature document types."),
     errorstr,
     __FILE__,
     __LINE__);

  if(m_ui.location->findText(biblioteq::s_unknown) == -1)
    m_ui.location->addItem(biblioteq::s_unknown);

  if(m_ui.type->findText(biblioteq::s_unknown) == -1)
    m_ui.type->addItem(biblioteq::s_unknown);

#ifdef Q_OS_MACOS
  foreach(auto tool_button, findChildren<QToolButton *> ())
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    tool_button->setStyleSheet
    ("QToolButton {border: none; padding-right: 10px}"
     "QToolButton::menu-button {border: none;}");
#else
    tool_button->setStyleSheet
      ("QToolButton {border: none; padding-right: 15px}"
       "QToolButton::menu-button {border: none; width: 15px;}");
#endif
#endif

  QApplication::restoreOverrideCursor();
  m_dt_orig_ss = m_ui.date->styleSheet();
  m_te_orig_pal = m_ui.author->viewport()->palette();
  updateFont(QApplication::font(), qobject_cast<QWidget *> (this));
  biblioteq_misc_functions::center(this, m_parentWid);
  biblioteq_misc_functions::hideAdminFields(this, qmain->getRoles());
}

biblioteq_grey_literature::~biblioteq_grey_literature()
{
}

bool biblioteq_grey_literature::validateWidgets(void)
{
  QString error("");

  m_ui.title->setText(m_ui.title->text().trimmed());

  if(m_ui.title->text().isEmpty())
    {
      error = tr("Please complete the Title field.");
      m_ui.title->setFocus();
      goto done_label;
    }

  m_ui.id->setText(m_ui.id->text().trimmed());

  if(m_ui.id->text().isEmpty())
    {
      error = tr("Please complete the ID field.");
      m_ui.id->setFocus();
      goto done_label;
    }

  m_ui.author->setPlainText(m_ui.author->toPlainText().trimmed());

  if(m_ui.author->toPlainText().isEmpty())
    {
      error = tr("Please complete the Authors field.");
      m_ui.author->setFocus();
      goto done_label;
    }

  m_ui.code_a->setText(m_ui.code_a->text().trimmed());

  if(m_ui.code_a->text().isEmpty())
    {
      error = tr("Please complete the Code-A field.");
      m_ui.code_a->setFocus();
      goto done_label;
    }

  m_ui.code_b->setText(m_ui.code_b->text().trimmed());

  if(m_ui.code_b->text().isEmpty())
    {
      error = tr("Please complete the Code-B field.");
      m_ui.code_b->setFocus();
      goto done_label;
    }

  m_ui.job_number->setText(m_ui.job_number->text().trimmed());

  if(m_ui.job_number->text().isEmpty())
    {
      error = tr("Please complete the Job Number field.");
      m_ui.job_number->setFocus();
      goto done_label;
    }

  m_ui.client->setPlainText(m_ui.client->toPlainText().trimmed());
  m_ui.notes->setPlainText(m_ui.notes->toPlainText().trimmed());
  m_ui.status->setText(m_ui.status->text().trimmed());
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().transaction())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to create a database transaction."),
	 qmain->getDB().lastError().text(),
	 __FILE__,
	 __LINE__);
      QMessageBox::critical
	(this,
	 tr("BiblioteQ: Database Error"),
	 tr("Unable to create a database transaction."));
      QApplication::processEvents();
      return false;
    }

  QApplication::restoreOverrideCursor();

 done_label:

  if(!error.isEmpty())
    {
      QMessageBox::critical(this, tr("BiblioteQ: User Error"), error);
      QApplication::processEvents();
      return false;
    }

  return true;
}

void biblioteq_grey_literature::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  m_ui.retranslateUi(this);
	  break;
	}
      default:
	break;
      }

  QMainWindow::changeEvent(event);
}

void biblioteq_grey_literature::closeEvent(QCloseEvent *event)
{
  if(qmain->isLocked())
    {
      if(event)
	event->ignore();

      return;
    }

  if(m_engWindowTitle.contains("Create") ||
     m_engWindowTitle.contains("Modify"))
    if(hasDataChanged(this))
      {
	if(QMessageBox::
	   question(this,
		    tr("BiblioteQ: Question"),
		    tr("Your changes have not been saved. Continue closing?"),
		    QMessageBox::No | QMessageBox::Yes,
		    QMessageBox::No) == QMessageBox::No)
	  {
	    QApplication::processEvents();

	    if(event)
	      event->ignore();

	    return;
	  }

	QApplication::processEvents();
      }

  QMainWindow::closeEvent(event);
  qmain->removeGreyLiterature(this);
}

void biblioteq_grey_literature::createFile(const QByteArray &bytes,
					   const QByteArray &digest,
					   const QString &fileName) const
{
  if(fileName.trimmed().isEmpty())
    return;

  QSqlQuery query(qmain->getDB());

  if(qmain->getDB().driverName() != "QSQLITE")
    query.prepare("INSERT INTO grey_literature_files "
		  "(file, file_digest, file_name, item_oid) "
		  "VALUES (?, ?, ?, ?)");
  else
    query.prepare("INSERT INTO grey_literature_files "
		  "(file, file_digest, file_name, item_oid, myoid) "
		  "VALUES (?, ?, ?, ?, ?)");

  query.addBindValue(bytes);
  query.addBindValue(digest.toHex().constData());
  query.addBindValue(fileName);
  query.addBindValue(m_oid);

  if(qmain->getDB().driverName() == "QSQLITE")
    {
      QString errorstr("");
      auto const value = biblioteq_misc_functions::getSqliteUniqueId
	(qmain->getDB(), errorstr);

      if(errorstr.isEmpty())
	query.addBindValue(value);
      else
	qmain->addError(tr("Database Error"),
			tr("Unable to generate a unique integer."),
			errorstr);
    }

  if(!query.exec())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to create a database transaction."),
       query.lastError().text(),
       __FILE__,
       __LINE__);
}

void biblioteq_grey_literature::duplicate
(const QString &p_oid, const int state)
{
  m_duplicate = true;
  modify(state); // Initial population.
  m_duplicate = false;
  m_engWindowTitle = "Create";
  m_oid = p_oid;
  m_ui.attach_files->setEnabled(false);
  m_ui.delete_files->setEnabled(false);
  m_ui.export_files->setEnabled(false);
  m_ui.showUserButton->setEnabled(false);
  setWindowTitle(tr("BiblioteQ: Duplicate Grey Literature Entry"));
  emit windowTitleChanged(windowTitle());
}

void biblioteq_grey_literature::highlightRequiredWidgets(void)
{
  biblioteq_misc_functions::highlightWidget
    (m_ui.author->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (m_ui.code_a, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (m_ui.code_b, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (m_ui.id, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (m_ui.job_number, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (m_ui.title, m_requiredHighlightColor);
}

void biblioteq_grey_literature::insert(void)
{
  m_engWindowTitle = "Create";
  m_te_orig_pal = m_ui.id->palette();
  m_ui.attach_files->setEnabled(false);
  m_ui.author->setPlainText("N/A");
  m_ui.client->clear();
  m_ui.code_a->setText("N/A");
  m_ui.code_b->setText("N/A");
  m_ui.date->setDate
    (QDate::fromString("01/01/2000", biblioteq::s_databaseDateFormat));
  m_ui.delete_files->setEnabled(false);
  m_ui.export_files->setEnabled(false);
  m_ui.id->setText(QUuid::createUuid().toString().remove("{").remove("}"));
  m_ui.job_number->setText("N/A");
  m_ui.location->setCurrentIndex(0);
  m_ui.notes->clear();
  m_ui.okButton->setText(tr("&Save"));
  m_ui.showUserButton->setEnabled(false);
  m_ui.status->clear();
  m_ui.title->clear();
  m_ui.title->setFocus();
  m_ui.type->setCurrentIndex(0);
  highlightRequiredWidgets();
  setWindowTitle(tr("BiblioteQ: Create Grey Literature Entry"));
  emit windowTitleChanged(windowTitle());
  prepareFavorites();
  storeData(this);
#ifdef Q_OS_ANDROID
  showMaximized();
#else
  resize(size().width(), sizeHint().height());
  biblioteq_misc_functions::center(this, m_parentWid);
  showNormal();
#endif
  activateWindow();
  raise();
  prepareIcons(this);
}

void biblioteq_grey_literature::insertDatabase(void)
{
  QSqlQuery query(qmain->getDB());

  if(qmain->getDB().driverName() != "QSQLITE")
    query.prepare
      ("INSERT INTO grey_literature "
       "(author, client, document_code_a, document_code_b, "
       "document_date, document_id, document_status, document_title, "
       "document_type, job_number, location, notes) "
       "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
       "RETURNING myoid");
  else
    query.prepare
      ("INSERT INTO grey_literature "
       "(author, client, document_code_a, document_code_b, "
       "document_date, document_id, document_status, document_title, "
       "document_type, job_number, location, notes, "
       "myoid) "
       "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

  query.addBindValue(m_ui.author->toPlainText());

  if(m_ui.client->toPlainText().isEmpty())
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    query.addBindValue(QVariant(QMetaType(QMetaType::QString)));
#else
    query.addBindValue(QVariant::String);
#endif
  else
    query.addBindValue(m_ui.client->toPlainText());

  query.addBindValue(m_ui.code_a->text());
  query.addBindValue(m_ui.code_b->text());
  query.addBindValue
    (m_ui.date->date().toString(biblioteq::s_databaseDateFormat));
  query.addBindValue(m_ui.id->text());
  query.addBindValue(m_ui.status->text());
  query.addBindValue(m_ui.title->text());
  query.addBindValue(m_ui.type->currentText());
  query.addBindValue(m_ui.job_number->text());
  query.addBindValue(m_ui.location->currentText());

  if(m_ui.notes->toPlainText().isEmpty())
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    query.addBindValue(QVariant(QMetaType(QMetaType::QString)));
#else
    query.addBindValue(QVariant::String);
#endif
  else
    query.addBindValue(m_ui.notes->toPlainText());

  if(qmain->getDB().driverName() == "QSQLITE")
    {
      QString errorstr("");
      auto const value = biblioteq_misc_functions::getSqliteUniqueId
	(qmain->getDB(), errorstr);

      if(errorstr.isEmpty())
	{
	  m_oid = QString::number(value);
	  query.addBindValue(value);
	}
      else
	qmain->addError(tr("Database Error"),
			tr("Unable to generate a unique integer."),
			errorstr);
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to create the entry."),
	 query.lastError().text(),
	 __FILE__,
	 __LINE__);
      goto db_rollback;
    }

  if(qmain->getDB().driverName() != "QSQLITE")
    {
      query.next();
      m_oid = query.value(0).toString().trimmed();
    }

  if(!qmain->getDB().commit())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to commit the current database transaction."),
	 qmain->getDB().lastError().text(),
	 __FILE__,
	 __LINE__);
      goto db_rollback;
    }

  m_ui.author->setMultipleLinks
    ("greyliterature_search", "author", m_ui.author->toPlainText());
  m_ui.client->setMultipleLinks
    ("greyliterature_search", "client", m_ui.client->toPlainText());
  m_ui.notes->setMultipleLinks
    ("greyliterature_search", "notes", m_ui.notes->toPlainText());
  QApplication::restoreOverrideCursor();
  qmain->replaceGreyLiterature(m_oid, this);
  updateWindow(biblioteq::EDITABLE);

  if(qmain->getUI().actionAutoPopulateOnCreation->isChecked())
    (void) qmain->populateTable(biblioteq::POPULATE_ALL, "Grey Literature", "");

  raise();
  storeData(this);
  return;

 db_rollback:

  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_oid.clear();

  if(!qmain->getDB().rollback())
    qmain->addError(tr("Database Error"),
		    tr("Rollback failure."),
		    qmain->getDB().lastError().text(),
		    __FILE__,
		    __LINE__);

  QApplication::restoreOverrideCursor();
  QMessageBox::critical(this,
			tr("BiblioteQ: Database Error"),
			tr("Unable to create the entry. Please verify that "
			   "the entry does not already exist."));
  QApplication::processEvents();
}

void biblioteq_grey_literature::modify(const int state)
{
  if(state == biblioteq::EDITABLE)
    {
      highlightRequiredWidgets();
      setReadOnlyFields(this, false);
      setWindowTitle(tr("BiblioteQ: Modify Grey Literature Entry"));
      emit windowTitleChanged(windowTitle());
      m_engWindowTitle = "Modify";
      m_te_orig_pal = m_ui.id->palette();
      m_ui.attach_files->setEnabled(true);
      m_ui.delete_files->setEnabled(true);
      m_ui.export_files->setEnabled(true);
      m_ui.okButton->setText(tr("&Save"));
      m_ui.okButton->setVisible(true);
      m_ui.resetButton->setVisible(true);
      m_ui.showUserButton->setEnabled(true);
    }
  else
    {
      setReadOnlyFields(this, true);
      setWindowTitle(tr("BiblioteQ: View Grey Literature Details"));
      emit windowTitleChanged(windowTitle());
      m_engWindowTitle = "View";
      m_ui.attach_files->setVisible(false);
      m_ui.delete_files->setVisible(false);
      m_ui.export_files->setEnabled(true);
      m_ui.okButton->setVisible(false);
      m_ui.resetButton->setVisible(false);
      m_ui.showUserButton->setEnabled(qmain->isGuest() ? false : true);
    }

  prepareIcons(this);

  QSqlQuery query(qmain->getDB());

  query.prepare("SELECT author, "
		"client, "
		"document_code_a, "
		"document_code_b, "
		"document_date, "
		"document_id, "
		"document_status, "
		"document_title, "
		"document_type, "
		"job_number, "
		"location, "
		"notes "
		"FROM grey_literature WHERE myoid = ?");
  query.addBindValue(m_oid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec() || !query.next())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to retrieve the selected grey literature's data."),
	 query.lastError().text(),
	 __FILE__,
	 __LINE__);
      QMessageBox::critical
	(this,
	 tr("BiblioteQ: Database Error"),
	 tr("Unable to retrieve the selected grey literature's data."));
      QApplication::processEvents();
      m_ui.title->setFocus();
      return;
    }
  else
    {
      QApplication::restoreOverrideCursor();
#ifdef Q_OS_ANDROID
      showMaximized();
#else
      resize(size().width(), sizeHint().height());
      biblioteq_misc_functions::center(this, m_parentWid);
      showNormal();
#endif
      activateWindow();
      raise();

      auto const record(query.record());

      for(int i = 0; i < record.count(); i++)
	{
	  auto const fieldName(record.fieldName(i));
	  auto const variant(record.field(i).value());

	  if(fieldName == "author")
	    m_ui.author->setMultipleLinks
	      ("greyliterature_search", "author", variant.toString().trimmed());
	  else if(fieldName == "client")
	    m_ui.client->setMultipleLinks
	      ("greyliterature_search", "client", variant.toString().trimmed());
	  else if(fieldName == "document_code_a")
	    m_ui.code_a->setText(variant.toString().trimmed());
	  else if(fieldName == "document_code_b")
	    m_ui.code_b->setText(variant.toString().trimmed());
	  else if(fieldName == "document_date")
	    m_ui.date->setDate
	      (QDate::fromString(variant.toString().trimmed(),
				 biblioteq::s_databaseDateFormat));
	  else if(fieldName == "document_id")
	    {
	      QString string("");

	      if(state == biblioteq::EDITABLE)
		{
		  if(!variant.toString().trimmed().trimmed().isEmpty())
		    string = tr("BiblioteQ: Modify Grey Literature Entry (") +
		      variant.toString().trimmed() +
		      tr(")");
		  else
		    string = tr("BiblioteQ: Modify Grey Literature Entry");
		}
	      else
		{
		  if(!variant.toString().trimmed().trimmed().isEmpty())
		    string = tr("BiblioteQ: View Grey Literature Details (") +
		      variant.toString().trimmed() +
		      tr(")");
		  else
		    string = tr("BiblioteQ: View Grey Literature Details");
		}

	      m_ui.id->setText(variant.toString().trimmed());
	      setWindowTitle(string);
	      emit windowTitleChanged(windowTitle());
	    }
	  else if(fieldName == "document_status")
	    m_ui.status->setText(variant.toString().trimmed());
	  else if(fieldName == "document_title")
	    m_ui.title->setText(variant.toString().trimmed());
	  else if(fieldName == "document_type")
	    {
	      if(m_ui.type->findText(variant.toString().trimmed()) > -1)
		m_ui.type->setCurrentIndex
		  (m_ui.type->findText(variant.toString().trimmed()));
	      else
		m_ui.type->setCurrentIndex(0);
	    }
	  else if(fieldName == "job_number")
	    m_ui.job_number->setText(variant.toString().trimmed());
	  else if(fieldName == "location")
	    {
	      if(m_ui.location->findText(variant.toString().trimmed()) > -1)
		m_ui.location->setCurrentIndex
		  (m_ui.location->findText(variant.toString().trimmed()));
	      else
		m_ui.location->setCurrentIndex(0);
	    }
	  else if(fieldName == "notes")
	    m_ui.notes->setMultipleLinks
	      ("greyliterature_search", "notes", variant.toString().trimmed());
	}

      foreach(auto textfield, findChildren<QLineEdit *> ())
	textfield->setCursorPosition(0);

      storeData(this);

      if(!m_duplicate)
	populateFiles();
    }

  m_ui.title->setFocus();
}

void biblioteq_grey_literature::populateFiles(void)
{
  m_ui.files->setRowCount(0);
  m_ui.files->setSortingEnabled(false);

  QSqlQuery query(qmain->getDB());
  int count = 0;

  query.prepare
    ("SELECT COUNT(*) FROM grey_literature_files WHERE item_oid = ?");
  query.addBindValue(m_oid);

  if(query.exec())
    if(query.next())
      {
	count = query.value(0).toInt();
	m_ui.files->setRowCount(count);
      }

  query.prepare("SELECT file_name, "
		"file_digest, "
		"LENGTH(file) AS f_s, "
		"description, "
		"myoid FROM grey_literature_files "
                "WHERE item_oid = ? ORDER BY file_name");
  query.addBindValue(m_oid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QLocale locale;
  int row = 0;
  int totalRows = 0;

  if(query.exec())
    while(query.next() && totalRows < m_ui.files->rowCount())
      {
	totalRows += 1;

	auto const record(query.record());

	for(int i = 0; i < record.count(); i++)
	  {
	    QTableWidgetItem *item = nullptr;

	    if(record.fieldName(i) == "f_s")
	      item = new biblioteq_filesize_table_item
		(locale.toString(query.value(i).toLongLong()));
	    else
	      item = new QTableWidgetItem(query.value(i).toString().trimmed());

	    item->setData
	      (Qt::UserRole, query.value(record.count() - 1).toLongLong());
	    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

	    if(m_engWindowTitle == "Modify")
	      if(record.fieldName(i) == "description")
		item->setToolTip(tr("Double-click to edit."));

	    m_ui.files->setItem(row, i, item);
	  }

	row += 1;
      }

  m_ui.files->horizontalHeader()->setSortIndicator
    (static_cast<int> (Columns::FILE), Qt::AscendingOrder);
  m_ui.files->setRowCount(totalRows);
  m_ui.files->setSortingEnabled(true);

  if(m_index->isValid() &&
     (qmain->getTypeFilterString() == "All" ||
      qmain->getTypeFilterString() == "All Available" ||
      qmain->getTypeFilterString() == "All Overdue" ||
      qmain->getTypeFilterString() == "All Requested" ||
      qmain->getTypeFilterString() == "All Reserved" ||
      qmain->getTypeFilterString() == "Grey Literature"))
    {
      qmain->getUI().table->setSortingEnabled(false);

      auto const names(qmain->getUI().table->columnNames());

      for(int i = 0; i < names.size(); i++)
	if(names.at(i) == "File Count")
	  {
	    qmain->getUI().table->item(m_index->row(), i)->setText
	      (QString::number(count));
	    break;
	  }

      qmain->getUI().table->setSortingEnabled(true);
      qmain->getUI().table->updateToolTips(m_index->row());
    }

  QApplication::restoreOverrideCursor();
}

void biblioteq_grey_literature::prepareFavorites(void)
{
  QSettings settings;

  m_ui.type->setCurrentIndex
    (m_ui.type->findText(settings.value("grey_literature_types_favorite").
			 toString().trimmed()));

  if(m_ui.type->currentIndex() < 0)
    m_ui.type->setCurrentIndex(0);
}

void biblioteq_grey_literature::search(const QString &field,
				       const QString &value)
{
  m_engWindowTitle = "Search";
  m_ui.attach_files->setVisible(false);
  m_ui.date->setDate(QDate::fromString("2001", "yyyy"));
  m_ui.date->setDisplayFormat("yyyy");
  m_ui.date_enabled->setVisible(true);
  m_ui.delete_files->setVisible(false);
  m_ui.export_files->setVisible(false);
  m_ui.files->setVisible(false);
  m_ui.files_label->setVisible(false);
  m_ui.location->insertItem(0, tr("Any"));
  m_ui.location->setCurrentIndex(0);
  m_ui.okButton->setText(tr("&Search"));
  m_ui.showUserButton->setEnabled(false);
  m_ui.type->insertItem(0, tr("Any"));
  m_ui.type->setCurrentIndex(0);
  prepareIcons(this);

  if(field.isEmpty() && value.isEmpty())
    {
      m_ui.title->setFocus();
      prepareFavorites();
      setWindowTitle(tr("BiblioteQ: Database Grey Literature Search"));
      emit windowTitleChanged(windowTitle());
#ifdef Q_OS_ANDROID
      showMaximized();
#else
      resize(size().width(), sizeHint().height());
      biblioteq_misc_functions::center(this, m_parentWid);
      showNormal();
#endif
      activateWindow();
      raise();
    }
  else
    {
      if(field == "author")
	m_ui.author->setPlainText(value);
      else if(field == "client")
	m_ui.client->setPlainText(value);
      else if(field == "notes")
	m_ui.notes->setPlainText(value);

      slotGo();
    }
}

void biblioteq_grey_literature::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach(auto widget, findChildren<QWidget *> ())
    {
      widget->setFont(font);
      widget->update();
    }

  m_ui.files->resizeRowsToContents();
  update();
}

void biblioteq_grey_literature::slotAttachFiles(void)
{
  QFileDialog fileDialog
    (this, tr("BiblioteQ: Grey Literature File Attachment(s)"));

  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setDirectory(QDir::homePath());
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  fileDialog.setOption(QFileDialog::DontUseNativeDialog);

  if(fileDialog.exec() == QDialog::Accepted)
    {
      repaint();
      QApplication::processEvents();

      QProgressDialog progress(this);
      auto const files(fileDialog.selectedFiles());

      progress.setLabelText(tr("Uploading files..."));
      progress.setMaximum(files.size());
      progress.setMinimum(0);
      progress.setMinimumWidth
	(qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	       progress.sizeHint().width()));
      progress.setModal(true);
      progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
      progress.show();
      progress.repaint();
      QApplication::processEvents();

      for(int i = 0; i < files.size() && !progress.wasCanceled(); i++)
	{
	  QByteArray data;
	  QByteArray digest;

	  if(biblioteq_misc_functions::
	     cryptographicDigestOfFile(data, digest, files.at(i)))
	    createFile(data, digest, QFileInfo(files.at(i)).fileName());

	  if(i + 1 <= progress.maximum())
	    progress.setValue(i + 1);

	  progress.repaint();
	  QApplication::processEvents();
	}

      QApplication::restoreOverrideCursor();
      populateFiles();
    }

  QApplication::processEvents();
}

void biblioteq_grey_literature::slotCancel(void)
{
  close();
}

void biblioteq_grey_literature::slotDatabaseEnumerationsCommitted(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QList<QComboBox *> widgets;

  widgets << m_ui.location
	  << m_ui.type;

  for(int i = 0; i < widgets.size(); i++)
    {
      QString errorstr("");
      auto const str(widgets.at(i)->currentText());

      widgets.at(i)->clear();

      switch(i)
	{
	case 0:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::getLocations(qmain->getDB(),
						      "Grey Literature",
						      errorstr));
	    break;
	  }
	case 1:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::
	       getGreyLiteratureTypes(qmain->getDB(), errorstr));
	    break;
	  }
	default:
	  {
	    break;
	  }
	}

      if(widgets.at(i)->findText(biblioteq::s_unknown) == -1)
	widgets.at(i)->addItem(biblioteq::s_unknown);

      widgets.at(i)->setCurrentIndex(widgets.at(i)->findText(str));

      if(widgets.at(i)->currentIndex() < 0)
	widgets.at(i)->setCurrentIndex(widgets.at(i)->count() - 1); // Unknown.
    }

  QApplication::restoreOverrideCursor();
}

void biblioteq_grey_literature::slotDeleteFiles(void)
{
  auto const list(m_ui.files->selectionModel()->
		  selectedRows(static_cast<int> (Columns::MYOID)));

  if(list.isEmpty())
    {
      QMessageBox::critical
	(this,
	 tr("BiblioteQ: User Error"),
	 tr("Please select at least one file to delete."));
      QApplication::processEvents();
      return;
    }

  if(QMessageBox::question(this,
			   tr("BiblioteQ: Question"),
			   tr("Are you sure that you wish to delete the "
			      "selected file(s)?"),
			   QMessageBox::No | QMessageBox::Yes,
			   QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      return;
    }

  QApplication::processEvents();
  QApplication::setOverrideCursor(Qt::WaitCursor);

  for(int i = 0; i < list.size(); i++)
    {
      QSqlQuery query(qmain->getDB());

      query.prepare("DELETE FROM grey_literature_files WHERE "
		    "item_oid = ? AND myoid = ?");
      query.addBindValue(m_oid);
      query.addBindValue(list.at(i).data());
      query.exec();
    }

  QApplication::restoreOverrideCursor();
  populateFiles();
}

void biblioteq_grey_literature::slotExportFiles(void)
{
  auto const list(m_ui.files->selectionModel()->
		  selectedRows(static_cast<int> (Columns::MYOID)));

  if(list.isEmpty())
    return;

  QFileDialog dialog(this);

  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: Grey Literature File Export"));
  dialog.exec();
  QApplication::processEvents();

  if(dialog.result() != QDialog::Accepted)
    return;

  repaint();
  QApplication::processEvents();

  QProgressDialog progress(this);

  progress.setLabelText(tr("Exporting file(s)..."));
  progress.setMaximum(list.size());
  progress.setMinimum(0);
  progress.setMinimumWidth
    (qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	   progress.sizeHint().width()));
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  for(int i = 0; i < list.size() && !progress.wasCanceled(); i++)
    {
      QSqlQuery query(qmain->getDB());

      query.setForwardOnly(true);
      query.prepare("SELECT file, file_name FROM grey_literature_files "
		    "WHERE item_oid = ? AND myoid = ?");
      query.addBindValue(m_oid);
      query.addBindValue(list.at(i).data());

      if(query.exec() && query.next())
	{
	  QFile file(dialog.selectedFiles().value(0) +
		     QDir::separator() +
		     query.value(1).toString().trimmed());

	  if(file.open(QIODevice::WriteOnly))
	    file.write(qUncompress(query.value(0).toByteArray()));

	  file.flush();
	  file.close();
	}

      if(i + 1 <= progress.maximum())
	progress.setValue(i + 1);

      progress.repaint();
      QApplication::processEvents();
    }
}

void biblioteq_grey_literature::slotFilesDoubleClicked(QTableWidgetItem *item)
{
  if(!item)
    return;

  if(item->column() != static_cast<int> (Columns::DESCRIPTION) ||
     m_engWindowTitle != "Modify")
    {
      auto item1 = m_ui.files->item
	(item->row(), static_cast<int> (Columns::FILE));

      if(!item1)
	return;

#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER) || \
    defined(BIBLIOTEQ_QT_PDF_SUPPORTED)
      if(item1->text().toLower().trimmed().endsWith(".pdf"))
	{
	  QApplication::setOverrideCursor(Qt::WaitCursor);

	  QByteArray data;
	  QSqlQuery query(qmain->getDB());

	  query.setForwardOnly(true);
	  query.prepare("SELECT file, file_name FROM grey_literature_files "
			"WHERE item_oid = ? AND myoid = ?");
	  query.addBindValue(m_oid);
	  query.addBindValue(item1->data(Qt::UserRole).toLongLong());

	  if(query.exec() && query.next())
	    data = qUncompress(query.value(0).toByteArray());

	  if(!data.isEmpty())
	    {
#ifdef BIBLIOTEQ_LINKED_WITH_POPPLER
	      auto reader = new biblioteq_pdfreader(qmain);

	      reader->load(data, item1->text());
#else
	      auto reader = new biblioteq_documentationwindow(qmain);

	      reader->load(data);
#endif
#ifdef Q_OS_ANDROID
	      reader->showMaximized();
#else
	      biblioteq_misc_functions::center
		(reader, parentWidget() ? m_parentWid : this);
	      reader->show();
#endif
	    }

	  QApplication::restoreOverrideCursor();
	}
#endif

      return;
    }

  if(m_engWindowTitle != "Modify")
    return;

  auto item1 = m_ui.files->item
    (item->row(), static_cast<int> (Columns::DESCRIPTION));

  if(!item1)
    return;

  auto const description(item1->text());
  auto item2 = m_ui.files->item
    (item->row(), static_cast<int> (Columns::MYOID));

  if(!item2)
    return;

  QString text("");
  auto ok = true;

  text = QInputDialog::getText(this,
			       tr("BiblioteQ: File Description"),
			       tr("Description"),
			       QLineEdit::Normal,
			       description,
			       &ok).trimmed();

  if(!ok)
    return;

  QSqlQuery query(qmain->getDB());
  auto const myoid(item2->text());

  query.prepare("UPDATE grey_literature_files SET description = ? "
		"WHERE item_oid = ? AND myoid = ?");
  query.addBindValue(text);
  query.addBindValue(m_oid);
  query.addBindValue(myoid);

  if(query.exec())
    item1->setText(text);
}

void biblioteq_grey_literature::slotGo(void)
{
  if(m_engWindowTitle.contains("Create"))
    {
      if(validateWidgets())
	insertDatabase();
    }
  else if(m_engWindowTitle.contains("Modify"))
    {
      if(validateWidgets())
	updateDatabase();
    }
  else if(m_engWindowTitle.contains("Search"))
    {
      QString frontCover("'' AS front_cover ");
      QString searchstr("");

      if(qmain->showMainTableImages())
	frontCover = "grey_literature.front_cover ";

      searchstr = "SELECT DISTINCT grey_literature.author, "
	"grey_literature.client, "
	"grey_literature.document_code_a, "
	"grey_literature.document_code_b, "
	"grey_literature.document_date, "
	"grey_literature.document_id, "
	"grey_literature.document_status, "
	"grey_literature.document_title, "
	"grey_literature.document_type, "
	"grey_literature.job_number, "
	"grey_literature.location, "
	"(SELECT COUNT(myoid) FROM grey_literature_files "
	"WHERE grey_literature_files.item_oid = grey_literature.myoid) "
	"AS file_count, "
	"grey_literature.quantity - COUNT(item_borrower.item_oid) "
	"AS availability, "
	"COUNT(item_borrower.item_oid) AS total_reserved, "
	"grey_literature.type, "
	"grey_literature.myoid, " +
	frontCover +
	"FROM "
	"grey_literature LEFT JOIN item_borrower ON "
	"grey_literature.myoid = item_borrower.item_oid "
	"AND item_borrower.type = 'Grey Literature' "
	"WHERE ";

      QString ESCAPE("");
      auto const UNACCENT(qmain->unaccent());

      if(qmain->getDB().driverName() != "QSQLITE")
	ESCAPE = "E";

      searchstr.append(UNACCENT +
		       "(LOWER(document_title)) LIKE " +
		       UNACCENT +
		       "(LOWER(" +
		       ESCAPE +
		       "'%' || "
		       "?"
		       " || '%')) AND ");
      searchstr.append(UNACCENT +
		       "(LOWER(document_id)) LIKE " +
		       UNACCENT +
		       "(LOWER(" +
		       ESCAPE +
		       "'%' || "
		       "?"
		       " || '%')) AND ");

      if(m_ui.date_enabled->isChecked())
	searchstr.append("SUBSTR(document_date, 7) = '" +
			 m_ui.date->date().toString("yyyy") +
			 "' AND ");

      searchstr.append
	(UNACCENT +
	 "(LOWER(author)) LIKE " +
	 UNACCENT +
	 "(LOWER(" +
	 ESCAPE +
	 "'%' || "
	 "?"
	 " || '%')) AND ");
      searchstr.append
	(UNACCENT +
	 "(LOWER(COALESCE(client, ''))) LIKE " +
	 UNACCENT +
	 "(LOWER(" +
	 ESCAPE +
	 "'%' || "
	 "?"
	 " || '%')) AND ");
      searchstr.append
	(UNACCENT +
	 "(LOWER(document_code_a)) LIKE " +
	 UNACCENT +
	 "(LOWER(" +
	 ESCAPE +
	 "'%' || "
	 "?"
	 " || '%')) AND ");
      searchstr.append
	(UNACCENT +
	 "(LOWER(document_code_b)) LIKE " +
	 UNACCENT +
	 "(LOWER(" +
	 ESCAPE +
	 "'%' || "
	 "?"
	 " || '%')) AND ");
      searchstr.append
	(UNACCENT +
	 "(LOWER(job_number)) LIKE " +
	 UNACCENT +
	 "(LOWER(" +
	 ESCAPE +
	 "'%' || "
	 "?"
	 " || '%')) AND ");
      searchstr.append
	(UNACCENT +
	 "(LOWER(COALESCE(notes, ''))) LIKE " +
	 UNACCENT +
	 "(LOWER(" +
	 ESCAPE +
	 "'%' || "
	 "?"
	 " || '%')) AND ");

      if(m_ui.location->currentIndex() != 0)
	searchstr.append
	  (UNACCENT +
	   "(location) = " +
	   UNACCENT +
	   "(" +
	   ESCAPE +
	   "'" +
	   m_ui.location->currentText().trimmed() +
	   "') AND ");

      searchstr.append
	(UNACCENT +
	 "(LOWER(COALESCE(document_status, ''))) LIKE " +
	 UNACCENT +
	 "(LOWER(" +
	 ESCAPE +
	 "'%' || "
	 "?"
	 " || '%')) ");

      if(m_ui.type->currentIndex() != 0)
	searchstr.append
	  ("AND " +
	   UNACCENT +
	   "(document_type) = " +
	   UNACCENT +
	   "(" +
	   ESCAPE +
	   "'" +
	   m_ui.type->currentText().trimmed() +
	   "') ");

      searchstr.append("GROUP BY grey_literature.author, "
		       "grey_literature.client, "
		       "grey_literature.document_code_a, "
		       "grey_literature.document_code_b, "
		       "grey_literature.document_date, "
		       "grey_literature.document_id, "
		       "grey_literature.document_status, "
		       "grey_literature.document_title, "
		       "grey_literature.document_type, "
		       "grey_literature.job_number, "
		       "grey_literature.location, "
		       "grey_literature.type, "
		       "grey_literature.myoid, "
		       "grey_literature.front_cover "
		       "ORDER BY grey_literature.author");

      auto query = new QSqlQuery(qmain->getDB());

      query->prepare(searchstr);
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.title->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.id->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.author->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.client->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.code_a->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.code_b->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.job_number->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.notes->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(m_ui.status->text().trimmed()));
      (void) qmain->populateTable
	(query,
	 "Grey Literature",
	 biblioteq::NEW_PAGE,
	 biblioteq::POPULATE_SEARCH);
    }
}

void biblioteq_grey_literature::slotPrepareIcons(void)
{
  prepareIcons(this);
}

void biblioteq_grey_literature::slotPrint(void)
{
  m_html = "<html>";

  QStringList titles;
  QStringList values;

  titles << tr("Title:")
	 << tr("ID:")
	 << tr("Date:")
	 << tr("Authors:")
	 << tr("Clients:")
	 << tr("Code-A:")
	 << tr("Code-B:")
	 << tr("Job Number:")
	 << tr("Notes:")
	 << tr("Location:")
	 << tr("Status:")
	 << tr("Type:");
  values << m_ui.title->text().trimmed()
	 << m_ui.id->text().trimmed()
	 << m_ui.date->date().toString(Qt::ISODate)
	 << m_ui.author->toPlainText().trimmed()
	 << m_ui.client->toPlainText().trimmed()
	 << m_ui.code_a->text().trimmed()
	 << m_ui.code_b->text().trimmed()
	 << m_ui.job_number->text().trimmed()
	 << m_ui.notes->toPlainText().trimmed()
	 << m_ui.location->currentText().trimmed()
	 << m_ui.status->text().trimmed()
	 << m_ui.type->currentText().trimmed();

  for(int i = 0; i < titles.size(); i++)
    {
      m_html += "<b>" + titles.at(i) + "</b>" + values.at(i);

      if(i != titles.size() - 1)
	m_html += "<br>";
    }

  m_html += "</html>";
  print(this);
}

void biblioteq_grey_literature::slotPublicationDateEnabled(bool state)
{
  Q_UNUSED(state);
}

void biblioteq_grey_literature::slotQuery(void)
{
}

void biblioteq_grey_literature::slotReset(void)
{
  auto action = qobject_cast<QAction *> (sender());

  if(action != nullptr)
    {
      auto const actions = m_ui.resetButton->menu()->actions();

      if(actions.size() < 12)
	{
	  // Error.
	}
      else if(action == actions[0])
	{
	  m_ui.title->clear();
	  m_ui.title->setFocus();
	  m_ui.title->setPalette(m_te_orig_pal);
	}
      else if(action == actions[1])
	{
	  m_ui.id->clear();
	  m_ui.id->setFocus();
	  m_ui.id->setPalette(m_te_orig_pal);
	}
      else if(action == actions[2])
	{
	  if(m_engWindowTitle.contains("Search"))
	    m_ui.date->setDate(QDate::fromString("2001", "yyyy"));
	  else
	    m_ui.date->setDate
	      (QDate::fromString("01/01/2000",
				 biblioteq::s_databaseDateFormat));

	  m_ui.date->setFocus();
	  m_ui.date->setStyleSheet(m_dt_orig_ss);
	  m_ui.date_enabled->setChecked(false);
	}
      else if(action == actions[3])
	{
	  if(!m_engWindowTitle.contains("Search"))
	    m_ui.author->setPlainText("N/A");
	  else
	    m_ui.author->clear();

	  m_ui.author->setFocus();
	  m_ui.author->viewport()->setPalette(m_te_orig_pal);
	}
      else if(action == actions[4])
	{
	  m_ui.client->clear();
	  m_ui.client->setFocus();
	}
      else if(action == actions[5])
	{
	  if(!m_engWindowTitle.contains("Search"))
	    m_ui.code_a->setText("N/A");
	  else
	    m_ui.code_a->clear();

	  m_ui.code_a->setFocus();
	  m_ui.code_a->setPalette(m_te_orig_pal);
	}
      else if(action == actions[6])
	{
	  if(!m_engWindowTitle.contains("Search"))
	    m_ui.code_b->setText("N/A");
	  else
	    m_ui.code_b->clear();

	  m_ui.code_b->setFocus();
	  m_ui.code_b->setPalette(m_te_orig_pal);
	}
      else if(action == actions[7])
	{
	  if(!m_engWindowTitle.contains("Search"))
	    m_ui.job_number->setText("N/A");
	  else
	    m_ui.job_number->clear();

	  m_ui.job_number->setFocus();
	  m_ui.job_number->setPalette(m_te_orig_pal);
	}
      else if(action == actions[8])
	{
	  m_ui.notes->clear();
	  m_ui.notes->setFocus();
	}
      else if(action == actions[9])
	{
	  m_ui.location->setCurrentIndex(0);
	  m_ui.location->setFocus();
	}
      else if(action == actions[10])
	{
	  m_ui.status->clear();
	  m_ui.status->setFocus();
	}
      else if(action == actions[11])
	{
	  m_ui.type->setCurrentIndex(0);
	  m_ui.type->setFocus();
	}
    }
  else
    {
      /*
      ** Reset all.
      */

      if(!m_engWindowTitle.contains("Search"))
	m_ui.author->setPlainText("N/A");
      else
	m_ui.author->clear();

      m_ui.client->clear();

      if(!m_engWindowTitle.contains("Search"))
	m_ui.code_a->setText("N/A");
      else
	m_ui.code_a->clear();

      if(!m_engWindowTitle.contains("Search"))
	m_ui.code_b->setText("N/A");
      else
	m_ui.code_b->clear();

      if(m_engWindowTitle.contains("Search"))
	m_ui.date->setDate(QDate::fromString("2001", "yyyy"));
      else
	m_ui.date->setDate(QDate::fromString("01/01/2000",
					     biblioteq::s_databaseDateFormat));

      m_ui.date_enabled->setChecked(false);
      m_ui.id->clear();

      if(!m_engWindowTitle.contains("Search"))
	m_ui.job_number->setText("N/A");
      else
	m_ui.job_number->clear();

      m_ui.location->setCurrentIndex(0);
      m_ui.notes->clear();
      m_ui.status->clear();
      m_ui.title->clear();
      m_ui.title->setFocus();
      m_ui.type->setCurrentIndex(0);
    }
}

void biblioteq_grey_literature::slotShowUsers(void)
{
  biblioteq_borrowers_editor *borrowerseditor = nullptr;
  int state = 0;

  if(!m_ui.okButton->isHidden())
    state = biblioteq::EDITABLE;
  else
    state = biblioteq::VIEW_ONLY;

  borrowerseditor = new biblioteq_borrowers_editor
    (qobject_cast<QWidget *> (this),
     qmain,
     static_cast<biblioteq_item *> (this),
     m_ui.copies->value(),
     m_oid,
     m_ui.id->text().trimmed(),
     font(),
     "Grey Literature",
     state);
  borrowerseditor->showUsers();
}

void biblioteq_grey_literature::updateDatabase(void)
{
  QSqlQuery query(qmain->getDB());
  QString string("");

  query.prepare("UPDATE grey_literature SET "
		"author = ?, "
		"client = ?, "
		"document_code_a = ?, "
		"document_code_b = ?, "
		"document_date = ?, "
		"document_id = ?, "
		"document_status = ?, "
		"document_title = ?, "
		"document_type = ?, "
		"job_number = ?, "
		"location = ?, "
		"notes = ? "
		"WHERE myoid = ?");

  query.addBindValue(m_ui.author->toPlainText());

  if(m_ui.client->toPlainText().isEmpty())
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    query.addBindValue(QVariant(QMetaType(QMetaType::QString)));
#else
    query.addBindValue(QVariant::String);
#endif
  else
    query.addBindValue(m_ui.client->toPlainText());

  query.addBindValue(m_ui.code_a->text());
  query.addBindValue(m_ui.code_b->text());
  query.addBindValue
    (m_ui.date->date().toString(biblioteq::s_databaseDateFormat));
  query.addBindValue(m_ui.id->text());
  query.addBindValue(m_ui.status->text());
  query.addBindValue(m_ui.title->text());
  query.addBindValue(m_ui.type->currentText());
  query.addBindValue(m_ui.job_number->text());
  query.addBindValue(m_ui.location->currentText());

  if(m_ui.notes->toPlainText().isEmpty())
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    query.addBindValue(QVariant(QMetaType(QMetaType::QString)));
#else
    query.addBindValue(QVariant::String);
#endif
  else
    query.addBindValue(m_ui.notes->toPlainText());

  query.addBindValue(m_oid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to update the entry."),
	 query.lastError().text(),
	 __FILE__,
	 __LINE__);
      goto db_rollback;
    }

  if(!qmain->getDB().commit())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to commit the current database transaction."),
	 qmain->getDB().lastError().text(),
	 __FILE__,
	 __LINE__);
      goto db_rollback;
    }

  m_ui.author->setMultipleLinks
    ("greyliterature_search", "author", m_ui.author->toPlainText());
  m_ui.client->setMultipleLinks
    ("greyliterature_search", "client", m_ui.client->toPlainText());
  m_ui.notes->setMultipleLinks
    ("greyliterature_search", "notes", m_ui.notes->toPlainText());
  QApplication::restoreOverrideCursor();

  if(!m_ui.id->text().isEmpty())
    string = tr("BiblioteQ: Modify Grey Literature Entry (") +
      m_ui.id->text() +
      tr(")");
  else
    string = tr("BiblioteQ: Modify Grey Literature Entry");

  setWindowTitle(string);
  emit windowTitleChanged(windowTitle());

  if(m_index->isValid() &&
     (qmain->getTypeFilterString() == "All" ||
      qmain->getTypeFilterString() == "All Available" ||
      qmain->getTypeFilterString() == "All Overdue" ||
      qmain->getTypeFilterString() == "All Requested" ||
      qmain->getTypeFilterString() == "All Reserved" ||
      qmain->getTypeFilterString() == "Grey Literature"))
    {
      qmain->getUI().table->setSortingEnabled(false);

      auto const names(qmain->getUI().table->columnNames());

      for(int i = 0; i < names.size(); i++)
	{
	  QString string("");
	  auto set = false;

	  if(names.at(i) == "Accession Number" || names.at(i) == "Job Number")
	    {
	      set = true;
	      string = m_ui.job_number->text();
	    }
	  else if(names.at(i) == "Author" ||
		  names.at(i) == "Authors" ||
		  names.at(i) == "Publisher")
	    {
	      set = true;
	      string = m_ui.author->toPlainText();
	    }
	  else if(names.at(i) == "Clients")
	    {
	      set = true;
	      string = m_ui.client->toPlainText();
	    }
	  else if(names.at(i) == "Date" ||
		  names.at(i) == "Document Date" ||
		  names.at(i) == "Publication Date")
	    {
	      set = true;

	      if(qmain->getTypeFilterString() == "Grey Literature")
		string = m_ui.date->date().toString
		  (qmain->publicationDateFormat("greyliterature"));
	      else
		string = m_ui.date->date().toString(Qt::ISODate);
	    }
	  else if(names.at(i) == "Document Code A")
	    {
	      set = true;
	      string = m_ui.code_a->text();
	    }
	  else if(names.at(i) == "Document Code B")
	    {
	      set = true;
	      string = m_ui.code_b->text();
	    }
	  else if(names.at(i) == "Document ID" ||
		  names.at(i) == "ID" ||
		  names.at(i) == "ID Number")
	    {
	      set = true;
	      string = m_ui.id->text();
	    }
	  else if(names.at(i) == "Document Status")
	    {
	      set = true;
	      string = m_ui.status->text();
	    }
	  else if(names.at(i) == "Document Type")
	    {
	      set = true;
	      string = m_ui.type->currentText();
	    }
	  else if(names.at(i) == "Location")
	    {
	      set = true;
	      string = m_ui.location->currentText();
	    }
	  else if(names.at(i) == "Title")
	    {
	      set = true;
	      string = m_ui.title->text();
	    }

	  if(set)
	    qmain->getUI().table->item(m_index->row(), i)->setText(string);
	}

      qmain->getUI().table->setSortingEnabled(true);
      qmain->getUI().table->updateToolTips(m_index->row());

      foreach(auto textfield, findChildren<QLineEdit *> ())
	textfield->setCursorPosition(0);

      qmain->slotResizeColumns();
    }

  qmain->slotDisplaySummary();
  storeData(this);
  return;

 db_rollback:

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().rollback())
    qmain->addError(tr("Database Error"),
		    tr("Rollback failure."),
		    qmain->getDB().lastError().text(),
		    __FILE__,
		    __LINE__);

  QApplication::restoreOverrideCursor();
  QMessageBox::critical(this,
			tr("BiblioteQ: Database Error"),
			tr("Unable to update the entry."));
  QApplication::processEvents();
}

void biblioteq_grey_literature::updateWindow(const int state)
{
  QString string("");

  if(state == biblioteq::EDITABLE)
    {
      m_engWindowTitle = "Modify";
      m_ui.attach_files->setEnabled(true);
      m_ui.delete_files->setEnabled(true);
      m_ui.export_files->setEnabled(true);
      m_ui.okButton->setVisible(true);
      m_ui.resetButton->setVisible(true);
      m_ui.showUserButton->setEnabled(true);
      string = tr("BiblioteQ: Modify Grey Literature Entry (") +
	m_ui.id->text() +
	tr(")");
    }
  else
    {
      m_engWindowTitle = "View";
      m_ui.attach_files->setVisible(false);
      m_ui.delete_files->setVisible(false);
      m_ui.export_files->setEnabled(true);
      m_ui.okButton->setVisible(false);
      m_ui.resetButton->setVisible(false);
      m_ui.showUserButton->setEnabled(qmain->isGuest() ? false : true);
      string = tr("BiblioteQ: View Grey Literature Details (") +
	m_ui.id->text() +
	tr(")");
    }

  setReadOnlyFields(this, state != biblioteq::EDITABLE);
  setWindowTitle(string);
  emit windowTitleChanged(windowTitle());
}
