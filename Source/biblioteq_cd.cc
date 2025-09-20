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
#include "biblioteq_cd.h"
#include "biblioteq_copy_editor.h"

#include <QFileDialog>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QSqlField>
#include <QSqlRecord>
#include <QtMath>

biblioteq_cd::biblioteq_cd(biblioteq *parentArg,
			   const QString &oidArg,
			   const QModelIndex &index):
  QMainWindow(), biblioteq_item(index)
{
  QRegularExpression rx1("[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]");
  auto menu = new QMenu(this);
  auto validator1 = new QRegularExpressionValidator(rx1, this);
  auto scene1 = new QGraphicsScene(this);
  auto scene2 = new QGraphicsScene(this);

  qmain = parentArg;
  m_isQueryEnabled = false;
  m_oid = oidArg;
  m_oldq = biblioteq_misc_functions::getColumnString
    (qmain->getUI().table,
     m_index->row(),
     qmain->getUI().table->columnNumber("Quantity")).toInt();
  m_parentWid = parentArg;
  m_tracks_diag = new QDialog(this);
  m_tracks_diag->setWindowModality(Qt::WindowModal);
  cd.setupUi(this);
  setQMain(this);
  biblioteq_misc_functions::sortCombinationBox(cd.audio);
  biblioteq_misc_functions::sortCombinationBox(cd.recording_type);
  cd.publication_date_enabled->setVisible(false);
  cd.quantity->setMaximum(static_cast<int> (biblioteq::Limits::QUANTITY));
  cd.release_date->setDisplayFormat(qmain->publicationDateFormat("musiccds"));
  updateFont(QApplication::font(), qobject_cast<QWidget *> (this));
  trd.setupUi(m_tracks_diag);
  trd.table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S),
		this,
		SLOT(slotGo(void)));
#else
  new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S),
		this,
		SLOT(slotGo(void)));
#endif
  updateFont(QApplication::font(), qobject_cast<QWidget *> (m_tracks_diag));
  connect(cd.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotCancel(void)));
  connect(cd.computeButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotComputeRuntime(void)));
  connect(cd.copiesButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotPopulateCopiesEditor(void)));
  connect(cd.front_image,
	  SIGNAL(imageChanged(const QImage &)),
	  this,
	  SIGNAL(imageChanged(const QImage &)));
  connect(cd.okButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotGo(void)));
  connect(cd.printButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotPrint(void)));
  connect(cd.queryButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotQuery(void)));
  connect(cd.resetButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(cd.showUserButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotShowUsers(void)));
  connect(cd.tracksButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotPopulateTracksBrowser(void)));
  connect(trd.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotCloseTracksBrowser(void)));
  connect(trd.deleteButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotDeleteTrack(void)));
  connect(trd.insertButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotInsertTrack(void)));
  connect(trd.saveButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSaveTracks(void)));
  connect(trd.table->horizontalHeader(),
	  SIGNAL(sectionClicked(int)),
	  qmain,
	  SLOT(slotResizeColumnsAfterSort(void)));
  connect(menu->addAction(tr("Reset Front Cover Image")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Back Cover Image")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Catalog Number")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Format")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Artist")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  m_composer_action = menu->addAction(tr("Reset Composer"));
  m_composer_action->setVisible(false);
  connect(m_composer_action,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Number of Discs")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Runtime")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Audio")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Recording Type")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Title")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Release Date")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Recording Label")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Categories")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Price")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Language")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Monetary Units")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Copies")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Location")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Abstract")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Keywords")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Accession Number")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(cd.frontButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectImage(void)));
  connect(cd.backButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectImage(void)));
  connect(qmain,
	  SIGNAL(fontChanged(const QFont &)),
	  this,
	  SLOT(setGlobalFonts(const QFont &)));
  connect(qmain,
	  SIGNAL(otherOptionsSaved(void)),
	  this,
	  SLOT(slotPrepareIcons(void)));
  cd.composer->setVisible(false);
  cd.composer_label->setVisible(false);
  cd.id->setValidator(validator1);
  cd.queryButton->setVisible(m_isQueryEnabled);
  cd.resetButton->setMenu(menu);

  QString errorstr("");

  QApplication::setOverrideCursor(Qt::WaitCursor);
  cd.format->addItems
    (biblioteq_misc_functions::getCDFormats(qmain->getDB(), errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the cd formats."),
       errorstr,
       __FILE__,
       __LINE__);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  cd.language->addItems
    (biblioteq_misc_functions::getLanguages(qmain->getDB(), errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the languages."),
       errorstr,
       __FILE__,
       __LINE__);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  cd.location->addItems
    (biblioteq_misc_functions::getLocations(qmain->getDB(), "CD", errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the cd locations."),
       errorstr,
       __FILE__,
       __LINE__);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  cd.monetary_units->addItems
    (biblioteq_misc_functions::getMonetaryUnits(qmain->getDB(), errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the monetary units."),
       errorstr,
       __FILE__,
       __LINE__);

  cd.back_image->setScene(scene2);
  cd.front_image->setScene(scene1);

  if(cd.format->findText(biblioteq::s_unknown) == -1)
    cd.format->addItem(biblioteq::s_unknown);

  if(cd.language->findText(biblioteq::s_unknown) == -1)
    cd.language->addItem(biblioteq::s_unknown);

  if(cd.location->findText(biblioteq::s_unknown) == -1)
    cd.location->addItem(biblioteq::s_unknown);

  if(cd.monetary_units->findText(biblioteq::s_unknown) == -1)
    cd.monetary_units->addItem(biblioteq::s_unknown);

#ifndef Q_OS_ANDROID
  if(m_parentWid)
    resize(qRound(0.95 * m_parentWid->size().width()),
	   qRound(0.95 * m_parentWid->size().height()));
#endif

#ifdef Q_OS_MACOS
  if(!biblioteq_misc_functions::isEnvironmentSet("QT_STYLE_OVERRIDE"))
    foreach(auto tool_button, findChildren<QToolButton *> ())
      {
#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
	tool_button->setStyleSheet
	  ("QToolButton {border: none; padding-right: 10px}"
	   "QToolButton::menu-button {border: none;}");
#else
	tool_button->setStyleSheet
	  ("QToolButton {border: none; padding-right: 15px}"
	   "QToolButton::menu-button {border: none; width: 15px;}");
#endif
      }
#endif

  cd.splitter->setStretchFactor(0, 0);
  cd.splitter->setStretchFactor(1, 1);
  cd.splitter->setStretchFactor(2, 0);
  biblioteq_misc_functions::center(this, m_parentWid);
  biblioteq_misc_functions::hideAdminFields(this, qmain->getRoles());
}

biblioteq_cd::~biblioteq_cd()
{
}

void biblioteq_cd::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  cd.retranslateUi(this);
	  trd.retranslateUi(m_tracks_diag);
	  break;
	}
      default:
	break;
      }

  QMainWindow::changeEvent(event);
}

void biblioteq_cd::closeEvent(QCloseEvent *event)
{
  if(qmain->isLocked())
    {
      if(event)
	event->ignore();

      return;
    }

  if(m_engWindowTitle.contains("Create") ||
     m_engWindowTitle.contains("Modify"))
    {
      QString key("");

      if(hasDataChanged(this, key))
	{
	  if(QMessageBox::
	     question(this,
		      tr("BiblioteQ: Question"),
		      tr("Your changes (%1) have not been saved. "
			 "Continue closing?").arg(key),
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
    }

  QMainWindow::closeEvent(event);
  qmain->removeCD(this);
}

void biblioteq_cd::duplicate(const QString &p_oid, const int state)
{
  modify(state); // Initial population.
  cd.copiesButton->setEnabled(false);
  cd.showUserButton->setEnabled(false);
  cd.tracksButton->setEnabled(false);
  m_engWindowTitle = "Create";
  m_oid = p_oid;
  setWindowTitle(tr("BiblioteQ: Duplicate Music CD Entry"));
  emit windowTitleChanged(windowTitle());
}

void biblioteq_cd::insert(void)
{
  slotReset();
  cd.accession_number->clear();
  cd.artist->setPlainText("N/A");
  cd.audio->setCurrentIndex(0);
  cd.category->setPlainText("N/A");
  cd.computeButton->setVisible(true);
  cd.copiesButton->setEnabled(false);
  cd.description->setPlainText("N/A");
  cd.format->setCurrentIndex(0);
  cd.id->clear();
  cd.keyword->clear();
  cd.language->setCurrentIndex(0);
  cd.location->setCurrentIndex(0);
  cd.monetary_units->setCurrentIndex(0);
  cd.no_of_discs->setMinimum(1);
  cd.no_of_discs->setValue(1);
  cd.okButton->setText(tr("&Save"));
  cd.price->setMinimum(0.00);
  cd.price->setValue(0.00);
  cd.quantity->setMinimum(1);
  cd.quantity->setValue(1);
  cd.queryButton->setEnabled(true);
  cd.recording_label->setPlainText("N/A");
  cd.recording_type->setCurrentIndex(0);
  cd.release_date->setDate
    (QDate::fromString("01/01/2000", biblioteq::s_databaseDateFormat));
  cd.runtime->setMinimumTime(QTime(0, 0, 1));
  cd.runtime->setTime(QTime(0, 0, 1));
  cd.showUserButton->setEnabled(false);
  cd.title->clear();
  cd.tracksButton->setEnabled(false);
  biblioteq_misc_functions::highlightWidget
    (cd.artist->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (cd.category->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (cd.description->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (cd.id, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (cd.recording_label->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (cd.title, m_requiredHighlightColor);
  cd.id->setFocus();
  m_engWindowTitle = "Create";
  prepareFavorites();
  setWindowTitle(tr("BiblioteQ: Create Music CD Entry"));
  emit windowTitleChanged(windowTitle());
  storeData(this);
#ifdef Q_OS_ANDROID
  showMaximized();
#else
  biblioteq_misc_functions::center(this, m_parentWid);
  showNormal();
#endif
  activateWindow();
  raise();
  prepareIcons(this);
}

void biblioteq_cd::modify(const int state)
{
  QSqlQuery query(qmain->getDB());
  QString fieldname = "";
  QString str = "";
  QVariant var;
  int i = 0;

  if(state == biblioteq::EDITABLE)
    {
      m_engWindowTitle = "Modify";
      setReadOnlyFields(this, false);
      setWindowTitle(tr("BiblioteQ: Modify Music CD Entry"));
      emit windowTitleChanged(windowTitle());
      cd.backButton->setVisible(true);
      cd.computeButton->setVisible(true);
      cd.copiesButton->setEnabled(true);
      cd.frontButton->setVisible(true);
      cd.okButton->setVisible(true);
      cd.queryButton->setVisible(m_isQueryEnabled);
      cd.resetButton->setVisible(true);
      cd.showUserButton->setEnabled(true);
      trd.deleteButton->setVisible(true);
      trd.insertButton->setVisible(true);
      trd.saveButton->setVisible(true);
      biblioteq_misc_functions::highlightWidget
	(cd.artist->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(cd.category->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(cd.description->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(cd.id, m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(cd.recording_label->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(cd.title, m_requiredHighlightColor);
    }
  else
    {
      m_engWindowTitle = "View";
      setReadOnlyFields(this, true);
      setWindowTitle(tr("BiblioteQ: View Music CD Details"));
      emit windowTitleChanged(windowTitle());
      cd.backButton->setVisible(false);
      cd.computeButton->setVisible(false);
      cd.copiesButton->setVisible(false);
      cd.frontButton->setVisible(false);
      cd.okButton->setVisible(false);
      cd.queryButton->setVisible(false);
      cd.resetButton->setVisible(false);
      cd.showUserButton->setVisible(qmain->isGuest() ? false : true);
      trd.deleteButton->setVisible(false);
      trd.insertButton->setVisible(false);
      trd.saveButton->setVisible(false);

      auto const actions = cd.resetButton->menu()->actions();

      if(actions.size() >= 2)
	{
	  actions[0]->setVisible(false);
	  actions[1]->setVisible(false);
	}
    }

  cd.no_of_discs->setMinimum(1);
  cd.no_of_discs->setValue(1);
  cd.okButton->setText(tr("&Save"));
  cd.price->setMinimum(0.00);
  cd.quantity->setMinimum(1);
  cd.quantity->setValue(1);
  cd.queryButton->setEnabled(true);
  cd.runtime->setMinimumTime(QTime(0, 0, 1));
  cd.tracksButton->setEnabled(true);
  prepareIcons(this);
  str = m_oid;
  query.prepare("SELECT id, "
		"title, "
		"cdformat, "
		"artist, "
		"cddiskcount, "
		"cdruntime, "
		"rdate, "
		"recording_label, "
		"category, "
		"price, "
		"language, "
		"monetary_units, "
		"description, "
		"quantity, "
		"cdaudio, "
		"cdrecording, "
		"location, "
		"front_cover, "
		"back_cover, "
		"keyword, "
		"accession_number "
		"FROM "
		"cd "
		"WHERE myoid = ?");
  query.bindValue(0, str);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec() || !query.next())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to retrieve the selected CD's data."),
	 query.lastError().text(),
	 __FILE__,
	 __LINE__);
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to retrieve the selected CD's data."));
      QApplication::processEvents();
      close();
      return;
    }
  else
    {
      QApplication::restoreOverrideCursor();
#ifdef Q_OS_ANDROID
      showMaximized();
#else
      biblioteq_misc_functions::center(this, m_parentWid);
      showNormal();
#endif
      activateWindow();
      raise();

      auto const record(query.record());

      for(i = 0; i < record.count(); i++)
	{
	  var = record.field(i).value();
	  fieldname = record.fieldName(i);

	  if(fieldname == "accession_number")
	    cd.accession_number->setText(var.toString());
	  else if(fieldname == "artist")
	    cd.artist->setMultipleLinks("cd_search", "artist", var.toString());
	  else if(fieldname == "back_cover")
	    {
	      if(!record.field(i).isNull())
		{
		  cd.back_image->loadFromData
		    (QByteArray::fromBase64(var.toByteArray()));

		  if(cd.back_image->m_image.isNull())
		    cd.back_image->loadFromData(var.toByteArray());
		}
	    }
	  else if(fieldname == "category")
	    cd.category->setMultipleLinks
	      ("cd_search", "category", var.toString());
	  else if(fieldname == "cdaudio")
	    {
	      if(cd.audio->findText(var.toString()) > -1)
		cd.audio->setCurrentIndex
		  (cd.audio->findText(var.toString()));
	      else
		cd.audio->setCurrentIndex(0);
	    }
	  else if(fieldname == "cddiskcount")
	    cd.no_of_discs->setValue(var.toInt());
	  else if(fieldname == "cdformat")
	    {
	      if(cd.format->findText(var.toString()) > -1)
		cd.format->setCurrentIndex
		  (cd.format->findText(var.toString()));
	      else
		cd.format->setCurrentIndex
		  (cd.format->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "cdrecording")
	    {
	      if(cd.recording_type->findText(var.toString()) > -1)
		cd.recording_type->setCurrentIndex
		  (cd.recording_type->findText(var.toString()));
	      else
		cd.recording_type->setCurrentIndex(0);
	    }
	  else if(fieldname == "cdruntime")
	    cd.runtime->setTime(QTime::fromString(var.toString(), "hh:mm:ss"));
	  else if(fieldname == "description")
	    cd.description->setPlainText(var.toString());
	  else if(fieldname == "front_cover")
	    {
	      if(!record.field(i).isNull())
		{
		  cd.front_image->loadFromData
		    (QByteArray::fromBase64(var.toByteArray()));

		  if(cd.front_image->m_image.isNull())
		    cd.front_image->loadFromData(var.toByteArray());
		}
	    }
	  else if(fieldname == "id")
	    {
	      if(state == biblioteq::EDITABLE)
		{
		  m_engWindowTitle = "Modify";
		  str = tr("BiblioteQ: Modify Music CD Entry (") +
		    var.toString() +
		    tr(")");
		}
	      else
		{
		  m_engWindowTitle = "View";
		  str = tr("BiblioteQ: View Music CD Details (") +
		    var.toString() +
		    tr(")");
		}

	      cd.id->setText(var.toString());
	      setWindowTitle(str);
	      emit windowTitleChanged(windowTitle());
	    }
	  else if(fieldname == "keyword")
	    cd.keyword->setMultipleLinks
	      ("cd_search", "keyword", var.toString());
	  else if(fieldname == "language")
	    {
	      if(cd.language->findText(var.toString()) > -1)
		cd.language->setCurrentIndex
		  (cd.language->findText(var.toString()));
	      else
		cd.language->setCurrentIndex
		  (cd.language->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "location")
	    {
	      if(cd.location->findText(var.toString()) > -1)
		cd.location->setCurrentIndex
		  (cd.location->findText(var.toString()));
	      else
		cd.location->setCurrentIndex
		  (cd.location->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "monetary_units")
	    {
	      if(cd.monetary_units->findText(var.toString()) > -1)
		cd.monetary_units->setCurrentIndex
		  (cd.monetary_units->findText(var.toString()));
	      else
		cd.monetary_units->setCurrentIndex
		  (cd.monetary_units->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "price")
	    cd.price->setValue(var.toDouble());
	  else if(fieldname == "quantity")
	    {
	      cd.quantity->setValue(var.toInt());
	      m_oldq = cd.quantity->value();
	    }
	  else if(fieldname == "rdate")
	    cd.release_date->setDate
	      (QDate::fromString(var.toString(),
				 biblioteq::s_databaseDateFormat));
	  else if(fieldname == "recording_label")
	    cd.recording_label->setMultipleLinks
	      ("cd_search", "recording_label", var.toString());
	  else if(fieldname == "title")
	    cd.title->setText(var.toString());
	}

      foreach(auto textfield, findChildren<QLineEdit *> ())
	textfield->setCursorPosition(0);

      storeData(this);
    }

  cd.id->setFocus();
  raise();
}

void biblioteq_cd::prepareFavorites(void)
{
  QSettings settings;

  cd.format->setCurrentIndex
    (cd.format->findText(settings.value("cd_formats_favorite").
			 toString().trimmed()));
  cd.language->setCurrentIndex
    (cd.language->
     findText(settings.value("languages_favorite").toString().trimmed()));
  cd.monetary_units->setCurrentIndex
    (cd.monetary_units->
     findText(settings.value("monetary_units_favorite").toString().trimmed()));

  if(cd.format->currentIndex() < 0)
    cd.format->setCurrentIndex(0);

  if(cd.language->currentIndex() < 0)
    cd.language->setCurrentIndex(0);

  if(cd.monetary_units->currentIndex() < 0)
    cd.monetary_units->setCurrentIndex(0);
}

void biblioteq_cd::search(const QString &field, const QString &value)
{
  cd.accession_number->clear();
  cd.artist->clear();
  cd.audio->insertItem(0, tr("Any"));
  cd.audio->setCurrentIndex(0);
  cd.category->clear();
  cd.composer->setVisible(true);
  cd.composer_label->setVisible(true);
  cd.computeButton->setVisible(false);
  cd.copiesButton->setVisible(false);
  cd.description->clear();
  cd.format->insertItem(0, tr("Any"));
  cd.format->setCurrentIndex(0);
  cd.id->clear();
  cd.keyword->clear();
  cd.language->insertItem(0, tr("Any"));
  cd.language->setCurrentIndex(0);
  cd.location->insertItem(0, tr("Any"));
  cd.location->setCurrentIndex(0);
  cd.monetary_units->insertItem(0, tr("Any"));
  cd.monetary_units->setCurrentIndex(0);
  cd.no_of_discs->setMinimum(0);
  cd.no_of_discs->setValue(0);
  cd.okButton->setText(tr("&Search"));
  cd.price->setMinimum(-0.01);
  cd.price->setValue(-0.01);
  cd.publication_date_enabled->setVisible(true);
  cd.quantity->setMinimum(0);
  cd.quantity->setValue(0);
  cd.queryButton->setVisible(false);
  cd.recording_label->clear();
  cd.recording_type->insertItem(0, tr("Any"));
  cd.recording_type->setCurrentIndex(0);
  cd.release_date->setDate(QDate::fromString("2001", "yyyy"));
  cd.release_date->setDisplayFormat("yyyy");
  cd.runtime->setMinimumTime(QTime(0, 0, 0));
  cd.runtime->setTime(QTime(0, 0, 0));
  cd.showUserButton->setVisible(false);
  cd.title->clear();
  cd.tracksButton->setVisible(false);
  cd.tracks_lbl->setVisible(false);
  m_composer_action->setVisible(true);
  m_engWindowTitle = "Search";
  prepareIcons(this);

  if(field.isEmpty() && value.isEmpty())
    {
      auto const actions = cd.resetButton->menu()->actions();

      if(actions.size() >= 2)
	{
	  actions[0]->setVisible(false);
	  actions[1]->setVisible(false);
	}

      cd.coverImages->setVisible(false);
      cd.id->setFocus();
      prepareFavorites();
      setWindowTitle(tr("BiblioteQ: Database Music CD Search"));
      emit windowTitleChanged(windowTitle());
#ifdef Q_OS_ANDROID
      showMaximized();
#else
      biblioteq_misc_functions::center(this, m_parentWid);
      showNormal();
#endif
      activateWindow();
      raise();
    }
  else
    {
      if(field == "artist")
	cd.artist->setPlainText(value);
      else if(field == "category")
	cd.category->setPlainText(value);
      else if(field == "keyword")
	cd.keyword->setPlainText(value);
      else if(field == "recording_label")
	cd.recording_label->setPlainText(value);

      slotGo();
    }
}

void biblioteq_cd::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach(auto widget, findChildren<QWidget *> ())
    {
      widget->setFont(font);
      widget->update();
    }

  trd.table->resizeRowsToContents();
  update();
}

void biblioteq_cd::slotCancel(void)
{
  close();
}

void biblioteq_cd::slotCloseTracksBrowser(void)
{
#ifdef Q_OS_ANDROID
  m_tracks_diag->hide();
#else
  m_tracks_diag->close();
#endif
  trd.table->setColumnCount(0);
  trd.table->setCurrentItem(nullptr);
  trd.table->setRowCount(0);
}

void biblioteq_cd::slotComputeRuntime(void)
{
  QSqlQuery query(qmain->getDB());
  QTime sum(0, 0, 0);
  QTime time(0, 0, 0);
  int count = 0;
  int secs = 0;

  query.prepare("SELECT runtime FROM cd_songs WHERE item_oid = ?");
  query.bindValue(0, m_oid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(query.exec())
    while(query.next())
      {
	count += 1;
	time = QTime::fromString(query.value(0).toString(), "hh:mm:ss");
	secs = time.hour() * 3600 + time.minute() * 60 + time.second();
	sum = sum.addSecs(secs);
      }

  QApplication::restoreOverrideCursor();

  if(count > 0)
    {
      if(sum.toString("hh:mm:ss") == "00:00:00")
	{
	  QMessageBox::critical
	    (this,
	     tr("BiblioteQ: User Error"),
	     tr("The total runtime of the available tracks is "
		"zero. Please set the individual runtimes."));
	  QApplication::processEvents();
	}
      else
	cd.runtime->setTime
	  (QTime::fromString(sum.toString("hh:mm:ss"), "hh:mm:ss"));
    }
  else
    cd.runtime->setTime(QTime::fromString("00:00:01"));
}

void biblioteq_cd::slotDatabaseEnumerationsCommitted(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QList<QComboBox *> widgets;

  widgets << cd.format
	  << cd.language
	  << cd.location
	  << cd.monetary_units;

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
	      (biblioteq_misc_functions::
	       getCDFormats(qmain->getDB(), errorstr));
	    break;
	  }
	case 1:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::
	       getLanguages(qmain->getDB(), errorstr));
	    break;
	  }
	case 2:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::
	       getLocations(qmain->getDB(), "CD", errorstr));
	    break;
	  }
	case 3:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::
	       getMonetaryUnits(qmain->getDB(), errorstr));
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

void biblioteq_cd::slotDeleteTrack(void)
{
  trd.table->removeRow(trd.table->currentRow());
}

void biblioteq_cd::slotGo(void)
{
  QString errorstr = "";
  QString searchstr = "";
  QString str = "";
  int i = 0;
  int maxcopynumber = 0;
  int newq = 0;

  if(m_engWindowTitle.contains("Create") ||
     m_engWindowTitle.contains("Modify"))
    {
      if(m_engWindowTitle.contains("Modify") && m_index->isValid())
	{
	  newq = cd.quantity->value();
	  QApplication::setOverrideCursor(Qt::WaitCursor);
	  maxcopynumber = biblioteq_misc_functions::getMaxCopyNumber
	    (qmain->getDB(), m_oid, "CD", errorstr);

	  if(maxcopynumber < 0)
	    {
	      QApplication::restoreOverrideCursor();
	      qmain->addError
		(tr("Database Error"),
		 tr("Unable to determine the maximum copy number of the item."),
		 errorstr,
		 __FILE__,
		 __LINE__);
	      QMessageBox::critical
		(this,
		 tr("BiblioteQ: Database Error"),
		 tr("Unable to determine the maximum copy number of "
		    "the item."));
	      QApplication::processEvents();
	      return;
	    }

	  QApplication::restoreOverrideCursor();

	  if(newq < maxcopynumber)
	    {
	      QMessageBox::critical
		(this,
		 tr("BiblioteQ: User Error"),
		 tr("It appears that you are attempting to decrease the "
		    "number of copies while there are copies "
		    "that have been reserved."));
	      QApplication::processEvents();
	      cd.quantity->setValue(m_oldq);
	      return;
	    }
	  else if(newq > m_oldq)
	    {
	      if(QMessageBox::question
		 (this,
		  tr("BiblioteQ: Question"),
		  tr("You have increased the number of copies. "
		     "Would you like to modify copy information?"),
		  QMessageBox::No | QMessageBox::Yes,
		  QMessageBox::No) == QMessageBox::Yes)
		{
		  QApplication::processEvents();
		  slotPopulateCopiesEditor();
		}

	      QApplication::processEvents();
	    }
	}

      str = cd.id->text().trimmed();
      cd.id->setText(str);

      if(cd.id->text().isEmpty())
	{
	  QMessageBox::critical
	    (this,
	     tr("BiblioteQ: User Error"),
	     tr("Please complete the Catalog Number field."));
	  QApplication::processEvents();
	  cd.id->setFocus();
	  return;
	}

      str = cd.artist->toPlainText().trimmed();
      cd.artist->setPlainText(str);

      if(cd.artist->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Artist field."));
	  QApplication::processEvents();
	  cd.artist->setFocus();
	  return;
	}

      if(cd.runtime->text() == "00:00:00")
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please provide a valid Runtime."));
	  QApplication::processEvents();
	  cd.runtime->setFocus();
	  return;
	}

      str = cd.title->text().trimmed();
      cd.title->setText(str);

      if(cd.title->text().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Title field."));
	  QApplication::processEvents();
	  cd.title->setFocus();
	  return;
	}

      str = cd.recording_label->toPlainText().trimmed();
      cd.recording_label->setPlainText(str);

      if(cd.recording_label->toPlainText().isEmpty())
	{
	  QMessageBox::
	    critical(this,
		     tr("BiblioteQ: User Error"),
		     tr("Please complete the Recording Label field."));
	  QApplication::processEvents();
	  cd.recording_label->setFocus();
	  return;
	}

      str = cd.category->toPlainText().trimmed();
      cd.category->setPlainText(str);

      if(cd.category->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Categories field."));
	  QApplication::processEvents();
	  cd.category->setFocus();
	  return;
	}

      str = cd.description->toPlainText().trimmed();
      cd.description->setPlainText(str);

      if(cd.description->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Abstract field."));
	  QApplication::processEvents();
	  cd.description->setFocus();
	  return;
	}

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

      QApplication::restoreOverrideCursor();
      str = cd.keyword->toPlainText().trimmed();
      cd.keyword->setPlainText(str);
      str = cd.accession_number->text().trimmed();
      cd.accession_number->setText(str);

      QSqlQuery query(qmain->getDB());

      if(m_engWindowTitle.contains("Modify"))
	query.prepare("UPDATE cd SET "
		      "id = ?, "
		      "title = ?, "
		      "cdformat = ?, "
		      "artist = ?, "
		      "cddiskcount = ?, "
		      "cdruntime = ?, "
		      "rdate = ?, "
		      "recording_label = ?, "
		      "category = ?, "
		      "price = ?, "
		      "language = ?, "
		      "monetary_units = ?, "
		      "description = ?, "
		      "quantity = ?, "
		      "location = ?, "
		      "cdrecording = ?, "
		      "cdaudio = ?, "
		      "front_cover = ?, "
		      "back_cover = ?, "
		      "keyword = ?, "
		      "accession_number = ? "
		      "WHERE "
		      "myoid = ?");
      else if(qmain->getDB().driverName() != "QSQLITE")
	query.prepare("INSERT INTO cd "
		      "(id, "
		      "title, "
		      "cdformat, "
		      "artist, "
		      "cddiskcount, "
		      "cdruntime, "
		      "rdate, "
		      "recording_label, "
		      "category, "
		      "price, "
		      "language, "
		      "monetary_units, "
		      "description, "
		      "quantity, "
		      "location, "
		      "cdrecording, "
		      "cdaudio, front_cover, "
		      "back_cover, keyword, accession_number) "
		      "VALUES "
		      "(?, ?, ?, ?, "
		      "?, ?, "
		      "?, ?, ?, "
		      "?, ?, ?, "
		      "?, "
		      "?, ?, ?, ?, ?, ?, ?, ?)");
      else
	query.prepare("INSERT INTO cd "
		      "(id, "
		      "title, "
		      "cdformat, "
		      "artist, "
		      "cddiskcount, "
		      "cdruntime, "
		      "rdate, "
		      "recording_label, "
		      "category, "
		      "price, "
		      "language, "
		      "monetary_units, "
		      "description, "
		      "quantity, "
		      "location, "
		      "cdrecording, "
		      "cdaudio, front_cover, "
		      "back_cover, "
		      "keyword, "
		      "accession_number, "
		      "myoid) "
		      "VALUES "
		      "(?, ?, ?, ?, "
		      "?, ?, "
		      "?, ?, "
		      "?, ?, ?, "
		      "?, ?, "
		      "?, ?, ?, ?, ?, ?, ?, ?, ?)");

      query.bindValue(0, cd.id->text());
      query.bindValue(1, cd.title->text());
      query.bindValue(2, cd.format->currentText().trimmed());
      query.bindValue(3, cd.artist->toPlainText());
      query.bindValue(4, cd.no_of_discs->text());
      query.bindValue(5, cd.runtime->text());
      query.bindValue
	(6, cd.release_date->date().toString(biblioteq::s_databaseDateFormat));
      query.bindValue(7, cd.recording_label->toPlainText());
      query.bindValue(8, cd.category->toPlainText());
      query.bindValue(9, cd.price->value());
      query.bindValue(10, cd.language->currentText().trimmed());
      query.bindValue(11, cd.monetary_units->currentText().trimmed());
      query.bindValue(12, cd.description->toPlainText());
      query.bindValue(13, cd.quantity->text());
      query.bindValue(14, cd.location->currentText().trimmed());
      query.bindValue(15, cd.recording_type->currentText().trimmed());
      query.bindValue(16, cd.audio->currentText().trimmed());

      if(!cd.front_image->m_image.isNull())
	{
	  QByteArray bytes;
	  QBuffer buffer(&bytes);

	  if(buffer.open(QIODevice::WriteOnly))
	    {
	      cd.front_image->m_image.save
		(&buffer, cd.front_image->m_imageFormat.toLatin1(), 100);
	      query.bindValue(17, bytes.toBase64());
	    }
	  else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	    query.bindValue(17, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	    query.bindValue(17, QVariant(QVariant::ByteArray));
#endif
	}
      else
	{
	  cd.front_image->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	  query.bindValue(17, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	  query.bindValue(17, QVariant(QVariant::ByteArray));
#endif
	}

      if(!cd.back_image->m_image.isNull())
	{
	  QByteArray bytes;
	  QBuffer buffer(&bytes);

	  if(buffer.open(QIODevice::WriteOnly))
	    {
	      cd.back_image->m_image.save
		(&buffer, cd.back_image->m_imageFormat.toLatin1(), 100);
	      query.bindValue(18, bytes.toBase64());
	    }
	  else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	    query.bindValue(18, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	    query.bindValue(18, QVariant(QVariant::ByteArray));
#endif
	}
      else
	{
	  cd.back_image->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	  query.bindValue(18, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	  query.bindValue(18, QVariant(QVariant::ByteArray));
#endif
	}

      query.bindValue(19, cd.keyword->toPlainText().trimmed());
      query.bindValue(20, cd.accession_number->text().trimmed());

      if(m_engWindowTitle.contains("Modify"))
	query.bindValue(21, m_oid);
      else if(qmain->getDB().driverName() == "QSQLITE")
	{
	  auto const value = biblioteq_misc_functions::getSqliteUniqueId
	    (qmain->getDB(), errorstr);

	  if(errorstr.isEmpty())
	    query.bindValue(21, value);
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
	     tr("Unable to create or update the entry."),
	     query.lastError().text(),
	     __FILE__,
	     __LINE__);
	  goto db_rollback;
	}
      else
	{
	  /*
	  ** Remove copies if the quantity has been decreased.
	  */

	  if(m_engWindowTitle.contains("Modify"))
	    {
	      /*
	      ** Retain quantity copies.
	      */

	      query.prepare("DELETE FROM cd_copy_info WHERE "
			    "myoid NOT IN "
			    "(SELECT myoid FROM cd_copy_info "
			    "WHERE item_oid = ? ORDER BY copy_number "
			    "LIMIT ?) AND item_oid = ?");
	      query.addBindValue(m_oid);
	      query.addBindValue(cd.quantity->text());
	      query.addBindValue(m_oid);

	      if(!query.exec())
		{
		  QApplication::restoreOverrideCursor();
		  qmain->addError
		    (tr("Database Error"),
		     tr("Unable to purge unnecessary copy data."),
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
	    }
	  else
	    {
	      /*
	      ** Create initial copies.
	      */

	      biblioteq_misc_functions::createInitialCopies
		(cd.id->text(),
		 cd.quantity->value(),
		 qmain->getDB(),
		 "CD", errorstr);

	      if(!errorstr.isEmpty())
		{
		  QApplication::restoreOverrideCursor();
		  qmain->addError
		    (tr("Database Error"),
		     tr("Unable to create initial copies."),
		     errorstr,
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
	    }

	  m_oldq = cd.quantity->value();

	  if(cd.back_image->m_image.isNull())
	    cd.back_image->m_imageFormat = "";

	  if(cd.front_image->m_image.isNull())
	    cd.front_image->m_imageFormat = "";

	  cd.artist->setMultipleLinks
	    ("cd_search", "artist", cd.artist->toPlainText());
	  cd.category->setMultipleLinks
	    ("cd_search", "category", cd.category->toPlainText());
	  cd.keyword->setMultipleLinks
	    ("cd_search", "keyword", cd.keyword->toPlainText());
	  cd.recording_label->setMultipleLinks
	    ("cd_search",
	     "recording_label",
	     cd.recording_label->toPlainText());
	  QApplication::restoreOverrideCursor();

	  if(m_engWindowTitle.contains("Modify"))
	    {
	      m_engWindowTitle = "Modify";
	      str = tr("BiblioteQ: Modify Music CD Entry (") +
		cd.id->text() +
		tr(")");
	      setWindowTitle(str);
	      emit windowTitleChanged(windowTitle());

	      if(m_index->isValid() &&
		 (qmain->getTypeFilterString() == "All" ||
		  qmain->getTypeFilterString() == "All Available" ||
		  qmain->getTypeFilterString() == "All Overdue" ||
		  qmain->getTypeFilterString() == "All Requested" ||
		  qmain->getTypeFilterString() == "All Reserved" ||
		  qmain->getTypeFilterString() == "Music CDs"))
		{
		  qmain->getUI().table->setSortingEnabled(false);

		  auto const names(qmain->getUI().table->columnNames());

		  for(i = 0; i < names.size(); i++)
		    {
		      if(i == 0 && qmain->showMainTableImages())
			{
			  auto const pixmap
			    (QPixmap::fromImage(cd.front_image->m_image));

			  if(!pixmap.isNull())
			    qmain->getUI().table->item(m_index->row(), i)->
			      setIcon(pixmap);
			  else
			    qmain->getUI().table->item(m_index->row(), i)->
			      setIcon(QIcon(":/missing_image.png"));
			}

		      if(names.at(i) == "Accession Number")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.accession_number->text());
		      else if(names.at(i) == "Artist")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.artist->toPlainText());
		      else if(names.at(i) == "Audio")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.audio->currentText().trimmed());
		      else if(names.at(i) == "Availability")
			{
			  qmain->getUI().table->item(m_index->row(), i)->setText
			    (biblioteq_misc_functions::getAvailability
			     (m_oid, qmain->getDB(), "CD", errorstr));

			  if(!errorstr.isEmpty())
			    qmain->addError
			      (tr("Database Error"),
			       tr("Retrieving availability."),
			       errorstr,
			       __FILE__,
			       __LINE__);
			}
		      else if(names.at(i) == "Catalog Number" ||
			      names.at(i) == "ID Number")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.id->text());
		      else if(names.at(i) == "Categories")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.category->toPlainText());
		      else if(names.at(i) == "Format")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.format->currentText().trimmed());
		      else if(names.at(i) == "Language")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.language->currentText().trimmed());
		      else if(names.at(i) == "Location")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.location->currentText().trimmed());
		      else if(names.at(i) == "Monetary Units")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.monetary_units->currentText().trimmed());
		      else if(names.at(i) == "Number of Discs")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.no_of_discs->text());
		      else if(names.at(i) == "Price")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (QLocale().toString(cd.price->value()));
		      else if(names.at(i) == "Publication Date" ||
			      names.at(i) == "Release Date")
			{
			  if(qmain->getTypeFilterString() == "Music CDs")
			    qmain->getUI().table->item(m_index->row(), i)->
			      setText
			      (cd.release_date->date().
			       toString(qmain->
					publicationDateFormat("musiccds")));
			  else
			    qmain->getUI().table->item(m_index->row(), i)->
			      setText
			      (cd.release_date->date().toString(Qt::ISODate));
			}
		      else if(names.at(i) == "Publisher" ||
			      names.at(i) == "Recording Label")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.recording_label->toPlainText());
		      else if(names.at(i) == "Quantity")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.quantity->text());
		      else if(names.at(i) == "Recording Type")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.recording_type->currentText().trimmed());
		      else if(names.at(i) == "Runtime")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.runtime->text());
		      else if(names.at(i) == "Title")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (cd.title->text());
		    }

		  qmain->getUI().table->setSortingEnabled(true);
		  qmain->getUI().table->updateToolTips(m_index->row());

		  foreach(auto textfield, findChildren<QLineEdit *> ())
		    textfield->setCursorPosition(0);

		  qmain->slotResizeColumns();
		}

	      qmain->slotDisplaySummary();
	      qmain->updateSceneItem(m_oid, "CD", cd.front_image->m_image);
	    }
	  else
	    {
	      QApplication::setOverrideCursor(Qt::WaitCursor);
	      m_oid = biblioteq_misc_functions::getOID(cd.id->text(),
						       "CD",
						       qmain->getDB(),
						       errorstr);
	      QApplication::restoreOverrideCursor();

	      if(!errorstr.isEmpty())
		{
		  qmain->addError
		    (tr("Database Error"),
		     tr("Unable to retrieve the CD's OID."),
		     errorstr,
		     __FILE__,
		     __LINE__);
		  QMessageBox::critical
		    (this,
		     tr("BiblioteQ: Database Error"),
		     tr("Unable to retrieve the CD's OID."));
		  QApplication::processEvents();
		}
	      else
		qmain->replaceCD(m_oid, this);

	      updateWindow(biblioteq::EDITABLE);

	      if(qmain->getUI().actionAutoPopulateOnCreation->isChecked())
		(void) qmain->populateTable
		  (biblioteq::POPULATE_ALL, "Music CDs", "");

	      raise();
	    }

	  storeData(this);
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
      QMessageBox::critical
	(this,
	 tr("BiblioteQ: Database Error"),
	 tr("Unable to create or update the entry. Please verify that "
	    "the entry does not already exist."));
      QApplication::processEvents();
    }
  else if(m_engWindowTitle.contains("Search"))
    {
      QString frontCover("'' AS front_cover ");

      if(qmain->showMainTableImages())
	frontCover = "cd.front_cover ";

      searchstr = "SELECT DISTINCT cd.title, "
	"cd.artist, "
	"cd.cdformat, "
	"cd.recording_label, "
	"cd.rdate, "
	"cd.cddiskcount, "
	"cd.cdruntime, "
	"cd.category, "
	"cd.language, "
	"cd.id, "
	"cd.price, "
	"cd.monetary_units, "
	"cd.quantity, "
	"cd.location, "
	"cd.cdaudio, "
	"cd.cdrecording, "
	"cd.quantity - COUNT(item_borrower.item_oid) AS availability, "
	"COUNT(item_borrower.item_oid) AS total_reserved, "
	"cd.accession_number, "
	"cd.type, "
	"cd.myoid, " +
	frontCover +
	"FROM "
	"cd LEFT JOIN item_borrower ON "
	"cd.myoid = item_borrower.item_oid "
	"AND item_borrower.type = 'CD' "
	"WHERE ";
      searchstr.append
	("LOWER(id) LIKE LOWER('%' || ? || '%') AND ");

      QString ESCAPE("");
      auto const UNACCENT(qmain->unaccent());

      if(qmain->getDB().driverName() != "QSQLITE")
	ESCAPE = "E";

      if(cd.format->currentIndex() != 0)
	searchstr.append
	  (UNACCENT + "(cdformat) = " + UNACCENT + "(?) AND ");

      searchstr.append
	("(" + UNACCENT + "(LOWER(cd.artist)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) OR ");
      searchstr.append
	("(cd.myoid IN (SELECT cd_songs.item_oid FROM cd_songs WHERE "
	 "cd_songs.item_oid = cd.myoid AND (");
      searchstr.append
	(UNACCENT + "(LOWER(cd_songs.artist)) LIKE " + UNACCENT + "(LOWER(" +
	 ESCAPE + "'%' || ? || '%')) OR ");
      searchstr.append
	(UNACCENT + "(LOWER(cd_songs.composer)) LIKE " + UNACCENT + "(LOWER(" +
	 ESCAPE + "'%' || ? || '%')))");
      searchstr.append("))) AND ");

      if(cd.no_of_discs->value() > 0)
	searchstr.append("cddiskcount = ").append
	  (QString::number(cd.no_of_discs->value())).append(" AND ");

      if(cd.runtime->text() != "00:00:00")
	searchstr.append("cdruntime = '" + cd.runtime->text() + "' AND ");

      if(cd.audio->currentIndex() != 0)
	searchstr.append("cdaudio = " + UNACCENT + "(?) AND ");

      if(cd.recording_type->currentIndex() != 0)
	searchstr.append("cdrecording = " + UNACCENT + "(?) AND ");

      searchstr.append
	(UNACCENT + "(LOWER(title)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

      if(cd.publication_date_enabled->isChecked())
	searchstr.append("SUBSTR(rdate, 7) = '" +
			 cd.release_date->date().toString("yyyy") +"' AND ");

      searchstr.append
	(UNACCENT + "(LOWER(recording_label)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");
      searchstr.append
	(UNACCENT + "(LOWER(category)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

      if(cd.price->value() > -0.01)
	{
	  searchstr.append("price = ");
	  searchstr.append(QString::number(cd.price->value()));
	  searchstr.append(" AND ");
	}

      if(cd.language->currentIndex() != 0)
	searchstr.append(UNACCENT + "(language) = " + UNACCENT + "(?) AND ");

      if(cd.monetary_units->currentIndex() != 0)
	searchstr.append
	  (UNACCENT + "(monetary_units) = " + UNACCENT + "(?) AND ");

      searchstr.append
	(UNACCENT + "(LOWER(description)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) ");
      searchstr.append
	("AND " + UNACCENT + "(LOWER(COALESCE(keyword, ''))) LIKE " +
	 UNACCENT + "(LOWER(" + ESCAPE + "'%' || ? || '%')) ");

      if(cd.quantity->value() != 0)
	searchstr.append
	  (" AND quantity = " + QString::number(cd.quantity->value()));

      if(cd.location->currentIndex() != 0)
	searchstr.append
	  (" AND " + UNACCENT + "(location) = " + UNACCENT + "(?) ");

      searchstr.append
	(" AND " + UNACCENT +
	 "(LOWER(COALESCE(accession_number, ''))) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) ");
      searchstr.append("GROUP BY "
		       "cd.title, "
		       "cd.artist, "
		       "cd.cdformat, "
		       "cd.recording_label, "
		       "cd.rdate, "
		       "cd.cddiskcount, "
		       "cd.cdruntime, "
		       "cd.category, "
		       "cd.language, "
		       "cd.id, "
		       "cd.price, "
		       "cd.monetary_units, "
		       "cd.quantity, "
		       "cd.location, "
		       "cd.cdaudio, "
		       "cd.cdrecording, "
		       "cd.accession_number, "
		       "cd.type, "
		       "cd.myoid, "
		       "cd.front_cover");

      auto query = new QSqlQuery(qmain->getDB());

      query->prepare(searchstr);
      query->addBindValue(cd.id->text().trimmed());

      if(cd.format->currentIndex() != 0)
	query->addBindValue
	  (biblioteq_myqstring::escape(cd.format->currentText().trimmed()));

      query->addBindValue
	(biblioteq_myqstring::escape(cd.artist->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(cd.artist->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(cd.composer->toPlainText().trimmed()));

      if(cd.audio->currentIndex() != 0)
	query->addBindValue(cd.audio->currentText().trimmed());

      if(cd.recording_type->currentIndex() != 0)
	query->addBindValue(cd.recording_type->currentText().trimmed());

      query->addBindValue
	(biblioteq_myqstring::escape(cd.title->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::
	 escape(cd.recording_label->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(cd.category->toPlainText().trimmed()));

      if(cd.language->currentIndex() != 0)
	query->addBindValue
	  (biblioteq_myqstring::escape(cd.language->currentText().trimmed()));

      if(cd.monetary_units->currentIndex() != 0)
	query->addBindValue
	  (biblioteq_myqstring::
	   escape(cd.monetary_units->currentText().trimmed()));

      query->addBindValue
	(biblioteq_myqstring::escape(cd.description->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(cd.keyword->toPlainText().trimmed()));

      if(cd.location->currentIndex() != 0)
	query->addBindValue
	  (biblioteq_myqstring::escape(cd.location->currentText().trimmed()));

      query->addBindValue
	(biblioteq_myqstring::escape(cd.accession_number->text().trimmed()));
      (void) qmain->populateTable
	(query, "Music CDs", biblioteq::NEW_PAGE, biblioteq::POPULATE_SEARCH);
    }
}

void biblioteq_cd::slotInsertTrack(void)
{
  QComboBox *comboBox = nullptr;
  QSpinBox *trackEdit = nullptr;
  QString str = "";
  QStringList list;
  QTableWidgetItem *item = nullptr;
  QTimeEdit *timeEdit = nullptr;
  auto trow = trd.table->currentRow();
  int i = 0;

  if(trow < 0)
    trow = trd.table->rowCount();
  else
    trow += 1;

  trd.table->insertRow(trow);
  trd.table->setSortingEnabled(false);

  for(i = 1; i <= cd.no_of_discs->value(); i++)
    list.append(QString::number(i));

  for(i = 0; i < trd.table->columnCount(); i++)
    {
      if(i == static_cast<int> (TracksColumns::ARTIST) ||
	 i == static_cast<int> (TracksColumns::COMPOSER))
	str = biblioteq::s_unknown;
      else if(i == static_cast<int> (TracksColumns::TRACK_NUMBER))
	str = "1";
      else if(i == static_cast<int> (TracksColumns::TRACK_TITLE))
	str = tr("Title");
      else
	str.clear();

      if(i == static_cast<int> (TracksColumns::ALBUM_NUMBER))
	{
	  auto layout = new QHBoxLayout();
	  auto spacer = new QSpacerItem
	    (40, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
	  auto widget = new QWidget();

	  comboBox = new QComboBox();
	  comboBox->addItems(list);
	  comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	  comboBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	  layout->addWidget(comboBox);
	  layout->addSpacerItem(spacer);
	  layout->setContentsMargins(0, 0, 0, 0);
	  widget->setLayout(layout);
	  trd.table->setCellWidget(trow, i, widget);
	}
      else if(i == static_cast<int> (TracksColumns::TRACK_NUMBER))
	{
	  trackEdit = new QSpinBox();
	  trd.table->setCellWidget(trow, i, trackEdit);
	  trackEdit->setMinimum(1);
	  trackEdit->setValue(trd.table->rowCount());
	}
      else if(i == static_cast<int> (TracksColumns::TRACK_RUNTIME))
	{
	  timeEdit = new QTimeEdit();
	  trd.table->setCellWidget(trow, i, timeEdit);
	  timeEdit->setDisplayFormat("hh:mm:ss");
	}
      else
	{
	  item = new QTableWidgetItem();
	  item->setFlags(Qt::ItemIsSelectable |
			 Qt::ItemIsEnabled |
			 Qt::ItemIsEditable);
	  item->setText(str);
	  trd.table->setItem(trow, i, item);
	}
    }

  trd.table->setSortingEnabled(false);

  for(int i = 0; i < trd.table->columnCount() - 1; i++)
    trd.table->resizeColumnToContents(i);

  trd.table->resizeRowsToContents();
}

void biblioteq_cd::slotPopulateCopiesEditor(void)
{
  auto copyeditor = new biblioteq_copy_editor
    (qobject_cast<QWidget *> (this),
     qmain,
     static_cast<biblioteq_item *> (this),
     false,
     cd.quantity->value(),
     m_oid,
     cd.quantity,
     font(),
     "CD",
     cd.id->text().trimmed(),
     false);

  copyeditor->populateCopiesEditor();
}

void biblioteq_cd::slotPopulateTracksBrowser(void)
{
  QComboBox *comboBox = nullptr;
  QProgressDialog progress(m_tracks_diag);
  QSpinBox *trackEdit = nullptr;
  QSqlQuery query(qmain->getDB());
  QString str = "";
  QStringList comboBoxList;
  QStringList list;
  QTableWidgetItem *item = nullptr;
  QTimeEdit *timeEdit = nullptr;
  int i = -1;
  int j = 0;

  query.prepare("SELECT albumnum, songnum, songtitle, runtime, "
		"artist, composer "
		"FROM cd_songs WHERE item_oid = ? "
		"ORDER BY albumnum, songnum, songtitle, runtime, "
		"artist, composer");
  query.bindValue(0, m_oid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError
	(tr("Database Error"),
	 tr("Unable to retrieve track data for table populating."),
	 query.lastError().text(),
	 __FILE__,
	 __LINE__);
      QMessageBox::critical
	(this,
	 tr("BiblioteQ: Database Error"),
	 tr("Unable to retrieve track data for table populating."));
      QApplication::processEvents();
      return;
    }

  QApplication::restoreOverrideCursor();

  for(i = 1; i <= cd.no_of_discs->value(); i++)
    comboBoxList.append(QString::number(i));

  trd.table->setColumnCount(0);
  trd.table->setCurrentItem(nullptr);
  trd.table->setRowCount(0);
  list.append(tr("Album Number"));
  list.append(tr("Track Number"));
  list.append(tr("Track Title"));
  list.append(tr("Track Runtime"));
  list.append(tr("Artist"));
  list.append(tr("Composer"));
  trd.table->horizontalScrollBar()->setValue(0);
  trd.table->scrollToTop();
  trd.table->setColumnCount(list.size());
  trd.table->setHorizontalHeaderLabels(list);
  trd.table->setRowCount(0);
  m_tracks_diag->setWindowTitle
    (tr("BiblioteQ: Album Tracks Browser (") + cd.id->text() + tr(")"));
  m_tracks_diag->updateGeometry();
#ifdef Q_OS_ANDROID
  m_tracks_diag->showMaximized();
#else
  m_tracks_diag->show();
#endif
  trd.table->setSortingEnabled(false);

  if(qmain->getDB().driverName() != "QSQLITE")
    trd.table->setRowCount(query.size());

  progress.setLabelText(tr("Populating the table..."));

  if(qmain->getDB().driverName() == "QSQLITE")
    {
      if(query.lastError().isValid())
	progress.setMaximum(0);
      else
	progress.setMaximum
	  (biblioteq_misc_functions::sqliteQuerySize(query.lastQuery(),
						     query.boundValues(),
						     qmain->getDB(),
						     __FILE__,
						     __LINE__,
						     qmain));
    }
  else
    progress.setMaximum(query.size());

  progress.setMinimum(0);
  progress.setModal(true);
  progress.setMinimumWidth
    (qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	   progress.sizeHint().width()));
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  progress.show();
  progress.repaint();
  QApplication::processEvents();
  i = -1;

  while(i++, !progress.wasCanceled() && query.next())
    {
      if(query.isValid())
	{
	  auto const record(query.record());

	  for(j = 0; j < record.count(); j++)
	    {
	      str = query.value(j).toString();

	      if(qmain->getDB().driverName() == "QSQLITE")
		trd.table->setRowCount(i + 1);

	      if(j == static_cast<int> (TracksColumns::ALBUM_NUMBER))
		{
		  auto layout = new QHBoxLayout();
		  auto spacer = new QSpacerItem
		    (40, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
		  auto widget = new QWidget();

		  comboBox = new QComboBox();
		  comboBox->addItems(comboBoxList);
		  comboBox->setCurrentIndex(comboBox->findText(str));

		  if(comboBox->currentIndex() < 0)
		    comboBox->setCurrentIndex(0);

		  comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
		  comboBox->setSizePolicy
		    (QSizePolicy::Preferred, QSizePolicy::Minimum);
		  layout->addWidget(comboBox);
		  layout->addSpacerItem(spacer);
		  layout->setContentsMargins(0, 0, 0, 0);
		  widget->setLayout(layout);
		  trd.table->setCellWidget(i, j, widget);
		}
	      else if(j == static_cast<int> (TracksColumns::TRACK_NUMBER))
		{
		  trackEdit = new QSpinBox();
		  trackEdit->setMinimum(1);
		  trackEdit->setValue(str.toInt());
		  trd.table->setCellWidget(i, j, trackEdit);
		}
	      else if(j == static_cast<int> (TracksColumns::TRACK_RUNTIME))
		{
		  timeEdit = new QTimeEdit();
		  timeEdit->setDisplayFormat("hh:mm:ss");
		  timeEdit->setTime(QTime::fromString(str, "hh:mm:ss"));
		  trd.table->setCellWidget(i, j, timeEdit);
		}
	      else
		{
		  item = new QTableWidgetItem();
		  item->setFlags(Qt::ItemIsSelectable |
				 Qt::ItemIsEnabled |
				 Qt::ItemIsEditable);
		  item->setText(str);
		  trd.table->setItem(i, j, item);
		}
	    }
	}

      if(i + 1 <= progress.maximum())
	progress.setValue(i + 1);

      progress.repaint();
      QApplication::processEvents();
    }

  progress.close();
  trd.table->setSortingEnabled(false);
  trd.table->setRowCount(i); // Support cancellation.

  for(int i = 0; i < trd.table->columnCount() - 1; i++)
    trd.table->resizeColumnToContents(i);

  trd.table->resizeRowsToContents();
}

void biblioteq_cd::slotPrepareIcons(void)
{
  prepareIcons(this);
}

void biblioteq_cd::slotPrint(void)
{
  m_html = "<html>";
  m_html += "<b>" +
    tr("Catalog Number:") +
    "</b> " +
    cd.id->text().trimmed() +
    "<br>";
  m_html += "<b>" + tr("Format:") + "</b> " + cd.format->currentText() + "<br>";
  m_html += "<b>" +
    tr("Artist:") +
    "</b> " +
    cd.artist->toPlainText().trimmed() +
    "<br>";
  m_html += "<b>" +
    tr("Number of Discs:") +
    "</b> " +
    cd.no_of_discs->text() +
    "<br>";
  m_html += "<b>" + tr("Runtime:") + "</b> " + cd.runtime->text() + "<br>";
  m_html += "<b>" + tr("Audio:") + "</b> " + cd.audio->currentText() + "<br>";
  m_html += "<b>" +
    tr("Recording Type:") +
    "</b> " +
    cd.recording_type->currentText() + "<br>";

  /*
  ** General information.
  */

  m_html += "<b>" +
    tr("Title:") +
    "</b> " +
    cd.title->text().trimmed() +
    "<br>";
  m_html += "<b>" +
    tr("Release Date:") +
    "</b> " +
    cd.release_date->date().toString(Qt::ISODate) +
    "<br>";
  m_html += "<b>" +
    tr("Recording Label:") +
    "</b> " +
    cd.recording_label->toPlainText().trimmed() +
    "<br>";
  m_html += "<b>" +
    tr("Categories:") +
    "</b> " +
    cd.category->toPlainText().trimmed() +
    "<br>";
  m_html += "<b>"+ tr("Price:") + "</b> " + cd.price->cleanText() + "<br>";
  m_html += "<b>" +
    tr("Language:") +
    "</b> " +
    cd.language->currentText() +
    "<br>";
  m_html += "<b>" +
    tr("Monetary Units:") +
    "</b> " +
    cd.monetary_units->currentText() +
    "<br>";
  m_html += "<b>" + tr("Copies:") + "</b> " + cd.quantity->text() + "<br>";
  m_html += "<b>" +
    tr("Location:") +
    "</b> " +
    cd.location->currentText() +
    "<br>";
  m_html += "<b>" +
    tr("Abstract:") +
    "</b> " +
    cd.description->toPlainText().trimmed() +
    "<br>";
  m_html += "<b>" +
    tr("Keywords:") +
    "</b> " +
    cd.keyword->toPlainText().trimmed() +
    "<br>";
  m_html += "<b>" +
    tr("Accession Number:") +
    "</b> " +
    cd.accession_number->text().trimmed();
  m_html += "</html>";
  print(this);
}

void biblioteq_cd::slotPublicationDateEnabled(bool state)
{
  cd.release_date->setEnabled(state);

  if(!state)
    cd.release_date->setDate(QDate::fromString("2001", "yyyy"));
}

void biblioteq_cd::slotQuery(void)
{
}

void biblioteq_cd::slotReset(void)
{
  auto action = qobject_cast<QAction *> (sender());

  if(action != nullptr)
    {
      auto const actions = cd.resetButton->menu()->actions();

      if(actions.size() < 22)
	{
	  // Error.
	}
      else if(action == actions[0])
	cd.front_image->clear();
      else if(action == actions[1])
	cd.back_image->clear();
      else if(action == actions[2])
	{
	  cd.id->clear();
	  cd.id->setFocus();
	}
      else if(action == actions[3])
	{
	  cd.format->setCurrentIndex(0);
	  cd.format->setFocus();
	}
      else if(action == actions[4])
	{
	  if(m_engWindowTitle.contains("Search"))
	    cd.artist->clear();
	  else
	    cd.artist->setPlainText("N/A");

	  cd.artist->setFocus();
	}
      else if(action == actions[5])
	{
	  cd.composer->clear();
	  cd.composer->setFocus();
	}
      else if(action == actions[6])
	{
	  cd.no_of_discs->setFocus();
	  cd.no_of_discs->setValue(cd.no_of_discs->minimum());
	}
      else if(action == actions[7])
	{
	  cd.runtime->setFocus();

	  if(m_engWindowTitle.contains("Search"))
	    cd.runtime->setTime(QTime(0, 0, 0));
	  else
	    cd.runtime->setTime(QTime(0, 0, 1));
	}
      else if(action == actions[8])
	{
	  cd.audio->setCurrentIndex(0);
	  cd.audio->setFocus();
	}
      else if(action == actions[9])
	{
	  cd.recording_type->setCurrentIndex(0);
	  cd.recording_type->setFocus();
	}
      else if(action == actions[10])
	{
	  cd.title->clear();
	  cd.title->setFocus();
	}
      else if(action == actions[11])
	{
	  if(m_engWindowTitle.contains("Search"))
	    {
	      cd.publication_date_enabled->setChecked(false);
	      cd.release_date->setDate(QDate::fromString("2001", "yyyy"));
	    }
	  else
	    cd.release_date->setDate
	      (QDate::fromString("01/01/2000",
				 biblioteq::s_databaseDateFormat));

	  cd.release_date->setFocus();
	}
      else if(action == actions[12])
	{
	  if(m_engWindowTitle.contains("Search"))
	    cd.recording_label->clear();
	  else
	    cd.recording_label->setPlainText("N/A");

	  cd.recording_label->setFocus();
	}
      else if(action == actions[13])
	{
	  if(m_engWindowTitle.contains("Search"))
	    cd.category->clear();
	  else
	    cd.category->setPlainText("N/A");

	  cd.category->setFocus();
	}
      else if(action == actions[14])
	{
	  cd.price->setFocus();
	  cd.price->setValue(cd.price->minimum());
	}
      else if(action == actions[15])
	{
	  cd.language->setCurrentIndex(0);
	  cd.language->setFocus();
	}
      else if(action == actions[16])
	{
	  cd.monetary_units->setCurrentIndex(0);
	  cd.monetary_units->setFocus();
	}
      else if(action == actions[17])
	{
	  cd.quantity->setFocus();
	  cd.quantity->setValue(cd.quantity->minimum());
	}
      else if(action == actions[18])
	{
	  cd.location->setCurrentIndex(0);
	  cd.location->setFocus();
	}
      else if(action == actions[19])
	{
	  if(m_engWindowTitle.contains("Search"))
	    cd.description->clear();
	  else
	    cd.description->setPlainText("N/A");

	  cd.description->setFocus();
	}
      else if(action == actions[20])
	{
	  cd.keyword->clear();
	  cd.keyword->setFocus();
	}
      else if(action == actions[21])
	{
	  cd.accession_number->clear();
	  cd.accession_number->setFocus();
	}
    }
  else
    {
      /*
      ** Reset all.
      */

      cd.title->clear();

      if(m_engWindowTitle.contains("Search"))
	cd.artist->clear();
      else
	cd.artist->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	cd.category->clear();
      else
	cd.category->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	cd.description->clear();
      else
	cd.description->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	cd.recording_label->clear();
      else
	cd.recording_label->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	{
	  cd.publication_date_enabled->setChecked(false);
	  cd.release_date->setDate(QDate::fromString("2001", "yyyy"));
	  cd.runtime->setTime(QTime(0, 0, 0));
	}
      else
	{
	  cd.release_date->setDate
	    (QDate::fromString("01/01/2000",
			       biblioteq::s_databaseDateFormat));
	  cd.runtime->setTime(QTime(0, 0, 1));
	}

      cd.accession_number->clear();
      cd.audio->setCurrentIndex(0);
      cd.back_image->clear();
      cd.composer->clear();
      cd.format->setCurrentIndex(0);
      cd.front_image->clear();
      cd.id->clear();
      cd.id->setFocus();
      cd.keyword->clear();
      cd.language->setCurrentIndex(0);
      cd.location->setCurrentIndex(0);
      cd.monetary_units->setCurrentIndex(0);
      cd.no_of_discs->setValue(cd.no_of_discs->minimum());
      cd.price->setValue(cd.price->minimum());
      cd.quantity->setValue(cd.quantity->minimum());
      cd.recording_type->setCurrentIndex(0);
    }
}

void biblioteq_cd::slotSaveTracks(void)
{
  QProgressDialog progress(this);
  QSqlQuery query(qmain->getDB());
  QString errormsg = "";
  QString lastError = "";
  int i = 0;

  for(i = 0; i < trd.table->rowCount(); i++)
    if(trd.table->item(i, static_cast<int> (TracksColumns::TRACK_TITLE)) !=
       nullptr &&
       trd.table->item(i, static_cast<int> (TracksColumns::TRACK_TITLE))->
       text().trimmed().isEmpty())
      {
	errormsg = tr("Row number ") + QString::number(i + 1) +
	  tr(" contains an empty Song Title.");
	QMessageBox::critical(m_tracks_diag,
			      tr("BiblioteQ: User Error"),
			      errormsg);
	QApplication::processEvents();
	return;
      }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!qmain->getDB().transaction())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to create a database transaction."),
		      qmain->getDB().lastError().text(),
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(m_tracks_diag,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to create a database transaction."));
      QApplication::processEvents();
      return;
    }

  QApplication::restoreOverrideCursor();
  query.prepare("DELETE FROM cd_songs WHERE item_oid = ?");
  query.bindValue(0, m_oid);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to purge track data."),
		      query.lastError().text(),
		      __FILE__,
		      __LINE__);
      QApplication::setOverrideCursor(Qt::WaitCursor);

      if(!qmain->getDB().rollback())
	qmain->addError(tr("Database Error"),
			tr("Rollback failure."),
			qmain->getDB().lastError().text(),
			__FILE__,
			__LINE__);

      QApplication::restoreOverrideCursor();
    }
  else
    {
      QApplication::restoreOverrideCursor();
      progress.setCancelButton(nullptr);
      progress.setLabelText(tr("Saving the track data..."));
      progress.setMaximum(trd.table->rowCount());
      progress.setMinimum(0);
      progress.setMinimumWidth
	(qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	       progress.sizeHint().width()));
      progress.setModal(true);
      progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
      progress.show();
      progress.update();
      progress.repaint();
      QApplication::processEvents();

      for(i = 0; i < trd.table->rowCount(); i++)
	{
	  query.prepare("INSERT INTO cd_songs ("
			"item_oid, "
			"albumnum, "
			"artist, "
			"composer, "
			"songnum, "
			"runtime, "
			"songtitle "
			") "
			"VALUES (?, ?, ?, ?, ?, ?, ?)");
	  query.addBindValue(m_oid);

	  if(trd.table->
	     cellWidget(i, static_cast<int> (TracksColumns::ALBUM_NUMBER)) !=
	     nullptr)
	    {
	      auto widget = trd.table->cellWidget
		(i, static_cast<int> (TracksColumns::ALBUM_NUMBER));

	      if(widget)
		{
		  auto comboBox = widget->findChild<QComboBox *> ();

		  if(comboBox)
		    query.addBindValue(comboBox->currentText());
		}
	    }

	  if(trd.table->item(i, static_cast<int> (TracksColumns::ARTIST)) !=
	     nullptr)
	    query.addBindValue
	      (trd.table->item(i, static_cast<int> (TracksColumns::ARTIST))->
	       text().trimmed());

	  if(trd.table->item(i, static_cast<int> (TracksColumns::COMPOSER)) !=
	     nullptr)
	    query.addBindValue
	      (trd.table->item(i, static_cast<int> (TracksColumns::COMPOSER))->
	       text().trimmed());

	  if(trd.table->
	     cellWidget(i, static_cast<int> (TracksColumns::TRACK_NUMBER)) !=
	     nullptr)
	    query.addBindValue
	      (qobject_cast<QSpinBox *>
	       (trd.table->cellWidget
		(i, static_cast<int> (TracksColumns::TRACK_NUMBER)))->value());

	  if(trd.table->
	     cellWidget(i, static_cast<int> (TracksColumns::TRACK_RUNTIME)) !=
	     nullptr)
	    query.addBindValue
	      (qobject_cast<QTimeEdit *>
	       (trd.table->cellWidget
		(i, static_cast<int> (TracksColumns::TRACK_RUNTIME)))->time().
	       toString("hh:mm:ss"));

	  if(trd.table->
	     item(i, static_cast<int> (TracksColumns::TRACK_TITLE)) != nullptr)
	    query.addBindValue
	      (trd.table->item
	       (i, static_cast<int> (TracksColumns::TRACK_TITLE))->
	       text().trimmed());

	  if(!query.exec())
	    {
	      qmain->addError(tr("Database Error"),
			      tr("Unable to create track data."),
			      query.lastError().text(),
			      __FILE__,
			      __LINE__);
	      lastError = query.lastError().text();
	    }

	  if(i + 1 <= progress.maximum())
	    progress.setValue(i + 1);

	  progress.repaint();
	  QApplication::processEvents();
	}

      progress.close();
      QApplication::setOverrideCursor(Qt::WaitCursor);

      if(!qmain->getDB().commit())
	{
	  qmain->addError(tr("Database Error"),
			  tr("Commit failure."),
			  qmain->getDB().lastError().text(),
			  __FILE__,
			  __LINE__);
	  qmain->getDB().rollback();
	}

      QApplication::restoreOverrideCursor();

      if(!lastError.isEmpty() ||
	 qmain->getDB().lastError().isValid())
	{
	  QMessageBox::critical
	    (m_tracks_diag,
	     tr("BiblioteQ: Database Error"),
	     tr("Some or all of the track data has not been saved."));
	  QApplication::processEvents();
	}

      /*
      ** Update the runtime.
      */

      if(lastError.isEmpty() && !qmain->getDB().lastError().isValid())
	slotComputeRuntime();
    }
}

void biblioteq_cd::slotSelectImage(void)
{
  QFileDialog dialog(this);
  auto button = qobject_cast<QPushButton *> (sender());

  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if(button == cd.frontButton)
    dialog.setWindowTitle(tr("BiblioteQ: Front Cover Image Selection"));
  else
    dialog.setWindowTitle(tr("BiblioteQ: Back Cover Image Selection"));

  dialog.exec();
  QApplication::processEvents();

  if(dialog.result() == QDialog::Accepted)
    {
      if(button == cd.frontButton)
	{
	  cd.front_image->clear();
	  cd.front_image->m_image = QImage(dialog.selectedFiles().value(0));

	  if(dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
	    cd.front_image->m_imageFormat = dialog.selectedFiles().value(0).mid
	      (dialog.selectedFiles().value(0).lastIndexOf(".") + 1).
	      toUpper();

	  cd.front_image->scene()->addPixmap
	    (QPixmap::fromImage(cd.front_image->m_image));

	  if(!cd.front_image->scene()->items().isEmpty())
	    cd.front_image->scene()->items().at(0)->setFlags
	      (QGraphicsItem::ItemIsSelectable);

	  cd.front_image->scene()->setSceneRect
	    (cd.front_image->scene()->itemsBoundingRect());
	}
      else
	{
	  cd.back_image->clear();
	  cd.back_image->m_image = QImage(dialog.selectedFiles().value(0));

	  if(dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
	    cd.back_image->m_imageFormat = dialog.selectedFiles().value(0).mid
	      (dialog.selectedFiles().value(0).lastIndexOf(".") + 1).
	      toUpper();

	  cd.back_image->scene()->addPixmap
	    (QPixmap::fromImage(cd.back_image->m_image));

	  if(!cd.back_image->scene()->items().isEmpty())
	    cd.back_image->scene()->items().at(0)->setFlags
	      (QGraphicsItem::ItemIsSelectable);

	  cd.back_image->scene()->setSceneRect
	    (cd.back_image->scene()->itemsBoundingRect());
	}
    }
}

void biblioteq_cd::slotShowUsers(void)
{
  int state = 0;

  if(!cd.okButton->isHidden())
    state = biblioteq::EDITABLE;
  else
    state = biblioteq::VIEW_ONLY;

  auto borrowerseditor = new biblioteq_borrowers_editor
    (qobject_cast<QWidget *> (this),
     qmain,
     static_cast<biblioteq_item *> (this),
     cd.quantity->value(),
     m_oid,
     cd.id->text(),
     font(),
     "CD",
     state);

  borrowerseditor->showUsers();
}

void biblioteq_cd::updateWindow(const int state)
{
  QString str = "";

  if(state == biblioteq::EDITABLE)
    {
      cd.backButton->setVisible(true);
      cd.computeButton->setVisible(true);
      cd.copiesButton->setEnabled(true);
      cd.frontButton->setVisible(true);
      cd.okButton->setVisible(true);
      cd.queryButton->setVisible(m_isQueryEnabled);
      cd.resetButton->setVisible(true);
      cd.showUserButton->setEnabled(true);
      m_engWindowTitle = "Modify";
      str = tr("BiblioteQ: Modify Music CD Entry (") + cd.id->text() + tr(")");
      trd.deleteButton->setVisible(true);
      trd.insertButton->setVisible(true);
      trd.saveButton->setVisible(true);
    }
  else
    {
      cd.backButton->setVisible(false);
      cd.computeButton->setVisible(false);
      cd.copiesButton->setVisible(false);
      cd.frontButton->setVisible(false);
      cd.okButton->setVisible(false);
      cd.queryButton->setVisible(false);
      cd.resetButton->setVisible(false);
      cd.showUserButton->setVisible(qmain->isGuest() ? false : true);
      m_engWindowTitle = "View";
      str = tr("BiblioteQ: View Music CD Details (") + cd.id->text() + tr(")");
      trd.deleteButton->setVisible(false);
      trd.insertButton->setVisible(false);
      trd.saveButton->setVisible(false);
    }

  cd.tracksButton->setEnabled(true);
  setReadOnlyFields(this, state != biblioteq::EDITABLE);
  setWindowTitle(str);
  emit windowTitleChanged(windowTitle());
}
