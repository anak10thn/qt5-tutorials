#!/usr/bin/env python

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

import os, sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtWebKit import *

if QT_VERSION < 0x0040500:
    sys.stderr.write("You need Qt 4.5 or newer to run this example.\n")
    sys.exit(1)

class S5Runner(QWebView):

    def __init__(self, parent = None):
    
        QWebView.__init__(self, parent)
        timer = QTimer(self)
        
        self.overlay = QColor(Qt.transparent)
        self.invert = False
        self.minutes = 30

        self.titleChanged.connect(self.setWindowTitle)

        timer.timeout.connect(self.tick)
        timer.start(60*1000)

    def tick(self):
    
        self.minutes -= 1
        self.update()
    
    def paintEvent(self, event):
    
        if self.overlay.alpha() != 0:
            p = QPainter()
            p.begin(self)
            p.fillRect(event.rect(), self.overlay)
            p.end()
        
        else:
            QWebView.paintEvent(self, event)
            p = QPainter()
            p.begin(self)
            
            if self.page().isContentEditable():
                opts = QTextOption(Qt.AlignCenter)
                p.save()
                p.setOpacity(0.5)
                p.setPen(Qt.white)
                p.drawText(QRectF(0, 0, self.width(), 25), "Edit Mode", opts)
                p.restore()
            
            if self.invert:
                p.setCompositionMode(QPainter.CompositionMode_Difference)
                p.fillRect(event.rect(), Qt.white)

            if True:
                opts = QTextOption(Qt.AlignCenter)
                corner = QRectF(self.width() - 100, self.height() - 50, 50, 50)
                f = QFont(self.font())
                f.setPixelSize(30)
                p.setFont(f)
                p.setOpacity(0.6)
                p.drawText(corner, str(self.minutes), opts)

            p.end()

    def keyPressEvent(self, event):
    
        # Esc resets everything
        if event.key() == Qt.Key_Escape:
            self.page().setContentEditable(False)
            self.overlay = QColor(Qt.transparent)
            self.showNormal()
            self.update()
            event.accept()
            return

        # F5 reloads the web content
        if event.key() == Qt.Key_F5:
            self.reload()
            event.accept()
            return

        # F3 allows editing
        if event.key() == Qt.Key_F3:
            self.page().setContentEditable(not self.page().isContentEditable())
            self.update()
            event.accept()
            return

        # F toggles full-screen
        if not self.page().isContentEditable() and event.key() == Qt.Key_F:
            if self.isFullScreen():
                self.showNormal()
            else:
                self.showFullScreen()
            event.accept()
            return

        # B sets the screen to black
        if not self.page().isContentEditable() and event.key() == Qt.Key_B:
            if self.overlay == Qt.black:
                self.overlay = QColor(Qt.transparent)
            else:
                self.overlay = QColor(Qt.black)
            self.update()
            event.accept()
            return

        # W sets the screen to white
        if not self.page().isContentEditable() and event.key() == Qt.Key_W:
            if self.overlay == Qt.white:
                self.overlay = QColor(Qt.transparent)
            else:
                self.overlay = QColor(Qt.white)
            self.update()
            event.accept()
            return

        # N toggles night-mode
        if not self.page().isContentEditable() and event.key() == Qt.Key_N:
            self.invert = not self.invert
            self.update()
            event.accept()
            return

        # Navigation keys
        if not self.page().isContentEditable():
            sendkey = -1
            if event.key() == Qt.Key_Left:
                sendkey = 37
            elif event.key() == Qt.Key_Right:
                sendkey = 39
            
            if sendkey > 0:
                f = self.page().mainFrame()
                trigger = QString("var k = { which: %1 } keys(k)").arg(sendkey)
                f.evaluateJavaScript(trigger)
                event.accept()
        
        else:
            QWebView.keyPressEvent(self, event)


if __name__ == "__main__":

    try:
        QX11Info
        QApplication.setGraphicsSystem("raster")
    except NameError:
        pass
    
    app = QApplication(sys.argv)
    
    if len(sys.argv) == 2:
        fname = QString.fromLocal8Bit(os.path.abspath(sys.argv[1]))
    else:
        fname = QFileDialog.getOpenFileName(None, "Open Presentation")
    
    if fname.isEmpty():
        sys.exit()
    
    w = S5Runner()
    w.resize(800, 600)
    w.load(QUrl.fromLocalFile(fname))
    w.show()
    
    sys.exit(app.exec_())
