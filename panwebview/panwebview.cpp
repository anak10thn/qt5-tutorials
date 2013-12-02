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
#include <QtWebKitWidgets>

class PanWebView : public QWebView
{
    Q_OBJECT

private:
    bool pressed;
    bool scrolling;
    QPoint position;
    QPoint offset;
    QList<QEvent*> ignored;

public:
    PanWebView(QWidget *parent = 0): QWebView(parent), pressed(false), scrolling(false) {
        QWebFrame *frame = page()->mainFrame();
        frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    }

protected:

    void mousePressEvent(QMouseEvent *mouseEvent) {

        if (ignored.removeAll(mouseEvent))
            return QWebView::mousePressEvent(mouseEvent);

        if (!pressed && !scrolling && mouseEvent->modifiers() == Qt::NoModifier)
            if (mouseEvent->buttons() == Qt::LeftButton) {
                pressed = true;
                scrolling = false;
                position = mouseEvent->pos();
                QWebFrame *frame = page()->mainFrame();
                int x = frame->evaluateJavaScript("window.scrollX").toInt();
                int y = frame->evaluateJavaScript("window.scrollY").toInt();
                offset = QPoint(x, y);
                QApplication::setOverrideCursor(Qt::OpenHandCursor);
                return;
            }

        return QWebView::mousePressEvent(mouseEvent);
    }

    void mouseReleaseEvent(QMouseEvent *mouseEvent) {

        if (ignored.removeAll(mouseEvent))
            return QWebView::mouseReleaseEvent(mouseEvent);

        if (scrolling) {
            pressed = false;
            scrolling = false;
            QApplication::restoreOverrideCursor();
            return;
        }

        if (pressed) {
            pressed = false;
            scrolling = false;

            QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonPress,
                                                  position, Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier);
            QMouseEvent *event2 = new QMouseEvent(*mouseEvent);

            ignored << event1;
            ignored << event2;
            QApplication::postEvent(this, event1);
            QApplication::postEvent(this, event2);
            QApplication::restoreOverrideCursor();
            return;
        }

        return QWebView::mouseReleaseEvent(mouseEvent);
    }

    void mouseMoveEvent(QMouseEvent *mouseEvent) {

        if (scrolling) {
            QPoint delta = mouseEvent->pos() - position;
            QPoint p = offset - delta;
            QWebFrame *frame = page()->mainFrame();
            frame->evaluateJavaScript(QString("window.scrollTo(%1,%2);").arg(p.x()).arg(p.y()));
            return;
        }

        if (pressed) {
            pressed = false;
            scrolling = true;
            return;
        }

        return QWebView::mouseMoveEvent(mouseEvent);
    }

};

#include "panwebview.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PanWebView web;
    web.setUrl(QUrl("http://news.google.com"));
    web.setWindowTitle("Web View - use mouse to drag and pan around");
    web.show();

    return app.exec();
}
