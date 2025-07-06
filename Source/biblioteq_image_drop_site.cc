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

#include "biblioteq_image_drop_site.h"
#include "biblioteq_misc_functions.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDropEvent>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeData>
#include <QUrl>

biblioteq_image_drop_site::biblioteq_image_drop_site(QWidget *parent):
  QGraphicsView(parent)
{
  m_doubleClickResizeEnabled = true;
  m_doubleClicked = false;
  m_image = QImage();
  m_imageFormat.clear();
  m_readOnly = false;
  setAcceptDrops(true);
}

QString biblioteq_image_drop_site::determineFormat
(const QByteArray &bytes) const
{
  return biblioteq_misc_functions::imageFormatGuess(bytes);
}

QString biblioteq_image_drop_site::determineFormat
(const QString &filename) const
{
  QByteArray bytes(10, 0);
  QFile file(filename);
  QString imgf("");

  if(file.open(QIODevice::ReadOnly) &&
     file.read(bytes.data(), bytes.length()) > 0)
    imgf = biblioteq_misc_functions::imageFormatGuess(bytes);

  if(imgf.isEmpty())
    {
      auto const ext(QFileInfo(filename).suffix());

      if(ext.isEmpty())
	imgf = "JPG";
      else
	imgf = ext.toUpper();
    }

  file.close();
  return imgf;
}

void biblioteq_image_drop_site::clear(void)
{
  m_doubleClicked = false;
  m_image = QImage();
  m_imageFormat.clear();
  scene()->clear();
  scene()->clearSelection();
}

void biblioteq_image_drop_site::dragEnterEvent(QDragEnterEvent *event)
{
  if(m_readOnly)
    {
      if(event)
	event->ignore();

      return;
    }

  QString filename("");

#if defined(Q_OS_WINDOWS)
  if(event)
    for(int i = 0; i < event->mimeData()->formats().size(); i++)
      if(event->mimeData()->formats()[i].toLower().contains("filename"))
	{
	  filename = QString
	    (event->mimeData()->data(event->mimeData()->formats()[i])).
	    trimmed();
	  break;
	}
#elif defined(Q_OS_MACOS)
  if(event)
    filename = event->mimeData()->urls().value(0).toLocalFile().trimmed();
#else
  if(event)
    filename = event->mimeData()->text().trimmed();
#endif

  if(event)
    {
      auto const f(determineFormat(filename));

      if(f == "BMP" || f == "JPEG" || f == "JPG" || f == "PNG")
	event->acceptProposedAction();
    }
}

void biblioteq_image_drop_site::dragLeaveEvent(QDragLeaveEvent *event)
{
  QGraphicsView::dragLeaveEvent(event);
}

void biblioteq_image_drop_site::dragMoveEvent(QDragMoveEvent *event)
{
  if(m_readOnly)
    {
      if(event)
	event->ignore();

      return;
    }

  QGraphicsView::dragMoveEvent(event);

  QString filename("");

#if defined(Q_OS_WINDOWS)
  if(event)
    for(int i = 0; i < event->mimeData()->formats().size(); i++)
      if(event->mimeData()->formats()[i].toLower().contains("filename"))
	{
	  filename = QString
	    (event->mimeData()->data(event->mimeData()->formats()[i])).
	    trimmed();
	  break;
	}
#elif defined(Q_OS_MACOS)
  if(event)
    filename = event->mimeData()->urls().value(0).toLocalFile().trimmed();
#else
  if(event)
    filename = event->mimeData()->text().trimmed();
#endif

  if(event)
    {
      auto const f(determineFormat(filename));

      if(f == "BMP" || f == "JPEG" || f == "JPG" || f == "PNG")
	event->acceptProposedAction();
    }
}

void biblioteq_image_drop_site::dropEvent(QDropEvent *event)
{
  if(m_readOnly)
    {
      if(event)
	event->ignore();

      return;
    }

  QString filename("");
  QString imgf("");

#if defined(Q_OS_WINDOWS)
  if(event)
    for(int i = 0; i < event->mimeData()->formats().size(); i++)
      if(event->mimeData()->formats()[i].toLower().contains("filename"))
	{
	  filename = QString
	    (event->mimeData()->data(event->mimeData()->formats()[i])).
	    trimmed();
	  break;
	}
#elif defined(Q_OS_MACOS)
  if(event)
    filename = event->mimeData()->urls().value(0).toLocalFile().trimmed();
#else
  if(event)
    {
      QUrl const url(event->mimeData()->text());

      filename = url.toLocalFile().trimmed();
    }
#endif

  imgf = determineFormat(filename);
  m_image = QImage(filename, imgf.toLatin1().data());

  if(!m_image.isNull())
    {
      if(event)
	event->acceptProposedAction();

      m_doubleClicked = false;
      m_imageFormat = imgf;
      scene()->clear();

      QPixmap pixmap;

      if(height() < m_image.height() || m_image.width() > width())
	{
	  pixmap = QPixmap::fromImage(m_image);

	  if(!pixmap.isNull())
	    pixmap = pixmap.scaled
	      (0.50 * size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
      else
	pixmap = QPixmap::fromImage(m_image);

      scene()->clear();

      if(pixmap.isNull())
	{
	  m_image = QImage(":/missing_image.png");
	  m_imageFormat = "PNG";
	  pixmap = QPixmap(":/missing_image.png");
	}

      scene()->addPixmap(pixmap);

      if(acceptDrops())
	if(!scene()->items().isEmpty())
	  scene()->items().at(0)->setFlags(QGraphicsItem::ItemIsSelectable);

      scene()->setSceneRect(scene()->itemsBoundingRect());
    }
}

void biblioteq_image_drop_site::enableDoubleClickResize(const bool state)
{
  m_doubleClickResizeEnabled = state;
}

void biblioteq_image_drop_site::keyPressEvent(QKeyEvent *event)
{
  if(event && m_readOnly == false)
    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
      if(scene()->selectedItems().isEmpty() == false)
	clear();

  QGraphicsView::keyPressEvent(event);
}

void biblioteq_image_drop_site::loadFromData(const QByteArray &bytes)
{
  QPixmap pixmap;

  m_doubleClicked = false;
  m_imageFormat = determineFormat(bytes);
  m_image.loadFromData(bytes, m_imageFormat.toLatin1().data());

  if(height() < m_image.height() || m_image.width() > width())
    {
      pixmap = QPixmap::fromImage(m_image);

      if(!pixmap.isNull())
	pixmap = pixmap.scaled
	  (0.50 * size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  else
    pixmap = QPixmap::fromImage(m_image);

  scene()->clear();

  if(pixmap.isNull())
    {
      m_image = QImage(":/missing_image.png");
      m_imageFormat = "PNG";
      pixmap = QPixmap(":/missing_image.png");
    }

  scene()->addPixmap(pixmap);

  if(acceptDrops())
    if(!scene()->items().isEmpty())
      scene()->items().at(0)->setFlags(QGraphicsItem::ItemIsSelectable);

  scene()->setSceneRect(scene()->itemsBoundingRect());
  emit imageChanged(m_image);
}

void biblioteq_image_drop_site::mouseDoubleClickEvent(QMouseEvent *event)
{
  QGraphicsView::mouseDoubleClickEvent(event);

  if(!m_doubleClickResizeEnabled)
    return;

  scene()->clear();

  if(!m_doubleClicked)
    {
      auto pixmap(QPixmap::fromImage(m_image));

      if(pixmap.isNull())
	pixmap = QPixmap(":/missing_image.png");

      scene()->addPixmap(pixmap);
    }
  else
    {
      auto pixmap(QPixmap::fromImage(m_image));

      if(!pixmap.isNull())
	pixmap = pixmap.scaled
	  (size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

      if(pixmap.isNull())
	pixmap = QPixmap(":/missing_image.png");

      scene()->addPixmap(pixmap);
    }

  m_doubleClicked = !m_doubleClicked;

  if(acceptDrops())
    if(!scene()->items().isEmpty())
      scene()->items().at(0)->setFlags(QGraphicsItem::ItemIsSelectable);

  scene()->setSceneRect(scene()->itemsBoundingRect());
}

void biblioteq_image_drop_site::setImage(const QImage &image)
{
  QByteArray bytes;
  QBuffer buffer(&bytes);
  QPixmap pixmap;

  m_doubleClicked = false;
  m_image = image;
  m_image.save(&buffer);
  m_imageFormat = determineFormat(bytes);

  if(height() < m_image.height() || m_image.width() > width())
    {
      pixmap = QPixmap::fromImage(m_image);

      if(!pixmap.isNull())
	pixmap = pixmap.scaled
	  (0.50 * size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  else
    pixmap = QPixmap::fromImage(m_image);

  scene()->clear();

  if(pixmap.isNull())
    {
      m_image = QImage(":/missing_image.png");
      m_imageFormat = "PNG";
      pixmap = QPixmap(":/missing_image.png");
    }

  scene()->addPixmap(pixmap);

  if(acceptDrops())
    if(!scene()->items().isEmpty())
      scene()->items().at(0)->setFlags(QGraphicsItem::ItemIsSelectable);

  emit imageChanged(m_image);
}

void biblioteq_image_drop_site::setImageFromClipboard(void)
{
  auto clipboard = QApplication::clipboard();

  if(clipboard)
    setImage(clipboard->image());
}

void biblioteq_image_drop_site::setReadOnly(const bool readOnly)
{
  m_readOnly = readOnly;
  setAcceptDrops(!readOnly);
}

void biblioteq_image_drop_site::update(void)
{
  auto item = qgraphicsitem_cast<QGraphicsPixmapItem *>
    (scene()->items().value(0));

  if(!item)
    return;

  item->setPixmap(QPixmap::fromImage(m_image));
}
