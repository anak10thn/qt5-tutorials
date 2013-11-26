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

from PyQt4.QtCore import QEvent, QObject, QPoint, Qt
from PyQt4.QtGui import QMouseEvent, QWidget

class DragMoveData:
    def __init__(self):
        self.isMoving = False
        self.startDrag = QPoint()

class DragMoveCharm(QObject):

    def __init__(self, parent = None):
    
        QObject.__init__(self, parent)
        self.dragMoveData = {}
    
    def activateOn(self, widget):
    
        if widget in self.dragMoveData:
            return
        
        data = DragMoveData()
        data.startDrag = QPoint(0, 0)
        data.isMoving = False
        self.dragMoveData[widget] = data
        
        widget.installEventFilter(self)
    
    def deactivateFrom(self, widget):
    
        del self.dragMoveData[widget]
        self.dragMoveData.remove(widget)
        widget.removeEventFilter(self)
    
    def eventFilter(self, object, event):
    
        if not isinstance(object, QWidget):
            return False
        
        widget = object
        
        type = event.type()
        if type != QEvent.MouseButtonPress and \
            type != QEvent.MouseButtonRelease and \
            type != QEvent.MouseMove:
            return False
        
        if isinstance(event, QMouseEvent):
            if event.modifiers() != Qt.NoModifier:
                return False
            button = event.button()
            mouseEvent = event
        
        try:
            data = self.dragMoveData[widget]
        except KeyError:
            return False
        
        consumed = False
        
        if type == QEvent.MouseButtonPress and button == Qt.LeftButton:
            data.startDrag = QPoint(mouseEvent.globalPos())
            data.isMoving = True
            event.accept()
            consumed = True
        
        if type == QEvent.MouseButtonRelease:
            data.startDrag = QPoint(0, 0)
            data.isMoving = False
        
        if type == QEvent.MouseMove and data.isMoving:
            pos = mouseEvent.globalPos()
            widget.move(widget.pos() + pos - data.startDrag)
            data.startDrag = QPoint(pos)
            consumed = True
        
        return consumed
