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
from PyQt4.QtCore import pyqtSignature, QString, Qt, QTimeLine, SIGNAL, SLOT
from PyQt4.QtGui import *

exit = False
try:
    from ui_controls import Ui_ControlsForm
except ImportError:
    sys.stderr.write(
        "Please process the controls.ui file before running this program. "
        "For example with:\n\n"
        "pyuic4 -o ui_controls.py controls.ui\n\n"
        )
    exit = True
try:
    import parallaxslide_rc
except ImportError:
    sys.stderr.write(
        "Please process the parallaxslide.qrc file before running this program. "
        "For example with:\n\n"
        "pyrcc4 -o parallaxslide_rc.py parallaxslide.qrc\n\n"
        )
    exit = True

if exit:
    sys.exit(1)

class ParallaxSlide(QGraphicsView):

    def __init__(self):
    
        QGraphicsView.__init__(self)
        self.ofs = 0
        self.factor = 1
        self.scene = QGraphicsScene()
        self.background = None
        self.icons = []
        self.iconTimeLine = QTimeLine()
        self.backgroundTimeLine = QTimeLine()
        
        self.setScene(self.scene)
        
        self.background = self.scene.addPixmap(QPixmap(":/background.jpg"))
        self.background.setZValue(0.0)
        self.background.setPos(0, 0)
    
        for i in range(7):
            str = QString(":/icon%1.png").arg(i+1)
            icon = self.scene.addPixmap(QPixmap(str))
            icon.setPos(320+i*64, 400)
            icon.setZValue(1.0)
            self.icons.append(icon)
    
        self.setFixedSize(320, 480)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
    
        self.connect(self.iconTimeLine, SIGNAL("frameChanged(int)"), self, SLOT("moveIcons(int)"))
        self.iconTimeLine.setCurveShape(QTimeLine.EaseInOutCurve)
    
        self.connect(self.backgroundTimeLine, SIGNAL("frameChanged(int)"), self, SLOT("moveBackground(int)"))
        self.connect(self.backgroundTimeLine, SIGNAL("finished()"), self, SLOT("adjustParameters()"))
        self.backgroundTimeLine.setCurveShape(QTimeLine.EaseInOutCurve)
        
        self.controls = Ui_ControlsForm()
        
        toolWidget = QWidget(self)
        toolWidget.setWindowFlags(Qt.Tool | Qt.WindowTitleHint)
        self.controls.setupUi(toolWidget)
        toolWidget.show()
    
        self.connect(self.controls.speedSlider, SIGNAL("valueChanged(int)"),
                     self, SLOT("adjustParameters()"))
        self.connect(self.controls.normalButton, SIGNAL("clicked()"),
                     self, SLOT("adjustParameters()"))
        self.connect(self.controls.parallaxButton, SIGNAL("clicked()"),
                     self, SLOT("adjustParameters()"))
        self.connect(self.controls.leftButton, SIGNAL("clicked()"),
                     self, SLOT("slideLeft()"))
        self.connect(self.controls.rightButton, SIGNAL("clicked()"),
                     self, SLOT("slideRight()"))
    
        self.slideBy(-320)
        self.adjustParameters()
    
    @pyqtSignature("")
    def slideLeft(self):
    
        if self.iconTimeLine.state() != QTimeLine.NotRunning:
            return
        
        if self.ofs > -640:
            self.slideBy(-320)
    
    @pyqtSignature("")
    def slideRight(self):
    
        if self.iconTimeLine.state() != QTimeLine.NotRunning:
            return
        
        if self.ofs < 0:
            self.slideBy(320)
    
    @pyqtSignature("int")
    def slideBy(self, dx):
    
        iconStart = self.ofs
        iconEnd = self.ofs + dx
        self.iconTimeLine.setFrameRange(iconStart, iconEnd)
        self.iconTimeLine.start()
        
        backgroundStart = -320 - int((-320 - iconStart)/self.factor)
        backgroundEnd = -320 - int((-320 - iconEnd)/self.factor)
        self.backgroundTimeLine.setFrameRange(backgroundStart, backgroundEnd)
        self.backgroundTimeLine.start()
        
        self.ofs = iconEnd
    
    @pyqtSignature("bool")
    def setParallaxEnabled(self, p):
    
        if p:
            self.factor = 2
            self.setWindowTitle("Sliding - Parallax mode")
        else:
            self.factor = 1
            self.setWindowTitle("Sliding - Normal mode")
    
    def keyPressEvent(self, event):
    
        if event.key() == Qt.Key_Left:
            self.slideLeft()

        if event.key() == Qt.Key_Right:
            self.slideRight()
    
    @pyqtSignature("int")
    def moveIcons(self, x):
    
        i = 0
        for icon in self.icons:
            icon.setPos(320 + x+i*64, icon.pos().y())
            i += 1
    
    @pyqtSignature("int")
    def moveBackground(self, x):
    
        self.background.setPos(x, self.background.pos().y())
    
    @pyqtSignature("")
    def adjustParameters(self):
    
        speed = self.controls.speedSlider.value()
        self.iconTimeLine.setDuration(1200 - speed*10)
        self.backgroundTimeLine.setDuration(1200 - speed*10)
        self.setParallaxEnabled(self.controls.parallaxButton.isChecked())
        self.controls.leftButton.setEnabled(self.ofs > -640)
        self.controls.rightButton.setEnabled(self.ofs < 0)


if __name__ == "__main__":

    app = QApplication(sys.argv)

    slider = ParallaxSlide()
    slider.show()
    
    sys.exit(app.exec_())
