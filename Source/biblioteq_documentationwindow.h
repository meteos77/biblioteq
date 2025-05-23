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

#ifndef _BIBLIOTEQ_DOCUMENTATIONWINDOW_H_
#define _BIBLIOTEQ_DOCUMENTATIONWINDOW_H_

#include <QMainWindow>

#include "ui_biblioteq_documentationwindow.h"

#ifdef BIBLIOTEQ_QT_PDF_SUPPORTED
class QPdfDocument;
class QPdfView;
#endif

class biblioteq_documentationwindow: public QMainWindow
{
  Q_OBJECT

 public:
  biblioteq_documentationwindow(QWidget *parent);
  biblioteq_documentationwindow(const QUrl &url, QWidget *parent);
  ~biblioteq_documentationwindow();
  void load(const QByteArray &data);
  void prepareIcons(void);
  void setAllowOpeningOfExternalLinks(const bool state);
  void setHtml(const QString &html);

 public slots:
  void show(void);

 private:
  QPalette m_originalFindPalette;
#ifdef BIBLIOTEQ_QT_PDF_SUPPORTED
  QPdfDocument *m_pdfDocument;
  QPdfView *m_pdfView;
#endif
  Ui_biblioteq_documentationwindow m_ui;
  bool m_openExternalLinks;
  void connectSignals(void);

 private slots:
  void slotAnchorClicked(const QUrl &url);
  void slotFind(void);
  void slotFindText(void);
  void slotPrint(void);
};

#endif
