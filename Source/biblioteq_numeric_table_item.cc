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

#include <QDate>

#include "biblioteq_numeric_table_item.h"

biblioteq_numeric_table_item::biblioteq_numeric_table_item
(const QDate &date):
  QTableWidgetItem(QLocale().toString(date, QLocale::LongFormat))
{
  m_type = QVariant::Date;
}

biblioteq_numeric_table_item::biblioteq_numeric_table_item
(const double value):QTableWidgetItem(QString::number(value, 'f', 2))
{
  m_type = QVariant::Double;
}

biblioteq_numeric_table_item::biblioteq_numeric_table_item
(const int value):QTableWidgetItem(QString::number(value))
{
  m_type = QVariant::Int;
}

QVariant biblioteq_numeric_table_item::value(void) const
{
  return text();
}

bool biblioteq_numeric_table_item::operator <
(const QTableWidgetItem &other) const
{
  /*
  ** Ignore conversion failures.
  */

  switch(m_type)
    {
    case QVariant::Date:
      {
	auto const date1
	  (QDate::fromString(text(),
			     QLocale().dateFormat(QLocale::LongFormat)));
	auto const date2
	  (QDate::fromString(other.text(),
			     QLocale().dateFormat(QLocale::LongFormat)));

	return date1 < date2;
      }
    case QVariant::Double:
      return text().toDouble() < other.text().toDouble();
    default:
      return text().toInt() < other.text().toInt();
    }
}
