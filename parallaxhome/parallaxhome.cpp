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
#include <QtSvg>

#if defined (Q_OS_SYMBIAN)
#include "sym_iap_util.h"
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif

#define PAGE_COUNT 5

class NaviBar : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    NaviBar();

    void setPageOffset(qreal ofs);

signals:
    void pageSelected(int page);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    QList<QGraphicsSvgItem*> m_icons;
    QGraphicsRectItem *m_cursor;
};

#define ICON_SIZE 50
#define ICON_PAD 4

NaviBar::NaviBar()
        : QGraphicsRectItem()
{
    setRect(0, 0, 5 * ICON_SIZE, ICON_SIZE);
    setPen(Qt::NoPen);

    QStringList names;
    names << "map" << "web" << "home" << "weather" << "contacts";
    for (int i = 0; i < names.count(); ++i) {
        QString fname = names[i];
        fname.prepend(":/icons/");
        fname.append("-page.svg");
        QGraphicsSvgItem *icon = new QGraphicsSvgItem(fname);
        icon->setParentItem(this);
        const int dim = ICON_SIZE - ICON_PAD * 2;
        qreal sw = dim / icon->boundingRect().width();
        qreal sh = dim / icon->boundingRect().height();
        icon->setTransform(QTransform().scale(sw, sh));
        icon->setZValue(2);
        m_icons << icon;
    }

    m_cursor = new QGraphicsRectItem;
    m_cursor->setParentItem(this);
    m_cursor->setRect(0, 0, ICON_SIZE, ICON_SIZE);
    m_cursor->setZValue(1);
    m_cursor->setPen(Qt::NoPen);
    m_cursor->setBrush(QColor(Qt::white));
    m_cursor->setOpacity(0.6);
}

void NaviBar::setPageOffset(qreal ofs)
{
    m_cursor->setPos(ofs * ICON_SIZE, 0);
    for (int i = 0; i < m_icons.count(); ++i) {
        int y = (i == static_cast<int>(ofs + 0.5)) ? ICON_PAD : ICON_PAD * 2;
        m_icons[i]->setPos(i * ICON_SIZE + ICON_PAD, y);
        m_icons[i]->setOpacity(1);
    }
}

void NaviBar::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit pageSelected(static_cast<int>(event->pos().x() / ICON_SIZE));
}


void NaviBar::paint(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::white);
    painter->setOpacity(0.2);
    painter->drawRect(option->rect.adjusted(-20, ICON_PAD, 20, 0));
}


class ParallaxHome: public QGraphicsView
{
    Q_OBJECT

public:
    QGraphicsScene m_scene;
    NaviBar *m_naviBar;
    QGraphicsPixmapItem *m_wallpaper;
    QTimeLine m_pageAnimator;
    qreal m_pageOffset;
    QList<QGraphicsPixmapItem*> m_items;
    QList<QPointF> m_positions;

public:
    ParallaxHome(QWidget *parent = 0)
            : QGraphicsView(parent)
            , m_pageOffset(-2) {

        setupScene();
        setScene(&m_scene);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setFrameShape(QFrame::NoFrame);
        setWindowTitle("Parallax Home");

        connect(&m_pageAnimator, SIGNAL(frameChanged(int)), SLOT(shiftPage(int)));
        m_pageAnimator.setDuration(500);
        m_pageAnimator.setFrameRange(0, 100);
        m_pageAnimator.setCurveShape(QTimeLine::EaseInCurve);

        pageChanged(static_cast<int>(m_pageOffset));
    }

signals:
    void pageChanged(int page);

public slots:

    void slideRight() {
        if (m_pageAnimator.state() != QTimeLine::NotRunning)
            return;
        int edge = -(m_pageOffset - 1);
        if (edge < PAGE_COUNT)
            slideBy(-1);
    }

    void slideLeft() {
        if (m_pageAnimator.state() != QTimeLine::NotRunning)
            return;
        if (m_pageOffset < 0)
            slideBy(1);
    }

    void slideBy(int dx) {
        int start = m_pageOffset * 1000;
        int end = (m_pageOffset + dx) * 1000;
        m_pageAnimator.setFrameRange(start, end);
        m_pageAnimator.start();
    }

    void choosePage(int page) {
        if (m_pageAnimator.state() != QTimeLine::NotRunning)
            return;
        if (static_cast<int>(-m_pageOffset) == page)
            return;
        slideBy(-page - m_pageOffset);
    }

private slots:

    void shiftPage(int frame) {

        int ww = width();
        int hh = height() - m_naviBar->rect().height();

        int oldPage = static_cast<int>(-m_pageOffset);
        m_pageOffset = static_cast<qreal>(frame) / qreal(1000);
        int newPage = static_cast<int>(-m_pageOffset);
        m_naviBar->setPageOffset(-m_pageOffset);
        if (oldPage != newPage)
            emit pageChanged(newPage);

        int ofs = m_pageOffset * ww;
        for (int i = 0; i < m_items.count(); ++i) {
            QPointF pos = m_positions[i];
            QPointF xy(pos.x() * ww, pos.y() * hh);
            m_items[i]->setPos(xy + QPointF(ofs, 0));
        }

        int center = m_wallpaper->pixmap().width() / 2;
        const int parallax = 3;
        int base = center - (ww / 2) - (PAGE_COUNT >> 1) * (ww / parallax);
        int wofs = base - m_pageOffset * ww / parallax;
        m_wallpaper->setPos(-wofs, 0);
    }

protected:

    void resizeEvent(QResizeEvent *event) {
        Q_UNUSED(event);
        layoutScene();
    }

    void keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Right)
            slideRight();
        if (event->key() == Qt::Key_Left)
            slideLeft();
        event->accept();
    }

private:

    void layoutScene() {


        int ww = width();
        int hh = height();

        m_scene.setSceneRect(0, 0, PAGE_COUNT * ww - 1, hh - 1);
        centerOn(ww / 2, hh / 2);

        int nw = m_naviBar->rect().width();
        int nh = m_naviBar->rect().height();
        m_naviBar->setPos((ww - nw) / 2, hh - nh);

        shiftPage(m_pageOffset * 1000);
    }

    void setupScene() {

        qsrand(QTime::currentTime().second());

        QStringList names;
        names << "brownies" << "cookies" << "mussels" << "pizza" << "sushi";
        names << "chocolate" << "fish" << "pasta" << "puding" << "trouts";

        for (int i = 0; i < PAGE_COUNT * 2; ++i) {
            QString fname = names[i];
            fname.prepend(":/images/");
            fname.append(".jpg");
            QPixmap pixmap(fname);
            pixmap = pixmap.scaledToWidth(200);
            QGraphicsPixmapItem *item = m_scene.addPixmap(pixmap);
            m_items << item;

            qreal x = (i >> 1) + (qrand() % 30) / 100.0;
            qreal y = (i & 1) / 2.0  + (qrand() % 20) / 100.0;
            m_positions << QPointF(x, y);
            item->setZValue(1);
        }

        m_naviBar = new NaviBar;
        m_scene.addItem(m_naviBar);
        m_naviBar->setZValue(2);
        connect(m_naviBar, SIGNAL(pageSelected(int)), SLOT(choosePage(int)));

        m_wallpaper = m_scene.addPixmap(QPixmap(":/icons/surfacing.png"));
        m_wallpaper->setZValue(0);

        m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    }

};

#include "parallaxhome.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ParallaxHome w;
#if defined(Q_OS_SYMBIAN)
    w.showMaximized();
    // lock orientation
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*>(CEikonEnv::Static()->AppUi());
    if (appUi)
        appUi->SetOrientationL(CAknAppUi::EAppUiOrientationPortrait);
#else
    w.resize(360, 504);
    w.show();
#endif

    return app.exec();
}
