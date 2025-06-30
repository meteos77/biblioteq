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

#ifndef _BIBLIOTEQ_BOOK_H_
#define _BIBLIOTEQ_BOOK_H_

#include "biblioteq_item.h"
#include "biblioteq_marc.h"
#include "ui_biblioteq_bookinfo.h"
#include "ui_biblioteq_passwordPrompt.h"

#include <QBuffer>
#include <QNetworkReply>
#include <QPointer>

class biblioteq_generic_thread;

class biblioteq_book: public QMainWindow, public biblioteq_item
{
  Q_OBJECT

 public:
  biblioteq_book(biblioteq *parentArg,
		 const QString &oidArg,
		 const QModelIndex &index);
  ~biblioteq_book();

  QString fancyTitleForTab(void) const
  {
    auto const title(id.title->text().trimmed());

    if(title.isEmpty())
      return windowTitle();
    else
      return QString(tr("Book (%1)").arg(title));
  }

  void duplicate(const QString &p_oid, const int state);
  void insert(void);
  void modify(const int state);
  void search(const QString &field = "", const QString &value = "");

  void setPublicationDateFormat(const QString &dateFormat)
  {
    id.publication_date->setDisplayFormat(dateFormat);
  }

  void updateWindow(const int);

 private:
  enum class Columns
    {
      COMPRESSED_SIZE = 2,
      DESCRIPTION = 3,
      DIGEST = 1,
      FILE = 0,
      MYOID = 4
    };

  QBuffer m_imageBuffer;
  QByteArray m_openLibraryResults;
  QByteArray m_sruResults;
  QDialog *m_proxyDialog;
  QNetworkAccessManager *m_imageManager;
  QNetworkAccessManager *m_openLibraryManager;
  QNetworkAccessManager *m_sruManager;
  QPalette m_te_orig_pal;
  QPalette m_white_pal;
  QPointer<biblioteq_generic_thread> m_thread;
  QPointer<biblioteq_item_working_dialog> m_openLibraryWorking;
  QPointer<biblioteq_item_working_dialog> m_sruWorking;
  QString m_cb_orig_ss;
  QString m_dt_orig_ss;
  QString m_engWindowTitle;
  Ui_informationDialog id;
  Ui_passwordDialog ui_p;
  bool m_duplicate;
  biblioteq_item_working_dialog *createImageDownloadDialog
    (const QString &downloadType);
  void changeEvent(QEvent *event);
  void closeEvent(QCloseEvent *event);
  void createFile(const QByteArray &bytes,
		  const QByteArray &digest,
		  const QString &fileName) const;
  void createOpenLibraryDialog(void);
  void createSRUDialog(void);
  void populateAfterOpenLibrary(void);
  void populateAfterSRU(const QString &recordSyntax, const QString &text);
  void populateAfterZ3950(const QString &text,
			  const biblioteq_marc::RECORD_SYNTAX recordSyntax);
  void populateFiles(void);
  void prepareFavorites(void);
  void resetQueryHighlights(void);

 private slots:
  void downloadFinished(void);
  void openLibraryDownloadFinished(void);
  void setGlobalFonts(const QFont &font);
  void slotAttachFiles(void);
  void slotCancel(void);
  void slotCancelImageDownload(void);
  void slotConvertISBN10to13(void);
  void slotConvertISBN13to10(void);
  void slotDataTransferProgress(qint64 bytesread, qint64 totalbytes);
  void slotDatabaseEnumerationsCommitted(void);
  void slotDeleteFiles(void);
  void slotDownloadFinished(void);
  void slotDownloadImage(void);
  void slotExportFiles(void);
  void slotFilesDoubleClicked(QTableWidgetItem *item);
  void slotGo(void);
  void slotOpenLibraryCanceled(void);
  void slotOpenLibraryDownloadFinished(bool error);
  void slotOpenLibraryDownloadFinished(void);
  void slotOpenLibraryError(QNetworkReply::NetworkError error);
  void slotOpenLibraryQuery(void);
  void slotOpenLibraryQueryError(const QString &text);
  void slotOpenLibraryReadyRead(void);
  void slotOpenLibrarySslErrors(const QList<QSslError> &list);
  void slotParseMarcTags(void);
  void slotPasteImage(void);
  void slotPopulateCopiesEditor(void);
  void slotPrepareIcons(void);
  void slotPrint(void);
  void slotPrintAuthorTitleDewey(void);
  void slotPrintCallDewey(void);
  void slotProxyAuthenticationRequired
    (const QNetworkProxy &proxy, QAuthenticator *authenticator);
  void slotPublicationDateEnabled(bool state);
  void slotReadyRead(void);
  void slotReset(void);
  void slotSRUCanceled(void);
  void slotSRUDownloadFinished(bool error);
  void slotSRUDownloadFinished(void);
  void slotSRUError(QNetworkReply::NetworkError error);
  void slotSRUQuery(void);
  void slotSRUQueryError(const QString &text);
  void slotSRUReadyRead(void);
  void slotSRUSslErrors(const QList<QSslError> &list);
  void slotSelectImage(void);
  void slotShowPDF(void);
  void slotShowUsers(void);
  void slotZ3950Query(void);
  void sruDownloadFinished(void);

 signals:
  void imageChanged(const QImage &image);
  void openLibraryQueryError(const QString &text);
  void sruQueryError(const QString &text);
  void windowTitleChanged(const QString &text);
};

#endif
