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

// for the explanation of the trick, check out:
// http://www.virtualdub.org/blog/pivot/entry.php?id=116
// http://www.compuphase.com/graphic/scale3.htm
#define AVG(a,b)  ( ((((a)^(b)) & 0xfefefefeUL) >> 1) + ((a)&(b)) )

// assume that the source image is ARGB32 formatted
QImage cheatScaled(int width, int height, const QImage &source)
{
    Q_ASSERT(source.format() == QImage::Format_ARGB32);

    QImage dest(width, height, QImage::Format_ARGB32);

    int sw = source.width();
    int sh = source.height();
    int xs = (sw << 8) / width;
    int ys = (sh << 8) / height;
    quint32 *dst = reinterpret_cast<quint32*>(dest.bits());
    int stride = dest.bytesPerLine() >> 2;

    for (int y = 0, yi = ys >> 2; y < height; ++y, yi += ys, dst += stride) {
        const quint32 *src1 = reinterpret_cast<const quint32*>(source.scanLine(yi >> 8));
        const quint32 *src2 = reinterpret_cast<const quint32*>(source.scanLine((yi + ys / 2) >> 8));
        for (int x = 0, xi1 = xs / 4, xi2 = xs * 3 / 4; x < width; ++x, xi1 += xs, xi2 += xs) {
            quint32 pixel1 = AVG(src1[xi1 >> 8], src1[xi2 >> 8]);
            quint32 pixel2 = AVG(src2[xi1 >> 8], src2[xi2 >> 8]);
            dst[x] = AVG(pixel1, pixel2);
        }
    }

    return dest;
}

class ThumbView: public QWidget
{
    Q_OBJECT

private:
    int m_method;
    QNetworkAccessManager m_manager;
    QImage m_fastScaled;
    QImage m_smoothScaled;
    QImage m_cheatScaled;

public:
    ThumbView(): QWidget(), m_method(0) {
        setAcceptDrops(true);
        connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(handleNetworkData(QNetworkReply*)));

        setWindowTitle("Drag and drop an image here!");
        resize(512, 256);
    }

    void loadImage(const QImage &image) {
        QImage img = image.convertToFormat(QImage::Format_ARGB32);
        if (img.isNull()) {
            resize(512, 256);
            setWindowTitle("Can't load the image. Please drag and drop an new image.");
        } else {
            int ww = 200;
            int hh = 150;

            m_fastScaled = img.scaled(ww, hh, Qt::IgnoreAspectRatio, Qt::FastTransformation);
            m_smoothScaled = img.scaled(ww, hh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            m_cheatScaled = cheatScaled(ww, hh, img);

            resize(20 + ww * 2 + 50, hh * 2 + 30 + 2 * 40);
        }
        update();
    }

public slots:

    void handleNetworkData(QNetworkReply *networkReply) {
        m_fastScaled = QImage();
        m_smoothScaled = QImage();
        m_cheatScaled = QImage();

        QUrl url = networkReply->url();
        if (networkReply->error()) {
            setWindowTitle(QString("Can't download %1: %2").arg(url.toString()).arg(networkReply->errorString()));
        } else {
            QImage image;
            image.load(networkReply, 0);
            QString fileName = QFileInfo(url.path()).fileName();
            setWindowTitle(QString("%1 - press a key to switch the scaling method").arg(fileName));
            loadImage(image);
        }

        networkReply->deleteLater();
    }

protected:

    void dragEnterEvent(QDragEnterEvent *event) {
        if (event->mimeData()->hasFormat("text/uri-list"))
            event->acceptProposedAction();
    }

    void dropEvent(QDropEvent *event) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.count()) {
            QUrl url = urls[0];
            if (event->mimeData()->hasImage()) {
                QImage img = qvariant_cast<QImage>(event->mimeData()->imageData());
                QString fileName = QFileInfo(url.path()).fileName();
                setWindowTitle(QString("%1 - press 1-4 to choose the scaling method").arg(fileName));
                loadImage(img);
            } else {
                m_manager.get(QNetworkRequest(url));
                setWindowTitle(QString("Loading %1...").arg(url.toString()));
            }

            event->acceptProposedAction();
        }
    }

    void keyPressEvent(QKeyEvent* event) {

        if (event->key() == Qt::Key_Space) {
            m_method = (m_method + 1) % 3;
            update();
            event->accept();
            return;
        }

        if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_3) {
            m_method = event->key() - Qt::Key_1;
            update();
            event->accept();
            return;
        }

    }

    void paintEvent(QPaintEvent*) {
        if (m_fastScaled.isNull())
            return;

        int w = m_fastScaled.width();
        int h = m_fastScaled.height();

        QPainter painter;
        painter.begin(this);

        // top left image: fast
        painter.translate(10, 40);
        painter.setPen(Qt::black);
        painter.drawText(0, -40, w, 40, Qt::AlignCenter, "[1] Qt::FastTransformation");
        if (m_method == 0) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(-2, -2, w + 4, h + 4);
            painter.drawLine(w + 2, h + 2, w + 50 - 10, h + 50 - 10);
        }
        painter.drawImage(0, 0, m_fastScaled);

        // top right image: smooth
        painter.translate(w + 50, 0);
        painter.setPen(Qt::black);
        painter.drawText(0, -40, w, 40, Qt::AlignCenter, "[2] Qt::SmoothTransformation");
        if (m_method == 1) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(-2, -2, w + 4, h + 4);
            painter.drawLine(w / 2, h + 2, w / 2, h + 50 - 10);
        }
        painter.drawImage(0, 0, m_smoothScaled);

        // bottom left image: cheat scaling with medium quality
        painter.translate(-w - 50, h + 50);
        painter.setPen(Qt::black);
        painter.drawText(0, -40, w, 40, Qt::AlignCenter, "[3] Cheat Scaled");
        if (m_method == 2) {
            painter.setPen(QPen(Qt::red, 2));
            painter.drawRect(-2, -2, w + 4, h + 4);
            painter.drawLine(w + 2, h / 2, w + 50 - 10, h / 2);
        }
        painter.drawImage(0, 0, m_cheatScaled);

        // bottom right image: chosen by the user
        painter.translate(w + 50, 0);
        QImage img = (m_method == 0) ? m_fastScaled :
                     (m_method == 1) ? m_smoothScaled : m_cheatScaled;
        painter.drawImage(0, 0, img);

        painter.end();
    }
};

#include "thumbview.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ThumbView widget;
    widget.show();

    return app.exec();
}
