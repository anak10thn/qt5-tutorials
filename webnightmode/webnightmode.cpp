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

#if QT_VERSION < 0x0040500
#error You need Qt 4.5 or newer
#endif

class NightModeView : public QWebView
{
    Q_OBJECT

public:

    NightModeView(QWidget *parent = 0): QWebView(parent), invert(true) {
        connect(this, SIGNAL(titleChanged(const QString&)), SLOT(setWindowTitle(const QString&)));
        load(QUrl("http://maps.google.com"));
    }

private:

    bool invert;

    void paintEvent(QPaintEvent *event) {
        QWebView::paintEvent(event);
        if (invert) {
            QPainter p(this);
            p.setCompositionMode(QPainter::CompositionMode_Difference);
            p.fillRect(event->rect(), Qt::white);
            p.end();
        }
    }

    void keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_F3) {
            invert = !invert;
            update();
            event->accept();
            return;
        }
        QWebView::keyPressEvent(event);
    }

};

#include "webnightmode.moc"

int main(int argc, char **argv)
{
#ifdef Q_WS_X11
    QApplication::setGraphicsSystem("raster");
#endif

    QApplication app(argc, argv);

    NightModeView w;
    w.show();

    QMessageBox::information(&w, "Hint", "Use F3 to toggle night mode");

    return app.exec();
}
