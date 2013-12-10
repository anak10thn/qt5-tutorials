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
from PyQt4.QtCore import QFile, QPointF, QSize, Qt
from PyQt4.QtGui import *

class BarChart(QWidget):

    def __init__(self, parent = None):
    
        QWidget.__init__(self, parent)
        self.suffix = ""
        self.path = ""
    
    def showEvent(self, event):
    
        # find the maximum value and widest (pixel-wise) label and suffix
        fm = QFontMetrics(self.font())
        
        self.margin = 20
        self.titleHeight = fm.height()
        self.barHeight = 1.5 * fm.height()
        self.barSpacing = 0.6 * fm.height()
        
        self.maxValue = self.suffixWidth = self.labelWidth = 0
        count = 0
        for key, value in self.data.items():
            self.labelWidth = max(self.labelWidth, fm.width(key))
            self.maxValue = max(self.maxValue, value)
            self.suffixWidth = max(self.suffixWidth, fm.width(str(value) + " " + self.suffix))
            count += 1
    
        self.startHue = 15
        self.hueDelta = 360 / count
    
        self.resize(QSize(640, self.titleHeight + 2 * self.margin + (self.barHeight + self.barSpacing) * count))
    
    def paintEvent(self, event):
    
        p = QPainter()
        p.begin(self)
    
        # background and title
        p.fillRect(self.rect(), QBrush(QColor(255, 255, 255)))
        p.drawText(0, 0, self.width(), self.margin + self.titleHeight, Qt.AlignCenter, self.windowTitle())
    
        ofs = self.labelWidth + self.margin
        ww = self.width() - self.suffixWidth - ofs - 2 * self.margin
        hue = self.startHue
        y = 0
    
        p.translate(self.margin, self.titleHeight + 1.5 * self.margin)
    
        for key, value in self.data.items():
    
            # label on the left side
            p.setPen(QColor(Qt.black))
            p.drawText(0, y, self.labelWidth, self.barHeight, Qt.AlignVCenter + Qt.AlignRight, key)
    
            # the colored bar
            gradient = QLinearGradient(QPointF(ofs, y), QPointF(ofs, y + self.barHeight))
            gradient.setColorAt(0, QColor.fromHsv(hue, 255, 240))
            gradient.setColorAt(1, QColor.fromHsv(hue, 255, 92))
            p.setBrush(QBrush(gradient))
            p.setPen(QColor(96, 96, 96))
            bw = value * ww / self.maxValue
            p.drawRect(ofs, y, bw, self.barHeight)
    
            # extra text at the end of the bar
            text = str(value) + " " + self.suffix
            p.setPen(QColor(Qt.black))
            p.drawText(ofs + bw + self.margin/2, y, self.suffixWidth, self.barHeight, Qt.AlignVCenter + Qt.AlignLeft, text)
    
            # for the next bar
            y += (self.barHeight + self.barSpacing)
            hue += self.hueDelta
            if hue >= 360:
                hue -= 360
        
        p.end()
    
    def wheelEvent(self, event):
        self.startHue += event.delta() / 8 / 5
        if self.startHue >= 360:
            self.startHue -= 360
        if self.startHue < 0:
            self.startHue += 360
        self.update()
        event.ignore()
    
    def mousePressEvent(self, event):
        fname = QFileDialog.getSaveFileName(self, self.tr("Save"), self.path,
                                            self.tr("PNG files (*.png)"))
        if not fname.isEmpty():
            img = QImage(self.size(), QImage.Format_ARGB32_Premultiplied)
            self.render(img)
            if img.save(QFile(fname)):
                self.path = fname
            else:
                QMessageBox.warning(self, self.tr("Failed to save image"),
                    self.tr("The specified image could not be saved."))


if __name__ == "__main__":

    app = QApplication(sys.argv)
    chart = BarChart()
    
    chart.setWindowTitle("SquirrelFish Performance")
    
    chart.suffix = "runs"
    
    chart.data = {"WebKit 3.0" :  5.4, 
                  "WebKit 3.1" : 18.8,
                "SquirrelFish" : 29.9,
        "SquirrelFish Extreme" : 63.6}
    
    chart.show()
    sys.exit(app.exec_())

