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
#include <QtWebKitWidgets>

#if QT_VERSION < 0x0050000
#error You need Qt 5.0.0 or newer
#endif

class Container : public QWidget
{
    Q_OBJECT

public:
    Container(QWidget *parent = 0): QWidget(parent) {
        view = new QWebView(this);

        QVBoxLayout *layout = new QVBoxLayout(this);
        setLayout(layout);
        layout->addWidget(view);

        QPalette palette = view->palette();
        palette.setBrush(QPalette::Base, Qt::transparent);
        view->page()->setPalette(palette);
        view->setAttribute(Qt::WA_OpaquePaintEvent, false);
        connect(view, SIGNAL(titleChanged(const QString&)), SLOT(setWindowTitle(const QString&)));

        view->load(QUrl("http://en.mobile.wikipedia.org/"));
        view->setZoomFactor(0.8);

        gradient.setColorAt(0.0, QColor(249, 247, 96));
        gradient.setColorAt(1.0, QColor(235, 203, 32));
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

        resize(320, 480);
    }

private:
    void paintEvent(QPaintEvent *event) {
        QPainter p(this);
        p.fillRect(event->rect(), Qt::transparent);
        p.setPen(Qt::NoPen);
        p.setBrush(gradient);
        p.setOpacity(0.6);
        p.drawRoundedRect(rect(), 10, 10);
        p.end();
    }

private:
    QWebView *view;
    QLinearGradient gradient;
};

#include "transparentweb.moc"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Container w;
    w.setAttribute(Qt::WA_TranslucentBackground, true);
    w.setWindowFlags(Qt::FramelessWindowHint);
    w.show();
    return app.exec();
}
