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
## http:#www.fsf.org/licensing/licenses/info/GPLv2.html and
## http:#www.gnu.org/copyleft/gpl.html.
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
from PyQt4.QtWebKit import *

if QT_VERSION < 0x0040500:
    sys.stderr.write("You need Qt 4.5 or newer to run this example.\n")
    sys.exit(1)

SNAP_THRESHOLD = 10

class SnapView(QWebView):

    def __init__(self):
    
        QWebView.__init__(self)
        self.snapEnabled = True
        self.setWindowTitle(self.tr("Snap-scrolling is ON"))

    # rects hit by the line, in main frame's view coordinate
    def hitBoundingRects(self, line):
    
        hitRects = []

        points = 8
        delta = QPoint(line.dx() / points, line.dy() / points)

        point = line.p1()
        i = 0
        while i < points - 1:
            point += delta
            hit = self.page().mainFrame().hitTestContent(point)
            if not hit.boundingRect().isEmpty():
                hitRects.append(hit.boundingRect())
            i += 1
        
        return hitRects
    
    def keyPressEvent(self, event):

        # toggle snapping
        if event.key() == Qt.Key_F3:
            self.snapEnabled = not self.snapEnabled
            if self.snapEnabled:
                self.setWindowTitle(self.tr("Snap-scrolling is ON"))
            else:
                self.setWindowTitle(self.tr("Snap-scrolling is OFF"))
            event.accept()
            return
        
        # no snapping? do not bother...
        if not self.snapEnabled:
            QWebView.keyReleaseEvent(self, event)
            return
        
        previousOffset = self.page().mainFrame().scrollPosition()

        QWebView.keyReleaseEvent(self, event)
        if not event.isAccepted():
            return

        if event.key() == Qt.Key_Down:
            ofs = self.page().mainFrame().scrollPosition()
            jump = ofs.y() - previousOffset.y()
            if jump == 0:
                return

            jump += SNAP_THRESHOLD

            rects = self.hitBoundingRects(QLine(1, 1, self.width() - 1, 1))
            i = 0
            while i < len(rects):
                j = rects[i].top() - previousOffset.y()
                if j > SNAP_THRESHOLD and j < jump:
                    jump = j
                i += 1
            
            self.page().mainFrame().setScrollPosition(previousOffset + QPoint(0, jump))


if __name__ == "__main__":

    app = QApplication(sys.argv)
    
    view = SnapView()
    view.load(QUrl("http://news.bbc.co.uk/text_only.stm"))
    view.resize(320, 500)
    view.show()

    QMessageBox.information(view, "Hint", "Use F3 to toggle snapping on and off")
    
    sys.exit(app.exec_())
