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
#include <QGraphicsObject>
#include <QAbstractScrollArea>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QApplication>
#include <QPushButton>
#include <math.h>

#define SLIDE_WIDTH 200
#define SLIDE_HEIGHT 150
#define SLIDE_SPACING 25

#define ITEM_WIDTH 300
#define ITEM_HEIGHT 30

class GenieItem: public QGraphicsRectItem
{
public:

    GenieItem(): QGraphicsRectItem(), shape(0) {
    }

    ~GenieItem() {
        delete [] shape;
    }

    void setPixmap(const QPixmap &pm) {
        sourceImage = pm.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    // frame goes from 0..255
    void animate(int frame) {

        effectImage.fill(0);

        int sw = sourceImage.width();
        int sh = sourceImage.height();
        int fh = effectImage.height();

        int dp = qMin(255, 2 * frame);
        int y1 = (fh * frame) >> 8;
        int y2 = ((fh - sh) * dp) >> 8;
        int yr = y2 - y1 + sh;
        if (yr <= 0)
            return;
        int dy = (sh << 8) / yr;

        const quint32 *bits = reinterpret_cast<quint32*>(sourceImage.bits());
        int words = sourceImage.bytesPerLine() / 4;
        quint32 *dest = reinterpret_cast<quint32*>(effectImage.scanLine(y1));
        int stride = effectImage.bytesPerLine() / 4;
        int *ptr = shape + y1 * 2;

        int dx = 0;

        for (int sy = 0; yr; --yr, sy += dy, dest += stride) {
            const quint32 *src = bits + (sy >> 8) * words;
            int x1 = (*ptr++ * dp) >> 8;
            int x2 = (*ptr++ * dp) >> 8;
            quint32 *q = dest + x1;
            int xr = x2 - x1 + sw;
#if 0
            // enable this block to optimize the division to be carried out
            // approximately for every 8 rows only
            // this gives less perfect scaling
            if (!dx || !(yr & 7))
#endif
            dx = (sw << 8) / xr;
            for (int sx = 0; xr; --xr, sx += dx)
                *q++ = src[sx >> 8];
        }

        update();
    }

    // prepare the tables which define the genie shape
    void setup() {
        int hh = (int)(rect().height());
        int ww = (int)(rect().width());
        effectImage = QImage(ww, hh, QImage::Format_ARGB32_Premultiplied);

        delete [] shape;
        shape = new int[hh*2];

        qreal y1 = 1 - 1 / (1 + exp(-3));
        qreal y2 = 1 - 1 / (1 + exp(3));
        for (int i = 0; i < rect().height(); i++) {
            qreal x = (qreal)i * 6 / rect().height() - 3;
            qreal y = 1 - 1 / (1 + exp(x));
            qreal ya = (y - y1) / (y2 - y1);
            shape[i * 2] = (int)(ya * effectImage.width());
            shape[i * 2 + 1] = (int)(ya * (effectImage.width() - sourceImage.width()));
        }
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem* option, QWidget*) {
        Q_UNUSED(option);
        painter->drawImage(0, 0, effectImage);
    }

private:
    QImage sourceImage;
    QImage effectImage;
    int *shape;
};

class GenieDemo : public QGraphicsView
{
    Q_OBJECT

public:
    GenieDemo();

public slots:
    void slideLeft();
    void slideRight();
    void trash();

protected:
    void keyPressEvent(QKeyEvent*);

private slots:
    void moveSlides(int);
    void updateGenieEffect(int);
    void finishGenie();

private:
    QGraphicsScene m_scene;
    QList<QGraphicsPixmapItem*> m_items;
    QTimeLine m_slidingTimeLine;
    int m_index;
    QPushButton *m_trashButton;
    GenieItem *m_genieItem;
    QTimeLine m_genieTimeLine;
};

GenieDemo::GenieDemo(): QGraphicsView(), m_index(0)
{
    setScene(&m_scene);

    m_items += m_scene.addPixmap(QPixmap(":/benedikte.jpg"));
    m_items += m_scene.addPixmap(QPixmap(":/castle.jpg"));
    m_items += m_scene.addPixmap(QPixmap(":/cookies.jpg"));
    m_items += m_scene.addPixmap(QPixmap(":/laptops.jpg"));
    m_items += m_scene.addPixmap(QPixmap(":/plasma.jpg"));
    m_items += m_scene.addPixmap(QPixmap(":/screenie.jpg"));
    m_items += m_scene.addPixmap(QPixmap(":/tagliatelle.jpg"));

    for (int i = 0; i < m_items.count(); ++i)
        m_items[i]->setZValue(0);

    connect(&m_slidingTimeLine, SIGNAL(frameChanged(int)), this, SLOT(moveSlides(int)));
    m_slidingTimeLine.setCurveShape(QTimeLine::EaseInOutCurve);
    m_slidingTimeLine.setDuration(150);

    m_trashButton = new QPushButton;
    m_trashButton->setText("Delete");
    m_scene.addWidget(m_trashButton);
    connect(m_trashButton, SIGNAL(clicked()), this, SLOT(trash()));

    m_genieItem = new GenieItem;
    m_genieItem->setZValue(1);
    m_scene.addItem(m_genieItem);
    m_genieItem->setVisible(false);

    connect(&m_genieTimeLine, SIGNAL(frameChanged(int)), this, SLOT(updateGenieEffect(int)));
    connect(&m_genieTimeLine, SIGNAL(finished()), this, SLOT(finishGenie()));
    m_genieTimeLine.setFrameRange(0, 255);
    m_genieTimeLine.setCurveShape(QTimeLine::EaseOutCurve);
    m_genieTimeLine.setDuration(800);

    setFixedSize(SLIDE_WIDTH * 2, SLIDE_HEIGHT + 200);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    moveSlides(0);
}

void GenieDemo::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        slideLeft();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Right) {
        slideRight();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Delete) {
        trash();
        event->accept();
        return;
    }
}

static int offsetFor(int index)
{
    return -index * (SLIDE_WIDTH + SLIDE_SPACING);
}

void GenieDemo::slideLeft()
{
    if (m_slidingTimeLine.state() != QTimeLine::NotRunning)
        return;

    if (m_genieTimeLine.state() != QTimeLine::NotRunning)
        return;

    if (m_index >= m_items.count() - 1)
        return;

    m_slidingTimeLine.setFrameRange(offsetFor(m_index), offsetFor(m_index + 1));
    m_slidingTimeLine.start();

    m_index++;
}

void GenieDemo::slideRight()
{
    if (m_slidingTimeLine.state() != QTimeLine::NotRunning)
        return;

    if (m_genieTimeLine.state() != QTimeLine::NotRunning)
        return;

    if (m_index <= 0)
        return;

    m_slidingTimeLine.setFrameRange(offsetFor(m_index), offsetFor(m_index - 1));
    m_slidingTimeLine.start();

    m_index--;
}

void GenieDemo::moveSlides(int x)
{
    int xofs = (width() - SLIDE_WIDTH - 2 * SLIDE_SPACING) / 2;
    for (int i = 0; i < m_items.count(); i++)
        m_items[i]->setPos(xofs + x - offsetFor(i), 0);

    m_trashButton->move(SLIDE_WIDTH * 3 / 2, SLIDE_HEIGHT + 100);

    int cx = m_trashButton->x() + m_trashButton->width() / 2;
    m_genieItem->setRect(0, 0, cx - xofs, m_trashButton->y());
    m_genieItem->setPos(xofs, 0);
}

void GenieDemo::trash()
{
    if (m_items.count() <= 1)
        return;

    if (m_slidingTimeLine.state() != QTimeLine::NotRunning)
        return;

    if (m_genieTimeLine.state() != QTimeLine::NotRunning)
        return;

    m_items[m_index]->setVisible(false);
    m_genieItem->setPixmap(m_items[m_index]->pixmap());
    m_genieItem->setup();
    m_genieItem->animate(0);
    m_genieItem->setVisible(true);

    m_genieTimeLine.start();
}

void GenieDemo::updateGenieEffect(int frame)
{
    m_genieItem->animate(frame);
}

void GenieDemo::finishGenie()
{
    delete m_items[m_index];
    m_items.removeAt(m_index);
    m_index = qMin(m_index, m_items.count() - 1);
    moveSlides(offsetFor(m_index));
    m_trashButton->setEnabled(m_items.count() > 1);
}

#include "genie.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GenieDemo genie;
    genie.setWindowTitle("Image Viewer (with Genie Effect)");
    genie.show();

    return app.exec();
}
