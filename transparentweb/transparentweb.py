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

class Container(QWidget):

    def __init__(self):

        QWidget.__init__(self)

        self.view = QWebView(self)

        layout = QVBoxLayout(self)
        self.setLayout(layout)
        layout.addWidget(self.view)

        palette = self.view.palette()
        palette.setBrush(QPalette.Base, Qt.transparent)
        self.view.page().setPalette(palette)
        self.view.setAttribute(Qt.WA_OpaquePaintEvent, False)
        self.connect(self.view, SIGNAL("titleChanged(const QString&)"), 
                     self.setWindowTitle)

        self.view.load(QUrl("http://en.mobile.wikipedia.org/"))

        self.resize(320, 480)

    def paintEvent(self, event):

        QWidget.paintEvent(self, event)

        p = QPainter(self)
        p.fillRect(event.rect(), Qt.transparent)
        p.setPen(Qt.NoPen)
        p.setBrush(QColor(249, 247, 96))
        p.setOpacity(0.6)
        p.drawRoundedRect(self.rect(), 10, 10)
        p.end()

if __name__ == "__main__":

    app = QApplication(sys.argv)

    w = Container()
    w.setAttribute(Qt.WA_TranslucentBackground, True)
    w.setWindowFlags(Qt.FramelessWindowHint)
    w.show()

    sys.exit(app.exec_())
