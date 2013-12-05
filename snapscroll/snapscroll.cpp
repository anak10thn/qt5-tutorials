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

#if QT_VERSION < 0x0040500
#error You need Qt 4.5 or newer
#endif

#define SNAP_THRESHOLD 10

class SnapView: public QWebView
{
public:

    SnapView(): QWebView(), snapEnabled(true) {
        setWindowTitle("Snap-scrolling is ON");
    }

private:

    bool snapEnabled;

    // rects hit by the line, in main frame's view coordinate
    QList<QRect> hitBoundingRects(const QLine &line) {
        QList<QRect> hitRects;

        const int points = 8;
        QPoint delta(line.dx() / points, line.dy() / points);

        QPoint point = line.p1();
        for (int i = 0; i < points - 1; ++i) {
            point += delta;
            QWebHitTestResult hit = page()->mainFrame()->hitTestContent(point);
            if (!hit.boundingRect().isEmpty())
                hitRects += hit.boundingRect();
        }

        return hitRects;
    }

    void keyPressEvent(QKeyEvent *event) {

        // toggle snapping
        if (event->key() == Qt::Key_F3) {
            snapEnabled = !snapEnabled;
            if (snapEnabled)
                setWindowTitle("Snap-scrolling is ON");
            else
                setWindowTitle("Snap-scrolling is OFF");
            event->accept();
            return;
        }

        // no snapping? do not bother...
        if (!snapEnabled) {
            QWebView::keyReleaseEvent(event);
            return;
        }

        QPoint previousOffset = page()->mainFrame()->scrollPosition();

        QWebView::keyReleaseEvent(event);
        if (!event->isAccepted())
            return;

        if (event->key() == Qt::Key_Down) {
            QPoint ofs = page()->mainFrame()->scrollPosition();
            int jump = ofs.y() - previousOffset.y();
            if (jump == 0)
                return;

            jump += SNAP_THRESHOLD;

            QList<QRect> rects = hitBoundingRects(QLine(1, 1, width() - 1, 1));
            for (int i = 0; i < rects.count(); ++i) {
                int j = rects[i].top() - previousOffset.y();
                if (j > SNAP_THRESHOLD && j < jump)
                    jump = j;
            }

            page()->mainFrame()->setScrollPosition(previousOffset + QPoint(0, jump));
        }
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    SnapView view;
    view.load(QUrl("http://news.bbc.co.uk/text_only.stm"));
    view.resize(320, 500);
    view.show();

    QMessageBox::information(&view, "Hint", "Use F3 to toggle snapping on and off");

    return app.exec();
}


