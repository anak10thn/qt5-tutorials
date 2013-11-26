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

import sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *

class Digits(QWidget):

    Slide, Flip, Rotate = range(3)

    def __init__(self, parent):

        QWidget.__init__(self, parent)

        self.m_number = 0
        self.m_transition = self.Slide
        self.m_pixmap = QPixmap()
        self.m_lastPixmap = QPixmap()
        self.m_animator = QTimeLine()

        self.setAttribute(Qt.WA_OpaquePaintEvent, True)
        self.setAttribute(Qt.WA_NoSystemBackground, True)
        self.connect(self.m_animator, SIGNAL("frameChanged(int)"), SLOT("update()"))
        self.m_animator.setFrameRange(0, 100)
        self.m_animator.setDuration(600)
        self.m_animator.setCurveShape(QTimeLine.EaseInOutCurve)

    def setTransition(self, tr):
        self.m_transition = tr

    def transition(self):
        return self.m_transition

    def setNumber(self, n):
        if self.m_number != n:
            self.m_number = max(0, min(n, 99))
            self.preparePixmap()
            self.update()

    def flipTo(self, n):
        if self.m_number != n:
            self.m_number = max(0, min(n, 99))
            self.m_lastPixmap = self.m_pixmap
            self.preparePixmap()
            self.m_animator.stop()
            self.m_animator.start()

    def drawFrame(self, p, rect):
        p.setPen(Qt.NoPen)
        gradient = QLinearGradient(QPointF(rect.topLeft()), QPointF(rect.bottomLeft()))
        gradient.setColorAt(0.00, QColor(245, 245, 245))
        gradient.setColorAt(0.49, QColor(192, 192, 192))
        gradient.setColorAt(0.51, QColor(245, 245, 245))
        gradient.setColorAt(1.00, QColor(192, 192, 192))
        p.setBrush(gradient)
        r = rect
        p.drawRoundedRect(r, 15, 15, Qt.RelativeSize)
        r.adjust(1, 4, -1, -4)
        p.setPen(QColor(181, 181, 181))
        p.setBrush(Qt.NoBrush)
        p.drawRoundedRect(r, 15, 15, Qt.RelativeSize)
        p.setPen(QColor(159, 159, 159))
        y = rect.top() + rect.height() / 2 - 1
        p.drawLine(rect.left(), y, rect.right(), y)

    def drawDigits(self, n, rect):
        s = "%02i" % n

        font = QFont()
        font.setFamily("Helvetica")
        fontHeight = 2 * 0.55 * rect.height()
        font.setPixelSize(fontHeight)
        font.setBold(True)

        pixmap = QPixmap(rect.size() * 2)
        pixmap.fill(Qt.transparent)

        gradient = QLinearGradient(QPointF(0, 0), QPointF(0, pixmap.height()))
        gradient.setColorAt(0.00, QColor(128, 128, 128))
        gradient.setColorAt(0.49, QColor(64, 64, 64))
        gradient.setColorAt(0.51, QColor(128, 128, 128))
        gradient.setColorAt(1.00, QColor(16, 16, 16))

        p = QPainter()
        p.begin(pixmap)
        p.setFont(font)
        pen = QPen()
        pen.setBrush(QBrush(gradient))
        p.setPen(pen)
        p.drawText(pixmap.rect(), Qt.AlignCenter, s)
        p.end()

        return pixmap.scaledToWidth(self.width(), Qt.SmoothTransformation)

    def preparePixmap(self):
        self.m_pixmap = QPixmap(self.size())
        self.m_pixmap.fill(Qt.transparent)
        p = QPainter()
        p.begin(self.m_pixmap)
        p.drawPixmap(0, 0, self.drawDigits(self.m_number, self.rect()))
        p.end()

    def resizeEvent(self, event):
        self.preparePixmap()
        self.update()

    def paintStatic(self):
        p = QPainter(self)
        p.fillRect(self.rect(), Qt.black)

        pad = self.width() / 10
        self.drawFrame(p, self.rect().adjusted(pad, pad, -pad, -pad))
        p.drawPixmap(0, 0, self.m_pixmap)

    def paintSlide(self):
        p = QPainter(self)
        p.fillRect(self.rect(), Qt.black)

        pad = self.width() / 10
        fr = self.rect().adjusted(pad, pad, -pad, -pad)
        self.drawFrame(p, fr)
        p.setClipRect(fr)

        y = self.height() * self.m_animator.currentFrame() / 100
        p.drawPixmap(0, y, self.m_lastPixmap)
        p.drawPixmap(0, y - self.height(), self.m_pixmap)

    def paintFlip(self):
        p = QPainter(self)
        p.setRenderHint(QPainter.SmoothPixmapTransform, True)
        p.setRenderHint(QPainter.Antialiasing, True)
        p.fillRect(self.rect(), Qt.black)

        hw = self.width() / 2
        hh = self.height() / 2

        # behind is the new pixmap
        pad = self.width() / 10
        fr = self.rect().adjusted(pad, pad, -pad, -pad)
        self.drawFrame(p, fr)
        p.drawPixmap(0, 0, self.m_pixmap)

        index = self.m_animator.currentFrame()

        if index <= 50:

            # the top part of the old pixmap is flipping
            angle = -180 * index / 100
            transform = QTransform()
            transform.translate(hw, hh)
            transform.rotate(angle, Qt.XAxis)
            p.setTransform(transform)
            self.drawFrame(p, fr.adjusted(-hw, -hh, -hw, -hh))
            p.drawPixmap(-hw, -hh, self.m_lastPixmap)

            # the bottom part is still the old pixmap
            p.resetTransform()
            p.setClipRect(0, hh, self.width(), hh)
            self.drawFrame(p, fr)
            p.drawPixmap(0, 0, self.m_lastPixmap)

        else:

            p.setClipRect(0, hh, self.width(), hh)

            # the bottom part is still the old pixmap
            self.drawFrame(p, fr)
            p.drawPixmap(0, 0, self.m_lastPixmap)

            # the bottom part of the new pixmap is flipping
            angle = 180 - 180 * self.m_animator.currentFrame() / 100
            transform = QTransform()
            transform.translate(hw, hh)
            transform.rotate(angle, Qt.XAxis)
            p.setTransform(transform)
            self.drawFrame(p, fr.adjusted(-hw, -hh, -hw, -hh))
            p.drawPixmap(-hw, -hh, self.m_pixmap)

    def paintRotate(self):
        p = QPainter(self)

        pad = self.width() / 10
        fr = self.rect().adjusted(pad, pad, -pad, -pad)
        self.drawFrame(p, fr)
        p.setClipRect(fr)

        angle1 = -180 * self.m_animator.currentFrame() / 100
        angle2 = 180 - 180 * self.m_animator.currentFrame() / 100
        if self.m_animator.currentFrame() <= 50:
            angle = angle1
        else:
            angle = angle2
        if self.m_animator.currentFrame() <= 50:
            pix = self.m_lastPixmap
        else:
            pix = self.m_pixmap

        transform = QTransform()
        transform.translate(self.width() / 2, self.height() / 2)
        transform.rotate(angle, Qt.XAxis)

        p.setTransform(transform)
        p.setRenderHint(QPainter.SmoothPixmapTransform, True)
        p.drawPixmap(-self.width() / 2, -self.height() / 2, pix)

    def paintEvent(self, event):
        if self.m_animator.state() == QTimeLine.Running:
            if self.m_transition == self.Slide:
                self.paintSlide()
            if self.m_transition == self.Flip:
                self.paintFlip()
            if self.m_transition == self.Rotate:
                self.paintRotate()
        else:
            self.paintStatic()


class DigiFlip(QMainWindow):

    def __init__(self, parent = None):

        QMainWindow.__init__(self, parent)

        self.m_ticker = QBasicTimer()
        self.m_hour = Digits(self)
        self.m_hour.show()
        self.m_minute = Digits(self)
        self.m_minute.show()

        pal = QPalette(self.palette())
        pal.setColor(QPalette.Window, Qt.black)
        self.setPalette(pal)

        self.m_ticker.start(1000, self)
        t = QTime.currentTime()
        self.m_hour.setNumber(t.hour())
        self.m_minute.setNumber(t.minute())
        self.updateTime()

        slideAction = QAction("Slide", self)
        flipAction = QAction("Flip", self)
        rotateAction = QAction("Rotate", self)
        self.connect(slideAction, SIGNAL("triggered()"), self.chooseSlide)
        self.connect(flipAction, SIGNAL("triggered()"), self.chooseFlip)
        self.connect(rotateAction, SIGNAL("triggered()"), self.chooseRotate)
#if defined(Q_OS_SYMBIAN)
#        menuBar().addAction(slideAction)
#        menuBar().addAction(flipAction)
#        menuBar().addAction(rotateAction)
#else
        self.addAction(slideAction)
        self.addAction(flipAction)
        self.addAction(rotateAction)
        self.setContextMenuPolicy(Qt.ActionsContextMenu)
#endif

    def updateTime(self):
        t = QTime.currentTime()
        self.m_hour.flipTo(t.hour())
        self.m_minute.flipTo(t.minute())
        if self.m_hour.transition() == Digits.Slide:
            s = u"Slide: "
        if self.m_hour.transition() == Digits.Flip:
            s = u"Flip: "
        if self.m_hour.transition() == Digits.Rotate:
            s = u"Rotate: "
        self.setWindowTitle(s + t.toString("hh:mm:ss"))

    def switchTransition(self, delta):
        i = (self.m_hour.transition() + delta + 3) % 3
        self.m_hour.setTransition(i)
        self.m_minute.setTransition(i)
        self.updateTime()

    def resizeEvent(self, event):
        digitsWidth = self.width() / 2
        digitsHeight = digitsWidth * 1.2

        y = (self.height() - digitsHeight) / 3

        self.m_hour.resize(digitsWidth, digitsHeight)
        self.m_hour.move(0, y)

        self.m_minute.resize(digitsWidth, digitsHeight)
        self.m_minute.move(self.width() / 2, y)

    def timerEvent(self, event):
        self.updateTime()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Right:
            self.switchTransition(1)
            event.accept()

        if event.key() == Qt.Key_Left:
            self.switchTransition(-1)
            event.accept()

    def chooseSlide(self):
        self.m_hour.setTransition(0)
        self.m_minute.setTransition(0)
        self.updateTime()

    def chooseFlip(self):
        self.m_hour.setTransition(1)
        self.m_minute.setTransition(1)
        self.updateTime()

    def chooseRotate(self):
        self.m_hour.setTransition(2)
        self.m_minute.setTransition(2)
        self.updateTime()


if __name__ == "__main__":

    app = QApplication(sys.argv)

    time = DigiFlip()
#if defined(Q_OS_SYMBIAN)
#    time.showMaximized()
#else
    time.resize(320, 240)
    time.show()
#endif

    sys.exit(app.exec_())
