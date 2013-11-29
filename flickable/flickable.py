#############################################################################
##
## Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
## Contact: Qt Software Information (qt-info@nokia.com)
##
## This file is part of the Graphics Dojo project on Qt Labs.
##
## This file may be used under the terms of the GNU General Public
## License version 2.0 or 3.0 as published by the Free Software Foundation
## and appearing in the file LICENSE.GPL included in the packaging of
## this file.  Please review the following information to ensure GNU
## General Public Licensing requirements will be met:
## http://www.fsf.org/licensing/licenses/info/GPLv2.html and
## http://www.gnu.org/copyleft/gpl.html.
##
## If you are unsure which license is appropriate for your use, please
## contact the sales department at qt-sales@nokia.com.
##
## This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
## WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
##
#############################################################################

import random, sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class FlickableTicker(QObject):

    def __init__(self, scroller, parent = None):
    
        QObject.__init__(self, parent)
        self.m_scroller = scroller
        self.m_timer = QBasicTimer()

    def start(self, interval):
        if not self.m_timer.isActive():
            self.m_timer.start(interval, self)

    def stop(self):
        self.m_timer.stop()
    
    def timerEvent(self, event):
        self.m_scroller.tick()


class FlickablePrivate:

    class State:
        def __init__(self, value):
            self.value = value
    
    Steady, Pressed, ManualScroll, AutoScroll, Stop = map(State, range(5))


class Flickable:

    def __init__(self):

        self.d = FlickablePrivate()
        self.d.state = FlickablePrivate.Steady
        self.d.threshold = 10
        self.d.ticker = FlickableTicker(self)
        self.d.timeStamp = QTime.currentTime()
        self.d.target = 0

        self.d.pressPos = QPoint()
        self.d.offset = QPoint()
        self.d.delta = QPoint()
        self.d.speed = QPoint()
        self.d.ignoreList = []

    def setThreshold(self, th):

        if th >= 0:
            self.d.threshold = th

    def threshold(self):

        return self.d.threshold

    def setAcceptMouseClick(self, target):

        self.d.target = target

    def deaccelerate(self, speed, a = 1, maximum = 64):

        x = min(max(-maximum, speed.x()), maximum)
        y = min(max(-maximum, speed.y()), maximum)
        if x > 0:
            x = max(0, x - a)
        elif x < 0:
            x = min(0, x + a)

        if y > 0:
            y = max(0, y - a)
        elif y < 0:
            y = min(0, y + a)

        return QPoint(x, y)

    def handleMousePress(self, event):

        event.ignore()

        if event.button() != Qt.LeftButton:
            return

        if event in self.d.ignoreList:
            self.d.ignoreList.remove(event)
            return

        if self.d.state == FlickablePrivate.Steady:
            event.accept()
            self.d.state = FlickablePrivate.Pressed
            self.d.pressPos = QPoint(event.pos())

        elif self.d.state == FlickablePrivate.AutoScroll:
            event.accept()
            self.d.state = FlickablePrivate.Stop
            self.d.speed = QPoint(0, 0)
            self.d.pressPos = QPoint(event.pos())
            self.d.offset = self.scrollOffset()
            self.d.ticker.stop()

    def handleMouseRelease(self, event):

        event.ignore()

        if event.button() != Qt.LeftButton:
            return

        if event in self.d.ignoreList:
            self.d.ignoreList.remove(event)
            return

        if self.d.state == FlickablePrivate.Pressed:
            event.accept()
            self.d.state = FlickablePrivate.Steady
            if self.d.target:
                event1 = QMouseEvent(QEvent.MouseButtonPress,
                                     self.d.pressPos, Qt.LeftButton,
                                     Qt.LeftButton, Qt.NoModifier)
                event2 = QMouseEvent(event)
                self.d.ignoreList.append(event1)
                self.d.ignoreList.append(event2)
                QApplication.postEvent(self.d.target, event1)
                QApplication.postEvent(self.d.target, event2)

        elif self.d.state == FlickablePrivate.ManualScroll:
            event.accept()
            delta = event.pos() - self.d.pressPos
            if self.d.timeStamp.elapsed() > 100:
                self.d.timeStamp = QTime.currentTime()
                self.d.speed = delta - self.d.delta
                self.d.delta = delta

            self.d.offset = self.scrollOffset()
            self.d.pressPos = QPoint(event.pos())
            if self.d.speed == QPoint(0, 0):
                self.d.state = FlickablePrivate.Steady
            else:
                self.d.speed /= 4
                self.d.state = FlickablePrivate.AutoScroll
                self.d.ticker.start(20)

        elif self.d.state == FlickablePrivate.Stop:
            event.accept()
            self.d.state = FlickablePrivate.Steady
            self.d.offset = self.scrollOffset()

    def handleMouseMove(self, event):

        event.ignore()

        if not (event.buttons() & Qt.LeftButton):
            return

        if event in self.d.ignoreList:
            self.d.ignoreList.remove(event)
            return

        if self.d.state in (FlickablePrivate.Pressed, FlickablePrivate.Stop):
            delta = event.pos() - self.d.pressPos
            if delta.x() > self.d.threshold or delta.x() < -self.d.threshold or \
               delta.y() > self.d.threshold or delta.y() < -self.d.threshold:

                self.d.timeStamp = QTime.currentTime()
                self.d.state = FlickablePrivate.ManualScroll
                self.d.delta = QPoint(0, 0)
                self.d.pressPos = QPoint(event.pos())
                event.accept()

        elif self.d.state == FlickablePrivate.ManualScroll:
            event.accept()
            delta = event.pos() - self.d.pressPos
            self.setScrollOffset(self.d.offset - delta)
            if self.d.timeStamp.elapsed() > 100:
                self.d.timeStamp = QTime.currentTime()
                self.d.speed = delta - self.d.delta
                self.d.delta = delta

    def tick(self):

        if self.d.state == FlickablePrivate. AutoScroll:
            self.d.speed = self.deaccelerate(self.d.speed)
            self.setScrollOffset(self.d.offset - self.d.speed)
            self.d.offset = self.scrollOffset()
            if self.d.speed == QPoint(0, 0):
                self.d.state = FlickablePrivate.Steady
                self.d.ticker.stop()

        else:
            self.d.ticker.stop()


# Returns a list of two-word color names
def colorPairs(maximum):

    # capitalize the first letter
    colors = QColor.colorNames()
    colors.removeAll("transparent")
    num = len(colors)
    for c in range(num):
        colors[c] = colors[c][0].toUpper() + colors[c].mid(1)

    # combine two colors, e.g. "lime skyblue"
    combinedColors = []
    for i in range(num):
        for j in range(num):
            combinedColors.append(QString("%1 %2").arg(colors[i]).arg(colors[j]))

    # randomize it
    colors = []
    while combinedColors:
        i = random.randint(0, len(combinedColors))
        colors.append(combinedColors[i])
        del combinedColors[i]
        if len(colors) == maximum:
            break

    return colors


class ColorList(QWidget, Flickable):

    def __init__(self, parent = None):

        QWidget.__init__(self, parent)
        Flickable.__init__(self)

        self.m_offset = 0
        self.m_height = QFontMetrics(self.font()).height() + 5
        self.m_highlight = -1
        self.m_selected = -1

        self.m_colorNames = []
        self.m_firstColor = []
        self.m_secondColor = []

        colors = colorPairs(999)
        for i in range(len(colors)):
            c = colors[i]
            s = "%4i" % (i + 1)
            self.m_colorNames.append(s + "   " + c)

            duet = c.split(' ')
            self.m_firstColor.append(QColor(duet[0]))
            self.m_secondColor.append(QColor(duet[1]))

        self.setAttribute(Qt.WA_OpaquePaintEvent, True)
        self.setAttribute(Qt.WA_NoSystemBackground, True)

        self.setMouseTracking(True)
        self.setAcceptMouseClick(self)

    # reimplement from Flickable
    def scrollOffset(self):
        return QPoint(0, self.m_offset)

    # reimplement from Flickable
    def setScrollOffset(self, offset):
        yy = offset.y()
        if yy != self.m_offset:
            self.m_offset = min(max(0, yy), self.m_height * len(self.m_colorNames) - self.height())
            self.update()

    def paintEvent(self, event):
        p = QPainter(self)
        p.fillRect(event.rect(), Qt.white)
        start = self.m_offset / self.m_height
        y = start * self.m_height - self.m_offset
        if self.m_offset <= 0:
            start = 0
            y = -self.m_offset

        end = start + self.height() / self.m_height + 1
        if end > len(self.m_colorNames) - 1:
            end = len(self.m_colorNames) - 1
        for i in range(start, end):

            p.setBrush(Qt.NoBrush)
            p.setPen(Qt.black)
            if i == self.m_highlight:
                p.fillRect(0, y, self.width(), self.m_height, QColor(0, 64, 128))
                p.setPen(Qt.white)
            
            if i == self.m_selected:
                p.fillRect(0, y, self.width(), self.m_height, QColor(0, 128, 240))
                p.setPen(Qt.white)

            p.drawText(self.m_height + 2, y, self.width(), self.m_height, Qt.AlignVCenter, self.m_colorNames[i])

            p.setPen(Qt.NoPen)
            p.setBrush(QBrush(self.m_firstColor[i]))
            p.drawRect(1, y + 1, self.m_height - 2, self.m_height - 2)
            p.setBrush(QBrush(self.m_secondColor[i]))
            p.drawRect(5, y + 5, self.m_height - 11, self.m_height - 11)

            y += self.m_height
        
        p.end()

    def keyReleaseEvent(self, event):
        if event.key() == Qt.Key_Down:
            self.m_offset += 20
            event.accept()
            self.update()
            return
        
        if event.key() == Qt.Key_Up:
            self.m_offset -= 20
            event.accept()
            self.update()
            return

    def mousePressEvent(self, event):
        self.handleMousePress(event)
        if event.isAccepted():
            return

        if event.button() == Qt.LeftButton:
            y = event.pos().y() + self.m_offset
            i = y / self.m_height
            if i != self.m_highlight:
                self.m_highlight = i
                self.m_selected = -1
                self.update()

            event.accept()

    def mouseMoveEvent(self, event):
        self.handleMouseMove(event)

    def mouseReleaseEvent(self, event):
        self.handleMouseRelease(event)
        if event.isAccepted():
            return

        if event.button() == Qt.LeftButton:
            self.m_selected = self.m_highlight
            event.accept()
            self.update()


if __name__ == "__main__":

    app = QApplication(sys.argv)

    random.seed()

    colorList = ColorList()
    colorList.setWindowTitle("Kinetic Scrolling")
#ifdef Q_OS_SYMBIAN
#    colorList.showMaximized()
#else
    colorList.resize(360, 640)
    colorList.show()
#endif

    sys.exit(app.exec_())
