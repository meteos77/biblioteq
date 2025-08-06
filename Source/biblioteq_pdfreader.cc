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
#include "biblioteq_misc_functions.h"
#include "biblioteq_pdfreader.h"

#include <QFileDialog>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QMatrix4x4>
#endif
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProgressDialog>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtMath>

#include <limits>

biblioteq_pdfreader::biblioteq_pdfreader(QWidget *parent):QMainWindow(parent)
{
  m_ui.setupUi(this);

  if(menuBar())
    menuBar()->setNativeMenuBar(true);

#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  m_document = nullptr;
#else
  m_ui.action_Contents->setEnabled(false);
  m_ui.action_Print->setEnabled(false);
  m_ui.action_Print_Preview->setEnabled(false);
  m_ui.action_Save_As->setEnabled(false);
  m_ui.case_sensitive->setEnabled(false);
  m_ui.find->setEnabled(false);
  m_ui.find_next->setEnabled(false);
  m_ui.find_previous->setEnabled(false);
  m_ui.page->setEnabled(false);
  m_ui.page_1->setText
    (tr("BiblioteQ was assembled without Poppler support."));
  m_ui.page_2->setText(m_ui.page_1->text());
#endif
  m_ui.splitter->setSizes
    (QList<int> () << 100 << std::numeric_limits<int>::max());
  m_ui.splitter->setStretchFactor(0, 0);
  m_ui.splitter->setStretchFactor(1, 1);
  connect(m_ui.action_Close,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotClose(void)));
  connect(m_ui.action_Contents,
	  SIGNAL(triggered(bool)),
	  this,
	  SLOT(slotShowContents(bool)));
  connect(m_ui.action_Find,
	  SIGNAL(triggered(void)),
	  m_ui.find,
	  SLOT(selectAll(void)));
  connect(m_ui.action_Find,
	  SIGNAL(triggered(void)),
	  m_ui.find,
	  SLOT(setFocus(void)));
  connect(m_ui.action_Print,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotPrint(void)));
  connect(m_ui.action_Print_Preview,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotPrintPreview(void)));
  connect(m_ui.action_Save_As,
	  SIGNAL(triggered(void)),
	  this,
	  SLOT(slotSaveAs(void)));
  connect(m_ui.contents,
	  SIGNAL(itemDoubleClicked(QListWidgetItem *)),
	  this,
	  SLOT(slotContentsDoubleClicked(QListWidgetItem *)));
  connect(m_ui.find,
	  SIGNAL(returnPressed(void)),
	  this,
	  SLOT(slotSearchNext(void)));
  connect(m_ui.find_next,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSearchNext(void)));
  connect(m_ui.find_previous,
	  SIGNAL(clicked(void)),
	  this,
	  SLOT(slotSearchPrevious(void)));
  connect(m_ui.page,
	  SIGNAL(valueChanged(int)),
	  this,
	  SLOT(slotShowPage(int)));
  connect(m_ui.scrollArea->verticalScrollBar(),
	  SIGNAL(actionTriggered(int)),
	  this,
	  SLOT(slotSliderTriggerAction(int)));
  connect(m_ui.view_size,
	  SIGNAL(currentIndexChanged(int)),
	  this,
	  SLOT(slotChangePageViewSize(int)));

  if(parent)
    connect(parent,
	    SIGNAL(fontChanged(const QFont &)),
	    this,
	    SLOT(setGlobalFonts(const QFont &)));

  biblioteq_misc_functions::center(this, qobject_cast<QMainWindow *> (parent));
  m_ui.contents->setVisible(false);
}

biblioteq_pdfreader::~biblioteq_pdfreader()
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
  delete m_document;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
  m_document ? m_document.reset() : static_cast<void> (0);
#endif
}

bool biblioteq_pdfreader::event(QEvent *event)
{
  if(event && event->type() == QEvent::KeyRelease)
    {
      auto keyEvent = dynamic_cast<QKeyEvent *> (event);

      if(keyEvent)
	switch(keyEvent->key())
	  {
	  case Qt::Key_Down:
	  case Qt::Key_PageDown:
	    {
	      if(!m_ui.scrollArea->verticalScrollBar()->isVisible() ||
		 m_ui.scrollArea->verticalScrollBar()->maximum() ==
		 m_ui.scrollArea->verticalScrollBar()->value())
		m_ui.page->setValue(m_ui.page->value() + 2);

	      break;
	    }
	  case Qt::Key_PageUp:
	  case Qt::Key_Up:
	    {
	      if(!m_ui.scrollArea->verticalScrollBar()->isVisible() ||
		 m_ui.scrollArea->verticalScrollBar()->minimum() ==
		 m_ui.scrollArea->verticalScrollBar()->value())
		m_ui.page->setValue(m_ui.page->value() - 2);

	      break;
	    }
	  }
    }

  return QMainWindow::event(event);
}

void biblioteq_pdfreader::changeEvent(QEvent *event)
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

void biblioteq_pdfreader::closeEvent(QCloseEvent *event)
{
  QMainWindow::closeEvent(event);
  deleteLater();
}

void biblioteq_pdfreader::keyPressEvent(QKeyEvent *event)
{
  if(event)
    {
      if(event->key() == Qt::Key_End)
	m_ui.page->setValue(m_ui.page->maximum());
      else if(event->key() == Qt::Key_Home)
	m_ui.page->setValue(m_ui.page->minimum());
    }

  QMainWindow::keyPressEvent(event);
}

void biblioteq_pdfreader::load(const QByteArray &data, const QString &fileName)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
  delete m_document;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
  m_document ? m_document.reset() : static_cast<void> (0);
#endif
  m_document = Poppler::Document::loadFromData(data);

  if(!m_document)
    {
      m_ui.action_Print->setEnabled(false);
      m_ui.action_Print_Preview->setEnabled(false);
      m_ui.action_Save_As->setEnabled(false);
      m_ui.page->setMaximum(1);
      m_ui.page_1->setText(tr("The PDF data could not be processed."));
      m_ui.page_2->setText(m_ui.page_1->text());
      return;
    }

  prepareContents();
  m_document->setRenderHint(Poppler::Document::Antialiasing, true);
  m_document->setRenderHint(Poppler::Document::TextAntialiasing, true);
  m_fileName = fileName.trimmed();
  m_ui.page->setMaximum(m_document->numPages());
  m_ui.page->setToolTip(tr("Page 1 of %1.").arg(m_ui.page->maximum()));

  if(fileName.trimmed().isEmpty())
    setWindowTitle(tr("BiblioteQ: PDF Reader"));
  else
    setWindowTitle(tr("BiblioteQ: PDF Reader (%1)").arg(fileName.trimmed()));

  slotShowPage(1);
#else
  Q_UNUSED(data);
  Q_UNUSED(fileName);
#endif
}

void biblioteq_pdfreader::load(const QString &fileName)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
  delete m_document;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
  m_document ? m_document.reset() : static_cast<void> (0);
#endif
  m_document = Poppler::Document::load(fileName);

  if(!m_document)
    {
      m_ui.action_Print->setEnabled(false);
      m_ui.action_Print_Preview->setEnabled(false);
      m_ui.action_Save_As->setEnabled(false);
      m_ui.page->setMaximum(1);
      m_ui.page_1->setText(tr("The PDF data could not be processed."));
      m_ui.page_2->setText(m_ui.page_1->text());
      return;
    }

  prepareContents();
  m_document->setRenderHint(Poppler::Document::Antialiasing, true);
  m_document->setRenderHint(Poppler::Document::TextAntialiasing, true);
  m_fileName = fileName.trimmed();
  m_ui.page->setMaximum(m_document->numPages());
  m_ui.page->setToolTip(tr("Page 1 of %1.").arg(m_ui.page->maximum()));

  if(fileName.trimmed().isEmpty())
    setWindowTitle(tr("BiblioteQ: PDF Reader"));
  else
    setWindowTitle(tr("BiblioteQ: PDF Reader (%1)").arg(fileName.trimmed()));

  slotShowPage(1);
#else
  Q_UNUSED(fileName);
#endif
}

void biblioteq_pdfreader::prepareContents(void)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document)
    return;

  m_ui.contents->clear();

  for(int i = 1; i <= m_document->numPages(); i++)
    {
      auto item = new QListWidgetItem(tr("Page %1").arg(i));

      item->setData(Qt::UserRole, i);
      m_ui.contents->addItem(item);
    }
#endif
}

void biblioteq_pdfreader::resizeEvent(QResizeEvent *event)
{
  if(m_ui.view_size->currentIndex() != 4)
    slotShowPage(m_ui.page->value());

  QMainWindow::resizeEvent(event);
}

void biblioteq_pdfreader::setGlobalFonts(const QFont &font)
{
  setFont(font);

  foreach(auto widget, findChildren<QWidget *> ())
    {
      widget->setFont(font);
      widget->update();
    }

  update();
}

void biblioteq_pdfreader::showNormal(void)
{
  QMainWindow::showNormal();
}

void biblioteq_pdfreader::slotChangePageViewSize(int value)
{
  Q_UNUSED(value);
  slotShowPage(m_ui.page->value());
}

void biblioteq_pdfreader::slotClose(void)
{
  close();
}

void biblioteq_pdfreader::slotContentsDoubleClicked(QListWidgetItem *item)
{
  if(item)
    m_ui.page->setValue(item->data(Qt::UserRole).toInt());
}

void biblioteq_pdfreader::slotPrint(void)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document)
    return;

  m_ui.menuBar->repaint();

  QPrinter printer;
  QScopedPointer<QPrintDialog> dialog(new QPrintDialog(&printer, this));

  dialog->setMinMax(1, m_document->numPages());
  printer.setColorMode(QPrinter::Color);
  printer.setDuplex(QPrinter::DuplexAuto);
  printer.setFromTo(1, m_document->numPages());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  printer.setPageSize(QPageSize(QPageSize::Letter));
#else
  printer.setPageSize(QPrinter::Letter);
#endif

  if(dialog->exec() == QDialog::Accepted)
    {
      QApplication::processEvents();

      QProgressDialog progress(this);

      progress.setLabelText(tr("Printing PDF..."));
      progress.setMaximum(0);
      progress.setMinimum(0);
      progress.setMinimumWidth
	(qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	       progress.sizeHint().width()));
      progress.setModal(true);
      progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
      biblioteq_misc_functions::center(&progress, this);
      progress.show();
      progress.repaint();
      QApplication::processEvents();

      QPainter painter(&printer);
      auto end = printer.toPage();
      auto start = printer.fromPage();

      if(end == 0 && start == 0)
	{
	  end = m_document->numPages();
	  start = 1;
	}

      for(int i = start; i <= end; i++)
	{
	  progress.repaint();
	  progress.setLabelText(tr("Printing PDF... Page %1...").arg(i));
	  QApplication::processEvents();

	  auto page = m_document->page(i - 1);

	  if(!page)
	    break;

	  auto const image
	    (page->renderToImage(printer.resolution(), printer.resolution()));
	  auto const rect(painter.viewport());
	  auto size(image.size());

	  size.scale(rect.size(), Qt::KeepAspectRatio);
	  painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
	  painter.setWindow(image.rect());
	  painter.drawImage(QPoint(0, 0), image);
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
	  delete page;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
	  page.reset();
#endif

	  if(i == end)
	    break;

	  if(progress.wasCanceled())
	    {
	      printer.abort();
	      break;
	    }

	  printer.newPage();
	}

      painter.end();
    }

  QApplication::processEvents();
#endif
}

void biblioteq_pdfreader::slotPrintPreview(QPrinter *printer)
{
  if(!printer)
    return;

#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document)
    return;

  QProgressDialog progress(this);
  auto preview = false;
  auto widget = qobject_cast<QWidget *> (sender());

  preview = !widget || !widget->isVisible();

  if(preview)
    progress.setLabelText(tr("Preparing preview..."));
  else
    progress.setLabelText(tr("Printing PDF..."));

  progress.setMaximum(0);
  progress.setMinimum(0);
  progress.setMinimumWidth
    (qCeil(biblioteq::PROGRESS_DIALOG_WIDTH_MULTIPLIER *
	   progress.sizeHint().width()));
  progress.setModal(true);
  progress.setWindowTitle(tr("BiblioteQ: Progress Dialog"));
  biblioteq_misc_functions::center(&progress, this);
  progress.show();
  progress.repaint();
  QApplication::processEvents();

  QPainter painter(printer);
  auto end = printer->toPage();
  auto start = printer->fromPage();

  if(end == 0 && start == 0)
    {
      end = m_document->numPages();
      start = 1;
    }

  painter.setRenderHints(QPainter::Antialiasing |
			 QPainter::SmoothPixmapTransform |
			 QPainter::TextAntialiasing);

  for(int i = start; i <= end; i++)
    {
      progress.repaint();

      if(preview)
	progress.setLabelText(tr("Preparing preview... Page %1...").arg(i));
      else
	progress.setLabelText(tr("Printing PDF... Page %1...").arg(i));

      QApplication::processEvents();

      auto page = m_document->page(i - 1);

      if(!page)
	break;

      auto const image
	(page->renderToImage(printer->resolution(), printer->resolution()));
      auto const rect(painter.viewport());
      auto size(image.size());

      size.scale(rect.size(), Qt::KeepAspectRatio);
      painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
      painter.setWindow(image.rect());
      painter.drawImage(QPoint(0, 0), image);
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
      delete page;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
      page.reset();
#endif

      if(i == end)
	break;

      if(progress.wasCanceled())
	{
	  printer->abort();
	  break;
	}

      printer->newPage();
    }

  painter.end();
#endif
}

void biblioteq_pdfreader::slotPrintPreview(void)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document)
    return;

  m_ui.menuBar->repaint();

  QPrinter printer;
  QScopedPointer<QPrintPreviewDialog> dialog
    (new QPrintPreviewDialog(&printer, this));

  connect(dialog.data(),
	  SIGNAL(paintRequested(QPrinter *)),
	  this,
	  SLOT(slotPrintPreview(QPrinter *)));
  printer.setColorMode(QPrinter::Color);
  printer.setDuplex(QPrinter::DuplexAuto);
  printer.setFromTo(1, m_document->numPages());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  printer.setPageSize(QPageSize(QPageSize::Letter));
#else
  printer.setPageSize(QPrinter::Letter);
#endif
  dialog->exec();
  QApplication::processEvents();
#endif
}

void biblioteq_pdfreader::slotSaveAs(void)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document)
    return;

  QFileDialog dialog(this);

  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setDirectory(QDir::homePath());
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  dialog.setWindowTitle(tr("BiblioteQ: Save PDF As"));
  dialog.selectFile(m_fileName);

  if(dialog.exec() == QDialog::Accepted)
    {
      repaint();
      QApplication::processEvents();
      QApplication::setOverrideCursor(Qt::WaitCursor);

      auto converter = m_document->pdfConverter();

      if(!converter)
	{
	  QApplication::restoreOverrideCursor();
	  return;
	}

      converter->setOutputFileName(dialog.selectedFiles().value(0));
      converter->convert();
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
      delete converter;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
      converter.reset();
#endif
      QApplication::restoreOverrideCursor();
    }

  QApplication::processEvents();
#endif
}

void biblioteq_pdfreader::slotSearchNext(void)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document || m_ui.find->text().isEmpty())
    {
      m_searchLocation = QRectF();
      slotShowPage(m_ui.page->value(), m_searchLocation);
      return;
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!m_searchLocation.isNull())
    m_searchLocation.setX(m_searchLocation.right());

  auto page = m_ui.page->value() - 1;

  while(page < m_document->numPages())
    {
      auto bottom = m_searchLocation.bottom();
      auto left = m_searchLocation.left();
      auto right = m_searchLocation.right();
      auto top = m_searchLocation.top();

      if(m_document->page(page)->
	 search(m_ui.find->text(),
		left,
		top,
		right,
		bottom,
		Poppler::Page::NextResult,
		m_ui.case_sensitive->isChecked() ?
		Poppler::Page::SearchFlag(0) : Poppler::Page::IgnoreCase))
	{
	  m_searchLocation = QRectF(left, top, right - left, bottom - top);

	  if(!m_searchLocation.isNull())
	    {
	      QApplication::restoreOverrideCursor();
	      slotShowPage(page + 1, m_searchLocation);
	      m_searchLocation.setX(right);
	      m_ui.find->setFocus();
	      m_ui.page->blockSignals(true);
	      m_ui.page->setValue(page + 1);
	      m_ui.page->blockSignals(false);
	      return;
	    }
	}

      m_searchLocation = QRectF();
      page += 1;
    }

  m_ui.find->setFocus();
  m_ui.page->setValue(1);
  QApplication::restoreOverrideCursor();
#endif
}

void biblioteq_pdfreader::slotSearchPrevious(void)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document || m_ui.find->text().isEmpty())
    {
      m_searchLocation = QRectF();
      slotShowPage(m_ui.page->value(), m_searchLocation);
      return;
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  if(!m_searchLocation.isNull())
    m_searchLocation.setX(m_searchLocation.right() - m_searchLocation.left());

  auto page = m_ui.page->value() - 1;

  while(page >= 0)
    {
      auto bottom = m_searchLocation.bottom();
      auto right = m_searchLocation.right();
      auto top = m_searchLocation.top();
      auto left = m_searchLocation.left();

      if(m_document->page(page)->
	 search(m_ui.find->text(),
		left,
		top,
		right,
		bottom,
		Poppler::Page::PreviousResult,
		m_ui.case_sensitive->isChecked() ?
		Poppler::Page::SearchFlag(0) : Poppler::Page::IgnoreCase))
	{
	  m_searchLocation = QRectF(left, top, right - left, bottom - top);

	  if(!m_searchLocation.isNull())
	    {
	      QApplication::restoreOverrideCursor();
	      slotShowPage(page + 1, m_searchLocation);
	      m_searchLocation.setX(m_searchLocation.right() -
				    m_searchLocation.left());
	      m_ui.find->setFocus();
	      m_ui.page->blockSignals(true);
	      m_ui.page->setValue(page + 1);
	      m_ui.page->blockSignals(false);
	      return;
	    }
	}

      m_searchLocation = QRectF
	(m_document->page(page)->pageSizeF().width(),
	 m_document->page(page)->pageSizeF().height(),
	 1,
	 1);
      page -= 1;
    }

  m_ui.find->setFocus();
  m_ui.page->setValue(1);
  QApplication::restoreOverrideCursor();
#endif
}

void biblioteq_pdfreader::slotShowContents(bool state)
{
  m_ui.contents->setVisible(state);
}

void biblioteq_pdfreader::slotShowPage(int value, const QRectF &location)
{
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER)
  if(!m_document)
    {
      m_searchLocation = QRectF();
      return;
    }
  else if(m_document->numPages() < value || value <= 0)
    {
      m_searchLocation = QRectF();
      return;
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  auto page = m_document->page(value - 1);

  if(!page)
    {
      m_searchLocation = QRectF();
      m_ui.page_1->setText(tr("The PDF data could not be processed."));
      m_ui.page_2->setText(m_ui.page_1->text());
      QApplication::restoreOverrideCursor();
      return;
    }

  m_ui.page->setToolTip
    (tr("Page %1 of %2.").arg(value).arg(m_ui.page->maximum()));

  QImage image;
  auto const pX = qMax(72, m_ui.page_1->physicalDpiX());
  auto const pY = qMax(72, m_ui.page_1->physicalDpiY());
  auto const scaleFactor = m_ui.view_size->currentText().remove("%").toInt() /
    100.0;

  if(m_ui.view_size->currentIndex() == 4)
    image = page->renderToImage(pX, pY);
  else
    image = page->renderToImage
      (scaleFactor * physicalDpiX(), scaleFactor * physicalDpiY());

  if(!location.isNull())
    {
      /*
      ** Highlight the discovered text.
      */

      QTransform const matrix(m_ui.view_size->currentIndex() != 4 ?
			      scaleFactor * physicalDpiX() / 72.0 :
			      physicalDpiX() / 72.0,
			      0,
			      0,
			      m_ui.view_size->currentIndex() != 4 ?
			      scaleFactor * physicalDpiY() / 72.0 :
			      physicalDpiY() / 72.0,
			      0,
			      0);
      auto highlightRect(matrix.mapRect(location).toRect());

      highlightRect.adjust(-2, -2, 2, 2);

      QPainter painter;
      auto const imageHighlight(image.copy(highlightRect));

      painter.begin(&image);
      painter.fillRect(image.rect(), QColor(0, 0, 0, 32));
      painter.drawImage(highlightRect, imageHighlight);
      painter.end();
    }

  m_ui.page_1->setFocus();
  m_ui.page_1->setPixmap(QPixmap::fromImage(image));
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
  delete page;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
  page.reset();
#endif
  page = m_document->page(value);

  if(!page)
    m_ui.page_2->setText(tr("The PDF data could not be processed."));
  else
    {
      if(m_ui.view_size->currentIndex() == 4)
	image = page->renderToImage(pX, pY);
      else
	image = page->renderToImage
	  (scaleFactor * physicalDpiX(), scaleFactor * physicalDpiY());

      m_ui.page_2->setPixmap(QPixmap::fromImage(image));
    }

#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER5)
  delete page;
#endif
#if defined(BIBLIOTEQ_LINKED_WITH_POPPLER6)
  page.reset();
#endif
  m_ui.contents->setCurrentRow(value - 1);
  QApplication::restoreOverrideCursor();
#else
  Q_UNUSED(location);
  Q_UNUSED(value);
#endif
}

void biblioteq_pdfreader::slotSliderTriggerAction(int action)
{
  if(action == QAbstractSlider::SliderSingleStepAdd &&
     m_ui.scrollArea->verticalScrollBar()->maximum() ==
     m_ui.scrollArea->verticalScrollBar()->value())
    {
      m_ui.page->setValue(m_ui.page->value() + 2);
      m_ui.scrollArea->verticalScrollBar()->setValue
	(m_ui.scrollArea->verticalScrollBar()->maximum());
    }
  else if(action == QAbstractSlider::SliderSingleStepSub &&
     m_ui.scrollArea->verticalScrollBar()->minimum() ==
     m_ui.scrollArea->verticalScrollBar()->value())
    {
      m_ui.page->setValue(m_ui.page->value() - 2);
      m_ui.scrollArea->verticalScrollBar()->setValue
	(m_ui.scrollArea->verticalScrollBar()->minimum());
    }
}
