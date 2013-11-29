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

#include "flickcharm.h"

#define ITEM_WIDTH 300
#define ITEM_HEIGHT 30

class TextItem: public QGraphicsItem
{
public:
    TextItem(const QString &str) {
        QStringList list = str.split(' ');
        str1 = list[0];
        str2 = list[1];
        font1 = QFont("Arial");
        font2 = QFont("Arial");
        font1.setBold(true);
        font1.setPixelSize(ITEM_HEIGHT / 2);
        font2.setPixelSize(ITEM_HEIGHT / 2);
        offset = QFontMetrics(font1).width(str1) + 15;
    }

    QRectF boundingRect() const {
        return QRectF(0, 0, ITEM_WIDTH, ITEM_HEIGHT);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget*) {
        if (option->state & QStyle::State_Selected) {
            painter->fillRect(boundingRect(), QColor(0, 128, 240));
            painter->setPen(Qt::white);
        } else {
            painter->setPen(Qt::lightGray);
            painter->drawRect(boundingRect());
            painter->setPen(Qt::black);
        }
        painter->setFont(font1);
        painter->drawText(QRect(10, 0, offset, ITEM_HEIGHT), Qt::AlignVCenter, str1);
        painter->setFont(font2);
        painter->drawText(QRect(offset, 0, ITEM_WIDTH, ITEM_HEIGHT), Qt::AlignVCenter,  str2);
    }

private:
    QFont font1;
    QFont font2;
    QString str1;
    QString str2;
    int offset;
};

// Returns a list of two-word color names
static QStringList colorPairs(int max)
{
    // capitalize the first letter
    QStringList colors = QColor::colorNames();
    colors.removeAll("transparent");
    int num = colors.count();
    for (int c = 0; c < num; ++c)
        colors[c] = colors[c][0].toUpper() + colors[c].mid(1);

    // combine two colors, e.g. "lime skyblue"
    QStringList combinedColors;
    for (int i = 0; i < num; ++i)
        for (int j = 0; j < num; ++j)
            combinedColors << QString("%1 %2").arg(colors[i]).arg(colors[j]);

    // randomize it
    colors.clear();
    while (combinedColors.count()) {
        int i = qrand() % combinedColors.count();
        colors << combinedColors[i];
        combinedColors.removeAt(i);
        if (colors.count() == max)
            break;
    }

    return colors;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList colors = colorPairs(5000);
    QGraphicsScene scene;
    scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    for (int i = 0; i < colors.count(); ++i) {
        TextItem *item = new TextItem(colors[i]);
        scene.addItem(item);
        item->setPos(0, i*ITEM_HEIGHT);
        item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
    scene.setItemIndexMethod(QGraphicsScene::BspTreeIndex);

    QGraphicsView canvas;
    canvas.setScene(&scene);
    canvas.setRenderHints(QPainter::TextAntialiasing);
    canvas.setFrameShape(QFrame::NoFrame);
    canvas.setWindowTitle("Flickable Canvas");
    canvas.show();

    QWebView web;
    web.setUrl(QUrl("http://news.google.com"));
    web.setWindowTitle("Flickable Web View");
    web.show();

    FlickCharm FlickCharm;
    FlickCharm.activateOn(&canvas);
    FlickCharm.activateOn(&web);

    return app.exec();
}
