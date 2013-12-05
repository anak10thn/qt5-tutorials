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

#include <QtCore>
#include <QtGui>
#include <QtWebKit>

#if QT_VERSION <= 0x040500
#error You must use Qt >= 4.5
#endif

class S5Runner : public QWebView
{
    Q_OBJECT

public:

    S5Runner(QWidget *parent = 0)
            : QWebView(parent)
            , overlay(Qt::transparent)
            , invert(false) {

        connect(this, SIGNAL(titleChanged(const QString&)), SLOT(setWindowTitle(const QString&)));

        timer.start(60*1000);
        connect(&timer, SIGNAL(timeout()), SLOT(tick()));
        minutes = 30;
    }

private slots:
    void tick() {
        minutes--;
        update();
    }

private:

    QColor overlay;
    bool invert;
    QTimer timer;
    int minutes;

    void paintEvent(QPaintEvent *event) {
        if (overlay.alpha() != 0) {
            QPainter p(this);
            p.fillRect(event->rect(), overlay);
            p.end();
        } else {
            QWebView::paintEvent(event);
            QPainter p(this);
            if (page()->isContentEditable()) {
                QTextOption opts(Qt::AlignCenter);
                p.save();
                p.setOpacity(0.5);
                p.setPen(Qt::white);
                p.drawText(QRect(0, 0, width(), 25), "Edit Mode", opts);
                p.restore();
            }
            if (invert) {
                p.setCompositionMode(QPainter::CompositionMode_Difference);
                p.fillRect(event->rect(), Qt::white);
            }

            if (true) {
                QTextOption opts(Qt::AlignCenter);
                QRect corner(width() - 100, height() - 50, 50, 50);
                QFont f = font();
                f.setPixelSize(30);
                p.setFont(f);
                p.setOpacity(0.6);
                p.drawText(corner, QString::number(minutes), opts);
            }

            p.end();
        }
    }

    void keyPressEvent(QKeyEvent *event) {

        // Esc resets everything
        if (event->key() == Qt::Key_Escape) {
            page()->setContentEditable(false);
            overlay = Qt::transparent;
            showNormal();
            update();
            event->accept();
            return;
        }

        // F5 reloads the web content
        if (event->key() == Qt::Key_F5) {
            reload();
            event->accept();
            return;
        }

        // F3 allows editing
        if (event->key() == Qt::Key_F3) {
            page()->setContentEditable(!page()->isContentEditable());
            update();
            event->accept();
            return;
        }

        // F toggles full-screen
        if (!page()->isContentEditable() && event->key() == Qt::Key_F) {
            if (isFullScreen())
                showNormal();
            else
                showFullScreen();
            event->accept();
            return;
        }

        // B sets the screen to black
        if (!page()->isContentEditable() && event->key() == Qt::Key_B) {
            overlay = (overlay == Qt::black) ? Qt::transparent : Qt::black;
            update();
            event->accept();
            return;
        }

        // W sets the screen to white
        if (!page()->isContentEditable() && event->key() == Qt::Key_W) {
            overlay = (overlay == Qt::white) ? Qt::transparent : Qt::white;
            update();
            event->accept();
            return;
        }

        // N toggles night-mode
        if (!page()->isContentEditable() && event->key() == Qt::Key_N) {
            invert = !invert;
            update();
            event->accept();
            return;
        }

        // Navigation keys
        if (!page()->isContentEditable()) {
            int sendkey = -1;
            switch (event->key()) {
            case Qt::Key_Left:
                sendkey = 37;
                break;
            case Qt::Key_Right:
                sendkey = 39;
                break;
            default:
                break;
            }
            if (sendkey > 0) {
                QWebFrame *f = page()->mainFrame();
                QString trigger = QString("var k = { which: %1 }; keys(k)").arg(sendkey);
                f->evaluateJavaScript(trigger);
                event->accept();
                return;
            }
        } else {
            QWebView::keyPressEvent(event);
        }
    }

};

#include "s5runner.moc"

int main(int argc, char **argv)
{
#ifdef Q_WS_X11
    QApplication::setGraphicsSystem("raster");
#endif

    QApplication app(argc, argv);

    QString fname;
    if (argc == 2)
        fname = QString::fromLocal8Bit(argv[1]);
    else
        fname = QFileDialog::getOpenFileName(0, "Open Presentation");

    S5Runner w;
    w.resize(800, 600);
    w.load(QUrl::fromLocalFile(fname));
    w.show();

    return app.exec();
}
