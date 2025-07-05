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
#include "biblioteq_bgraphicsscene.h"
#include "biblioteq_graphicsitempixmap.h"
#include "biblioteq_photographcollection.h"

#include <QFileDialog>
#include <QScrollBar>
#include <QShortcut>
#include <QSqlField>
#include <QSqlRecord>
#include <QTextBrowser>
#include <QUuid>
#include <QtMath>

#include <limits>

biblioteq_photographcollection::biblioteq_photographcollection
(biblioteq *parentArg,
 const QString &oidArg,
 const QModelIndex &index):QMainWindow(), biblioteq_item(index)
{
  qmain = parentArg;

  auto menu1 = new QMenu(this);
  auto menu2 = new QMenu(this);
  auto scene1 = new QGraphicsScene(this);
  auto scene2 = new QGraphicsScene(this);
  auto scene3 = new QGraphicsScene(this);

  pc.setupUi(this);
  setQMain(this);
  pc.publication_date->setDisplayFormat
    (qmain->publicationDateFormat("photographcollections"));
  pc.quantity->setMaximum(static_cast<int> (biblioteq::Limits::QUANTITY));
  pc.thumbnail_item->enableDoubleClickResize(false);
  m_photo_diag = new QDialog(this);
  m_scene = new biblioteq_bgraphicsscene(pc.graphicsView);
  connect(m_scene,
	  SIGNAL(selectionChanged(void)),
	  this,
	  SLOT(slotSceneSelectionChanged(void)));
  m_isQueryEnabled = false;
  m_oid = oidArg;
  m_parentWid = parentArg;
  photo.setupUi(m_photo_diag);
  photo.quantity->setMaximum(static_cast<int> (biblioteq::Limits::QUANTITY));
  photo.thumbnail_item->enableDoubleClickResize(false);
  pc.graphicsView->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
  pc.graphicsView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(pc.graphicsView,
	  SIGNAL(customContextMenuRequested(const QPoint &)),
	  this,
	  SLOT(slotViewContextMenu(const QPoint &)));
  pc.graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
  pc.graphicsView->setRubberBandSelectionMode(Qt::IntersectsItemShape);
  pc.graphicsView->setScene(m_scene);

  if(photographsPerPage() != -1) // Unlimited.
    pc.graphicsView->setSceneRect
      (0.0,
       0.0,
       5.0 * 150.0,
       (photographsPerPage() / 5.0) * 200.0 + 200.0);

  pc.thumbnail_item->setReadOnly(true);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S),
		this,
		SLOT(slotGo(void)));
#else
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S),
		this,
		SLOT(slotGo(void)));
#endif
  updateFont(QApplication::font(), qobject_cast<QWidget *> (this));
  m_photo_diag->setWindowModality(Qt::WindowModal);
  updateFont(QApplication::font(), qobject_cast<QWidget *> (m_photo_diag));
  connect(pc.addItemButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotAddItem(void)));
  connect(pc.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotCancel(void)));
  connect(pc.exportPhotographsToolButton,
	  SIGNAL(clicked(void)),
	  pc.exportPhotographsToolButton,
	  SLOT(showMenu(void)));
  connect(pc.graphicsView->scene(),
	  SIGNAL(itemDoubleClicked(void)),
	  this,
	  SLOT(slotViewPhotograph(void)));
  connect(pc.importItems,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotImportItems(void)));
  connect(pc.okButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotGo(void)));
  connect(pc.page,
	  SIGNAL(currentIndexChanged(int)),
	  this,
	  SLOT(slotPageChanged(int)));
  connect(pc.printButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotPrint(void)));
  connect(pc.resetButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(pc.selectAllButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectAll(void)));
  connect(pc.select_image_collection,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectImage(void)));
  connect(pc.thumbnail_collection,
	  SIGNAL(imageChanged(const QImage &)),
	  this,
	  SIGNAL(imageChanged(const QImage &)));
  connect(photo.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotClosePhoto(void)));
  connect(photo.select_image_item,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectImage(void)));
  connect(menu1->addAction(tr("Reset Collection Image")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu1->addAction(tr("Reset Collection ID")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu1->addAction(tr("Reset Collection Title")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu1->addAction(tr("Reset Collection Location")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu1->addAction(tr("Reset Collection About")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu1->addAction(tr("Reset Collection Notes")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu1->addAction(tr("Reset Accession Number")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu2->addAction(tr("&All...")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotExportPhotographs(void)));
  connect(menu2->addAction(tr("&Current Page...")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotExportPhotographs(void)));
  connect(menu2->addAction(tr("&Selected...")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotExportPhotographs(void)));
  connect(qmain,
	  SIGNAL(fontChanged(const QFont &)),
	  this,
	  SLOT(setGlobalFonts(const QFont &)));
  connect(qmain,
	  SIGNAL(otherOptionsSaved(void)),
	  this,
	  SLOT(slotPrepareIcons(void)));
  pc.exportPhotographsToolButton->setMenu(menu2);
  pc.resetButton->setMenu(menu1);

  if(menu2->actions().size() >= 3)
    menu2->actions().at(2)->setEnabled(false);

  QString errorstr("");

  QApplication::setOverrideCursor(Qt::WaitCursor);
  pc.location->addItems
    (biblioteq_misc_functions::getLocations(qmain->getDB(),
					    "Photograph Collection",
					    errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the photograph collection locations."),
       errorstr,
       __FILE__,
       __LINE__);

  pc.thumbnail_collection->setScene(scene1);
  pc.thumbnail_item->setScene(scene2);
  photo.thumbnail_item->setScene(scene3);

  if(pc.location->findText(biblioteq::s_unknown) == -1)
    pc.location->addItem(biblioteq::s_unknown);

  if(m_parentWid)
    resize(qRound(0.95 * m_parentWid->size().width()),
	   qRound(0.95 * m_parentWid->size().height()));

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

  pc.splitter->setStretchFactor(0, 0);
  pc.splitter->setStretchFactor(1, 1);
  pc.splitter->setStretchFactor(2, 0);
  pc.statistics->setText(tr("0 Images"));
  biblioteq_misc_functions::center(this, m_parentWid);
  biblioteq_misc_functions::hideAdminFields(this, qmain->getRoles());
  biblioteq_misc_functions::highlightWidget
    (photo.copyright_item->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (photo.creators_item->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (photo.id_item, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (photo.medium_item, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (photo.reproduction_number_item->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (photo.title_item, m_requiredHighlightColor);
}

biblioteq_photographcollection::~biblioteq_photographcollection()
{
  while(!m_uis.isEmpty())
    delete m_uis.takeFirst();
}

bool biblioteq_photographcollection::verifyItemFields(void)
{
  auto str(photo.id_item->text().trimmed());

  photo.id_item->setText(str);

  if(photo.id_item->text().isEmpty())
    {
      QMessageBox::critical(m_photo_diag,
			    tr("BiblioteQ: User Error"),
			    tr("Please complete the item's ID field."));
      QApplication::processEvents();
      photo.id_item->setFocus();
      return false;
    }

  str = photo.title_item->text().trimmed();
  photo.title_item->setText(str);

  if(photo.title_item->text().isEmpty())
    {
      QMessageBox::critical(m_photo_diag,
			    tr("BiblioteQ: User Error"),
			    tr("Please complete the item's Title field."));
      QApplication::processEvents();
      photo.title_item->setFocus();
      return false;
    }

  str = photo.creators_item->toPlainText().trimmed();
  photo.creators_item->setPlainText(str);

  if(photo.creators_item->toPlainText().isEmpty())
    {
      QMessageBox::critical(m_photo_diag,
			    tr("BiblioteQ: User Error"),
			    tr("Please complete the item's Creators field."));
      QApplication::processEvents();
      photo.creators_item->setFocus();
      return false;
    }

  str = photo.medium_item->text().trimmed();
  photo.medium_item->setText(str);

  if(photo.medium_item->text().isEmpty())
    {
      QMessageBox::critical(m_photo_diag,
			    tr("BiblioteQ: User Error"),
			    tr("Please complete the item's Medium field."));
      QApplication::processEvents();
      photo.medium_item->setFocus();
      return false;
    }

  str = photo.reproduction_number_item->toPlainText().trimmed();
  photo.reproduction_number_item->setPlainText(str);

  if(photo.reproduction_number_item->toPlainText().isEmpty())
    {
      QMessageBox::critical
	(m_photo_diag,
	 tr("BiblioteQ: User Error"),
	 tr("Please complete the item's Reproduction Number field."));
      QApplication::processEvents();
      photo.reproduction_number_item->setFocus();
      return false;
    }

  str = photo.copyright_item->toPlainText().trimmed();
  photo.copyright_item->setPlainText(str);

  if(photo.copyright_item->toPlainText().isEmpty())
    {
      QMessageBox::critical(m_photo_diag,
			    tr("BiblioteQ: User Error"),
			    tr("Please complete the item's Copyright field."));
      QApplication::processEvents();
      photo.copyright_item->setFocus();
      return false;
    }

  str = photo.accession_number_item->text().trimmed();
  photo.accession_number_item->setText(str);
  return true;
}

int biblioteq_photographcollection::photographsPerPage(void)
{
  auto integer = qmain->setting("photographs_per_page").toInt();

  if(!(integer == -1 || (integer >= 25 && integer <= 100)))
    integer = 25;

  return integer;
}

void biblioteq_photographcollection::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  pc.retranslateUi(this);
	  photo.retranslateUi(m_photo_diag);
	  break;
	}
      default:
	break;
      }

  QMainWindow::changeEvent(event);
}

void biblioteq_photographcollection::closeEvent(QCloseEvent *event)
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
  qmain->removePhotographCollection(this);
}

void biblioteq_photographcollection::duplicate(const QString &p_oid,
					       const int state)
{
  modify(state, "Create"); // Initial population.
  pc.addItemButton->setEnabled(false);
  pc.importItems->setEnabled(false);
  m_oid = p_oid;
  setWindowTitle(tr("BiblioteQ: Duplicate Photograph Collection Entry"));
  emit windowTitleChanged(windowTitle());
}

void biblioteq_photographcollection::insert(void)
{
  pc.accession_number->clear();
  pc.addItemButton->setEnabled(false);
  pc.importItems->setEnabled(false);
  pc.okButton->setText(tr("&Save"));
  pc.publication_date->setDate
    (QDate::fromString("01/01/2000", biblioteq::s_databaseDateFormat));
  biblioteq_misc_functions::highlightWidget
    (pc.id_collection, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (pc.title_collection, m_requiredHighlightColor);
  setWindowTitle(tr("BiblioteQ: Create Photograph Collection Entry"));
  emit windowTitleChanged(windowTitle());
  m_engWindowTitle = "Create";
  pc.id_collection->setFocus();
  pc.id_collection->setText
    (QUuid::createUuid().toString().remove("{").remove("}"));
  pc.page->blockSignals(true);
  pc.page->clear();
  pc.page->addItem("1");
  pc.page->blockSignals(false);
  storeData();
#ifdef Q_OS_ANDROID
  showMaximized();
#else
  biblioteq_misc_functions::center(this, m_parentWid);
  showNormal();
#endif
  activateWindow();
  raise();
  prepareIcons(this);
  pc.selectAllButton->setIcon(QIcon());
}

void biblioteq_photographcollection::loadPhotographFromItem
(QGraphicsPixmapItem *item,
 QGraphicsScene *scene,
 QTextBrowser *text,
 const int percent)
{
  if(!item || !scene || !scene->views().value(0) || !text)
    return;

  QSqlQuery query(qmain->getDB());

  QApplication::setOverrideCursor(Qt::WaitCursor);
  query.setForwardOnly(true);
  query.prepare("SELECT image, notes FROM photograph WHERE "
		"collection_oid = ? AND myoid = ?");
  query.addBindValue(m_oid);
  query.addBindValue(item->data(0).toLongLong());

  if(query.exec())
    if(query.next())
      {
	QImage image;
	auto bytes(QByteArray::fromBase64(query.value(0).toByteArray()));
	auto const format(biblioteq_misc_functions::imageFormatGuess(bytes));

	image.loadFromData(bytes);

	if(image.isNull())
	  {
	    bytes = query.value(0).toByteArray();
	    image.loadFromData(bytes);
	  }

	if(image.isNull())
	  image = QImage(":/missing_image.png");

	QSize size;

	if(percent == 0)
	  size = scene->views().value(0)->size();
	else
	  {
	    size = image.size();
	    size.setHeight((percent * size.height()) / 100);
	    size.setWidth((percent * size.width()) / 100);
	  }

	if(!image.isNull())
	  image = image.scaled
	    (size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	pc.graphicsView->scene()->clearSelection();

	QGraphicsPixmapItem *pixmapItem = nullptr;

	if(!scene->items().isEmpty())
	  {
	    pixmapItem = qgraphicsitem_cast<QGraphicsPixmapItem *>
	      (scene->items().at(0));

	    if(pixmapItem)
	      pixmapItem->setPixmap(QPixmap::fromImage(image));
	  }
	else
	  pixmapItem = scene->addPixmap(QPixmap::fromImage(image));

	if(pixmapItem)
	  pixmapItem->setData(1, bytes);

	item->setSelected(true);

	if(!scene->items().isEmpty())
	  {
	    scene->items().at(0)->setData(0, item->data(0)); // myoid
	    scene->items().at(0)->setData(2, item->data(2)); // Navigation.
	  }

	scene->setSceneRect(scene->itemsBoundingRect());
	text->setPlainText(query.value(1).toString().trimmed());
	text->setVisible(!text->toPlainText().isEmpty());

	auto view = qobject_cast<biblioteq_photograph_view *>
	  (scene->views().value(0));

	if(view)
	  {
	    connect(view,
		    SIGNAL(save(const QImage &,
				const QString &,
				const qint64)),
		    this,
		    SLOT(slotSaveRotatedImage(const QImage &,
					      const QString &,
					      const qint64)),
		    Qt::UniqueConnection);
	    view->horizontalScrollBar()->setValue(0);
	    view->setBestFit(percent == 0);
	    view->setImage(image, format, item->data(0).toLongLong());
	    view->verticalScrollBar()->setValue(0);
	  }
      }

  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::loadPhotographFromItemInNewWindow
(QGraphicsPixmapItem *item)
{
  if(item)
    {
      auto mainWindow = new QMainWindow(this);
      auto ui = new Ui_photographView;

      m_uis << ui;
      ui->setupUi(mainWindow);
      connect(ui->closeButton,
	      SIGNAL(clicked(void)),
	      mainWindow,
	      SLOT(close(void)));
      connect(ui->exportItem,
	      SIGNAL(clicked(void)),
	      this,
	      SLOT(slotExportItem(void)));
      connect(ui->next,
	      SIGNAL(clicked(void)),
	      this,
	      SLOT(slotViewNextPhotograph(void)));
      connect(ui->previous,
	      SIGNAL(clicked(void)),
	      this,
	      SLOT(slotViewPreviousPhotograph(void)));
      connect(ui->rotate_left,
	      SIGNAL(clicked(void)),
	      ui->view,
	      SLOT(slotRotateLeft(void)));
      connect(ui->rotate_right,
	      SIGNAL(clicked(void)),
	      ui->view,
	      SLOT(slotRotateRight(void)));
      connect(ui->save,
	      SIGNAL(clicked(void)),
	      ui->view,
	      SLOT(slotSave(void)));
      connect(ui->view_size,
	      SIGNAL(currentIndexChanged(int)),
	      this,
	      SLOT(slotImageViewSizeChanged(int)));
      ui->save->setVisible(m_engWindowTitle.contains("Modify"));

      auto scene = new QGraphicsScene(mainWindow);

      mainWindow->resize
	(qRound(0.95 * size().width()), qRound(0.95 * size().height()));
#ifndef Q_OS_ANDROID
      biblioteq_misc_functions::center
	(mainWindow, parentWidget() ? m_parentWid : this);
      mainWindow->show();
      mainWindow->hide();
#endif
      scene->setProperty("view_size", ui->view->viewport()->size());
      ui->notes->setPlainText("");
      ui->notes->setVisible(false);
      ui->view->setScene(scene);
      loadPhotographFromItem
	(item,
	 scene,
	 ui->notes,
	 ui->view_size->currentText().remove("%").toInt());
#ifdef Q_OS_ANDROID
      mainWindow->showMaximized();
#else
      mainWindow->show();
#endif
    }
}

void biblioteq_photographcollection::modify(const int state,
					    const QString &behavior)
{
  QSqlQuery query(qmain->getDB());
  QString fieldname("");
  QString str("");
  QVariant var;

  if(state == biblioteq::EDITABLE)
    {
      disconnect(m_scene,
		 SIGNAL(deleteKeyPressed(void)),
		 this,
		 SLOT(slotDeleteItem(void)));
      connect(m_scene,
	      SIGNAL(deleteKeyPressed(void)),
	      this,
	      SLOT(slotDeleteItem(void)));

      if(behavior.isEmpty())
	{
	  setWindowTitle(tr("BiblioteQ: Modify Photograph Collection Entry"));
	  emit windowTitleChanged(windowTitle());
	  m_engWindowTitle = "Modify";
	}
      else
	m_engWindowTitle = behavior;

      setReadOnlyFields(this, false);
      setReadOnlyFieldsOverride();
      pc.addItemButton->setEnabled(true);
      pc.importItems->setEnabled(true);
      pc.okButton->setVisible(true);
      pc.resetButton->setVisible(true);
      pc.select_image_collection->setVisible(true);
      biblioteq_misc_functions::highlightWidget
	(pc.id_collection, m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(pc.title_collection, m_requiredHighlightColor);
    }
  else
    {
      if(behavior.isEmpty())
	{
	  setWindowTitle(tr("BiblioteQ: View Photograph Collection Details"));
	  emit windowTitleChanged(windowTitle());
	  m_engWindowTitle = "View";
	}
      else
	m_engWindowTitle = behavior;

      setReadOnlyFields(this, true);
      setReadOnlyFieldsOverride();
      pc.addItemButton->setVisible(false);
      pc.importItems->setVisible(false);
      pc.okButton->setVisible(false);
      pc.page->setEnabled(true);
      pc.resetButton->setVisible(false);
      pc.select_image_collection->setVisible(false);

      auto const actions = pc.resetButton->menu()->actions();

      if(!actions.isEmpty())
	actions[0]->setVisible(false);
    }

  query.setForwardOnly(true);
  query.prepare("SELECT id, "
		"title, "
		"location, "
		"about, "
		"notes, "
		"image, "
		"accession_number "
		"FROM "
		"photograph_collection "
		"WHERE myoid = ?");
  query.bindValue(0, m_oid);
  pc.okButton->setText(tr("&Save"));
  prepareIcons(this);
  pc.selectAllButton->setIcon(QIcon());
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec() || !query.next())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to retrieve the selected photograph collection's data."),
	 query.lastError().text(),
	 __FILE__,
	 __LINE__);
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to retrieve the selected photograph "
			       "collection's data."));
      QApplication::processEvents();
      close();
      return;
    }
  else
    {
      QApplication::restoreOverrideCursor();

      auto const record(query.record());

      for(int i = 0; i < record.count(); i++)
	{
	  var = record.field(i).value();
	  fieldname = record.fieldName(i);

	  if(fieldname == "about")
	    pc.about_collection->setPlainText(var.toString().trimmed());
	  else if(fieldname == "accession_number")
	    {
	      pc.accession_number->setText(var.toString().trimmed());
	      pc.accession_number->setToolTip(pc.accession_number->text());
	    }
	  else if(fieldname == "id")
	    {
	      pc.id_collection->setText(var.toString().trimmed());
	      pc.id_collection->setToolTip(pc.id_collection->text());

	      if(behavior.isEmpty())
		{
		  if(state == biblioteq::EDITABLE)
		    {
		      str = QString
			(tr("BiblioteQ: Modify Photograph Collection "
			    "Entry (")) +
			var.toString().trimmed() +
			tr(")");
		      m_engWindowTitle = "Modify";
		    }
		  else
		    {
		      str = tr("BiblioteQ: View Photograph "
			       "Collection Details (") +
			var.toString().trimmed() +
			tr(")");
		      m_engWindowTitle = "View";
		    }

		  setWindowTitle(str);
		  emit windowTitleChanged(windowTitle());
		}
	    }
	  else if(fieldname == "image")
	    {
	      if(!record.field(i).isNull())
		{
		  pc.thumbnail_collection->loadFromData
		    (QByteArray::fromBase64(var.toByteArray()));

		  if(pc.thumbnail_collection->m_image.isNull())
		    pc.thumbnail_collection->loadFromData(var.toByteArray());
		}
	    }
	  else if(fieldname == "location")
	    {
	      if(pc.location->findText(var.toString().trimmed()) > -1)
		pc.location->setCurrentIndex
		  (pc.location->findText(var.toString().trimmed()));
	      else
		pc.location->setCurrentIndex
		  (pc.location->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "notes")
	    pc.notes_collection->setPlainText(var.toString().trimmed());
	  else if(fieldname == "title")
	    {
	      pc.title_collection->setText(var.toString().trimmed());
	      pc.title_collection->setToolTip(pc.title_collection->text());
	    }
	}

      int pages = 1;

      if(!m_engWindowTitle.contains("Create"))
	{
	  QApplication::setOverrideCursor(Qt::WaitCursor);
	  query.prepare("SELECT COUNT(*) "
			"FROM photograph "
			"WHERE collection_oid = ?");
	  query.bindValue(0, m_oid);

	  if(query.exec())
	    if(query.next())
	      {
		pc.statistics->setText
		  (tr("%1 Image(s)").arg(query.value(0).toLongLong()));

		auto const i = photographsPerPage();

		if(i == -1) // Unlimited.
		  {
		    pages = 1;
		    setSceneRect(query.value(0).toLongLong());
		  }
		else
		  pages = qCeil(query.value(0).toDouble() / qMax(1, i));
	      }

	  QApplication::restoreOverrideCursor();
	  pages = qMax(1, pages);
	}

      pc.page->blockSignals(true);
      pc.page->clear();

      for(int i = 1; i <= pages; i++)
	pc.page->addItem(QString::number(i));

      pc.page->blockSignals(false);

      foreach(auto textfield, findChildren<QLineEdit *> ())
	textfield->setCursorPosition(0);

      storeData();
#ifdef Q_OS_ANDROID
      showMaximized();
#else
      biblioteq_misc_functions::center(this, m_parentWid);
      showNormal();
#endif
      activateWindow();
      raise();
      repaint();
      QApplication::processEvents();

      if(!m_engWindowTitle.contains("Create"))
	showPhotographs(pc.page->currentText().toInt());
    }

  pc.id_collection->setFocus();
}

void biblioteq_photographcollection::search(const QString &field,
					    const QString &value)
{
  Q_UNUSED(field);
  Q_UNUSED(value);
  pc.accession_number->clear();
  pc.addItemButton->setVisible(false);
  pc.collectionGroup->setVisible(false);
  pc.exportPhotographsToolButton->setVisible(false);
  pc.importItems->setVisible(false);
  pc.itemGroup->setVisible(false);
  pc.location->insertItem(0, tr("Any"));
  pc.location->setCurrentIndex(0);
  pc.select_image_collection->setVisible(false);
  pc.thumbnail_collection->setVisible(false);

  auto const actions = pc.resetButton->menu()->actions();

  if(!actions.isEmpty())
    actions[0]->setVisible(false);

  for(int i = 7; i < actions.size(); i++)
    actions.at(i)->setVisible(false);

  setWindowTitle(tr("BiblioteQ: Database Photograph Collection Search"));
  emit windowTitleChanged(windowTitle());
  m_engWindowTitle = "Search";
  pc.id_collection->setFocus();
  pc.okButton->setText(tr("&Search"));
#ifdef Q_OS_ANDROID
  showMaximized();
#else
  biblioteq_misc_functions::center(this, m_parentWid);
  showNormal();
#endif
  activateWindow();
  raise();
  prepareIcons(this);
  pc.selectAllButton->setIcon(QIcon());
}

void biblioteq_photographcollection::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach(auto widget, findChildren<QWidget *> ())
    {
      widget->setFont(font);
      widget->update();
    }

  update();
}

void biblioteq_photographcollection::setReadOnlyFieldsOverride(void)
{
  pc.accession_number_item->setReadOnly(true);
  pc.call_number_item->setReadOnly(true);
  pc.copyright_item->setReadOnly(true);
  pc.creators_item->setReadOnly(true);
  pc.format_item->setReadOnly(true);
  pc.id_item->setReadOnly(true);
  pc.medium_item->setReadOnly(true);
  pc.notes_item->setReadOnly(true);
  pc.other_number_item->setReadOnly(true);
  pc.publication_date->setReadOnly(true);
  pc.quantity->setReadOnly(true);
  pc.reproduction_number_item->setReadOnly(true);
  pc.subjects_item->setReadOnly(true);
  pc.thumbnail_item->setReadOnly(true);
  pc.title_item->setReadOnly(true);
}

void biblioteq_photographcollection::setSceneRect(const qint64 size)
{
  pc.graphicsView->setSceneRect
    (0.0, 0.0, 5.0 * 150.0, (size / 5.0) * 200.0 + 200.0);
}

void biblioteq_photographcollection::showPhotographs(const int page)
{
  QProgressDialog progress(this);

  progress.setLabelText(tr("Loading image(s)..."));
  progress.setMaximum(0);
  progress.setMinimum(0);
  progress.setMinimumWidth
    (qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	   progress.sizeHint().width()));
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  QSqlQuery query(qmain->getDB());

  query.setForwardOnly(true);

  if(photographsPerPage() == -1) // Unlimited.
    {
      query.prepare("SELECT image_scaled, myoid FROM "
		    "photograph WHERE "
		    "collection_oid = ? "
		    "ORDER BY id");
      query.bindValue(0, m_oid);
    }
  else
    {
      query.prepare("SELECT image_scaled, myoid FROM "
		    "photograph WHERE "
		    "collection_oid = ? "
		    "ORDER BY id "
		    "LIMIT ? "
		    "OFFSET ?");
      query.bindValue(0, m_oid);
      query.bindValue(1, photographsPerPage());
      query.bindValue(2, photographsPerPage() * (page - 1));
    }

  if(query.exec())
    {
      pc.graphicsView->horizontalScrollBar()->setValue(0);
      pc.graphicsView->resetTransform();
      pc.graphicsView->scene()->clear();
      pc.graphicsView->verticalScrollBar()->setValue(0);

      int columnIdx = 0;
      int i = -1;
      int rowIdx = 0;

      while(query.next())
	{
	  i += 1;
	  progress.repaint();
	  QApplication::processEvents();

	  if(progress.wasCanceled())
	    break;

	  QImage image;
	  biblioteq_graphicsitempixmap *pixmapItem = nullptr;

	  image.loadFromData
	    (QByteArray::fromBase64(query.value(0).
				    toByteArray()));

	  if(image.isNull())
	    image.loadFromData(query.value(0).toByteArray());

	  if(image.isNull())
	    image = QImage(":/missing_image.png");

	  /*
	  ** The size of missing_image.png is AxB.
	  */

	  if(!image.isNull())
	    image = image.scaled
	      (biblioteq::s_noImageResize,
	       Qt::KeepAspectRatio,
	       Qt::SmoothTransformation);

	  pixmapItem = new biblioteq_graphicsitempixmap
	    (QPixmap::fromImage(image), nullptr);

	  if(rowIdx == 0)
	    pixmapItem->setPos(140 * columnIdx + 15, 15);
	  else
	    pixmapItem->setPos(140 * columnIdx + 15, 200 * rowIdx);

	  pixmapItem->setData(0, query.value(1)); // myoid
	  pixmapItem->setData(2, i); // Next / previous navigation.
	  pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
	  pc.graphicsView->scene()->addItem(pixmapItem);
	  columnIdx += 1;

	  if(columnIdx >= 5)
	    {
	      rowIdx += 1;
	      columnIdx = 0;
	    }
	}

      pc.graphicsView->setSceneRect
	(pc.graphicsView->scene()->itemsBoundingRect());
    }

  progress.close();
}

void biblioteq_photographcollection::slotAddItem(void)
{
  photo.saveButton->disconnect(SIGNAL(clicked(void)));
  connect(photo.saveButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotInsertItem(void)));
  m_photo_diag->resize(m_photo_diag->width(), qRound(0.95 * size().height()));
  biblioteq_misc_functions::center
    (m_photo_diag, parentWidget() ? m_parentWid : this);
  photo.accession_number_item->clear();
  photo.call_number_item->clear();
  photo.copyright_item->setPlainText("N/A");
  photo.creators_item->setPlainText("N/A");
  photo.format_item->clear();
  photo.id_item->setFocus();
  photo.id_item->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));
  photo.medium_item->setText("N/A");
  photo.notes_item->clear();
  photo.other_number_item->clear();
  photo.publication_date->setDate
    (QDate::fromString("01/01/2000", biblioteq::s_databaseDateFormat));
  photo.quantity->setValue(1);
  photo.reproduction_number_item->setPlainText("N/A");
  photo.scrollArea->ensureVisible(0, 0);
  photo.subjects_item->clear();
  photo.thumbnail_item->clear();
  photo.title_item->setText("N/A");
#ifdef Q_OS_ANDROID
  m_photo_diag->showMaximized();
#else
  m_photo_diag->show();
#endif
}

void biblioteq_photographcollection::slotCancel(void)
{
  close();
}

void biblioteq_photographcollection::slotClosePhoto(void)
{
#ifdef Q_OS_ANDROID
  m_photo_diag->hide();
#else
  m_photo_diag->close();
#endif
}

void biblioteq_photographcollection::slotDatabaseEnumerationsCommitted(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QList<QComboBox *> widgets;

  widgets << pc.location;

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
						      "Photograph Collection",
						      errorstr));
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

void biblioteq_photographcollection::slotDeleteItem(void)
{
  auto const items(pc.graphicsView->scene()->selectedItems());

  if(items.isEmpty())
    return;
  else
    {
      if(QMessageBox::question(this,
			       tr("BiblioteQ: Question"),
			       tr("Are you sure that you wish to permanently "
				  "delete the selected %1 item(s)?").
			       arg(items.size()),
			       QMessageBox::No | QMessageBox::Yes,
			       QMessageBox::No) == QMessageBox::No)
	{
	  QApplication::processEvents();
	  return;
	}

      QApplication::processEvents();
    }

  QProgressDialog progress(this);

  progress.setCancelButton(nullptr);
  progress.setLabelText(tr("Deleting the selected item(s)..."));
  progress.setMaximum(items.size());
  progress.setMinimum(0);
  progress.setMinimumWidth
    (qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	   progress.sizeHint().width()));
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  for(int i = 0; i < items.size(); i++)
    {
      if(i + 1 <= progress.maximum())
	progress.setValue(i + 1);

      progress.repaint();
      QApplication::processEvents();

      QGraphicsPixmapItem *item = nullptr;

      if((item = qgraphicsitem_cast<QGraphicsPixmapItem *> (items.at(i))))
	{
	  QSqlQuery query(qmain->getDB());
	  auto const itemOid(item->data(0).toString());

	  query.prepare("DELETE FROM photograph WHERE "
			"collection_oid = ? AND myoid = ?");
	  query.bindValue(0, m_oid);
	  query.bindValue(1, itemOid);

	  if(query.exec())
	    {
	      pc.graphicsView->scene()->removeItem(item);
	      delete item;
	    }
	}
    }

  QSqlQuery query(qmain->getDB());
  int pages = 1;

  query.prepare("SELECT COUNT(*) "
		"FROM photograph "
		"WHERE collection_oid = ?");
  query.bindValue(0, m_oid);

  if(query.exec())
    if(query.next())
      {
	pc.statistics->setText
	  (tr("%1 Image(s)").arg(query.value(0).toLongLong()));
	updateTablePhotographCount(query.value(0).toLongLong());

	auto const i = photographsPerPage();

	if(i == -1) // Unlimited.
	  {
	    pages = 1;
	    setSceneRect(query.value(0).toLongLong());
	  }
	else
	  pages = qCeil(query.value(0).toDouble() / qMax(1, i));
      }

  pages = qMax(1, pages);
  pc.page->blockSignals(true);
  pc.page->clear();

  for(int i = 1; i <= pages; i++)
    pc.page->addItem(QString::number(i));

  pc.page->blockSignals(false);
  progress.close();
  repaint();
  QApplication::processEvents();
  showPhotographs(pc.page->currentText().toInt());
}

void biblioteq_photographcollection::slotExportItem(void)
{
  auto pushButton = qobject_cast<QPushButton *> (sender());

  if(!pushButton)
    return;

  auto parent = pushButton->parentWidget();

  do
    {
      if(!parent)
	break;

      if(qobject_cast<QMainWindow *> (parent))
	break;

      parent = parent->parentWidget();
    }
  while(true);

  if(!parent)
    return;

  QByteArray bytes;
  auto scene = parent->findChild<QGraphicsScene *> ();

  if(scene)
    {
      auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>
	(scene->items().value(0));

      if(item)
	bytes = item->data(1).toByteArray();
    }

  if(bytes.isEmpty())
    return;

  QFileDialog dialog(this);

  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle
    (tr("BiblioteQ: Photograph Collection Photograph Export"));
  dialog.selectFile
    (QString("biblioteq-image-export.%1").
     arg(biblioteq_misc_functions::imageFormatGuess(bytes)));

  if(dialog.exec() == QDialog::Accepted)
    {
      QApplication::processEvents();

      QFile file(dialog.selectedFiles().value(0));

      if(file.open(QIODevice::WriteOnly))
	{
	  file.write(bytes);
	  file.flush();
	  file.close();
	}
    }

  QApplication::processEvents();
}

void biblioteq_photographcollection::slotExportPhotographs(void)
{
  if(pc.graphicsView->scene()->items().isEmpty())
    return;
  else
    {
      if(!qobject_cast<QAction *> (sender()) &&
	 pc.graphicsView->scene()->selectedItems().isEmpty())
	return;
    }

  QFileDialog dialog(this);

  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle
    (tr("BiblioteQ: Photograph Collection Photographs Export"));
  dialog.exec();
  QApplication::processEvents();

  if(dialog.result() == QDialog::Accepted &&
     dialog.selectedFiles().size() > 0)
    {
      repaint();
      QApplication::processEvents();

      auto action = qobject_cast<QAction *> (sender());

      if(!action ||
	 action == pc.exportPhotographsToolButton->menu()->actions().value(0))
	/*
	** Export all photographs.
	*/

	biblioteq_misc_functions::exportPhotographs
	  (qmain->getDB(),
	   m_oid,
	   -1,
	   photographsPerPage(),
	   dialog.selectedFiles().value(0),
	   this);
      else if(action ==
	      pc.exportPhotographsToolButton->menu()->actions().value(1))
	/*
	** Export the current page.
	*/

	biblioteq_misc_functions::exportPhotographs
	  (qmain->getDB(),
	   m_oid,
	   pc.page->currentText().toInt(),
	   photographsPerPage(),
	   dialog.selectedFiles().value(0),
	   this);
      else
	/*
	** Export the selected photograph(s).
	*/

	biblioteq_misc_functions::exportPhotographs
	  (qmain->getDB(),
	   m_oid,
	   dialog.selectedFiles().value(0),
	   pc.graphicsView->scene()->selectedItems(),
	   this);
    }
}

void biblioteq_photographcollection::slotGo(void)
{
  QString errorstr("");
  QString str("");

  if(m_engWindowTitle.contains("Create") ||
     m_engWindowTitle.contains("Modify"))
    {
      str = pc.id_collection->text().trimmed();
      pc.id_collection->setText(str);

      if(pc.id_collection->text().isEmpty())
	{
	  QMessageBox::critical
	    (this,
	     tr("BiblioteQ: User Error"),
	     tr("Please complete the collection's ID field."));
	  QApplication::processEvents();
	  pc.id_collection->setFocus();
	  return;
	}

      str = pc.title_collection->text().trimmed();
      pc.title_collection->setText(str);

      if(pc.title_collection->text().isEmpty())
	{
	  QMessageBox::critical
	    (this,
	     tr("BiblioteQ: User Error"),
	     tr("Please complete the collection's Title field."));
	  QApplication::processEvents();
	  pc.title_collection->setFocus();
	  return;
	}

      pc.about_collection->setPlainText
	(pc.about_collection->toPlainText().trimmed());
      pc.notes_collection->setPlainText
	(pc.notes_collection->toPlainText().trimmed());
      pc.accession_number->setText
	(pc.accession_number->text().trimmed());
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
	  return;
	}
      else
	QApplication::restoreOverrideCursor();

      QSqlQuery query(qmain->getDB());

      if(m_engWindowTitle.contains("Modify"))
	query.prepare("UPDATE photograph_collection SET "
		      "id = ?, "
		      "title = ?, "
		      "location = ?, "
		      "about = ?, "
		      "notes = ?, "
		      "image = ?, "
		      "image_scaled = ?, "
		      "accession_number = ? "
		      "WHERE "
		      "myoid = ?");
      else if(qmain->getDB().driverName() != "QSQLITE")
	query.prepare("INSERT INTO photograph_collection "
		      "(id, title, location, about, notes, image, "
		      "image_scaled, accession_number) VALUES (?, "
		      "?, ?, ?, ?, ?, ?, ?)");
      else
	query.prepare("INSERT INTO photograph_collection "
		      "(id, title, location, about, notes, image, "
		      "image_scaled, accession_number, myoid) "
		      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

      query.bindValue(0, pc.id_collection->text().trimmed());
      query.bindValue(1, pc.title_collection->text().trimmed());
      query.bindValue(2, pc.location->currentText().trimmed());
      query.bindValue(3, pc.about_collection->toPlainText().trimmed());
      query.bindValue(4, pc.notes_collection->toPlainText().trimmed());

      if(!pc.thumbnail_collection->m_image.isNull())
	{
	  QBuffer buffer;
	  QByteArray bytes;
	  QImage image;

	  buffer.setBuffer(&bytes);

	  if(buffer.open(QIODevice::WriteOnly))
	    {
	      pc.thumbnail_collection->m_image.save
		(&buffer, pc.thumbnail_collection->m_imageFormat.toLatin1(),
		 100);
	      query.bindValue(5, bytes.toBase64());
	    }
	  else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	    query.bindValue(5, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	    query.bindValue(5, QVariant(QVariant::ByteArray));
#endif

	  buffer.close();
	  bytes.clear();
	  image = pc.thumbnail_collection->m_image;

	  if(!image.isNull())
	    image = image.scaled
	      (biblioteq::s_noImageResize,
	       Qt::KeepAspectRatio,
	       Qt::SmoothTransformation);

	  if(buffer.open(QIODevice::WriteOnly))
	    {
	      image.save
		(&buffer, pc.thumbnail_collection->m_imageFormat.toLatin1(),
		 100);
	      query.bindValue(6, bytes.toBase64());
	    }
	  else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	    query.bindValue(6, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	    query.bindValue(6, QVariant(QVariant::ByteArray));
#endif
	}
      else
	{
	  pc.thumbnail_collection->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	  query.bindValue(5, QVariant(QMetaType(QMetaType::QByteArray)));
	  query.bindValue(6, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	  query.bindValue(5, QVariant(QVariant::ByteArray));
	  query.bindValue(6, QVariant(QVariant::ByteArray));
#endif
	}

      query.bindValue(7, pc.accession_number->text().trimmed());

      if(m_engWindowTitle.contains("Modify"))
	query.bindValue(8, m_oid);
      else if(qmain->getDB().driverName() == "QSQLITE")
	{
	  auto const value = biblioteq_misc_functions::getSqliteUniqueId
	    (qmain->getDB(), errorstr);

	  if(errorstr.isEmpty())
	    query.bindValue(8, value);
	  else
	    qmain->addError
	      (tr("Database Error"),
	       tr("Unable to generate a unique integer."),
	       errorstr);
	}

      QApplication::setOverrideCursor(Qt::WaitCursor);

      if(!query.exec())
	{
	  QApplication::restoreOverrideCursor();
	  qmain->addError(tr("Database Error"),
			  tr("Unable to create or update the entry."),
			  query.lastError().text(),
			  __FILE__,
			  __LINE__);
	  goto db_rollback;
	}
      else
	{
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

	  QApplication::restoreOverrideCursor();

	  if(m_engWindowTitle.contains("Modify"))
	    {
	      str = tr("BiblioteQ: Modify Photograph Collection Entry (") +
		pc.id_collection->text() +
		tr(")");
	      setWindowTitle(str);
	      emit windowTitleChanged(windowTitle());
	      m_engWindowTitle = "Modify";

	      if(m_index->isValid() &&
		 (qmain->getTypeFilterString() == "All" ||
		  qmain->getTypeFilterString() == "All Available" ||
		  qmain->getTypeFilterString() == "All Overdue" ||
		  qmain->getTypeFilterString() == "All Requested" ||
		  qmain->getTypeFilterString() == "All Reserved" ||
		  qmain->getTypeFilterString() == "Photograph Collections"))
		{
		  qmain->getUI().table->setSortingEnabled(false);

		  auto const names(qmain->getUI().table->columnNames());

		  for(int i = 0; i < names.size(); i++)
		    {
		      if(i == 0 && qmain->showMainTableImages())
			{
			  auto const pixmap
			    (QPixmap::
			     fromImage(pc.thumbnail_collection->m_image));

			  if(!pixmap.isNull())
			    qmain->getUI().table->item(m_index->row(), i)->
			      setIcon(pixmap);
			  else
			    qmain->getUI().table->item(m_index->row(), i)->
			      setIcon(QIcon(":/missing_image.png"));
			}

		      if(names.at(i) == "About")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (pc.about_collection->toPlainText().trimmed());
		      else if(names.at(i) == "Accession Number")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (pc.accession_number->text());
		      else if(names.at(i) == "Call Number")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (pc.call_number_item->text());
		      else if(names.at(i) == "ID" ||
			      names.at(i) == "ID Number")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (pc.id_collection->text());
		      else if(names.at(i) == "Location")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (pc.location->currentText().trimmed());
		      else if(names.at(i) == "Title")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (pc.title_collection->text());
		    }

		  qmain->getUI().table->setSortingEnabled(true);
		  qmain->getUI().table->updateToolTips(m_index->row());

		  foreach(auto textfield, findChildren<QLineEdit *> ())
		    textfield->setCursorPosition(0);

		  qmain->slotResizeColumns();
		}

	      qmain->slotDisplaySummary();
	      qmain->updateSceneItem(m_oid, "Photograph Collection",
				     pc.thumbnail_collection->m_image);
	    }
	  else
	    {
	      QApplication::setOverrideCursor(Qt::WaitCursor);
	      m_oid = biblioteq_misc_functions::getOID
		(pc.id_collection->text(),
		 "Photograph Collection",
		 qmain->getDB(),
		 errorstr);
	      QApplication::restoreOverrideCursor();

	      if(!errorstr.isEmpty())
		{
		  qmain->addError
		    (tr("Database Error"),
		     tr("Unable to retrieve the photograph collection's OID."),
		     errorstr,
		     __FILE__,
		     __LINE__);
		  QMessageBox::critical(this,
					tr("BiblioteQ: Database Error"),
					tr("Unable to retrieve the "
					   "photograph collection's OID."));
		  QApplication::processEvents();
		}
	      else
		qmain->replacePhotographCollection(m_oid, this);

	      updateWindow(biblioteq::EDITABLE);

	      if(qmain->getUI().actionAutoPopulateOnCreation->isChecked())
		(void) qmain->populateTable(biblioteq::POPULATE_ALL,
					    "Photograph Collections",
					    "");

	      raise();
	    }

	  storeData();
	}

      return;

    db_rollback:

      QApplication::setOverrideCursor(Qt::WaitCursor);

      if(m_engWindowTitle.contains("Create"))
	m_oid.clear();

      if(!qmain->getDB().rollback())
	qmain->addError
	  (tr("Database Error"),
	   tr("Rollback failure."),
	   qmain->getDB().lastError().text(),
	   __FILE__,
	   __LINE__);

      QApplication::restoreOverrideCursor();
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to create or update the entry. "
			       "Please verify that "
			       "the entry does not already exist."));
      QApplication::processEvents();
    }
  else if(m_engWindowTitle.contains("Search"))
    {
      QString frontCover("'' AS image_scaled ");
      QString searchstr("");

      if(qmain->showMainTableImages())
	frontCover = "photograph_collection.image_scaled ";

      searchstr = "SELECT DISTINCT photograph_collection.title, "
	"photograph_collection.id, "
	"photograph_collection.location, "
	"COUNT(photograph.myoid) AS photograph_count, "
	"photograph_collection.about, "
	"photograph_collection.accession_number, "
	"photograph_collection.type, "
	"photograph_collection.myoid, " +
	frontCover +
	"FROM photograph_collection LEFT JOIN "
	"photograph "
	"ON photograph_collection.myoid = photograph.collection_oid "
	"WHERE ";

      QString ESCAPE("");
      auto const UNACCENT(qmain->unaccent());

      if(qmain->getDB().driverName() != "QSQLITE")
	ESCAPE = "E";

      searchstr.append
	(UNACCENT + "(LOWER(photograph_collection.id)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

      searchstr.append
	(UNACCENT + "(LOWER(photograph_collection.title)) LIKE " +
	 UNACCENT + "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

      if(pc.location->currentIndex() != 0)
	searchstr.append
	  (UNACCENT + "(photograph_collection.location) = " + UNACCENT +
	   "(" + ESCAPE + "'" +
	   biblioteq_myqstring::escape(pc.location->currentText().
				       trimmed()) + "') AND ");

      searchstr.append
	(UNACCENT + "(LOWER(COALESCE(photograph_collection.about, ''))) " +
	 "LIKE " + UNACCENT + "(LOWER(" +
	 ESCAPE + "'%' || ? || '%')) AND ");
      searchstr.append
	(UNACCENT + "(LOWER(COALESCE(photograph_collection.notes, ''))) " +
	 "LIKE " + UNACCENT + "(LOWER(" +
	 ESCAPE + "'%' || ? || '%')) AND ");
      searchstr.append
	(UNACCENT +
	 "(LOWER(COALESCE(photograph_collection.accession_number, ''))) "
	 "LIKE " + UNACCENT + "(LOWER(" + ESCAPE + "'%' || ? || '%'))");
      searchstr.append("GROUP BY photograph_collection.title, "
		       "photograph_collection.id, "
		       "photograph_collection.location, "
		       "photograph_collection.about, "
		       "photograph_collection.accession_number, "
		       "photograph_collection.type, "
		       "photograph_collection.myoid, "
		       "photograph_collection.image_scaled");

      auto query = new QSqlQuery(qmain->getDB());

      query->prepare(searchstr);
      query->addBindValue(pc.id_collection->text().trimmed());
      query->addBindValue
	(biblioteq_myqstring::escape(pc.title_collection->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(pc.about_collection->toPlainText().
				     trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(pc.notes_collection->toPlainText().
				     trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(pc.accession_number->text().trimmed()));
      (void) qmain->populateTable
	(query,
	 "Photograph Collections",
	 biblioteq::NEW_PAGE,
	 biblioteq::POPULATE_SEARCH);
    }
}

void biblioteq_photographcollection::slotImageViewSizeChanged(int index)
{
  auto comboBox = qobject_cast<QComboBox *> (sender());

  if(!comboBox)
    return;

  auto parent = comboBox->parentWidget();

  do
    {
      if(!parent)
	break;

      if(qobject_cast<QMainWindow *> (parent))
	break;

      parent = parent->parentWidget();
    }
  while(true);

  if(!parent)
    return;

  auto scene = parent->findChild<QGraphicsScene *> ();

  if(scene)
    {
      auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>
	(scene->items().value(0));

      if(item)
	{
	  QImage image;

	  if(image.loadFromData(item->data(1).toByteArray()))
	    {
	      QSize size;
	      auto percent = index;

	      if(percent == 1)
		percent = 100;
	      else if(percent == 2)
		percent = 80;
	      else if(percent == 3)
		percent = 50;
	      else if(percent == 4)
		percent = 25;

	      if(percent == 0)
		{
		  if(scene->views().value(0))
		    {
		      scene->setProperty
			("view_size", scene->views().value(0)->size());
		      size = scene->views().value(0)->size();
		    }
		  else
		    size = scene->property("view_size").toSize();
		}
	      else
		{
		  size = image.size();
		  size.setHeight((percent * size.height()) / 100);
		  size.setWidth((percent * size.width()) / 100);
		}

	      if(!image.isNull())
		image = image.scaled
		  (size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	      item->setPixmap(QPixmap::fromImage(image));
	      scene->setSceneRect(scene->itemsBoundingRect());

	      auto view = qobject_cast<biblioteq_photograph_view *>
		(scene->views().value(0));

	      if(view)
		view->setBestFit(percent == 0);
	    }
	}
    }
}

void biblioteq_photographcollection::slotImportItems(void)
{
  QFileDialog dialog(this);
  QStringList list;

  list << "*" << "*.bmp" << "*.jpg" << "*.jpeg" << "*.png";

  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilters(list);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection Import"));
  dialog.exec();
  QApplication::processEvents();

  if(dialog.result() != QDialog::Accepted)
    return;

  repaint();
  QApplication::processEvents();

  auto const files(dialog.selectedFiles());

  if(files.isEmpty())
    return;

  QProgressDialog progress(this);

  progress.setLabelText(tr("Importing image(s)..."));
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

  int imported = 0;
  int pages = 0;

  for(int i = 0; i < files.size(); i++)
    {
      if(i + 1 <= progress.maximum())
	progress.setValue(i + 1);

      progress.repaint();
      QApplication::processEvents();

      if(progress.wasCanceled())
	break;

      QByteArray bytes1;
      QFile file(files.at(i));

      if(!file.open(QIODevice::ReadOnly))
	continue;
      else
	bytes1 = file.readAll();

      if(static_cast<qint64> (bytes1.length()) != file.size())
	continue;

      QImage image;

      if(!image.loadFromData(bytes1))
	continue;

      QSqlQuery query(qmain->getDB());

      if(qmain->getDB().driverName() != "QSQLITE")
	query.prepare("INSERT INTO photograph "
		      "(id, collection_oid, title, creators, pdate, "
		      "quantity, medium, reproduction_number, "
		      "copyright, callnumber, other_number, notes, subjects, "
		      "format, image, image_scaled, accession_number) "
		      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, "
		      "?, ?, ?, ?, ?, ?, ?, ?)");
      else
	query.prepare("INSERT INTO photograph "
		      "(id, collection_oid, title, creators, pdate, "
		      "quantity, medium, reproduction_number, "
		      "copyright, callnumber, other_number, notes, subjects, "
		      "format, image, image_scaled, accession_number, myoid) "
		      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, "
		      "?, ?, ?, ?, ?, ?, ?, ?, ?)");

      QString id("");

      id = QString::number(QDateTime::currentMSecsSinceEpoch() +
			   static_cast<qint64> (imported));
      query.bindValue(0, id);
      query.bindValue(1, m_oid);
      query.bindValue(2, "N/A");
      query.bindValue(3, "N/A");
      query.bindValue(4, "01/01/2000");
      query.bindValue(5, 1);
      query.bindValue(6, "N/A");
      query.bindValue(7, "N/A");
      query.bindValue(8, "N/A");
      query.bindValue(9, "N/A");
      query.bindValue(10, "N/A");
      query.bindValue(11, "N/A");
      query.bindValue(12, "N/A");
      query.bindValue(13, "N/A");
      query.bindValue(14, bytes1.toBase64());

      QBuffer buffer;
      QByteArray bytes2;
      auto const format(biblioteq_misc_functions::imageFormatGuess(bytes1));

      buffer.setBuffer(&bytes2);
      buffer.open(QIODevice::WriteOnly);

      if(!image.isNull())
	image = image.scaled
	  (biblioteq::s_noImageResize,
	   Qt::KeepAspectRatio,
	   Qt::SmoothTransformation);

      if(image.isNull() || !image.save(&buffer,
				       format.toLatin1().constData(),
				       100))
	bytes2 = bytes1;

      query.bindValue(15, bytes2.toBase64());
      query.bindValue(16, "N/A");

      if(qmain->getDB().driverName() == "QSQLITE")
	{
	  QString errorstr("");
	  auto const value = biblioteq_misc_functions::getSqliteUniqueId
	    (qmain->getDB(), errorstr);

	  if(errorstr.isEmpty())
	    query.bindValue(17, value);
	  else
	    qmain->addError
	      (tr("Database Error"),
	       tr("Unable to generate a unique integer."),
	       errorstr);
	}

      if(!query.exec())
	qmain->addError(tr("Database Error"),
			tr("Unable to import photograph."),
			query.lastError().text(),
			__FILE__,
			__LINE__);
      else
	imported += 1;
    }

  progress.close();
  repaint();
  QApplication::processEvents();
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QSqlQuery query(qmain->getDB());

  query.prepare("SELECT COUNT(*) "
		"FROM photograph "
		"WHERE collection_oid = ?");
  query.bindValue(0, m_oid);

  if(query.exec())
    if(query.next())
      {
	pc.statistics->setText
	  (tr("%1 Image(s)").arg(query.value(0).toLongLong()));
	updateTablePhotographCount(query.value(0).toLongLong());

	auto const i = photographsPerPage();

	if(i == -1) // Unlimited.
	  {
	    pages = 1;
	    setSceneRect(query.value(0).toLongLong());
	  }
	else
	  pages = qCeil(query.value(0).toDouble() / qMax(1, i));
      }

  QApplication::restoreOverrideCursor();
  pages = qMax(1, pages);
  pc.page->blockSignals(true);
  pc.page->clear();

  for(int i = 1; i <= pages; i++)
    pc.page->addItem(QString::number(i));

  pc.page->blockSignals(false);
  showPhotographs(1);
  QMessageBox::information(this,
			   tr("BiblioteQ: Information"),
			   tr("Imported a total of %1 image(s) from the "
			      "directory %2.").
			   arg(imported).
			   arg(dialog.directory().absolutePath()));
  QApplication::processEvents();
}

void biblioteq_photographcollection::slotInsertItem(void)
{
  if(!verifyItemFields())
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().transaction())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to create a database transaction."),
		      qmain->getDB().lastError().text(),
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(m_photo_diag,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to create a database transaction."));
      QApplication::processEvents();
      return;
    }
  else
    QApplication::restoreOverrideCursor();

  QSqlQuery query(qmain->getDB());
  QString errorstr("");
  int pages = 1;

  if(qmain->getDB().driverName() != "QSQLITE")
    query.prepare("INSERT INTO photograph "
		  "(id, collection_oid, title, creators, pdate, "
		  "quantity, medium, reproduction_number, "
		  "copyright, callnumber, other_number, notes, subjects, "
		  "format, image, image_scaled, accession_number) "
		  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, "
		  "?, ?, ?, ?, ?, ?, ?, ?)");
  else
    query.prepare("INSERT INTO photograph "
		  "(id, collection_oid, title, creators, pdate, "
		  "quantity, medium, reproduction_number, "
		  "copyright, callnumber, other_number, notes, subjects, "
		  "format, image, image_scaled, accession_number, myoid) "
		  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, "
		  "?, ?, ?, ?, ?, ?, ?, ?, ?)");

  query.bindValue(0, photo.id_item->text());
  query.bindValue(1, m_oid);
  query.bindValue(2, photo.title_item->text());
  query.bindValue(3, photo.creators_item->toPlainText());
  query.bindValue
    (4, photo.publication_date->date().toString(biblioteq::
						s_databaseDateFormat));
  query.bindValue(5, photo.quantity->value());
  query.bindValue(6, photo.medium_item->text());
  query.bindValue(7, photo.reproduction_number_item->toPlainText());
  query.bindValue(8, photo.copyright_item->toPlainText());
  query.bindValue(9, photo.call_number_item->text().trimmed());
  query.bindValue(10, photo.other_number_item->text().trimmed());
  query.bindValue(11, photo.notes_item->toPlainText().trimmed());
  query.bindValue(12, photo.subjects_item->toPlainText().trimmed());
  query.bindValue(13, photo.format_item->toPlainText().trimmed());

  if(!photo.thumbnail_item->m_image.isNull())
    {
      QBuffer buffer;
      QByteArray bytes;
      QImage image;

      buffer.setBuffer(&bytes);

      if(buffer.open(QIODevice::WriteOnly))
	{
	  photo.thumbnail_item->m_image.save
	    (&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
	  query.bindValue(14, bytes.toBase64());
	}
      else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	query.bindValue(14, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	query.bindValue(14, QVariant(QVariant::ByteArray));
#endif

      buffer.close();
      bytes.clear();
      image = photo.thumbnail_item->m_image;

      if(!image.isNull())
	image = image.scaled
	  (biblioteq::s_noImageResize,
	   Qt::KeepAspectRatio,
	   Qt::SmoothTransformation);

      if(buffer.open(QIODevice::WriteOnly))
	{
	  image.save
	    (&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
	  query.bindValue(15, bytes.toBase64());
	}
      else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	query.bindValue(15, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	query.bindValue(15, QVariant(QVariant::ByteArray));
#endif
    }
  else
    {
      photo.thumbnail_item->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      query.bindValue(14, QVariant(QMetaType(QMetaType::QByteArray)));
      query.bindValue(15, QVariant(QMetaType(QMetaType::QByteArray)));
#else
      query.bindValue(14, QVariant(QVariant::ByteArray));
      query.bindValue(15, QVariant(QVariant::ByteArray));
#endif
    }

  query.bindValue(16, photo.accession_number_item->text().trimmed());

  if(qmain->getDB().driverName() == "QSQLITE")
    {
      auto const value = biblioteq_misc_functions::getSqliteUniqueId
	(qmain->getDB(), errorstr);

      if(errorstr.isEmpty())
	query.bindValue(17, value);
      else
	qmain->addError(tr("Database Error"),
			tr("Unable to generate a unique integer."),
			errorstr);
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to create or update the entry."),
		      query.lastError().text(),
		      __FILE__,
		      __LINE__);
      goto db_rollback;
    }
  else
    {
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

      QApplication::restoreOverrideCursor();
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  query.prepare("SELECT COUNT(*) "
		"FROM photograph "
		"WHERE collection_oid = ?");
  query.bindValue(0, m_oid);

  if(query.exec())
    if(query.next())
      {
	pc.statistics->setText
	  (tr("%1 Image(s)").arg(query.value(0).toLongLong()));
	updateTablePhotographCount(query.value(0).toLongLong());

	auto const i = photographsPerPage();

	if(i == -1) // Unlimited.
	  {
	    pages = 1;
	    setSceneRect(query.value(0).toLongLong());
	  }
	else
	  pages = qCeil(query.value(0).toDouble() / qMax(1, i));
      }

  QApplication::restoreOverrideCursor();
  pages = qMax(1, pages);
  pc.page->blockSignals(true);
  pc.page->clear();

  for(int i = 1; i <= pages; i++)
    pc.page->addItem(QString::number(i));

  pc.page->blockSignals(false);
  showPhotographs(pc.page->currentText().toInt());
  photo.saveButton->disconnect(SIGNAL(clicked(void)));
  connect(photo.saveButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotModifyItem(void)));
  return;

 db_rollback:

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().rollback())
    qmain->addError
      (tr("Database Error"),
       tr("Rollback failure."),
       qmain->getDB().lastError().text(),
       __FILE__,
       __LINE__);

  QApplication::restoreOverrideCursor();
  QMessageBox::critical(m_photo_diag,
			tr("BiblioteQ: Database Error"),
			tr("Unable to create the item. Please verify that "
			   "the item does not already exist."));
  QApplication::processEvents();
}

void biblioteq_photographcollection::slotModifyItem(void)
{
  photo.saveButton->disconnect(SIGNAL(clicked(void)));
  connect(photo.saveButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotUpdateItem(void)));
  m_photo_diag->resize(m_photo_diag->width(), qRound(0.95 * size().height()));
  photo.id_item->setFocus();
  photo.scrollArea->ensureVisible(0, 0);
#ifdef Q_OS_ANDROID
  m_photo_diag->showMaximized();
#else
  biblioteq_misc_functions::center
    (m_photo_diag, parentWidget() ? m_parentWid : this);
  m_photo_diag->show();
#endif
}

void biblioteq_photographcollection::slotPageChanged(int index)
{
  pc.page->repaint();
  QApplication::processEvents();
  showPhotographs(index + 1);
}

void biblioteq_photographcollection::slotPrepareIcons(void)
{
  prepareIcons(this);
  pc.selectAllButton->setIcon(QIcon());
}

void biblioteq_photographcollection::slotPrint(void)
{
  m_html = "<html>";
  m_html += "<b>" + tr("Collection ID:") + "</b> " +
    pc.id_collection->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Collection Title:") + "</b> " +
    pc.title_collection->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Collection Location:") + "</b> " +
    pc.location->currentText().trimmed() + "<br>";
  m_html += "<b>" + tr("Collection About:") + "</b> " +
    pc.about_collection->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Collection Notes:") + "</b> " +
    pc.notes_collection->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Item ID:") + "</b> " +
    pc.id_item->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Title:") + "</b> " +
    pc.title_item->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Creators:") + "</b> " +
    pc.creators_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Publication Date:") + "</b> " +
    pc.publication_date->date().toString(Qt::ISODate) + "<br>";
  m_html += "<b>" + tr("Item Copies:") + "</b> " +
    pc.quantity->text() + "<br>";
  m_html += "<b>" + tr("Item Medium:") + "</b> " +
    pc.medium_item->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Reproduction Number:") + "</b> " +
    pc.reproduction_number_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Copyright:") + "</b> " +
    pc.copyright_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Call Number:") + "</b> " +
    pc.call_number_item->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Other Number:") + "</b> " +
    pc.other_number_item->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Notes:") + "</b> " +
    pc.notes_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Subjects:") + "</b> " +
    pc.subjects_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Item Format:") + "</b> " +
    pc.format_item->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Accession Number:") + "</b> " +
    pc.accession_number_item->text().trimmed();
  m_html += "</html>";
  print(this);
}

void biblioteq_photographcollection::slotQuery(void)
{
}

void biblioteq_photographcollection::slotReset(void)
{
  auto action = qobject_cast<QAction *> (sender());

  if(action != nullptr)
    {
      auto const actions = pc.resetButton->menu()->actions();

      if(actions.size() < 7)
	{
	  // Error.
	}
      else if(action == actions[0])
	pc.thumbnail_collection->clear();
      else if(action == actions[1])
	{
	  pc.id_collection->clear();
	  pc.id_collection->setFocus();
	}
      else if(action == actions[2])
	{
	  pc.title_collection->clear();
	  pc.title_collection->setFocus();
	}
      else if(action == actions[3])
	{
	  pc.location->setCurrentIndex(0);
	  pc.location->setFocus();
	}
      else if(action == actions[4])
	{
	  pc.about_collection->clear();
	  pc.about_collection->setFocus();
	}
      else if(action == actions[5])
	{
	  pc.notes_collection->clear();
	  pc.notes_collection->setFocus();
	}
      else if(action == actions[6])
	{
	  pc.accession_number->clear();
	  pc.accession_number->setFocus();
	}
    }
  else
    {
      /*
      ** Reset all.
      */

      pc.about_collection->clear();
      pc.accession_number->clear();
      pc.id_collection->clear();
      pc.id_collection->setFocus();
      pc.location->setCurrentIndex(0);
      pc.notes_collection->clear();
      pc.thumbnail_collection->clear();
      pc.title_collection->clear();
    }
}

void biblioteq_photographcollection::slotSaveRotatedImage
(const QImage &image, const QString &format, const qint64 oid)
{
  if(image.isNull() || oid < 0)
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QBuffer buffer;
  QByteArray bytes1;

  buffer.setBuffer(&bytes1);

  if(buffer.open(QIODevice::WriteOnly) &&
     image.save(&buffer, format.toUpper().toLatin1().constData(), 100))
    {
      QSqlQuery query(qmain->getDB());

      query.prepare("UPDATE photograph SET "
		    "image = ?, "
		    "image_scaled = ? "
		    "WHERE myoid = ?");
      query.addBindValue(bytes1);

      QBuffer buffer;
      QByteArray bytes2;
      auto i(image);

      buffer.setBuffer(&bytes2);
      buffer.open(QIODevice::WriteOnly);
      i = i.scaled
	(biblioteq::s_noImageResize,
	 Qt::KeepAspectRatio,
	 Qt::SmoothTransformation);

      if(i.isNull() || !i.save(&buffer,
			       format.toUpper().toLatin1().constData(),
			       100))
	bytes2 = bytes1;

      query.addBindValue(bytes2);
      query.addBindValue(oid);

      if(!query.exec())
	qmain->addError(tr("Database Error"),
			tr("Unable to update photograph."),
			query.lastError().text(),
			__FILE__,
			__LINE__);
      else
	{
	  auto const list(pc.graphicsView->scene()->items(Qt::AscendingOrder));

	  for(int i = 0; i < list.size(); i++)
	    if(list.at(i)->data(0).toLongLong() == oid)
	      {
		auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>
		  (list.at(i));

		if(item)
		  item->setPixmap
		    (QPixmap::
		     fromImage(image.scaled(biblioteq::s_noImageResize,
					    Qt::KeepAspectRatio,
					    Qt::SmoothTransformation)));

		break;
	      }
	}
    }

  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::slotSceneSelectionChanged(void)
{
  auto const items(pc.graphicsView->scene()->selectedItems());

  if(items.size() > 1)
    {
      m_itemOid.clear();
      pc.accession_number_item->clear();
      pc.call_number_item->clear();
      pc.copyright_item->clear();
      pc.creators_item->clear();

      if(pc.exportPhotographsToolButton->menu() &&
	 pc.exportPhotographsToolButton->menu()->actions().size() >= 3)
	pc.exportPhotographsToolButton->menu()->actions()[2]->
	  setEnabled(false);

      pc.format_item->clear();
      pc.id_item->clear();
      pc.medium_item->clear();
      pc.notes_item->clear();
      pc.other_number_item->clear();
      pc.publication_date->setDate
	(QDate::fromString("01/01/2000", biblioteq::s_databaseDateFormat));
      pc.quantity->setValue(1);
      pc.reproduction_number_item->clear();
      pc.subjects_item->clear();
      pc.thumbnail_item->clear();
      pc.title_item->clear();
      return;
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(pc.exportPhotographsToolButton->menu() &&
     pc.exportPhotographsToolButton->menu()->actions().size() >= 3)
    pc.exportPhotographsToolButton->menu()->actions()[2]->
      setEnabled(true);

  QGraphicsPixmapItem *item = nullptr;

  if((item = qgraphicsitem_cast<QGraphicsPixmapItem *> (items.at(0))))
    {
      m_itemOid = item->data(0).toString();

      QSqlQuery query(qmain->getDB());

      query.setForwardOnly(true);
      query.prepare("SELECT id, "
		    "title, "
		    "creators, "
		    "pdate, "
		    "quantity, "
		    "medium, "
		    "reproduction_number, "
		    "copyright, "
		    "callnumber, "
		    "other_number, "
		    "notes, "
		    "subjects, "
		    "format, "
		    "image, "
		    "accession_number "
		    "FROM photograph "
		    "WHERE collection_oid = ? AND "
		    "myoid = ?");
      query.bindValue(0, m_oid);
      query.bindValue(1, item->data(0).toString());

      if(query.exec())
	if(query.next())
	  {
	    auto const record(query.record());

	    for(int i = 0; i < record.count(); i++)
	      {
		auto const fieldname(record.fieldName(i));
		auto const var(record.field(i).value());

		if(fieldname == "accession_number")
		  {
		    pc.accession_number_item->setText(var.toString().trimmed());
		    photo.accession_number_item->setText
		      (var.toString().trimmed());
		  }
		else if(fieldname == "callnumber")
		  {
		    pc.call_number_item->setText(var.toString().trimmed());
		    photo.call_number_item->setText(var.toString().trimmed());
		  }
		else if(fieldname == "copyright")
		  {
		    pc.copyright_item->setPlainText(var.toString().trimmed());
		    photo.copyright_item->setPlainText
		      (var.toString().trimmed());
		  }
		else if(fieldname == "creators")
		  {
		    pc.creators_item->setPlainText(var.toString().trimmed());
		    photo.creators_item->setPlainText(var.toString().trimmed());
		  }
		else if(fieldname == "format")
		  {
		    pc.format_item->setPlainText(var.toString().trimmed());
		    photo.format_item->setPlainText(var.toString().trimmed());
		  }
		else if(fieldname == "id")
		  {
		    pc.id_item->setText(var.toString().trimmed());
		    photo.id_item->setText(var.toString().trimmed());
		  }
		else if(fieldname == "image")
		  {
		    if(!record.field(i).isNull())
		      {
			pc.thumbnail_item->loadFromData
			  (QByteArray::fromBase64(var.toByteArray()));

			if(pc.thumbnail_item->m_image.isNull())
			  pc.thumbnail_item->loadFromData
			    (var.toByteArray());

			photo.thumbnail_item->loadFromData
			  (QByteArray::fromBase64(var.toByteArray()));

			if(photo.thumbnail_item->m_image.isNull())
			  photo.thumbnail_item->loadFromData
			    (var.toByteArray());
		      }
		    else
		      {
			pc.thumbnail_item->clear();
			photo.thumbnail_item->clear();
		      }
		  }
		else if(fieldname == "medium")
		  {
		    pc.medium_item->setText(var.toString().trimmed());
		    photo.medium_item->setText(var.toString().trimmed());
		  }
		else if(fieldname == "notes")
		  {
		    pc.notes_item->setPlainText(var.toString().trimmed());
		    photo.notes_item->setPlainText(var.toString().trimmed());
		  }
		else if(fieldname == "other_number")
		  {
		    pc.other_number_item->setText(var.toString().trimmed());
		    photo.other_number_item->setText(var.toString().trimmed());
		  }
		else if(fieldname == "pdate")
		  {
		    pc.publication_date->setDate
		      (QDate::fromString(var.toString().trimmed(),
					 biblioteq::s_databaseDateFormat));
		    photo.publication_date->setDate
		      (QDate::fromString(var.toString().trimmed(),
					 biblioteq::s_databaseDateFormat));
		  }
		else if(fieldname == "quantity")
		  {
		    pc.quantity->setValue(var.toInt());
		    photo.quantity->setValue(var.toInt());
		  }
		else if(fieldname == "reproduction_number")
		  {
		    pc.reproduction_number_item->setPlainText
		      (var.toString().trimmed());
		    photo.reproduction_number_item->setPlainText
		      (var.toString().trimmed());
		  }
		else if(fieldname == "subjects")
		  {
		    pc.subjects_item->setPlainText(var.toString().trimmed());
		    photo.subjects_item->setPlainText
		      (var.toString().trimmed());
		  }
		else if(fieldname == "title")
		  {
		    pc.title_item->setText(var.toString().trimmed());
		    photo.title_item->setText(var.toString().trimmed());
		  }
	      }
	  }
    }

  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::slotSelectAll(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QPainterPath painterPath;

  painterPath.addRect(pc.graphicsView->sceneRect());
  pc.graphicsView->scene()->setSelectionArea(painterPath, QTransform());
  QApplication::restoreOverrideCursor();
}

void biblioteq_photographcollection::slotSelectImage(void)
{
  QFileDialog dialog(this);
  auto button = qobject_cast<QPushButton *> (sender());

  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if(button == pc.select_image_collection)
    dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection "
			     "Image Selection"));
  else
    dialog.setWindowTitle(tr("BiblioteQ: Photograph Collection Item "
			     "Image Selection"));

  dialog.exec();
  QApplication::processEvents();

  if(dialog.result() == QDialog::Accepted)
    {
      if(button == pc.select_image_collection)
	{
	  pc.thumbnail_collection->clear();
	  pc.thumbnail_collection->m_image = QImage(dialog.selectedFiles().
						    value(0));

	  if(dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
	    pc.thumbnail_collection->m_imageFormat =
	      dialog.selectedFiles().value(0).mid
	      (dialog.selectedFiles().value(0).lastIndexOf(".") + 1).
	      toUpper();

	  pc.thumbnail_collection->scene()->addPixmap
	    (QPixmap::fromImage(pc.thumbnail_collection->m_image));

	  if(!pc.thumbnail_collection->scene()->items().isEmpty())
	    pc.thumbnail_collection->scene()->items().at(0)->setFlags
	      (QGraphicsItem::ItemIsSelectable);

	  pc.thumbnail_collection->scene()->setSceneRect
	    (pc.thumbnail_collection->scene()->itemsBoundingRect());
	}
      else
	{
	  photo.thumbnail_item->clear();
	  photo.thumbnail_item->m_image = QImage(dialog.selectedFiles().
						 value(0));

	  if(dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
	    photo.thumbnail_item->m_imageFormat = dialog.selectedFiles().
	      value(0).mid
	      (dialog.selectedFiles().value(0).lastIndexOf(".") + 1).
	      toUpper();

	  photo.thumbnail_item->scene()->addPixmap
	    (QPixmap::fromImage(photo.thumbnail_item->m_image));

	  if(!photo.thumbnail_item->scene()->items().isEmpty())
	    photo.thumbnail_item->scene()->items().at(0)->setFlags
	      (QGraphicsItem::ItemIsSelectable);

	  photo.thumbnail_item->scene()->setSceneRect
	    (photo.thumbnail_item->scene()->itemsBoundingRect());
	}
    }
}

void biblioteq_photographcollection::slotShowMoveCollectionPanel(void)
{
}

void biblioteq_photographcollection::slotUpdateItem(void)
{
  if(!verifyItemFields())
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().transaction())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to create a database transaction."),
		      qmain->getDB().lastError().text(),
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(m_photo_diag,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to create a database transaction."));
      QApplication::processEvents();
      return;
    }
  else
    QApplication::restoreOverrideCursor();

  QSqlQuery query(qmain->getDB());

  query.prepare("UPDATE photograph SET "
		"id = ?, title = ?, "
		"creators = ?, pdate = ?, "
		"quantity = ?, medium = ?, reproduction_number = ?, "
		"copyright = ?, callnumber = ?, other_number = ?, "
		"notes = ?, subjects = ?, "
		"format = ?, image = ?, image_scaled = ?, "
		"accession_number = ? "
		"WHERE collection_oid = ? AND myoid = ?");
  query.bindValue(0, photo.id_item->text());
  query.bindValue(1, photo.title_item->text());
  query.bindValue(2, photo.creators_item->toPlainText());
  query.bindValue
    (3, photo.publication_date->date().toString(biblioteq::
						s_databaseDateFormat));
  query.bindValue(4, photo.quantity->value());
  query.bindValue(5, photo.medium_item->text());
  query.bindValue(6, photo.reproduction_number_item->toPlainText());
  query.bindValue(7, photo.copyright_item->toPlainText());
  query.bindValue(8, photo.call_number_item->text().trimmed());
  query.bindValue(9, photo.other_number_item->text().trimmed());
  query.bindValue(10, photo.notes_item->toPlainText().trimmed());
  query.bindValue(11, photo.subjects_item->toPlainText().trimmed());
  query.bindValue(12, photo.format_item->toPlainText().trimmed());

  if(!photo.thumbnail_item->m_image.isNull())
    {
      QBuffer buffer;
      QByteArray bytes;
      QImage image;

      buffer.setBuffer(&bytes);

      if(buffer.open(QIODevice::WriteOnly))
	{
	  photo.thumbnail_item->m_image.save
	    (&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
	  query.bindValue(13, bytes.toBase64());
	}
      else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	query.bindValue(13, QVariant(QMetaType(QMetaType::QByteArray)));
#else
        query.bindValue(13, QVariant(QVariant::ByteArray));
#endif

      buffer.close();
      bytes.clear();
      image = photo.thumbnail_item->m_image;

      if(!image.isNull())
	image = image.scaled
	  (biblioteq::s_noImageResize,
	   Qt::KeepAspectRatio,
	   Qt::SmoothTransformation);

      if(buffer.open(QIODevice::WriteOnly))
	{
	  image.save
	    (&buffer, photo.thumbnail_item->m_imageFormat.toLatin1(), 100);
	  query.bindValue(14, bytes.toBase64());
	}
      else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	query.bindValue(14, QVariant(QMetaType(QMetaType::QByteArray)));
#else
        query.bindValue(14, QVariant(QVariant::ByteArray));
#endif
    }
  else
    {
      photo.thumbnail_item->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
      query.bindValue(13, QVariant(QMetaType(QMetaType::QByteArray)));
      query.bindValue(14, QVariant(QMetaType(QMetaType::QByteArray)));
#else
      query.bindValue(13, QVariant(QVariant::ByteArray));
      query.bindValue(14, QVariant(QVariant::ByteArray));
#endif
    }

  query.bindValue(15, photo.accession_number_item->text().trimmed());
  query.bindValue(16, m_oid);
  query.bindValue(17, m_itemOid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to create or update the entry."),
		      query.lastError().text(),
		      __FILE__,
		      __LINE__);
      goto db_rollback;
    }
  else
    {
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

      QApplication::restoreOverrideCursor();
      pc.accession_number_item->setText(photo.accession_number_item->text());
      pc.call_number_item->setText(photo.call_number_item->text());
      pc.copyright_item->setPlainText(photo.copyright_item->toPlainText());
      pc.creators_item->setPlainText(photo.creators_item->toPlainText());
      pc.format_item->setPlainText(photo.format_item->toPlainText());
      pc.id_item->setText(photo.id_item->text());
      pc.medium_item->setText(photo.medium_item->text());
      pc.notes_item->setPlainText(photo.notes_item->toPlainText());
      pc.other_number_item->setText(photo.other_number_item->text());
      pc.publication_date->setDate(photo.publication_date->date());
      pc.quantity->setValue(photo.quantity->value());
      pc.reproduction_number_item->setPlainText
	(photo.reproduction_number_item->toPlainText());
      pc.subjects_item->setPlainText(photo.subjects_item->toPlainText());
      pc.thumbnail_item->setImage(photo.thumbnail_item->m_image);
      pc.title_item->setText(photo.title_item->text());
    }

  return;

 db_rollback:

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().rollback())
    qmain->addError
      (tr("Database Error"),
       tr("Rollback failure."),
       qmain->getDB().lastError().text(),
       __FILE__,
       __LINE__);

  QApplication::restoreOverrideCursor();
  QMessageBox::critical(m_photo_diag,
			tr("BiblioteQ: Database Error"),
			tr("Unable to update the item. Please verify that "
			   "the item does not already exist."));
  QApplication::processEvents();
}

void biblioteq_photographcollection::slotViewContextMenu(const QPoint &pos)
{
  auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>
    (pc.graphicsView->itemAt(pos));

  if(item)
    {
      item->setSelected(true);

      QAction *action = nullptr;
      QMenu menu(this);

      action = menu.addAction(tr("&Delete Photograph"),
			      this,
			      SLOT(slotDeleteItem(void)));
      action->setData(pos);

      if(m_engWindowTitle != "Modify")
	action->setEnabled(false);

      action = menu.addAction(tr("&Modify Photograph..."),
			      this,
			      SLOT(slotModifyItem(void)));
      action->setData(pos);

      if(m_engWindowTitle != "Modify")
	action->setEnabled(false);

      menu.addSeparator();
      action = menu.addAction(tr("&View Photograph..."),
			      this,
			      SLOT(slotViewPhotograph(void)));
      action->setData(pos);
      menu.exec(QCursor::pos());
    }
}

void biblioteq_photographcollection::slotViewNextPhotograph(void)
{
  auto toolButton = qobject_cast<QToolButton *> (sender());

  if(!toolButton)
    return;

  auto parent = toolButton->parentWidget();

  do
    {
      if(!parent)
	break;

      if(qobject_cast<QMainWindow *> (parent))
	break;

      parent = parent->parentWidget();
    }
  while(true);

  if(!parent)
    return;

  auto comboBox = parent->findChild<QComboBox *> ();
  auto const percent = comboBox ?
    comboBox->currentText().remove("%").toInt() : 100;
  auto scene = parent->findChild<QGraphicsScene *> ();
  auto text = parent->findChild<QTextBrowser *> ();

  if(scene)
    {
      auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>
	(scene->items().value(0));

      if(item)
	{
	  QApplication::setOverrideCursor(Qt::WaitCursor);

	  auto const list(pc.graphicsView->scene()->items(Qt::AscendingOrder));
	  int idx = -1;

	  for(int i = 0; i < list.size(); i++)
	    if(item->data(0) == list.at(i)->data(0))
	      {
		idx = i;
		break;
	      }

	  idx += 1;

	  if(idx >= list.size())
	    idx = 0;

	  QApplication::restoreOverrideCursor();
	  loadPhotographFromItem
	    (qgraphicsitem_cast<QGraphicsPixmapItem *> (list.value(idx)),
	     scene,
	     text,
	     percent);
	}
    }
}

void biblioteq_photographcollection::slotViewPhotograph(void)
{
  QPoint pos;
  auto action = qobject_cast<QAction *> (sender());

  if(action)
    pos = action->data().toPoint();

  if(pos.isNull())
    pos = pc.graphicsView->mapFromGlobal(QCursor::pos());

  loadPhotographFromItemInNewWindow
    (qgraphicsitem_cast<QGraphicsPixmapItem *> (pc.graphicsView->itemAt(pos)));
}

void biblioteq_photographcollection::slotViewPreviousPhotograph(void)
{
  auto toolButton = qobject_cast<QToolButton *> (sender());

  if(!toolButton)
    return;

  auto parent = toolButton->parentWidget();

  do
    {
      if(!parent)
	break;

      if(qobject_cast<QMainWindow *> (parent))
	break;

      parent = parent->parentWidget();
    }
  while(true);

  if(!parent)
    return;

  auto comboBox = parent->findChild<QComboBox *> ();
  auto const percent = comboBox ?
    comboBox->currentText().remove("%").toInt() : 100;
  auto scene = parent->findChild<QGraphicsScene *> ();
  auto text = parent->findChild<QTextBrowser *> ();

  if(scene)
    {
      auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>
	(scene->items().value(0));

      if(item)
	{
	  QApplication::setOverrideCursor(Qt::WaitCursor);

	  auto const list(pc.graphicsView->scene()->items(Qt::AscendingOrder));
	  int idx = -1;

	  for(int i = 0; i < list.size(); i++)
	    if(item->data(0) == list.at(i)->data(0))
	      {
		idx = i;
		break;
	      }

	  idx -= 1;

	  if(idx < 0)
	    idx = list.size() - 1;

	  QApplication::restoreOverrideCursor();
	  loadPhotographFromItem
	    (qgraphicsitem_cast<QGraphicsPixmapItem *> (list.value(idx)),
	     scene,
	     text,
	     percent);
	}
    }
}

void biblioteq_photographcollection::storeData(void)
{
  QList<QWidget *> list;
  QString classname("");
  QString objectname("");

  m_widgetValues.clear();
  list << pc.about_collection
       << pc.accession_number
       << pc.id_collection
       << pc.location
       << pc.notes_collection
       << pc.thumbnail_collection
       << pc.title_collection;

  foreach(auto widget, list)
    {
      classname = widget->metaObject()->className();
      objectname = widget->objectName();

      if(classname == "QComboBox")
	m_widgetValues[objectname] =
	  (qobject_cast<QComboBox *> (widget))->currentText().trimmed();
      else if(classname == "QLineEdit")
	m_widgetValues[objectname] =
	  (qobject_cast<QLineEdit *> (widget))->text().trimmed();
      else if(classname == "QTextEdit")
	m_widgetValues[objectname] =
	  (qobject_cast<QTextEdit *> (widget))->toPlainText().trimmed();
      else if(classname == "biblioteq_image_drop_site")
	m_imageValues[objectname] =
	  (qobject_cast<biblioteq_image_drop_site *> (widget))->m_image;
    }
}

void biblioteq_photographcollection::updateTablePhotographCount
(const qint64 count)
{
  if(m_index->isValid() &&
     (qmain->getTypeFilterString() == "All" ||
      qmain->getTypeFilterString() == "All Available" ||
      qmain->getTypeFilterString() == "All Overdue" ||
      qmain->getTypeFilterString() == "All Requested" ||
      qmain->getTypeFilterString() == "All Reserved" ||
      qmain->getTypeFilterString() == "Photograph Collections"))
    {
      qmain->getUI().table->setSortingEnabled(false);

      auto const names(qmain->getUI().table->columnNames());

      for(int i = 0; i < names.size(); i++)
	if(names.at(i) == "Photograph Count")
	  {
	    qmain->getUI().table->item(m_index->row(), i)->
	      setText(QString::number(count));
	    qmain->slotDisplaySummary();
	    break;
	  }

      qmain->getUI().table->setSortingEnabled(true);
      qmain->getUI().table->updateToolTips(m_index->row());
    }
}

void biblioteq_photographcollection::updateWindow(const int state)
{
  QString str("");

  if(state == biblioteq::EDITABLE)
    {
      pc.addItemButton->setEnabled(true);
      pc.importItems->setEnabled(true);
      pc.okButton->setVisible(true);
      pc.resetButton->setVisible(true);
      str = tr("BiblioteQ: Modify Photograph Collection Entry (") +
	pc.id_collection->text() +
	tr(")");
      m_engWindowTitle = "Modify";
      disconnect(m_scene,
		 SIGNAL(deleteKeyPressed(void)),
		 this,
		 SLOT(slotDeleteItem(void)));
      connect(m_scene,
	      SIGNAL(deleteKeyPressed(void)),
	      this,
	      SLOT(slotDeleteItem(void)));
    }
  else
    {
      pc.addItemButton->setEnabled(false);
      pc.importItems->setEnabled(false);
      pc.okButton->setVisible(false);
      pc.resetButton->setVisible(false);
      str = tr("BiblioteQ: View Photograph Collection Details (") +
	pc.id_collection->text() +
	tr(")");
      m_engWindowTitle = "View";
    }

  setReadOnlyFields(this, state != biblioteq::EDITABLE);
  setReadOnlyFieldsOverride();
  setWindowTitle(str);
  emit windowTitleChanged(windowTitle());
  pc.page->setEnabled(true);
}
