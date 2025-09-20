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
#include "biblioteq_copy_editor.h"
#include "biblioteq_videogame.h"

#include <QFileDialog>
#include <QSettings>
#include <QShortcut>
#include <QSqlField>
#include <QSqlRecord>

biblioteq_videogame::biblioteq_videogame(biblioteq *parentArg,
					 const QString &oidArg,
					 const QModelIndex &index):
  QMainWindow(), biblioteq_item(index)
{
  qmain = parentArg;

  QRegularExpression rx
    ("[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]");
  auto menu = new QMenu(this);
  auto scene1 = new QGraphicsScene(this);
  auto scene2 = new QGraphicsScene(this);
  auto validator1 = new QRegularExpressionValidator(rx, this);

  m_isQueryEnabled = false;
  m_oid = oidArg;
  m_oldq = biblioteq_misc_functions::getColumnString
    (qmain->getUI().table,
     m_index->row(),
     qmain->getUI().table->columnNumber("Quantity")).toInt();
  m_parentWid = parentArg;
  vg.setupUi(this);
  setQMain(this);
  biblioteq_misc_functions::sortCombinationBox(vg.mode);
  vg.publication_date_enabled->setVisible(false);
  vg.quantity->setMaximum(static_cast<int> (biblioteq::Limits::QUANTITY));
  vg.release_date->setDisplayFormat(qmain->publicationDateFormat("videogames"));
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
  connect(qmain,
	  SIGNAL(fontChanged(const QFont &)),
	  this,
	  SLOT(setGlobalFonts(const QFont &)));
  connect(qmain,
	  SIGNAL(otherOptionsSaved(void)),
	  this,
	  SLOT(slotPrepareIcons(void)));
  connect(vg.cancelButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotCancel(void)));
  connect(vg.copiesButton,
	  SIGNAL(clicked()),
	  this,
	  SLOT(slotPopulateCopiesEditor(void)));
  connect(vg.okButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotGo(void)));
  connect(vg.printButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotPrint(void)));
  connect(vg.queryButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotQuery(void)));
  connect(vg.resetButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(vg.showUserButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotShowUsers(void)));
  connect(menu->addAction(tr("Reset Front Cover Image")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Back Cover Image")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset UPC")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Game Rating")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Developers")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Platform")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Mode")),
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
  connect(menu->addAction(tr("Reset Publisher")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Place of Publication")),
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotReset(void)));
  connect(menu->addAction(tr("Reset Genres")),
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
  connect(vg.backButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectImage(void)));
  connect(vg.frontButton,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSelectImage(void)));
  connect(vg.front_image,
	  SIGNAL(imageChanged(const QImage &)),
	  this,
	  SIGNAL(imageChanged(const QImage &)));
  vg.id->setValidator(validator1);
  vg.resetButton->setMenu(menu);

  QString errorstr("");

  QApplication::setOverrideCursor(Qt::WaitCursor);
  vg.rating->addItems
    (biblioteq_misc_functions::getVideoGameRatings(qmain->getDB(), errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the video game ratings."),
       errorstr,
       __FILE__,
       __LINE__);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  vg.platform->addItems
    (biblioteq_misc_functions::getVideoGamePlatforms(qmain->getDB(), errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the video game platforms."),
       errorstr,
       __FILE__,
       __LINE__);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  vg.language->addItems
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
  vg.monetary_units->addItems
    (biblioteq_misc_functions::getMonetaryUnits(qmain->getDB(), errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the monetary units."),
       errorstr,
       __FILE__,
       __LINE__);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  vg.location->addItems
    (biblioteq_misc_functions::getLocations(qmain->getDB(),
					    "Video Game",
					    errorstr));
  QApplication::restoreOverrideCursor();

  if(!errorstr.isEmpty())
    qmain->addError
      (tr("Database Error"),
       tr("Unable to retrieve the video game locations."),
       errorstr,
       __FILE__,
       __LINE__);

  vg.back_image->setScene(scene2);
  vg.front_image->setScene(scene1);
  vg.queryButton->setVisible(m_isQueryEnabled);

  if(vg.language->findText(biblioteq::s_unknown) == -1)
    vg.language->addItem(biblioteq::s_unknown);

  if(vg.location->findText(biblioteq::s_unknown) == -1)
    vg.location->addItem(biblioteq::s_unknown);

  if(vg.monetary_units->findText(biblioteq::s_unknown) == -1)
    vg.monetary_units->addItem(biblioteq::s_unknown);

  if(vg.platform->findText(biblioteq::s_unknown) == -1)
    vg.platform->addItem(biblioteq::s_unknown);

  if(vg.rating->findText(biblioteq::s_unknown) == -1)
    vg.rating->addItem(biblioteq::s_unknown);

  if(m_parentWid)
    resize(qRound(0.95 * m_parentWid->size().width()),
	   qRound(0.95 * m_parentWid->size().height()));

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

  vg.splitter->setStretchFactor(0, 0);
  vg.splitter->setStretchFactor(1, 1);
  vg.splitter->setStretchFactor(2, 0);
  biblioteq_misc_functions::center(this, m_parentWid);
  biblioteq_misc_functions::hideAdminFields(this, qmain->getRoles());
}

biblioteq_videogame::~biblioteq_videogame()
{
}

void biblioteq_videogame::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  vg.retranslateUi(this);
	  break;
	}
      default:
	break;
      }

  QMainWindow::changeEvent(event);
}

void biblioteq_videogame::closeEvent(QCloseEvent *event)
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
  qmain->removeVideoGame(this);
}

void biblioteq_videogame::duplicate(const QString &p_oid, const int state)
{
  modify(state); // Initial population.
  m_engWindowTitle = "Create";
  m_oid = p_oid;
  setWindowTitle(tr("BiblioteQ: Duplicate Video Game Entry"));
  emit windowTitleChanged(windowTitle());
  vg.copiesButton->setEnabled(false);
  vg.showUserButton->setEnabled(false);
}

void biblioteq_videogame::insert(void)
{
  slotReset();
  vg.accession_number->clear();
  vg.copiesButton->setEnabled(false);
  vg.description->setPlainText("N/A");
  vg.developer->setPlainText("N/A");
  vg.genre->setPlainText("N/A");
  vg.keyword->clear();
  vg.language->setCurrentIndex(0);
  vg.location->setCurrentIndex(0);
  vg.mode->setCurrentIndex(0);
  vg.monetary_units->setCurrentIndex(0);
  vg.okButton->setText(tr("&Save"));
  vg.place->setPlainText("N/A");
  vg.price->setMinimum(0.00);
  vg.price->setValue(0.00);
  vg.publisher->setPlainText("N/A");
  vg.quantity->setMinimum(1);
  vg.quantity->setValue(1);
  vg.queryButton->setEnabled(true);
  vg.rating->setCurrentIndex(0);
  vg.release_date->setDate
    (QDate::fromString("01/01/2000", biblioteq::s_databaseDateFormat));
  vg.showUserButton->setEnabled(false);
  biblioteq_misc_functions::highlightWidget
    (vg.description->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (vg.developer->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (vg.genre->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (vg.id, m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (vg.place->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (vg.publisher->viewport(), m_requiredHighlightColor);
  biblioteq_misc_functions::highlightWidget
    (vg.title, m_requiredHighlightColor);
  m_engWindowTitle = "Create";
  prepareFavorites();
  setWindowTitle(tr("BiblioteQ: Create Video Game Entry"));
  emit windowTitleChanged(windowTitle());
  vg.id->setFocus();
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

void biblioteq_videogame::modify(const int state)
{
  QSqlQuery query(qmain->getDB());
  QString fieldname = "";
  QString str = "";
  QVariant var;
  int i = 0;

  if(state == biblioteq::EDITABLE)
    {
      setReadOnlyFields(this, false);
      setWindowTitle(tr("BiblioteQ: Modify Video Game Entry"));
      emit windowTitleChanged(windowTitle());
      m_engWindowTitle = "Modify";
      vg.backButton->setVisible(true);
      vg.copiesButton->setEnabled(true);
      vg.frontButton->setVisible(true);
      vg.okButton->setVisible(true);
      vg.queryButton->setVisible(m_isQueryEnabled);
      vg.resetButton->setVisible(true);
      vg.showUserButton->setEnabled(true);
      biblioteq_misc_functions::highlightWidget
	(vg.description->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(vg.developer->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(vg.genre->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(vg.id, m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(vg.place->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(vg.publisher->viewport(), m_requiredHighlightColor);
      biblioteq_misc_functions::highlightWidget
	(vg.title, m_requiredHighlightColor);
    }
  else
    {
      setReadOnlyFields(this, true);
      setWindowTitle(tr("BiblioteQ: View Video Game Details"));
      emit windowTitleChanged(windowTitle());
      m_engWindowTitle = "View";
      vg.backButton->setVisible(false);
      vg.copiesButton->setVisible(false);
      vg.frontButton->setVisible(false);
      vg.okButton->setVisible(false);
      vg.queryButton->setVisible(false);
      vg.resetButton->setVisible(false);

      if(qmain->isGuest())
	vg.showUserButton->setVisible(false);
      else
	vg.showUserButton->setEnabled(true);

      auto const actions = vg.resetButton->menu()->actions();

      if(actions.size() >= 2)
	{
	  actions[0]->setVisible(false);
	  actions[1]->setVisible(false);
	}
    }

  vg.okButton->setText(tr("&Save"));
  vg.price->setMinimum(0.00);
  vg.quantity->setMinimum(1);
  vg.queryButton->setEnabled(true);
  prepareIcons(this);
  str = m_oid;
  query.prepare("SELECT title, "
		"vgrating, "
		"vgplatform, "
		"vgmode, "
		"developer, "
		"publisher, rdate, place, "
		"genre, language, id, "
		"price, monetary_units, quantity, "
		"location, description, "
		"front_cover, "
		"back_cover, "
		"keyword, "
		"accession_number "
		"FROM "
		"videogame "
		"WHERE myoid = ?");
  query.bindValue(0, str);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!query.exec() || !query.next())
    {
      QApplication::restoreOverrideCursor();
      qmain->addError(tr("Database Error"),
		      tr("Unable to retrieve the selected video game's data."),
		      query.lastError().text(),
		      __FILE__,
		      __LINE__);
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to retrieve the selected video "
			       "game's data."));
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
	    vg.accession_number->setText(var.toString().trimmed());
	  else if(fieldname == "back_cover")
	    {
	      if(!record.field(i).isNull())
		{
		  vg.back_image->loadFromData
		    (QByteArray::fromBase64(var.toByteArray()));

		  if(vg.back_image->m_image.isNull())
		    vg.back_image->loadFromData(var.toByteArray());
		}
	    }
	  else if(fieldname == "description")
	    vg.description->setPlainText(var.toString().trimmed());
	  else if(fieldname == "developer")
	    vg.developer->setMultipleLinks
	      ("videogame_search", "developer", var.toString().trimmed());
	  else if(fieldname == "front_cover")
	    {
	      if(!record.field(i).isNull())
		{
		  vg.front_image->loadFromData
		    (QByteArray::fromBase64(var.toByteArray()));

		  if(vg.front_image->m_image.isNull())
		    vg.front_image->loadFromData(var.toByteArray());
		}
	    }
	  else if(fieldname == "genre")
	    vg.genre->setMultipleLinks
	      ("videogame_search", "genre", var.toString().trimmed());
	  else if(fieldname == "id")
	    {
	      if(state == biblioteq::EDITABLE)
		{
		  str = tr("BiblioteQ: Modify Video Game Entry (") +
		    var.toString().trimmed() +
		    tr(")");
		  m_engWindowTitle = "Modify";
		}
	      else
		{
		  str = tr("BiblioteQ: View Video Game Details (") +
		    var.toString().trimmed() +
		    tr(")");
		  m_engWindowTitle = "View";
		}

	      vg.id->setText(var.toString().trimmed());
	      setWindowTitle(str);
	      emit windowTitleChanged(windowTitle());
	    }
	  else if(fieldname == "keyword")
	    vg.keyword->setMultipleLinks
	      ("videogame_search", "keyword", var.toString().trimmed());
	  else if(fieldname == "language")
	    {
	      if(vg.language->findText(var.toString().trimmed()) > -1)
		vg.language->setCurrentIndex
		  (vg.language->findText(var.toString().trimmed()));
	      else
		vg.language->setCurrentIndex
		  (vg.language->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "location")
	    {
	      if(vg.location->findText(var.toString().trimmed()) > -1)
		vg.location->setCurrentIndex
		  (vg.location->findText(var.toString().trimmed()));
	      else
		vg.location->setCurrentIndex
		  (vg.location->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "monetary_units")
	    {
	      if(vg.monetary_units->findText(var.toString().trimmed()) > -1)
		vg.monetary_units->setCurrentIndex
		  (vg.monetary_units->findText(var.toString().trimmed()));
	      else
		vg.monetary_units->setCurrentIndex
		  (vg.monetary_units->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "place")
	    vg.place->setMultipleLinks
	      ("videogame_search", "place", var.toString().trimmed());
	  else if(fieldname == "price")
	    vg.price->setValue(var.toDouble());
	  else if(fieldname == "publisher")
	    vg.publisher->setMultipleLinks
	      ("videogame_search", "publisher", var.toString().trimmed());
	  else if(fieldname == "quantity")
	    {
	      vg.quantity->setValue(var.toInt());
	      m_oldq = vg.quantity->value();
	    }
	  else if(fieldname == "rdate")
	    vg.release_date->setDate
	      (QDate::fromString(var.toString().trimmed(),
				 biblioteq::s_databaseDateFormat));
	  else if(fieldname == "title")
	    vg.title->setText(var.toString().trimmed());
	  else if(fieldname == "vgmode")
	    {
	      if(vg.mode->findText(var.toString().trimmed()) > -1)
		vg.mode->setCurrentIndex
		  (vg.mode->findText(var.toString().trimmed()));
	      else
		vg.mode->setCurrentIndex(0);
	    }
	  else if(fieldname == "vgplatform")
	    {
	      if(vg.platform->findText(var.toString().trimmed()) > -1)
		vg.platform->setCurrentIndex
		  (vg.platform->findText(var.toString().trimmed()));
	      else
		vg.platform->setCurrentIndex
		  (vg.platform->findText(biblioteq::s_unknown));
	    }
	  else if(fieldname == "vgrating")
	    {
	      if(vg.rating->findText(var.toString().trimmed()) > -1)
		vg.rating->setCurrentIndex
		  (vg.rating->findText(var.toString().trimmed()));
	      else
		vg.rating->setCurrentIndex
		  (vg.rating->findText(biblioteq::s_unknown));
	    }
	}

      foreach(auto textfield, findChildren<QLineEdit *> ())
	textfield->setCursorPosition(0);

      storeData(this);
    }

  vg.id->setFocus();
  raise();
}

void biblioteq_videogame::prepareFavorites(void)
{
  QSettings settings;

  vg.language->setCurrentIndex
    (vg.language->
     findText(settings.value("languages_favorite").toString().trimmed()));
  vg.monetary_units->setCurrentIndex
    (vg.monetary_units->
     findText(settings.value("monetary_units_favorite").toString().trimmed()));
  vg.platform->setCurrentIndex
    (vg.platform->
     findText(settings.value("videogame_platforms_favorite").
	      toString().trimmed()));
  vg.rating->setCurrentIndex
    (vg.rating->findText
     (settings.value("videogame_ratings_favorite").toString().trimmed()));

  if(vg.language->currentIndex() < 0)
    vg.language->setCurrentIndex(0);

  if(vg.monetary_units->currentIndex() < 0)
    vg.monetary_units->setCurrentIndex(0);

  if(vg.platform->currentIndex() < 0)
    vg.platform->setCurrentIndex(0);

  if(vg.rating->currentIndex() < 0)
    vg.rating->setCurrentIndex(0);
}

void biblioteq_videogame::search(const QString &field, const QString &value)
{
  m_engWindowTitle = "Search";
  vg.accession_number->clear();
  vg.copiesButton->setVisible(false);
  vg.coverImages->setVisible(false);
  vg.description->clear();
  vg.developer->clear();
  vg.genre->clear();
  vg.id->clear();
  vg.keyword->clear();
  vg.language->insertItem(0, tr("Any"));
  vg.language->setCurrentIndex(0);
  vg.location->insertItem(0, tr("Any"));
  vg.location->setCurrentIndex(0);
  vg.mode->insertItem(0, tr("Any"));
  vg.mode->setCurrentIndex(0);
  vg.monetary_units->insertItem(0, tr("Any"));
  vg.monetary_units->setCurrentIndex(0);
  vg.okButton->setText(tr("&Search"));
  vg.platform->insertItem(0, tr("Any"));
  vg.platform->setCurrentIndex(0);
  vg.price->setMinimum(-0.01);
  vg.price->setValue(-0.01);
  vg.publication_date_enabled->setVisible(true);
  vg.publisher->clear();
  vg.quantity->setMinimum(0);
  vg.quantity->setValue(0);
  vg.queryButton->setVisible(false);
  vg.rating->insertItem(0, tr("Any"));
  vg.rating->setCurrentIndex(0);
  vg.release_date->setDate(QDate::fromString("2001", "yyyy"));
  vg.release_date->setDisplayFormat("yyyy");
  vg.showUserButton->setVisible(false);
  vg.title->clear();
  prepareIcons(this);

  if(field.isEmpty() && value.isEmpty())
    {
      auto const actions = vg.resetButton->menu()->actions();

      if(actions.size() >= 2)
	{
	  actions[0]->setVisible(false);
	  actions[1]->setVisible(false);
	}

      prepareFavorites();
      setWindowTitle(tr("BiblioteQ: Database Video Game Search"));
      emit windowTitleChanged(windowTitle());
      vg.id->setFocus();
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
      if(field == "developer")
	vg.developer->setPlainText(value);
      else if(field == "genre")
	vg.genre->setPlainText(value);
      else if(field == "keyword")
	vg.keyword->setPlainText(value);
      else if(field == "place")
	vg.place->setPlainText(value);
      else if(field == "publisher")
	vg.publisher->setPlainText(value);

      slotGo();
    }
}

void biblioteq_videogame::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach(auto widget, findChildren<QWidget *> ())
    {
      widget->setFont(font);
      widget->update();
    }

  update();
}

void biblioteq_videogame::slotCancel(void)
{
  close();
}

void biblioteq_videogame::slotDatabaseEnumerationsCommitted(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QList<QComboBox *> widgets;

  widgets << vg.language
	  << vg.location
	  << vg.monetary_units
	  << vg.platform
	  << vg.rating;

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
	       getLanguages(qmain->getDB(), errorstr));
	    break;
	  }
	case 1:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::getLocations(qmain->getDB(),
						      "Video Game",
						      errorstr));
	    break;
	  }
	case 2:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::
	       getMonetaryUnits(qmain->getDB(), errorstr));
	    break;
	  }
	case 3:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::
	       getVideoGamePlatforms(qmain->getDB(), errorstr));
	    break;
	  }
	case 4:
	  {
	    widgets.at(i)->addItems
	      (biblioteq_misc_functions::
	       getVideoGameRatings(qmain->getDB(), errorstr));
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

void biblioteq_videogame::slotGo(void)
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
	  newq = vg.quantity->value();
	  QApplication::setOverrideCursor(Qt::WaitCursor);
	  maxcopynumber = biblioteq_misc_functions::getMaxCopyNumber
	    (qmain->getDB(), m_oid, "Video Game", errorstr);

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
		    "number of copies while there are copies that have "
		    "been reserved."));
	      QApplication::processEvents();
	      vg.quantity->setValue(m_oldq);
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

      str = vg.id->text().trimmed();
      vg.id->setText(str);

      if(vg.id->text().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the UPC field."));
	  QApplication::processEvents();
	  vg.id->setFocus();
	  return;
	}

      str = vg.developer->toPlainText().trimmed();
      vg.developer->setPlainText(str);

      if(vg.developer->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Developers field."));
	  QApplication::processEvents();
	  vg.developer->setFocus();
	  return;
	}

      str = vg.title->text().trimmed();
      vg.title->setText(str);

      if(vg.title->text().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Title field."));
	  QApplication::processEvents();
	  vg.title->setFocus();
	  return;
	}

      str = vg.publisher->toPlainText().trimmed();
      vg.publisher->setPlainText(str);

      if(vg.publisher->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Publisher field."));
	  QApplication::processEvents();
	  vg.publisher->setFocus();
	  return;
	}

      str = vg.place->toPlainText().trimmed();
      vg.place->setPlainText(str);

      if(vg.place->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Place of Publication "
				   "field."));
	  QApplication::processEvents();
	  vg.place->setFocus();
	  return;
	}

      str = vg.genre->toPlainText().trimmed();
      vg.genre->setPlainText(str);

      if(vg.genre->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Genres field."));
	  QApplication::processEvents();
	  vg.genre->setFocus();
	  return;
	}

      str = vg.description->toPlainText().trimmed();
      vg.description->setPlainText(str);

      if(vg.description->toPlainText().isEmpty())
	{
	  QMessageBox::critical(this,
				tr("BiblioteQ: User Error"),
				tr("Please complete the Abstract field."));
	  QApplication::processEvents();
	  vg.description->setFocus();
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

      str = vg.keyword->toPlainText().trimmed();
      vg.keyword->setPlainText(str);
      str = vg.accession_number->text().trimmed();
      vg.accession_number->setText(str);
      QApplication::restoreOverrideCursor();

      QSqlQuery query(qmain->getDB());

      if(m_engWindowTitle.contains("Modify"))
	query.prepare("UPDATE videogame SET id = ?, "
		      "title = ?, "
		      "vgrating = ?, developer = ?, "
		      "rdate = ?, "
		      "publisher = ?, "
		      "genre = ?, price = ?, "
		      "description = ?, "
		      "language = ?, "
		      "monetary_units = ?, "
		      "quantity = ?, "
		      "vgplatform = ?, "
		      "location = ?, "
		      "vgmode = ?, "
		      "front_cover = ?, "
		      "back_cover = ?, "
		      "place = ?, "
		      "keyword = ?, "
		      "accession_number = ? "
		      "WHERE "
		      "myoid = ?");
      else if(qmain->getDB().driverName() != "QSQLITE")
	query.prepare("INSERT INTO videogame (id, title, "
		      "vgrating, developer, rdate, publisher, "
		      "genre, price, description, language, "
		      "monetary_units, quantity, "
		      "vgplatform, location, vgmode, "
		      "front_cover, "
		      "back_cover, "
		      "place, keyword, accession_number) "
		      "VALUES (?, ?, ?, ?, ?, "
		      "?, ?, ?, "
		      "?, ?, ?, "
		      "?, ?, ?, ?, ?, ?, ?, ?, ?)");
      else
	query.prepare("INSERT INTO videogame (id, title, "
		      "vgrating, developer, rdate, publisher, "
		      "genre, price, description, language, "
		      "monetary_units, quantity, "
		      "vgplatform, location, vgmode, "
		      "front_cover, "
		      "back_cover, "
		      "place, keyword, accession_number, myoid) "
		      "VALUES (?, ?, ?, ?, ?, "
		      "?, ?, ?, "
		      "?, ?, ?, "
		      "?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

      query.bindValue(0, vg.id->text());
      query.bindValue(1, vg.title->text());
      query.bindValue(2, vg.rating->currentText().trimmed());
      query.bindValue(3, vg.developer->toPlainText().trimmed());
      query.bindValue
	(4, vg.release_date->date().toString(biblioteq::s_databaseDateFormat));
      query.bindValue(5, vg.publisher->toPlainText().trimmed());
      query.bindValue(6, vg.genre->toPlainText().trimmed());
      query.bindValue(7, vg.price->value());
      query.bindValue(8, vg.description->toPlainText().trimmed());
      query.bindValue(9, vg.language->currentText().trimmed());
      query.bindValue(10, vg.monetary_units->currentText().trimmed());
      query.bindValue(11, vg.quantity->text());
      query.bindValue(12, vg.platform->currentText().trimmed());
      query.bindValue(13, vg.location->currentText().trimmed());
      query.bindValue(14, vg.mode->currentText().trimmed());

      if(!vg.front_image->m_image.isNull())
	{
	  QByteArray bytes;
	  QBuffer buffer(&bytes);

	  if(buffer.open(QIODevice::WriteOnly))
	    {
	      vg.front_image->m_image.save
		(&buffer, vg.front_image->m_imageFormat.toLatin1(), 100);
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
	  vg.front_image->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	  query.bindValue(15, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	  query.bindValue(15, QVariant(QVariant::ByteArray));
#endif
	}

      if(!vg.back_image->m_image.isNull())
	{
	  QByteArray bytes;
	  QBuffer buffer(&bytes);

	  if(buffer.open(QIODevice::WriteOnly))
	    {
	      vg.back_image->m_image.save
		(&buffer, vg.back_image->m_imageFormat.toLatin1(), 100);
	      query.bindValue(16, bytes.toBase64());
	    }
	  else
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	    query.bindValue(16, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	    query.bindValue(16, QVariant(QVariant::ByteArray));
#endif
	}
      else
	{
	  vg.back_image->m_imageFormat = "";
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	  query.bindValue(16, QVariant(QMetaType(QMetaType::QByteArray)));
#else
	  query.bindValue(16, QVariant(QVariant::ByteArray));
#endif
	}

      query.bindValue(17, vg.place->toPlainText().trimmed());
      query.bindValue(18, vg.keyword->toPlainText().trimmed());
      query.bindValue(19, vg.accession_number->text().trimmed());

      if(m_engWindowTitle.contains("Modify"))
	query.bindValue(20, m_oid);
      else if(qmain->getDB().driverName() == "QSQLITE")
	{
	  auto const value = biblioteq_misc_functions::getSqliteUniqueId
	    (qmain->getDB(), errorstr);

	  if(errorstr.isEmpty())
	    query.bindValue(20, value);
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
	  /*
	  ** Remove copies if the quantity has been decreased.
	  */

	  if(m_engWindowTitle.contains("Modify"))
	    {
	      /*
	      ** Retain quantity copies.
	      */

	      query.prepare("DELETE FROM videogame_copy_info WHERE "
			    "myoid NOT IN "
			    "(SELECT myoid FROM videogame_copy_info "
			    "WHERE item_oid = ? ORDER BY copy_number "
			    "LIMIT ?) AND item_oid = ?");
	      query.addBindValue(m_oid);
	      query.addBindValue(vg.quantity->text());
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
		(vg.id->text(),
		 vg.quantity->value(),
		 qmain->getDB(),
		 "Video Game",
		 errorstr);

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

	  m_oldq = vg.quantity->value();

	  if(vg.front_image->m_image.isNull())
	    vg.front_image->m_imageFormat = "";

	  if(vg.back_image->m_image.isNull())
	    vg.back_image->m_imageFormat = "";

	  vg.developer->setMultipleLinks
	    ("videogame_search", "developer", vg.developer->toPlainText());
	  vg.genre->setMultipleLinks
	    ("videogame_search", "genre", vg.genre->toPlainText());
	  vg.keyword->setMultipleLinks
	    ("videogame_search", "keyword", vg.keyword->toPlainText());
	  vg.place->setMultipleLinks
	    ("videogame_search", "place", vg.place->toPlainText());
	  vg.publisher->setMultipleLinks
	    ("videogame_search", "publisher", vg.publisher->toPlainText());
	  QApplication::restoreOverrideCursor();

	  if(m_engWindowTitle.contains("Modify"))
	    {
	      str = tr("BiblioteQ: Modify Video Game Entry (") +
		vg.id->text() +
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
		  qmain->getTypeFilterString() == "Video Games"))
		{
		  qmain->getUI().table->setSortingEnabled(false);

		  auto const names(qmain->getUI().table->columnNames());

		  for(i = 0; i < names.size(); i++)
		    {
		      if(i == 0 && qmain->showMainTableImages())
			{
			  auto const pixmap
			    (QPixmap::fromImage(vg.front_image->m_image));

			  if(!pixmap.isNull())
			    qmain->getUI().table->item(m_index->row(), i)->
			      setIcon(pixmap);
			  else
			    qmain->getUI().table->item(m_index->row(), i)->
			      setIcon(QIcon(":/missing_image.png"));
			}

		      if(names.at(i) == "Accession Number")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.accession_number->text());
		      else if(names.at(i) == "Availability")
			{
			  qmain->getUI().table->item(m_index->row(), i)->setText
			    (biblioteq_misc_functions::getAvailability
			     (m_oid, qmain->getDB(), "Video Game", errorstr));

			  if(!errorstr.isEmpty())
			    qmain->addError
			      (tr("Database Error"),
			       tr("Retrieving availability."),
			       errorstr,
			       __FILE__,
			       __LINE__);
			}
		      else if(names.at(i) == "Categories" ||
			      names.at(i) == "Genres")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.genre->toPlainText().trimmed());
		      else if(names.at(i) == "Game Rating")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.rating->currentText().trimmed());
		      else if(names.at(i) == "ID Number" ||
			      names.at(i) == "UPC")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.id->text());
		      else if(names.at(i) == "Language")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.language->currentText().trimmed());
		      else if(names.at(i) == "Location")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.location->currentText().trimmed());
		      else if(names.at(i) == "Mode")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.mode->currentText().trimmed());
		      else if(names.at(i) == "Monetary Units")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.monetary_units->currentText().trimmed());
		      else if(names.at(i) == "Place of Publication")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.place->toPlainText());
		      else if(names.at(i) == "Platform")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.platform->currentText().trimmed());
		      else if(names.at(i) == "Price")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (QLocale().toString(vg.price->value()));
		      else if(names.at(i) == "Publication Date" ||
			      names.at(i) == "Release Date")
			{
			  if(qmain->getTypeFilterString() == "Video Games")
			    qmain->getUI().table->item(m_index->row(), i)->
			      setText
			      (vg.release_date->date().
			       toString(qmain->
					publicationDateFormat("videogames")));
			  else
			    qmain->getUI().table->item(m_index->row(), i)->
			      setText
			      (vg.release_date->date().toString(Qt::ISODate));
			}
		      else if(names.at(i) == "Publisher")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.publisher->toPlainText());
		      else if(names.at(i) == "Quantity")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.quantity->text());
		      else if(names.at(i) == "Title")
			qmain->getUI().table->item(m_index->row(), i)->setText
			  (vg.title->text());
		    }

		  qmain->getUI().table->setSortingEnabled(true);
		  qmain->getUI().table->updateToolTips(m_index->row());

		  foreach(auto textfield, findChildren<QLineEdit *> ())
		    textfield->setCursorPosition(0);

		  qmain->slotResizeColumns();
		}

	      qmain->slotDisplaySummary();
	      qmain->updateSceneItem(m_oid, "Video Game",
				     vg.front_image->m_image);
	    }
	  else
	    {
	      QApplication::setOverrideCursor(Qt::WaitCursor);
	      m_oid = biblioteq_misc_functions::getOID(vg.id->text(),
						       "Video Game",
						       qmain->getDB(),
						       errorstr);
	      QApplication::restoreOverrideCursor();

	      if(!errorstr.isEmpty())
		{
		  qmain->addError
		    (tr("Database Error"),
		     tr("Unable to retrieve the video game's OID."),
		     errorstr,
		     __FILE__,
		     __LINE__);
		  QMessageBox::critical(this,
					tr("BiblioteQ: Database Error"),
					tr("Unable to retrieve the "
					   "video game's OID."));
		  QApplication::processEvents();
		}
	      else
		qmain->replaceVideoGame(m_oid, this);

	      updateWindow(biblioteq::EDITABLE);

	      if(qmain->getUI().actionAutoPopulateOnCreation->isChecked())
		(void) qmain->populateTable
		  (biblioteq::POPULATE_ALL, "Video Games", "");

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
      QMessageBox::critical(this,
			    tr("BiblioteQ: Database Error"),
			    tr("Unable to create or update the entry. "
			       "Please verify that the entry does not "
			       "already exist."));
      QApplication::processEvents();
    }
  else if(m_engWindowTitle.contains("Search"))
    {
      QString frontCover("'' AS front_cover ");

      if(qmain->showMainTableImages())
	frontCover = "videogame.front_cover ";

      searchstr = "SELECT DISTINCT videogame.title, "
	"videogame.vgrating, "
	"videogame.vgplatform, "
	"videogame.vgmode, "
	"videogame.publisher, videogame.rdate, videogame.place, "
	"videogame.genre, videogame.language, videogame.id, "
	"videogame.price, videogame.monetary_units, "
	"videogame.quantity, "
	"videogame.location, "
	"videogame.quantity - COUNT(item_borrower.item_oid) "
	"AS availability, "
	"COUNT(item_borrower.item_oid) AS total_reserved, "
	"videogame.accession_number, "
	"videogame.type, "
	"videogame.myoid, " +
	frontCover +
	"FROM "
	"videogame LEFT JOIN item_borrower ON "
	"videogame.myoid = item_borrower.item_oid "
	"AND item_borrower.type = 'Video Game' "
	"WHERE ";
      searchstr.append("LOWER(id) LIKE LOWER('%' || ? || '%') AND ");

      QString ESCAPE("");
      auto const UNACCENT(qmain->unaccent());

      if(qmain->getDB().driverName() != "QSQLITE")
	ESCAPE = "E";

      searchstr.append
	(UNACCENT + "(LOWER(title)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

      if(vg.rating->currentIndex() != 0)
	searchstr.append
	  (UNACCENT + "(vgrating) = " + UNACCENT + "(" + ESCAPE + "'" +
	   biblioteq_myqstring::escape(vg.rating->currentText().
				       trimmed()) +
	   "') AND ");

      searchstr.append
	(UNACCENT + "(LOWER(developer)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

      if(vg.publication_date_enabled->isChecked())
	searchstr.append("SUBSTR(rdate, 7) = '" +
			 vg.release_date->date().toString("yyyy") +
			 "' AND ");

      searchstr.append
	(UNACCENT + "(LOWER(publisher)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");
      searchstr.append
	(UNACCENT + "(LOWER(place)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");
      searchstr.append
	(UNACCENT + "(LOWER(genre)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) AND ");

      if(vg.price->value() > -0.01)
	{
	  searchstr.append("price = ");
	  searchstr.append(QString::number(vg.price->value()));
	  searchstr.append(" AND ");
	}

      if(vg.language->currentIndex() != 0)
	searchstr.append
	  (UNACCENT + "(language) = " + UNACCENT + "(" + ESCAPE + "'" +
	   biblioteq_myqstring::escape(vg.language->currentText().trimmed()) +
	   "') AND ");

      if(vg.monetary_units->currentIndex() != 0)
	searchstr.append
	  (UNACCENT + "(monetary_units) = " + UNACCENT + "(" + ESCAPE + "'" +
	   biblioteq_myqstring::escape(vg.monetary_units->currentText().
				       trimmed()) +
	   "') AND ");

      if(vg.platform->currentIndex() != 0)
	searchstr.append
	  (UNACCENT + "(vgplatform) = " + UNACCENT + "(" + ESCAPE + "'" +
	   biblioteq_myqstring::escape(vg.platform->currentText().trimmed()) +
	   "') AND ");

      searchstr.append
	(UNACCENT + "(LOWER(description)) LIKE " + UNACCENT +
	 "(LOWER(" + ESCAPE + "'%' || ? || '%')) ");

      if(vg.quantity->value() != 0)
	searchstr.append("AND quantity = " + vg.quantity->text() + " ");

      if(vg.location->currentIndex() != 0)
	searchstr.append
	  ("AND " + UNACCENT + "(location) = " + UNACCENT + "(" +
	   ESCAPE + "'" +
	   biblioteq_myqstring::escape(vg.location->currentText().
				       trimmed()) + "') ");

      if(vg.mode->currentIndex() != 0)
	searchstr.append
	  ("AND " + UNACCENT + "(vgmode) = " + UNACCENT + "(" + ESCAPE + "'" +
	   biblioteq_myqstring::escape(vg.mode->currentText().
				       trimmed()) + "') ");

      searchstr.append
	("AND " + UNACCENT + "(LOWER(COALESCE(keyword, ''))) LIKE " +
	 UNACCENT + "(LOWER(" + ESCAPE + "'%' || ? || '%')) ");
      searchstr.append
	("AND " + UNACCENT + "(LOWER(COALESCE(accession_number, ''))) LIKE " +
	 UNACCENT + "(LOWER(" + ESCAPE + "'%' || ? || '%')) ");
      searchstr.append("GROUP BY "
		       "videogame.title, "
		       "videogame.vgrating, "
		       "videogame.vgplatform, "
		       "videogame.vgmode, "
		       "videogame.publisher, "
		       "videogame.rdate, "
		       "videogame.place, "
		       "videogame.genre, "
		       "videogame.language, "
		       "videogame.id, "
		       "videogame.price, "
		       "videogame.monetary_units, "
		       "videogame.quantity, "
		       "videogame.location, "
		       "videogame.accession_number, "
		       "videogame.type, "
		       "videogame.myoid, "
		       "videogame.front_cover");

      auto query = new QSqlQuery(qmain->getDB());

      query->prepare(searchstr);
      query->addBindValue(vg.id->text().trimmed());
      query->addBindValue
	(biblioteq_myqstring::escape(vg.title->text().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(vg.developer->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(vg.publisher->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(vg.place->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(vg.genre->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(vg.description->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(vg.keyword->toPlainText().trimmed()));
      query->addBindValue
	(biblioteq_myqstring::escape(vg.accession_number->text().trimmed()));
      (void) qmain->populateTable
	(query, "Video Games", biblioteq::NEW_PAGE, biblioteq::POPULATE_SEARCH);
    }
}

void biblioteq_videogame::slotPopulateCopiesEditor(void)
{
  auto copyeditor = new biblioteq_copy_editor
    (qobject_cast<QWidget *> (this),
     qmain,
     static_cast<biblioteq_item *> (this),
     false,
     vg.quantity->value(),
     m_oid,
     vg.quantity,
     font(),
     "Video Game",
     vg.id->text().trimmed(),
     false);

  copyeditor->populateCopiesEditor();
}

void biblioteq_videogame::slotPrepareIcons(void)
{
  prepareIcons(this);
}

void biblioteq_videogame::slotPrint(void)
{
  m_html = "<html>";
  m_html += "<b>" + tr("UPC:") + "</b> " + vg.id->text().trimmed() + "<br>";
  m_html += "<b>" + tr("Game Rating:") + "</b> " +
    vg.rating->currentText() + "<br>";
  m_html += "<b>" + tr("Developers:") + "</b> " +
    vg.developer->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Platform:") + "</b> " +
    vg.platform->currentText() + "<br>";
  m_html += "<b>" + tr("Mode:") + "</b> " + vg.mode->currentText() + "<br>";

  /*
  ** General information.
  */

  m_html += "<b>" + tr("Title:") + "</b> " + vg.title->text().trimmed() +
    "<br>";
  m_html += "<b>" + tr("Release Date:") + "</b> " + vg.release_date->date().
    toString(Qt::ISODate) + "<br>";
  m_html += "<b>" + tr("Publisher:") + "</b> " +
    vg.publisher->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Place of Publication:") + "</b> " +
    vg.place->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Genre:") + "</b> " +
    vg.genre->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Price:") + "</b> " + vg.price->cleanText() + "<br>";
  m_html += "<b>" + tr("Language:") + "</b> " +
    vg.language->currentText() + "<br>";
  m_html += "<b>" + tr("Monetary Units:") + "</b> " +
    vg.monetary_units->currentText() + "<br>";
  m_html += "<b>" + tr("Copies:") + "</b> " + vg.quantity->text() + "<br>";
  m_html += "<b>" + tr("Location:") + "</b> " +
    vg.location->currentText() + "<br>";
  m_html += "<b>" + tr("Abstract:") + "</b> " +
    vg.description->toPlainText().trimmed() +
    "<br>";
  m_html += "<b>" + tr("Keywords:") + "</b> " +
    vg.keyword->toPlainText().trimmed() + "<br>";
  m_html += "<b>" + tr("Accession Number:") + "</b> " +
    vg.accession_number->text().trimmed();
  m_html += "</html>";
  print(this);
}

void biblioteq_videogame::slotPublicationDateEnabled(bool state)
{
  vg.release_date->setEnabled(state);

  if(!state)
    vg.release_date->setDate(QDate::fromString("2001", "yyyy"));
}

void biblioteq_videogame::slotQuery(void)
{
}

void biblioteq_videogame::slotReset(void)
{
  auto action = qobject_cast<QAction *> (sender());

  if(action != nullptr)
    {
      auto const actions = vg.resetButton->menu()->actions();

      if(actions.size() < 20)
	{
	  // Error.
	}
      else if(action == actions[0])
	vg.front_image->clear();
      else if(action == actions[1])
	vg.back_image->clear();
      else if(action == actions[2])
	{
	  vg.id->clear();
	  vg.id->setFocus();
	}
      else if(action == actions[3])
	{
	  vg.rating->setCurrentIndex(0);
	  vg.rating->setFocus();
	}
      else if(action == actions[4])
	{
	  if(m_engWindowTitle.contains("Search"))
	    vg.developer->clear();
	  else
	    vg.developer->setPlainText("N/A");

	  vg.developer->setFocus();
	}
      else if(action == actions[5])
	{
	  vg.platform->setCurrentIndex(0);
	  vg.platform->setFocus();
	}
      else if(action == actions[6])
	{
	  vg.mode->setCurrentIndex(0);
	  vg.mode->setFocus();
	}
      else if(action == actions[7])
	{
	  vg.title->clear();
	  vg.title->setFocus();
	}
      else if(action == actions[8])
	{
	  if(m_engWindowTitle.contains("Search"))
	    {
	      vg.publication_date_enabled->setChecked(false);
	      vg.release_date->setDate(QDate::fromString("2001", "yyyy"));
	    }
	  else
	    vg.release_date->setDate
	      (QDate::fromString("01/01/2000",
				 biblioteq::s_databaseDateFormat));

	  vg.release_date->setFocus();
	}
      else if(action == actions[9])
	{
	  if(m_engWindowTitle.contains("Search"))
	    vg.publisher->clear();
	  else
	    vg.publisher->setPlainText("N/A");

	  vg.publisher->setFocus();
	}
      else if(action == actions[10])
	{
	  if(m_engWindowTitle.contains("Search"))
	    vg.place->clear();
	  else
	    vg.place->setPlainText("N/A");

	  vg.place->setFocus();
	}
      else if(action == actions[11])
	{
	  if(m_engWindowTitle.contains("Search"))
	    vg.genre->clear();
	  else
	    vg.genre->setPlainText("N/A");

	  vg.genre->setFocus();
	}
      else if(action == actions[12])
	{
	  vg.price->setValue(vg.price->minimum());
	  vg.price->setFocus();
	}
      else if(action == actions[13])
	{
	  vg.language->setCurrentIndex(0);
	  vg.language->setFocus();
	}
      else if(action == actions[14])
	{
	  vg.monetary_units->setCurrentIndex(0);
	  vg.monetary_units->setFocus();
	}
      else if(action == actions[15])
	{
	  vg.quantity->setValue(vg.quantity->minimum());
	  vg.quantity->setFocus();
	}
      else if(action == actions[16])
	{
	  vg.location->setCurrentIndex(0);
	  vg.location->setFocus();
	}
      else if(action == actions[17])
	{
	  if(m_engWindowTitle.contains("Search"))
	    vg.description->clear();
	  else
	    vg.description->setPlainText("N/A");

	  vg.description->setFocus();
	}
      else if(action == actions[18])
	{
	  vg.keyword->clear();
	  vg.keyword->setFocus();
	}
      else if(action == actions[19])
	{
	  vg.accession_number->clear();
	  vg.accession_number->setFocus();
	}
    }
  else
    {
      /*
      ** Reset all.
      */

      vg.id->clear();
      vg.title->clear();

      if(m_engWindowTitle.contains("Search"))
	vg.developer->clear();
      else
	vg.developer->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	vg.publisher->clear();
      else
	vg.publisher->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	vg.place->clear();
      else
	vg.place->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	vg.genre->clear();
      else
	vg.genre->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	vg.description->clear();
      else
	vg.description->setPlainText("N/A");

      if(m_engWindowTitle.contains("Search"))
	{
	  vg.publication_date_enabled->setChecked(false);
	  vg.release_date->setDate(QDate::fromString("2001", "yyyy"));
	}
      else
	vg.release_date->setDate
	  (QDate::fromString("01/01/2000", biblioteq::s_databaseDateFormat));

      vg.accession_number->clear();
      vg.back_image->clear();
      vg.front_image->clear();
      vg.id->setFocus();
      vg.keyword->clear();
      vg.language->setCurrentIndex(0);
      vg.location->setCurrentIndex(0);
      vg.mode->setCurrentIndex(0);
      vg.monetary_units->setCurrentIndex(0);
      vg.platform->setCurrentIndex(0);
      vg.price->setValue(vg.price->minimum());
      vg.quantity->setValue(vg.quantity->minimum());
      vg.rating->setCurrentIndex(0);
    }
}

void biblioteq_videogame::slotSelectImage(void)
{
  QFileDialog dialog(this);
  auto button = qobject_cast<QPushButton *> (sender());

  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);

  if(button == vg.frontButton)
    dialog.setWindowTitle(tr("BiblioteQ: Front Cover Image Selection"));
  else
    dialog.setWindowTitle(tr("BiblioteQ: Back Cover Image Selection"));

  dialog.exec();
  QApplication::processEvents();

  if(dialog.result() == QDialog::Accepted)
    {
      if(button == vg.frontButton)
	{
	  vg.front_image->clear();
	  vg.front_image->m_image = QImage(dialog.selectedFiles().value(0));

	  if(dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
	    vg.front_image->m_imageFormat = dialog.selectedFiles().value(0).mid
	      (dialog.selectedFiles().value(0).lastIndexOf(".") + 1).
	      toUpper();

	  vg.front_image->scene()->addPixmap
	    (QPixmap::fromImage(vg.front_image->m_image));

	  if(!vg.front_image->scene()->items().isEmpty())
	    vg.front_image->scene()->items().at(0)->setFlags
	      (QGraphicsItem::ItemIsSelectable);

	  vg.front_image->scene()->setSceneRect
	    (vg.front_image->scene()->itemsBoundingRect());
	}
      else
	{
	  vg.back_image->clear();
	  vg.back_image->m_image = QImage(dialog.selectedFiles().value(0));

	  if(dialog.selectedFiles().value(0).lastIndexOf(".") > -1)
	    vg.back_image->m_imageFormat = dialog.selectedFiles().value(0).mid
	      (dialog.selectedFiles().value(0).lastIndexOf(".") + 1).
	      toUpper();

	  vg.back_image->scene()->addPixmap
	    (QPixmap::fromImage(vg.back_image->m_image));

	  if(!vg.back_image->scene()->items().isEmpty())
	    vg.back_image->scene()->items().at(0)->setFlags
	      (QGraphicsItem::ItemIsSelectable);

	  vg.back_image->scene()->setSceneRect
	    (vg.back_image->scene()->itemsBoundingRect());
	}
    }
}

void biblioteq_videogame::slotShowUsers(void)
{
  int state = 0;

  if(!vg.okButton->isHidden())
    state = biblioteq::EDITABLE;
  else
    state = biblioteq::VIEW_ONLY;

  auto borrowerseditor = new biblioteq_borrowers_editor
    (qobject_cast<QWidget *> (this),
     qmain,
     static_cast<biblioteq_item *> (this),
     vg.quantity->value(),
     m_oid,
     vg.id->text(),
     font(),
     "Video Game",
     state);

  borrowerseditor->showUsers();
}

void biblioteq_videogame::updateWindow(const int state)
{
  QString str = "";

  if(state == biblioteq::EDITABLE)
    {
      vg.backButton->setVisible(true);
      vg.copiesButton->setEnabled(true);
      vg.frontButton->setVisible(true);
      vg.okButton->setVisible(true);
      vg.queryButton->setVisible(m_isQueryEnabled);
      vg.resetButton->setVisible(true);
      vg.showUserButton->setEnabled(true);
      str = tr("BiblioteQ: Modify Video Game Entry (") +
	vg.id->text() +
	tr(")");
      m_engWindowTitle = "Modify";
    }
  else
    {
      vg.backButton->setVisible(false);
      vg.copiesButton->setVisible(false);
      vg.frontButton->setVisible(false);
      vg.okButton->setVisible(false);
      vg.queryButton->setVisible(false);
      vg.resetButton->setVisible(false);

      if(qmain->isGuest())
	vg.showUserButton->setVisible(false);
      else
	vg.showUserButton->setEnabled(true);

      str = tr("BiblioteQ: View Video Game Details (") +
	vg.id->text() +
	tr(")");
      m_engWindowTitle = "View";
    }

  vg.coverImages->setVisible(true);
  setReadOnlyFields(this, state != biblioteq::EDITABLE);
  setWindowTitle(str);
  emit windowTitleChanged(windowTitle());
}
