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
#include <QtWebKit>

#include "dragmovecharm.h"

QWidget *imageShow()
{
    QLabel *label = new QLabel;
    label->setText("<img src=':/pudding.jpg'>");
    label->adjustSize();
    label->setWindowTitle("Drag to move around");
    label->show();

    return label;
}

QWidget *miniBrowser()
{
    QDialog *widget = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(3, 15, 3, 3);
    widget->setLayout(layout);
    widget->setWindowFlags(Qt::FramelessWindowHint);
    widget->setSizeGripEnabled(true);
    widget->setWindowTitle("Drag to move around");
    widget->show();

    QTabWidget *tab = new QTabWidget(widget);
    QWebView *search = new QWebView(tab);
    search->load(QUrl("http://www.google.com/m?hl=en"));
    tab->addTab(search, "Search");
    QWebView *news = new QWebView(tab);
    news->load(QUrl("http://www.google.com/m/news?source=mobileproducts"));
    tab->addTab(news, "News");
    QWebView *bbc = new QWebView(tab);
    bbc->load(QUrl("http://news.bbc.co.uk/text_only.stm"));
    tab->addTab(bbc, "BBC");
    QWebView *fb = new QWebView(tab);
    fb->load(QUrl("http://iphone.facebook.com"));
    tab->addTab(fb, "Facebook");

    layout->addWidget(tab);
    widget->resize(350, 500);
    search->setFocus();

#if QT_VERSION >= 0x040500
    tab->setDocumentMode(true);
#endif

    return widget;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DragMoveCharm charm;
    charm.activateOn(imageShow());
    charm.activateOn(miniBrowser());

    return app.exec();
}

