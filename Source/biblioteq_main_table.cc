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
#include "biblioteq_main_table.h"

#include <QScrollBar>
#include <QSettings>

biblioteq_main_table::biblioteq_main_table(QWidget *parent):
  QTableWidget(parent)
{
  allowUtf8Printable
    (QSettings().value("otheroptions/only_utf8_printable_text", false).
     toBool());
  m_qmain = nullptr;
  horizontalHeader()->setSectionsMovable(true);
  horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
  horizontalHeader()->setSortIndicatorShown(true);
  horizontalHeader()->setStretchLastSection(true);
  prepareConnections();
  setAcceptDrops(false);
  setDragEnabled(false);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

QHash<QString, QString> biblioteq_main_table::friendlyStates(void) const
{
  QHash<QString, QString> states;
  QHashIterator<QString, QList<int> > it(m_hiddenColumns);

  while(it.hasNext())
    {
      it.next();

      QString state("");

      auto const list(it.value());

      for(int j = 0; j < list.size(); j++)
	state += QString::number(list.at(j)).append(",");

      if(state.endsWith(","))
	state = state.mid(0, state.length() - 1);

      states[it.key()] = state;
    }

  return states;
}

QStringList biblioteq_main_table::columnNames(void) const
{
  return m_columnHeaderIndexes.toList();
}

bool biblioteq_main_table::isColumnHidden
(const int index, const QString &type, const QString &username) const
{
  QString indexstr("");
  auto lType(type);

  indexstr.append(username);
  indexstr.append(lType.replace(' ', '_'));
  indexstr.append("_header_state");
  return m_hiddenColumns.value(indexstr).contains(index);
}

bool biblioteq_main_table::isColumnHidden(int index) const
{
  return QTableWidget::isColumnHidden(index);
}

int biblioteq_main_table::columnNumber(const QString &name) const
{
  auto const index = m_columnHeaderIndexes.indexOf(name);

  if(index >= 0)
    return index;

  for(int i = 0; i < m_columnHeaderIndexes.size(); i++)
    if(QString::compare(m_columnHeaderIndexes.at(i),
			name,
			Qt::CaseInsensitive) == 0)
      return i;

  return index;
}

void biblioteq_main_table::allowUtf8Printable(const bool state)
{
  if(state)
    connect(this,
	    SIGNAL(itemChanged(QTableWidgetItem *)),
	    this,
	    SLOT(slotItemChanged(QTableWidgetItem *)),
	    Qt::UniqueConnection);
  else
    disconnect(this,
	       SIGNAL(itemChanged(QTableWidgetItem *)),
	       this,
	       SLOT(slotItemChanged(QTableWidgetItem *)));
}

void biblioteq_main_table::keyPressEvent(QKeyEvent *event)
{
  if(event)
    switch(event->key())
      {
      case Qt::Key_Backspace:
      case Qt::Key_Delete:
	{
	  emit deleteKeyPressed();
	  break;
	}
      case Qt::Key_Enter:
      case Qt::Key_Return:
	{
	  emit enterKeyPressed();
	  break;
	}
      default:
	{
	  break;
	}
      }

  QTableWidget::keyPressEvent(event);
}

void biblioteq_main_table::parseStates(const QHash<QString, QString> &states)
{
  m_hiddenColumns.clear();

  QHashIterator<QString, QString> it(states);

  while(it.hasNext())
    {
      it.next();

      QList<int> intList;
      auto const strList
	(it.value().split(",",
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
			  Qt::SkipEmptyParts
#else
			  QString::SkipEmptyParts
#endif
			  ));

      for(int j = 0; j < strList.size(); j++)
	intList.append(qMax(0, strList.at(j).toInt()));

      m_hiddenColumns[it.key()] = intList;
    }
}

void biblioteq_main_table::prepareConnections(void)
{
  disconnect(this,
	     SIGNAL(cellChanged(int, int)),
	     this,
	     SLOT(slotCellChanged(int, int)));

  QSettings settings;

  if(settings.value("otheroptions/enable_special_values_colors").toBool())
    connect(this,
	    SIGNAL(cellChanged(int, int)),
	    this,
	    SLOT(slotCellChanged(int, int)));
}

void biblioteq_main_table::recordColumnHidden(const QString &username,
					      const QString &type,
					      const int index,
					      const bool hidden)
{
  QString indexstr("");
  auto lType(type);

  indexstr.append(username);
  indexstr.append(lType.replace(" ", "_"));
  indexstr.append("_header_state");

  if(hidden)
    {
      if(!m_hiddenColumns[indexstr].contains(index))
	m_hiddenColumns[indexstr].append(index);
    }
  else if(m_hiddenColumns.contains(indexstr))
    m_hiddenColumns[indexstr].removeAll(index);
}

void biblioteq_main_table::resetTable(const QString &username,
				      const QString &type,
				      const QString &roles)
{
  horizontalScrollBar()->setValue(0);
  scrollToTop();
  setColumnCount(0);
  setColumns(username, type, roles);
  setRowCount(0);

  if(m_qmain && m_qmain->setting("automatically_resize_column_widths").toBool())
    {
      for(int i = 0; i < columnCount() - 1; i++)
	resizeColumnToContents(i);

      horizontalHeader()->setStretchLastSection(true);
    }

  clearSelection();
  horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
  horizontalHeader()->setSortIndicatorShown(true);
  setCurrentItem(nullptr);
  sortByColumn(0, Qt::AscendingOrder);
}

void biblioteq_main_table::setColumnNames(const QStringList &list)
{
  m_columnHeaderIndexes.clear();

  for(int i = 0; i < list.size(); i++)
    m_columnHeaderIndexes.append(list.at(i));
}

void biblioteq_main_table::setColumns(const QString &username,
				      const QString &t,
				      const QString &roles)
{
  QStringList list;
  auto type(t.trimmed());

  m_columnHeaderIndexes.clear();

  if(type.isEmpty())
    type = "All";

  if(type == "All" ||
     type == "All Available" ||
     type == "All Overdue" ||
     type == "All Requested" ||
     type == "All Reserved")
    {
      if(type == "All Overdue" || type == "All Reserved")
	{
	  if(!roles.isEmpty())
	    {
	      list.append(tr("Borrower"));
	      list.append(tr("Member ID"));
	      list.append(tr("Contact Information"));
	      m_columnHeaderIndexes.append("Borrower");
	      m_columnHeaderIndexes.append("Member ID");
	      m_columnHeaderIndexes.append("Contact Information");
	    }

	  list.append(tr("Barcode"));
	  list.append(tr("Reservation Date"));
	  list.append(tr("Due Date"));
	  m_columnHeaderIndexes.append("Barcode");
	  m_columnHeaderIndexes.append("Reservation Date");
	  m_columnHeaderIndexes.append("Due Date");
	}
      else if(type == "All Requested")
	{
	  if(!roles.isEmpty())
	    {
	      list.append(tr("Borrower"));
	      list.append(tr("Member ID"));
	      list.append(tr("Contact Information"));
	      m_columnHeaderIndexes.append("Borrower");
	      m_columnHeaderIndexes.append("Member ID");
	      m_columnHeaderIndexes.append("Contact Information");
	    }

	  list.append(tr("Request Date"));
	  m_columnHeaderIndexes.append("Request Date");
	}

      list.append(tr("Title"));
      list.append(tr("ID Number"));

      if(type == "All Overdue" ||
	 type == "All Requested" ||
	 type == "All Reserved")
	list.append(tr("Call Number"));

      list.append(tr("Publisher"));
      list.append(tr("Publication Date"));
      list.append(tr("Categories"));
      list.append(tr("Language"));
      list.append(tr("Price"));
      list.append(tr("Monetary Units"));
      list.append(tr("Quantity"));
      list.append(tr("Location"));
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("ID Number");

      if(type == "All Overdue" ||
	 type == "All Requested" ||
	 type == "All Reserved")
	m_columnHeaderIndexes.append("Call Number");

      m_columnHeaderIndexes.append("Publisher");
      m_columnHeaderIndexes.append("Publication Date");
      m_columnHeaderIndexes.append("Categories");
      m_columnHeaderIndexes.append("Language");
      m_columnHeaderIndexes.append("Price");
      m_columnHeaderIndexes.append("Monetary Units");
      m_columnHeaderIndexes.append("Quantity");
      m_columnHeaderIndexes.append("Location");

      if(type != "All Requested")
	{
	  list.append(tr("Availability"));
	  list.append(tr("Total Reserved"));
	  m_columnHeaderIndexes.append("Availability");
	  m_columnHeaderIndexes.append("Total Reserved");
	}

      list.append(tr("Accession Number"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Accession Number");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");

      if(type == "All Requested")
	{
	  list.append("REQUESTOID");
	  m_columnHeaderIndexes.append("REQUESTOID");
	}
    }
  else if(type == "Books")
    {
      list.append(tr("Title"));
      list.append(tr("Series Title"));
      list.append(tr("Authors"));
      list.append(tr("Publisher"));
      list.append(tr("Publication Date"));
      list.append(tr("Place of Publication"));
      list.append(tr("Edition"));
      list.append(tr("Categories"));
      list.append(tr("Language"));
      list.append(tr("ISBN-10"));
      list.append(tr("Price"));
      list.append(tr("Monetary Units"));
      list.append(tr("Quantity"));
      list.append(tr("Book Binding Type"));
      list.append(tr("Location"));
      list.append(tr("ISBN-13"));
      list.append(tr("LC Control Number"));
      list.append(tr("Call Number"));
      list.append(tr("Dewey Class Number"));
      list.append(tr("Availability"));
      list.append(tr("Total Reserved"));
      list.append(tr("Originality"));
      list.append(tr("Condition"));
      list.append(tr("Accession Number"));
      list.append(tr("Alternate Identifier"));
      list.append(tr("Target Audience"));
      list.append(tr("Volume Number"));
      list.append(tr("Reform Date"));
      list.append(tr("Origin"));
      list.append(tr("Purchase Date"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("Series Title");
      m_columnHeaderIndexes.append("Authors");
      m_columnHeaderIndexes.append("Publisher");
      m_columnHeaderIndexes.append("Publication Date");
      m_columnHeaderIndexes.append("Place of Publication");
      m_columnHeaderIndexes.append("Edition");
      m_columnHeaderIndexes.append("Categories");
      m_columnHeaderIndexes.append("Language");
      m_columnHeaderIndexes.append("ISBN-10");
      m_columnHeaderIndexes.append("Price");
      m_columnHeaderIndexes.append("Monetary Units");
      m_columnHeaderIndexes.append("Quantity");
      m_columnHeaderIndexes.append("Book Binding Type");
      m_columnHeaderIndexes.append("Location");
      m_columnHeaderIndexes.append("ISBN-13");
      m_columnHeaderIndexes.append("LC Control Number");
      m_columnHeaderIndexes.append("Call Number");
      m_columnHeaderIndexes.append("Dewey Class Number");
      m_columnHeaderIndexes.append("Availability");
      m_columnHeaderIndexes.append("Total Reserved");
      m_columnHeaderIndexes.append("Originality");
      m_columnHeaderIndexes.append("Condition");
      m_columnHeaderIndexes.append("Accession Number");
      m_columnHeaderIndexes.append("Alternate Identifier");
      m_columnHeaderIndexes.append("Target Audience");
      m_columnHeaderIndexes.append("Volume Number");
      m_columnHeaderIndexes.append("Reform Date");
      m_columnHeaderIndexes.append("Origin");
      m_columnHeaderIndexes.append("Purchase Date");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");
    }
  else if(type == "DVDs")
    {
      list.append(tr("Title"));
      list.append(tr("Format"));
      list.append(tr("Studio"));
      list.append(tr("Release Date"));
      list.append(tr("Number of Discs"));
      list.append(tr("Runtime"));
      list.append(tr("Categories"));
      list.append(tr("Language"));
      list.append(tr("UPC"));
      list.append(tr("Price"));
      list.append(tr("Monetary Units"));
      list.append(tr("Quantity"));
      list.append(tr("Location"));
      list.append(tr("Rating"));
      list.append(tr("Region"));
      list.append(tr("Aspect Ratio"));
      list.append(tr("Availability"));
      list.append(tr("Total Reserved"));
      list.append(tr("Accession Number"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("Format");
      m_columnHeaderIndexes.append("Studio");
      m_columnHeaderIndexes.append("Release Date");
      m_columnHeaderIndexes.append("Number of Discs");
      m_columnHeaderIndexes.append("Runtime");
      m_columnHeaderIndexes.append("Categories");
      m_columnHeaderIndexes.append("Language");
      m_columnHeaderIndexes.append("UPC");
      m_columnHeaderIndexes.append("Price");
      m_columnHeaderIndexes.append("Monetary Units");
      m_columnHeaderIndexes.append("Quantity");
      m_columnHeaderIndexes.append("Location");
      m_columnHeaderIndexes.append("Rating");
      m_columnHeaderIndexes.append("Region");
      m_columnHeaderIndexes.append("Aspect Ratio");
      m_columnHeaderIndexes.append("Availability");
      m_columnHeaderIndexes.append("Total Reserved");
      m_columnHeaderIndexes.append("Accession Number");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");
    }
  else if(type == "Grey Literature")
    {
      list.append(tr("Authors"));
      list.append(tr("Clients"));
      list.append(tr("Document Code A"));
      list.append(tr("Document Code B"));
      list.append(tr("Document Date"));
      list.append(tr("Document ID"));
      list.append(tr("Document Status"));
      list.append(tr("Title"));
      list.append(tr("Document Type"));
      list.append(tr("Job Number"));
      list.append(tr("Location"));
      list.append(tr("File Count"));
      list.append(tr("Availability"));
      list.append(tr("Total Reserved"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Authors");
      m_columnHeaderIndexes.append("Clients");
      m_columnHeaderIndexes.append("Document Code A");
      m_columnHeaderIndexes.append("Document Code B");
      m_columnHeaderIndexes.append("Document Date");
      m_columnHeaderIndexes.append("Document ID");
      m_columnHeaderIndexes.append("Document Status");
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("Document Type");
      m_columnHeaderIndexes.append("Job Number");
      m_columnHeaderIndexes.append("Location");
      m_columnHeaderIndexes.append("File Count");
      m_columnHeaderIndexes.append("Availability");
      m_columnHeaderIndexes.append("Total Reserved");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");
    }
  else if(type == "Journals" || type == "Magazines")
    {
      list.append(tr("Title"));
      list.append(tr("Publisher"));
      list.append(tr("Publication Date"));
      list.append(tr("Place of Publication"));
      list.append(tr("Volume"));
      list.append(tr("Issue"));
      list.append(tr("Categories"));
      list.append(tr("Language"));
      list.append(tr("ISSN"));
      list.append(tr("Price"));
      list.append(tr("Monetary Units"));
      list.append(tr("Quantity"));
      list.append(tr("Location"));
      list.append(tr("LC Control Number"));
      list.append(tr("Call Number"));
      list.append(tr("Dewey Number"));
      list.append(tr("Availability"));
      list.append(tr("Total Reserved"));
      list.append(tr("Accession Number"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("Publisher");
      m_columnHeaderIndexes.append("Publication Date");
      m_columnHeaderIndexes.append("Place of Publication");
      m_columnHeaderIndexes.append("Volume");
      m_columnHeaderIndexes.append("Issue");
      m_columnHeaderIndexes.append("Categories");
      m_columnHeaderIndexes.append("Language");
      m_columnHeaderIndexes.append("ISSN");
      m_columnHeaderIndexes.append("Price");
      m_columnHeaderIndexes.append("Monetary Units");
      m_columnHeaderIndexes.append("Quantity");
      m_columnHeaderIndexes.append("Location");
      m_columnHeaderIndexes.append("LC Control Number");
      m_columnHeaderIndexes.append("Call Number");
      m_columnHeaderIndexes.append("Dewey Number");
      m_columnHeaderIndexes.append("Availability");
      m_columnHeaderIndexes.append("Total Reserved");
      m_columnHeaderIndexes.append("Accession Number");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");
    }
  else if(type == "Music CDs")
    {
      list.append(tr("Title"));
      list.append(tr("Artist"));
      list.append(tr("Format"));
      list.append(tr("Recording Label"));
      list.append(tr("Release Date"));
      list.append(tr("Number of Discs"));
      list.append(tr("Runtime"));
      list.append(tr("Categories"));
      list.append(tr("Language"));
      list.append(tr("Catalog Number"));
      list.append(tr("Price"));
      list.append(tr("Monetary Units"));
      list.append(tr("Quantity"));
      list.append(tr("Location"));
      list.append(tr("Audio"));
      list.append(tr("Recording Type"));
      list.append(tr("Availability"));
      list.append(tr("Total Reserved"));
      list.append(tr("Accession Number"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("Artist");
      m_columnHeaderIndexes.append("Format");
      m_columnHeaderIndexes.append("Recording Label");
      m_columnHeaderIndexes.append("Release Date");
      m_columnHeaderIndexes.append("Number of Discs");
      m_columnHeaderIndexes.append("Runtime");
      m_columnHeaderIndexes.append("Categories");
      m_columnHeaderIndexes.append("Language");
      m_columnHeaderIndexes.append("Catalog Number");
      m_columnHeaderIndexes.append("Price");
      m_columnHeaderIndexes.append("Monetary Units");
      m_columnHeaderIndexes.append("Quantity");
      m_columnHeaderIndexes.append("Location");
      m_columnHeaderIndexes.append("Audio");
      m_columnHeaderIndexes.append("Recording Type");
      m_columnHeaderIndexes.append("Availability");
      m_columnHeaderIndexes.append("Total Reserved");
      m_columnHeaderIndexes.append("Accession Number");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");
    }
  else if(type == "Photograph Collections")
    {
      list.append(tr("Title"));
      list.append(tr("ID"));
      list.append(tr("Location"));
      list.append(tr("Photograph Count"));
      list.append(tr("About"));
      list.append(tr("Accession Number"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("ID");
      m_columnHeaderIndexes.append("Location");
      m_columnHeaderIndexes.append("Photograph Count");
      m_columnHeaderIndexes.append("About");
      m_columnHeaderIndexes.append("Accession Number");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");
    }
  else if(type == "Video Games")
    {
      list.append(tr("Title"));
      list.append(tr("Game Rating"));
      list.append(tr("Platform"));
      list.append(tr("Mode"));
      list.append(tr("Publisher"));
      list.append(tr("Release Date"));
      list.append(tr("Place of Publication"));
      list.append(tr("Genres"));
      list.append(tr("Language"));
      list.append(tr("UPC"));
      list.append(tr("Price"));
      list.append(tr("Monetary Units"));
      list.append(tr("Quantity"));
      list.append(tr("Location"));
      list.append(tr("Availability"));
      list.append(tr("Total Reserved"));
      list.append(tr("Accession Number"));
      list.append(tr("Type"));
      list.append("MYOID");
      m_columnHeaderIndexes.append("Title");
      m_columnHeaderIndexes.append("Game Rating");
      m_columnHeaderIndexes.append("Platform");
      m_columnHeaderIndexes.append("Mode");
      m_columnHeaderIndexes.append("Publisher");
      m_columnHeaderIndexes.append("Release Date");
      m_columnHeaderIndexes.append("Place of Publication");
      m_columnHeaderIndexes.append("Genres");
      m_columnHeaderIndexes.append("Language");
      m_columnHeaderIndexes.append("UPC");
      m_columnHeaderIndexes.append("Price");
      m_columnHeaderIndexes.append("Monetary Units");
      m_columnHeaderIndexes.append("Quantity");
      m_columnHeaderIndexes.append("Location");
      m_columnHeaderIndexes.append("Availability");
      m_columnHeaderIndexes.append("Total Reserved");
      m_columnHeaderIndexes.append("Accession Number");
      m_columnHeaderIndexes.append("Type");
      m_columnHeaderIndexes.append("MYOID");
    }

  if(m_qmain &&
     m_qmain->getDB().driverName() == "QSQLITE" &&
     m_qmain->showBookReadStatus() &&
     type == "Books")
    {
      list.prepend(tr("Read Status"));
      m_columnHeaderIndexes.prepend("Read Status");
    }

  setColumnCount(list.size());
  QTableWidget::setHorizontalHeaderLabels(list);

  if(type != "All" &&
     type != "All Available" &&
     type != "All Overdue" &&
     type != "All Requested" &&
     type != "All Reserved" &&
     type != "Custom")
    {
      /*
      ** Hide the Type and MYOID columns.
      */

      setColumnHidden(list.size() - 1, true);
      setColumnHidden(list.size() - 2, true);
    }
  else if(type != "Custom")
    {
      /*
      ** Hide the MYOID and REQUESTOID columns.
      */

      setColumnHidden(list.size() - 1, true);

      if(type == "All Requested")
	setColumnHidden(list.size() - 2, true);
    }

  QString indexstr("");
  auto lType(type);

  indexstr.append(username);
  indexstr.append(lType.replace(" ", "_"));
  indexstr.append("_header_state");

  for(int i = 0; i < m_hiddenColumns[indexstr].size(); i++)
    setColumnHidden(m_hiddenColumns[indexstr][i], true);
}

void biblioteq_main_table::setHorizontalHeaderLabels(const QStringList &labels)
{
  QTableWidget::setHorizontalHeaderLabels(labels);

  if(Q_UNLIKELY(!m_qmain))
    return;

  QSettings settings;

  if(!settings.value("otheroptions/enable_special_values_colors").toBool())
    return;

  QMapIterator<QPair<QString, QString>, QColor> it
    (m_qmain->specialValueColors());

  while(it.hasNext())
    {
      it.next();

      if(Q_UNLIKELY(!it.value().isValid()))
	continue;

      auto const pair(it.key());

      if(Q_UNLIKELY(pair.first.isEmpty() || pair.second.isEmpty()))
	continue;

      for(int i = 0; i < columnCount(); i++)
	{
	  auto header = horizontalHeaderItem(i);

	  if(Q_UNLIKELY(!header))
	    continue;

	  if(header->text().trimmed() == pair.second) // Column title.
	    {
	      for(int j = 0; j < rowCount(); j++)
		{
		  auto item = this->item(j, i);

		  if(Q_LIKELY(item) && item->text().trimmed() == pair.first)
		    item->setBackground(it.value());
		}

	      break;
	    }
	}
    }
}

void biblioteq_main_table::setQMain(biblioteq *biblioteq)
{
  m_qmain = biblioteq;
}

void biblioteq_main_table::slotCellChanged(int row, int column)
{
  if(Q_UNLIKELY(column < 0 || m_qmain == nullptr || row < 0))
    return;

  auto header = horizontalHeaderItem(column);

  if(Q_UNLIKELY(!header)) // Custom query?
    return;

  auto item = this->item(row, column);

  if(Q_UNLIKELY(!item))
    return;

  auto const color
    (m_qmain->specialValueColors().
     value(qMakePair(item->text().trimmed(), header->text().trimmed())));

  if(color.isValid())
    item->setBackground(color);
}

void biblioteq_main_table::slotItemChanged(QTableWidgetItem *item)
{
  if(Q_UNLIKELY(!item))
    return;

  blockSignals(true);
  item->setText(qUtf8Printable(item->text()));
  blockSignals(false);
}

void biblioteq_main_table::updateToolTips(const int row)
{
  if(row < 0)
    return;

  QSettings settings;
  auto const showToolTips = settings.value
    ("show_maintable_tooltips", false).toBool();

  if(!showToolTips)
    return;

  QString tooltip("<html>");

  for(int i = 0; i < columnCount(); i++)
    {
      auto columnName(columnNames().value(i));
      auto item = this->item(row, i);

      if(columnName.isEmpty())
	columnName = "N/A";

      tooltip.append("<b>");
      tooltip.append(columnName);
      tooltip.append(":</b> ");

      if(item)
	tooltip.append(item->text().trimmed());
      else
	tooltip.append("");

      tooltip.append("<br>");
    }

  if(tooltip.endsWith("<br>"))
    tooltip = tooltip.mid(0, tooltip.length() - 4);

  tooltip.append("</html>");

  for(int i = 0; i < columnCount(); i++)
    {
      auto item = this->item(row, i);

      if(item)
	item->setToolTip(tooltip);
    }
}
