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

class NightModeView(QWebView):

    def __init__(self):
    
        QWebView.__init__(self)
        self.invert = True

        self.connect(self, SIGNAL("titleChanged(const QString&)"), 
                     self.setWindowTitle)

        self.load(QUrl("http://maps.google.com"))
    
    def paintEvent(self, event):
        QWebView.paintEvent(self, event)
        if self.invert:
            p = QPainter(self)
            p.setCompositionMode(QPainter.CompositionMode_Difference)
            p.fillRect(event.rect(), Qt.white)
            p.end()

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_F3:
            self.invert = not self.invert
            self.update()
            event.accept()

        QWebView.keyPressEvent(self, event)

if __name__ == "__main__":

    QApplication.setGraphicsSystem("raster")

    app = QApplication(sys.argv)
    
    w = NightModeView()
    w.show()

    QMessageBox.information(w, "Hint", "Use F3 to toggle night mode")
    
    sys.exit(app.exec_())
