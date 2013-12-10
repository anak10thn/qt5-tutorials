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
from PyQt4.QtNetwork import *
from PyQt4.QtWebKit import *

try:
    import webmonster_rc
except ImportError:
    sys.stderr.write(
        "Please process the webmonster.qrc file before running this program. "
        "For example with:\n\n"
        "pyrcc4 -o webmonster_rc.py webmonster.qrc\n\n"
        )
    sys.exit(1)

MONSTER_JS_URL = "http://deanm.github.com/pre3d/monster.js"

class PaintCommand:
    def __init__(self, opacity = 0.0, stroke = QPen(),
                       fill = QBrush(), polygon = QPolygonF()):
        self.opacity = opacity
        self.stroke = stroke
        self.fill = fill
        self.polygon = polygon


def parseColor(name):

    if name == "none":
        return QColor()
    
    name = str(name)
    if name.startswith("rgba("):
        start = name.find('(')
        end = name.rfind(')')
        if end != -1:
            substr = name[start+1:-1]
            colors = map(float, substr.split(','))
            if len(colors) == 4:
                colors[3] = max(0, colors[3] * 255)
                return QColor(*colors)

    return QColor(name)


class WebMonster(QWidget):

    def __init__(self, parent = None):
    
        QWidget.__init__(self, parent)
        self.commandBuffer = []
        self.fps = 0
        self.frameTick = QTime()
        self.totalFrame = 0
        
        self.page = QWebPage(self)
        self.page.settings().setAttribute(QWebSettings.JavascriptEnabled, True)
        self.page.mainFrame().addToJavaScriptWindowObject("webmonster", self)
        
        self.manager = QNetworkAccessManager(self)
        self.connect(self.manager, SIGNAL("finished(QNetworkReply*)"),
                     self.start)
        
        url = QUrl(MONSTER_JS_URL)
        reply = self.manager.get(QNetworkRequest(url))
        self.connect(reply, SIGNAL("downloadProgress(qint64, qint64)"),
                     self.progress)
        
        self.setWindowTitle(self.tr("About to download %1").arg(MONSTER_JS_URL))
    
    def progress(self, received, total):
    
        percent = received * 100 / total
        self.setWindowTitle(self.tr("Downloading %1: %2%").arg(MONSTER_JS_URL).arg(percent))
    
    def start(self, networkReply):
    
        url = networkReply.url()
        if networkReply.error():
            self.setWindowTitle(self.tr("Can't download %1: %2").arg(
                url.toString()).arg(networkReply.errorString()))
        else:
            self.setWindowTitle(self.tr("Monster Evolution in Qt"))
            self.setAttribute(Qt.WA_OpaquePaintEvent, True)
            
            file = QFile()
            file.setFileName(":/magic.js")
            file.open(QIODevice.ReadOnly)
            magicCode = QString(file.readAll())
            file.close()
            self.page.mainFrame().evaluateJavaScript(magicCode)
            
            scriptCode = QString(networkReply.readAll())
            self.page.mainFrame().evaluateJavaScript(scriptCode)
            self.page.mainFrame().evaluateJavaScript("window.onLoad()")
        
        networkReply.deleteLater()
    
    @pyqtSignature("drawPolygons(int, QVariantList)")
    def drawPolygons(self, polygonCount, polygonArray):
    
        extra = polygonCount - len(self.commandBuffer)
        if extra > 0:
            self.commandBuffer += [None] * extra
        elif extra < 0:
            self.commandBuffer = self.commandBuffer[:polygonCount]
        
        i = 0
        while i < polygonCount:
        
            cmd = polygonArray[i].toList()
            alpha = cmd[0].toDouble()[0]
            strokeColor = cmd[1].toString()
            fillColor = cmd[2].toString()
            pointCount = cmd[3].toInt()[0]
            pointArray = cmd[4].toList()
            
            polygon = QPolygonF(pointCount / 2)
            p = 0
            while p < pointCount / 2:
                polygon[p] = QPointF(pointArray[2 * p].toDouble()[0],
                                     pointArray[2 * p + 1].toDouble()[0])
                p += 1
            
            paint = self.commandBuffer[i]
            if not paint:
                paint = self.commandBuffer[i] = PaintCommand()
            
            paint.opacity = alpha
            if strokeColor != "none":
                paint.stroke.setStyle(Qt.SolidLine)
                paint.stroke.setColor(parseColor(strokeColor))
            else:
                paint.stroke = QPen(Qt.NoPen)
                    
            if fillColor != "none":
                paint.fill = QBrush(parseColor(fillColor))
            else:
                paint.fill = QBrush(Qt.NoBrush)
            paint.polygon = polygon
            i += 1
        
        self.update()
        
        if self.totalFrame % 15 == 0:
            elapsed = self.frameTick.elapsed()
            self.frameTick.start()
            self.fps = 16 * 1000 / (1 + elapsed)
        
        self.totalFrame += 1
    
    def paintEvent(self, event):
    
        p = QPainter(self)
        p.setRenderHint(QPainter.Antialiasing, True)
        for cmd in self.commandBuffer:
            p.setPen(cmd.stroke)
            p.setOpacity(cmd.opacity)
            p.setBrush(cmd.fill)
            p.drawPolygon(cmd.polygon)
        
        p.setPen(Qt.white)
        p.drawText(10, 600 - 20, QString("%1 FPS").arg(self.fps))


if __name__ == "__main__":

    app = QApplication(sys.argv)
    
    webmonster = WebMonster()
    webmonster.setFixedSize(800, 600)
    webmonster.show()
    
    sys.exit(app.exec_())
