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

#include <QInputDialog>
#include <QSettings>
#include <QSqlField>
#include <QSqlRecord>

#include "biblioteq.h"
#include "biblioteq_custom_query.h"
#include "biblioteq_otheroptions.h"
#include "biblioteq_sql_syntax_highlighter.h"
#include "biblioteq_woody.h"

biblioteq_custom_query::biblioteq_custom_query
(biblioteq_otheroptions *other, biblioteq *parent):QMainWindow(parent)
{
  cq.setupUi(this);
  connect(cq.clear,
	  SIGNAL(clicked(void)),
	  cq.query_te,
	  SLOT(clear(void)));
  connect(cq.close_pb,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(close(void)));
  connect(cq.comment_label,
	  SIGNAL(linkActivated(const QString &)),
	  this,
	  SIGNAL(linkActivated(const QString &)));
  connect(cq.delete_favorite,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotDeleteFavoriteQuery(void)));
  connect(cq.execute_pb,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotExecuteCustomQuery(void)));
  connect(cq.favorite,
	  SIGNAL(toggled(bool)),
	  this,
	  SLOT(slotCustomQueryFavorite(bool)));
  connect(cq.favorites,
	  SIGNAL(activated(int)),
	  this,
	  SLOT(slotLoadFavorite(void)));
  connect(cq.refresh_pb,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRefreshCustomQuery(void)));
  connect(cq.rename_favorite,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotRenameFavoriteQuery(void)));
  connect(cq.save,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSaveCustomQuery(void)));
  connect(other,
	  SIGNAL(saved(void)),
	  this,
	  SLOT(slotOtherOptionsChanged(void)));
  connect(parent,
	  SIGNAL(favoritesChanged(void)),
	  this,
	  SLOT(slotPopulateFavorites(void)));
  connect(parent,
	  SIGNAL(newFavorite(void)),
	  this,
	  SLOT(slotNewFavorite(void)));
  connect(this,
	  SIGNAL(favoritesChanged(void)),
	  parent,
	  SIGNAL(favoritesChanged(void)));
  connect(this,
	  SIGNAL(newFavorite(void)),
	  parent,
	  SIGNAL(newFavorite(void)));
  m_otheroptions = other;
  m_parent = parent;
  m_sqlSyntaxHighlighter = new biblioteq_sql_syntax_highlighter
    (cq.query_te->document());
  m_sqlSyntaxHighlighter->setKeywordsColors
    (m_otheroptions->customQueryColors());
  m_woody = new woody_collapse_expand_tool_button(cq.tables_t);
  slotPopulateFavorites();
  slotPrepareIcons();
  slotRefreshCustomQuery();
}

biblioteq_custom_query::~biblioteq_custom_query()
{
}

void biblioteq_custom_query::changeEvent(QEvent *event)
{
  if(event)
    switch(event->type())
      {
      case QEvent::LanguageChange:
	{
	  cq.retranslateUi(this);
	  QApplication::processEvents();
	  break;
	}
      default:
	{
	  break;
	}
      }

  QMainWindow::changeEvent(event);
}

void biblioteq_custom_query::closeEvent(QCloseEvent *event)
{
  QMainWindow::closeEvent(event);
  deleteLater();
}

void biblioteq_custom_query::slotCustomQueryFavorite(bool state)
{
  QSettings settings;

  if(cq.favorites->count() > 0 && state)
    settings.setValue("custom_query_favorite", cq.favorites->currentText());
  else
    settings.remove("custom_query_favorite");

  m_parent->prepareCustomQueryFavoriteShortcut();
  emit newFavorite();
}

void biblioteq_custom_query::slotDeleteFavoriteQuery(void)
{
  if(biblioteq::s_empty == cq.favorites->currentText())
    {
      cq.delete_favorite->animateNegatively(2500);
      return;
    }

  if(QMessageBox::
     question(this,
	      tr("BiblioteQ: Question"),
	      tr("Are you sure that you wish to delete the favorite %1?").
	      arg(cq.favorites->currentText()),
	      QMessageBox::No | QMessageBox::Yes,
	      QMessageBox::No) == QMessageBox::No)
    {
      QApplication::processEvents();
      return;
    }
  else
    QApplication::processEvents();

  QSettings settings;

  if(cq.favorite->isChecked())
    settings.remove("custom_query_favorite");

  settings.remove(QString("customqueries/%1").arg(cq.favorites->currentText()));

  if(settings.status() == QSettings::NoError)
    cq.delete_favorite->animate(2500);
  else
    cq.delete_favorite->animateNegatively(2500);

  emit favoritesChanged();
  m_parent->populateFavorites();
}

void biblioteq_custom_query::slotExecuteCustomQuery(void)
{
  m_parent->executeCustomQuery(this, cq.query_te->toPlainText().trimmed());
}

void biblioteq_custom_query::slotLoadFavorite(void)
{
  if(biblioteq::s_empty == cq.favorites->currentText())
    {
      cq.query_te->clear();
      return;
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QSettings settings;
  auto const favorite
    (settings.value("custom_query_favorite").toString().trimmed());

  cq.query_te->setPlainText
    (QString::
     fromUtf8(QByteArray::
	      fromBase64(settings.
			 value(QString("customqueries/%1").
			       arg(cq.favorites->currentText())).
			 toByteArray()).constData()));
  cq.favorite->blockSignals(true);

  if(cq.favorites->currentText() == favorite)
    cq.favorite->setChecked(true);
  else
    cq.favorite->setChecked(false);

  cq.favorite->blockSignals(false);
  QApplication::restoreOverrideCursor();
}

void biblioteq_custom_query::slotNewFavorite(void)
{
  QSettings settings;
  auto const favorite
    (settings.value("custom_query_favorite").toString().trimmed());

  cq.favorite->blockSignals(true);
  cq.favorite->setChecked(cq.favorites->currentText() == favorite);
  cq.favorite->blockSignals(false);
}

void biblioteq_custom_query::slotOtherOptionsChanged(void)
{
  m_sqlSyntaxHighlighter->setKeywordsColors
    (m_otheroptions->customQueryColors());
  cq.query_te->setPlainText(cq.query_te->toPlainText());
}

void biblioteq_custom_query::slotPopulateFavorites(void)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QSettings settings;
  QStringList list;
  auto const text(cq.favorites->currentText());

  cq.favorites->clear();
  settings.beginGroup("customqueries");

  foreach(auto const &key, settings.childKeys())
    if(!key.trimmed().isEmpty() && biblioteq::s_empty != key)
      {
	auto const k
	  (key.mid(0, static_cast<int> (biblioteq::Limits::FAVORITES_LENGTH)).
	   remove('\n').remove('\r'));

	list << k;
      }
    else
      settings.remove(key);

  if(!list.isEmpty())
    {
      std::sort(list.begin(), list.end());
      cq.favorite->setEnabled(true);
      cq.favorites->addItems(list);

      for(int i = 0; i < cq.favorites->count(); i++)
	if(cq.favorites->itemText(i) == text)
	  {
	    cq.favorites->setCurrentIndex(i);
	    break;
	  }
    }
  else
    {
      cq.favorite->blockSignals(true);
      cq.favorite->setChecked(false);
      cq.favorite->blockSignals(false);
      cq.favorite->setEnabled(false);
      cq.favorites->addItem(biblioteq::s_empty);
    }

  QApplication::restoreOverrideCursor();
  slotLoadFavorite();
}

void biblioteq_custom_query::slotPrepareIcons(void)
{
  QSettings setting;
  auto const index = setting.value
    ("otheroptions/display_icon_set_index", 0).toInt();

  if(index == 1)
    {
      // System.

      cq.clear->setIcon
	(QIcon::fromTheme("edit-clear", QIcon(":/16x16/eraser.png")));
      cq.close_pb->setIcon
	(QIcon::fromTheme("window-close", QIcon(":/16x16/cancel.png")));
      cq.delete_favorite->setIcon
	(QIcon::fromTheme("edit-delete", QIcon(":/16x16/eraser.png")));
      cq.execute_pb->setIcon
	(QIcon::fromTheme("dialog-ok", QIcon(":/16x16/ok.png")));
      cq.refresh_pb->setIcon
	(QIcon::fromTheme("view-refresh", QIcon(":/16x16/reload.png")));
      cq.save->setIcon
	(QIcon::fromTheme("document-save", QIcon(":/16x16/filesave.png")));
    }
  else
    {
      // Faenza.

      cq.clear->setIcon(QIcon(":/16x16/eraser.png"));
      cq.close_pb->setIcon(QIcon(":/16x16/cancel.png"));
      cq.delete_favorite->setIcon(QIcon(":/16x16/eraser.png"));
      cq.execute_pb->setIcon(QIcon(":/16x16/ok.png"));
      cq.refresh_pb->setIcon(QIcon(":/16x16/reload.png"));
      cq.save->setIcon(QIcon(":/16x16/filesave.png"));
    }
}

void biblioteq_custom_query::slotRefreshCustomQuery(void)
{
  if(!m_parent->getDB().isOpen())
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QSqlField field;
  QStringList list;
  QTreeWidgetItem *item1 = nullptr;
  QTreeWidgetItem *item2 = nullptr;

  cq.tables_t->clear();

  if(m_parent->getDB().driverName() == "QSQLITE")
    list << "book"
	 << "book_binding_types"
	 << "book_conditions"
	 << "book_copy_info"
	 << "book_files"
	 << "book_originality"
	 << "book_target_audiences"
	 << "cd"
	 << "cd_copy_info"
	 << "cd_formats"
	 << "cd_songs"
	 << "dvd"
	 << "dvd_aspect_ratios"
	 << "dvd_copy_info"
	 << "dvd_ratings"
	 << "dvd_regions"
	 << "grey_literature"
	 << "grey_literature_files"
	 << "grey_literature_types"
	 << "item_borrower"
	 << "journal"
	 << "journal_copy_info"
	 << "journal_files"
	 << "languages"
	 << "locations"
	 << "magazine"
	 << "magazine_copy_info"
	 << "magazine_files"
	 << "member"
	 << "member_history"
	 << "minimum_days"
	 << "monetary_units"
	 << "photograph"
	 << "photograph_collection"
	 << "return_reminders"
	 << "videogame"
	 << "videogame_copy_info"
	 << "videogame_platforms"
	 << "videogame_ratings";
  else
    list << "admin"
	 << "book"
	 << "book_binding_types"
	 << "book_conditions"
	 << "book_copy_info"
	 << "book_files"
	 << "book_originality"
	 << "book_target_audiences"
	 << "cd"
	 << "cd_copy_info"
	 << "cd_formats"
	 << "cd_songs"
	 << "dvd"
	 << "dvd_aspect_ratios"
	 << "dvd_copy_info"
	 << "dvd_ratings"
	 << "dvd_regions"
	 << "grey_literature"
	 << "grey_literature_files"
	 << "grey_literature_types"
	 << "item_borrower"
	 << "item_request"
	 << "journal"
	 << "journal_copy_info"
	 << "journal_files"
	 << "languages"
	 << "locations"
	 << "magazine"
	 << "magazine_copy_info"
	 << "magazine_files"
	 << "member"
	 << "member_history"
	 << "member_history_dnt"
	 << "minimum_days"
	 << "monetary_units"
	 << "photograph"
	 << "photograph_collection"
	 << "videogame"
	 << "videogame_copy_info"
	 << "videogame_platforms"
	 << "videogame_ratings";

  list.sort();
  cq.tables_t->setColumnCount(4);
  cq.tables_t->setHeaderLabels(QStringList()
			       << tr("Table Name")
			       << tr("Column")
			       << tr("Column Type")
			       << tr("NULL"));
  cq.tables_t->setSortingEnabled(false);

  for(int i = 0; i < list.size(); i++)
    {
      item1 = new QTreeWidgetItem(cq.tables_t);
      item1->setText(0, list[i]);

      QStringList names;
      auto const record(m_parent->getDB().record(list[i]));

      for(int j = 0; j < record.count(); j++)
	names << record.fieldName(j);

      std::sort(names.begin(), names.end());

      for(int j = 0; j < names.size(); j++)
	{
	  field = record.field(names.at(j));
	  item2 = new QTreeWidgetItem(item1);
	  item2->setText(1, names.at(j));
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	  item2->setText(2, QMetaType(field.metaType().id()).name());
#else
	  item2->setText(2, QVariant::typeToName(field.type()));
#endif

	  if(field.requiredStatus() == QSqlField::Required)
	    item2->setText(3, tr("No"));
	  else
	    item2->setText(3, "");
	}
    }

  for(int i = 0; i < cq.tables_t->columnCount() - 1; i++)
    cq.tables_t->resizeColumnToContents(i);

  cq.tables_t->setSortingEnabled(true);
  cq.tables_t->sortByColumn(0, Qt::AscendingOrder);

  if(!m_woody->isChecked())
    cq.tables_t->collapseAll();
  else
    cq.tables_t->expandAll();

  QApplication::restoreOverrideCursor();
}

void biblioteq_custom_query::slotRenameFavoriteQuery(void)
{
  if(biblioteq::s_empty == cq.favorites->currentText())
    {
      cq.rename_favorite->animateNegatively(2500);
      return;
    }

  QString name("");
  auto ok = true;

  name = QInputDialog::getText
    (this,
     tr("BiblioteQ: Rename Custom Query Favorite"),
     tr("Query Name"),
     QLineEdit::Normal,
     cq.favorites->currentText(),
     &ok).remove('\n').remove('\r').trimmed();

  if(!ok || biblioteq::s_empty == name || name.isEmpty())
    {
      cq.rename_favorite->animateNegatively(2500);
      return;
    }

  name = name.mid(0, static_cast<int> (biblioteq::Limits::FAVORITES_LENGTH));

  QSettings settings;
  auto const favorite
    (settings.value("custom_query_favorite").toString().trimmed());
  auto const value
    (settings.
     value(QString("customqueries/%1").arg(cq.favorites->currentText())));

  if(cq.favorites->currentText() == favorite)
    settings.setValue("custom_query_favorite", name);

  settings.remove(QString("customqueries/%1").arg(cq.favorites->currentText()));
  settings.setValue(QString("customqueries/%1").arg(name), value);

  if(settings.status() == QSettings::NoError)
    cq.rename_favorite->animate(2500);
  else
    cq.rename_favorite->animateNegatively(2500);

  slotPopulateFavorites();
  emit favoritesChanged();
  m_parent->populateFavorites();
}

void biblioteq_custom_query::slotSaveCustomQuery(void)
{
  if(cq.query_te->toPlainText().trimmed().isEmpty())
    {
      cq.save->animateNegatively(2500);
      return;
    }

  QString name("");
  auto ok = true;

  name = QInputDialog::getText(this,
			       tr("BiblioteQ: Custom Query Favorite"),
			       tr("Query Name"),
			       QLineEdit::Normal,
			       cq.favorites->currentText(),
			       &ok).remove('\n').remove('\r').trimmed();

  if(!ok || biblioteq::s_empty == name || name.isEmpty())
    {
      cq.save->animateNegatively(2500);
      return;
    }

  QSettings settings;

  settings.setValue
    (QString("customqueries/%1").
     arg(name.mid(0, static_cast<int> (biblioteq::Limits::FAVORITES_LENGTH))),
     cq.query_te->toPlainText().trimmed().toUtf8().toBase64());

  if(settings.status() == QSettings::NoError)
    cq.save->animate(2500);
  else
    cq.save->animateNegatively(2500);

  emit favoritesChanged();
  m_parent->populateFavorites();
}
