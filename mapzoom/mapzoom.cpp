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

// how long (milliseconds) the user need to hold (after a tap on the screen)
// before triggering the magnifying glass feature
// 701, a prime number, is the sum of 229, 233, 239
// (all three are also prime numbers, consecutive!)
#define HOLD_TIME 701

// maximum size of the magnifier
// Hint: see above to find why I picked this one :)
#define MAX_MAGNIFIER 229

#define MAP_HTML "<html><head><script type=\"text/javascript\" " \
    "src=\"http://maps.google.com/maps/api/js?sensor=false\"></script>" \
    "<script  type=\"text/javascript\">" \
    "var map; function init(lat, lng) { "\
    "map = new google.maps.Map(document.getElementById(\"map_canvas\"), " \
    "{ zoom: 15, center: new google.maps.LatLng(lat, lng), " \
    "disableDefaultUI: true, mapTypeId: google.maps.MapTypeId.ROADMAP  });" \
    "} </script>" \
    "</head><body style=\"margin:0px; padding:0px;\">" \
    "<div id=\"map_canvas\" style=\"width:100%; height:100%\"></div>" \
    "</body></html>"

class GMaps: public QWebView
{
    Q_OBJECT

private:
    bool pressed;
    bool snapped;
    QPoint pressPos;
    QPoint dragPos;
    QBasicTimer tapTimer;
    bool zoomed;
    QWebPage *zoomPage;
    QPixmap zoomPixmap;
    QPixmap maskPixmap;

public:
    GMaps(QWidget *parent = 0)
        : QWebView(parent)
        , pressed(false)
        , snapped(false)
        , zoomed(false)
    {
        zoomPage = new QWebPage(this);
        connect(zoomPage, SIGNAL(repaintRequested(QRect)), SLOT(update()));

        QString content = MAP_HTML;
        QWebFrame *frame = page()->mainFrame();
        frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        frame->setHtml(content);

        frame = zoomPage->mainFrame();
        frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        content.replace("zoom: 15", "zoom: 16");
        frame->setHtml(content);

        QTimer::singleShot(1000, this, SLOT(triggerLoading()));
    }

    void setCenter(qreal latitude, qreal longitude) {
        QString code = "map.set_center(new google.maps.LatLng(%1, %2));";
        QWebFrame *frame = page()->mainFrame();
        frame->evaluateJavaScript(code.arg(latitude).arg(longitude));
        frame = zoomPage->mainFrame();
        frame->evaluateJavaScript(code.arg(latitude).arg(longitude));
    }

private slots:

    void triggerLoading() {
        QString code = "init(59.9138204,  10.7387413)";
        QWebFrame *frame = page()->mainFrame();
        frame->evaluateJavaScript(code);
        frame = zoomPage->mainFrame();
        frame->evaluateJavaScript(code);
    }

protected:

    void timerEvent(QTimerEvent *event) {
        QWebView::timerEvent(event);
        if (!zoomed) {
            zoomed = true;
            tapTimer.stop();
            zoomPage->setViewportSize(size() * 2);
            QWebFrame *frame = page()->mainFrame();
            qreal lat = frame->evaluateJavaScript("map.get_center().lat()").toDouble();
            qreal lng = frame->evaluateJavaScript("map.get_center().lng()").toDouble();
            setCenter(lat, lng);
        }
        update();
    }

    void mousePressEvent(QMouseEvent *event) {
        pressed = snapped = true;
        pressPos = dragPos = event->pos();
        tapTimer.stop();
        tapTimer.start(HOLD_TIME, this);
        QWebView::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) {
        pressed = snapped = false;
        tapTimer.stop();
        if (!zoomed) {
            QWebView::mouseReleaseEvent(event);
        } else {
            zoomed = false;
            event->accept();
            update();
            // fake at the press position
            QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonRelease,
                                              pressPos, Qt::LeftButton,
                                              Qt::LeftButton, Qt::NoModifier);
            QWebView::mouseReleaseEvent(ev);
            delete ev;
        }
    }

    void mouseMoveEvent(QMouseEvent *event) {
        if (!zoomed) {
            if (!pressed || !snapped) {
                QWebView::mouseMoveEvent(event);
                return;
            } else {
                const int threshold = 40;
                QPoint delta = event->pos() - pressPos;
                if (snapped) {
                    snapped &= delta.x() < threshold;
                    snapped &= delta.y() < threshold;
                    snapped &= delta.x() > -threshold;
                    snapped &= delta.y() > -threshold;
                }
                if (!snapped)
                    tapTimer.stop();
            }
        } else {
            dragPos = event->pos();
            update();
        }
    }

    void paintEvent(QPaintEvent *event) {
        QWebView::paintEvent(event);
        if (zoomed) {

            int dim = qMin(width(), height());
            int magnifierSize = qMin(MAX_MAGNIFIER, dim * 2 / 3);
            int radius = magnifierSize / 2;
            int ring = radius - 15;
            QSize box = QSize(magnifierSize, magnifierSize);

            // reupdate our mask
            if (maskPixmap.size() != box) {
                maskPixmap = QPixmap(box);
                maskPixmap.fill(Qt::transparent);

                QRadialGradient g;
                g.setCenter(radius, radius);
                g.setFocalPoint(radius, radius);
                g.setRadius(radius);
                g.setColorAt(1.0, QColor(255, 255, 255, 0));
                g.setColorAt(0.5, QColor(128, 128, 128, 255));

                QPainter mask(&maskPixmap);
                mask.setRenderHint(QPainter::Antialiasing);
                mask.setCompositionMode(QPainter::CompositionMode_Source);
                mask.setBrush(g);
                mask.setPen(Qt::NoPen);
                mask.drawRect(maskPixmap.rect());
                mask.setBrush(QColor(Qt::transparent));
                mask.drawEllipse(g.center(), ring, ring);
                mask.end();
            }

            QPoint center = dragPos - QPoint(0, radius);
            center = center + QPoint(0, radius / 2);
            QPoint corner = center - QPoint(radius, radius);
            QPoint xy = center * 2 - QPoint(radius, radius);

            if (zoomPixmap.size() != box) {
                zoomPixmap = QPixmap(box);
                zoomPixmap.fill(Qt::lightGray);
            }

            if (true) {
                QPainter p(&zoomPixmap);
                p.translate(-xy);
                QWebFrame *frame = zoomPage->mainFrame();
                frame->render(&p, QRect(xy, box));
                p.end();
            }

            QPainterPath clipPath;
            clipPath.addEllipse(center, ring, ring);

            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            p.setClipPath(clipPath);
            p.drawPixmap(corner, zoomPixmap);
            p.setClipping(false);
            p.drawPixmap(corner, maskPixmap);
            p.setPen(Qt::gray);
            p.drawPath(clipPath);
            p.end();
        }
    }
};

class MapZoom : public QMainWindow
{
    Q_OBJECT

private:
    GMaps *map;

public:
    MapZoom(): QMainWindow(0) {
        map = new GMaps(this);
        setCentralWidget(map);
        QMenu *menu = menuBar()->addMenu("&Place");
        menu->addAction("&Oslo", this, SLOT(chooseOslo()));
        menu->addAction("&Berlin", this, SLOT(chooseBerlin()));
        menu->addAction("&Jakarta", this, SLOT(chooseJakarta()));
    }

private slots:
    void chooseOslo() {
        map->setCenter(59.9138204, 10.7387413);
    }

    void chooseBerlin() {
        map->setCenter(52.5056819, 13.3232027);
    }

    void chooseJakarta() {
        map->setCenter(-6.211544, 106.845172);
    }
};

#include "mapzoom.moc"

int main(int argc, char **argv)
{
#if defined(Q_WS_X11)
    QApplication::setGraphicsSystem("raster");
#endif

    QApplication app(argc, argv);

    MapZoom w;
    w.setWindowTitle("Maps (powered by Google)");
    w.resize(600, 450);
    w.show();

    return app.exec();
}
