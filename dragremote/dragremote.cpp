/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include <QtNetwork>
#include <QtWidgets>

class ImageViewer: public QWidget
{
Q_OBJECT

public:
    ImageViewer();
    void loadImage(const QString &file);
    void loadImage(const QImage &image);

protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent *event);
    void paintEvent(QPaintEvent*);

public slots:
    void handleNetworkData(QNetworkReply*);

private:
    QString m_fileName;
    QImage m_image;
    QNetworkAccessManager m_manager;
};

ImageViewer::ImageViewer(): QWidget(), m_manager(0)
{
    setAcceptDrops(true);

    setAttribute(Qt::WA_StaticContents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));
}

void ImageViewer::handleNetworkData(QNetworkReply *networkReply)
{
    QImage image;

    qDebug() << "Received" << networkReply->size() << "bytes";
    QUrl url = networkReply->url();
    if (networkReply->error()) {
        m_fileName = QString();
        qDebug() << "Can't download" << url.toString()
                 << ":" << networkReply->errorString();
    } else {
        m_fileName = url.toString();
        if (url.scheme() == "file")
            m_fileName = url.toLocalFile();
        image.load(networkReply, 0);
    }

    networkReply->deleteLater();
    loadImage(image);
}

void ImageViewer::loadImage(const QImage &image)
{
    m_image = image;
    if (m_image.isNull()) {
        setFixedSize(512, 256);
        setWindowTitle(QString("Can not load %1").arg(m_fileName));
    } else {
        QString title = "Image Viewer ";
        if ((m_image.width() > 1024) || (m_image.height() > 800)) {
            qDebug() << "Image is too large. Rescaling....";
            int w = m_image.width();
            m_image = m_image.scaled(640, 480, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            title += QString("[Zoom %1%] ").arg(m_image.width() * 100 / w);
        }
        setWindowTitle(QString("%1: %2 (%3 x %4)").arg(title).arg(m_fileName).
                       arg(m_image.width()).arg(m_image.height()));
        setFixedSize(m_image.width(), m_image.height());
    }
    update();
}

void ImageViewer::loadImage(const QString &fileName)
{
    m_fileName = QFileInfo(fileName).fileName();
    loadImage(QImage(fileName));
}

void ImageViewer::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void ImageViewer::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.count()) {
        QUrl url = urls[0];
        if (event->mimeData()->hasImage()) {
            QImage img = qvariant_cast<QImage>(event->mimeData()->imageData());
            m_fileName = url.toString();
            loadImage(img);
        } else {
            m_manager.get(QNetworkRequest(url));
            setWindowTitle(QString("Loading %1...").arg(url.toString()));
        }

        event->acceptProposedAction();
    }
}

void ImageViewer::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (!m_image.isNull())
        painter.drawImage(0, 0, m_image);
    else
        painter.fillRect(rect(), palette().window());
}

#include "dragremote.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ImageViewer widget;
    widget.show();
    widget.loadImage(":/paris.jpg");

    return app.exec();
}
