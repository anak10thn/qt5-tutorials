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
from PyQt4.uic import loadUi

from ui_search import Ui_searchForm
from dragmovecharm import DragMoveCharm

GSEARCH_URL = "http://www.google.com/search?q=%1"
GSUGGEST_URL = "http://google.com/complete/search?output=toolbar&q=%1"

class GSuggestCompletion(QObject):

    def __init__(self, parent = None):
    
        QObject.__init__(self, parent)
        self.editor = parent
        self.networkManager = QNetworkAccessManager()

        self.popup = QTreeWidget()
        self.popup.setColumnCount(2)
        self.popup.setUniformRowHeights(True)
        self.popup.setRootIsDecorated(False)
        self.popup.setEditTriggers(QTreeWidget.NoEditTriggers)
        self.popup.setSelectionBehavior(QTreeWidget.SelectRows)
        self.popup.setFrameStyle(QFrame.Box | QFrame.Plain)
        self.popup.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

        self.popup.header().hide()
        self.popup.installEventFilter(self)
        self.popup.setMouseTracking(True)

        self.connect(self.popup, SIGNAL("itemClicked(QTreeWidgetItem*, int)"),
                     self.doneCompletion)

        self.popup.setWindowFlags(Qt.Popup)
        self.popup.setFocusPolicy(Qt.NoFocus)
        self.popup.setFocusProxy(parent)

        self.timer = QTimer(self)
        self.timer.setSingleShot(True)
        self.timer.setInterval(500)
        self.connect(self.timer, SIGNAL("timeout()"), self.autoSuggest)
        self.connect(self.editor, SIGNAL("textEdited(QString)"), self.timer, SLOT("start()"))

        self.connect(self.networkManager, SIGNAL("finished(QNetworkReply*)"),
                     self.handleNetworkData)
    
    def eventFilter(self, obj, ev):
    
        if obj != self.popup:
            return False

        if ev.type() == QEvent.MouseButtonPress:
            self.popup.hide()
            self.editor.setFocus()
            return True

        if ev.type() == QEvent.KeyPress:

            consumed = False
            key = ev.key()
            if key == Qt.Key_Enter or key == Qt.Key_Return:
                self.doneCompletion()
                consumed = True

            elif key == Qt.Key_Escape:
                self.editor.setFocus()
                self.popup.hide()
                consumed = True

            elif key in (Qt.Key_Up, Qt.Key_Down, Qt.Key_Home, Qt.Key_End,
                         Qt.Key_PageUp, Qt.Key_PageDown):
                pass

            else:
                self.editor.setFocus()
                self.editor.event(ev)
                self.popup.hide()

            return consumed

        return False

    def showCompletion(self, choices, hits):

        if not choices or len(choices) != len(hits):
            return

        pal = self.editor.palette()
        color = pal.color(QPalette.Disabled, QPalette.WindowText)

        self.popup.setUpdatesEnabled(False)
        self.popup.clear()
        for choice, hit in zip(choices, hits):
            item = QTreeWidgetItem(self.popup)
            item.setText(0, choice)
            item.setText(1, hit)
            item.setTextAlignment(1, Qt.AlignRight)
            item.setTextColor(1, color)
        
        self.popup.setCurrentItem(self.popup.topLevelItem(0))
        self.popup.resizeColumnToContents(0)
        self.popup.resizeColumnToContents(1)
        self.popup.adjustSize()
        self.popup.setUpdatesEnabled(True)

        h = self.popup.sizeHintForRow(0) * min(7, len(choices)) + 3
        self.popup.resize(self.popup.width(), h)

        self.popup.move(self.editor.mapToGlobal(QPoint(0, self.editor.height())))
        self.popup.setFocus()
        self.popup.show()
    
    def doneCompletion(self):
        self.timer.stop()
        self.popup.hide()
        self.editor.setFocus()
        item = self.popup.currentItem()
        if item:
            self.editor.setText(item.text(0))
            e = QKeyEvent(QEvent.KeyPress, Qt.Key_Enter, Qt.NoModifier)
            QApplication.postEvent(self.editor, e)
            e = QKeyEvent(QEvent.KeyRelease, Qt.Key_Enter, Qt.NoModifier)
            QApplication.postEvent(self.editor, e)

    def preventSuggest(self):
        self.timer.stop()
    
    def autoSuggest(self):
        str = self.editor.text()
        url = QString(GSUGGEST_URL).arg(str)
        self.networkManager.get(QNetworkRequest(QUrl(url)))

    def handleNetworkData(self, networkReply):
        url = networkReply.url()
        if not networkReply.error():
            choices = []
            hits = []

            response = networkReply.readAll()
            xml = QXmlStreamReader(response)
            while not xml.atEnd():
                xml.readNext()
                if xml.tokenType() == QXmlStreamReader.StartElement:
                    if xml.name() == "suggestion":
                        str = xml.attributes().value("data")
                        choices.append(str.toString())
                    elif xml.name() == "num_queries":
                        str = xml.attributes().value("int")
                        hits.append(str.toString())
            
            self.showCompletion(choices, hits)
        
        networkReply.deleteLater()


class SearchBox(QFrame):

    def __init__(self, parent = None):
    
        QFrame.__init__(self, parent)
        
        self.ui = Ui_searchForm()
        self.ui.setupUi(self)
        self.setFrameStyle(QFrame.StyledPanel + QFrame.Raised)
        
        self.completion = GSuggestCompletion(self.ui.searchEdit)
        
        self.connect(self.ui.searchEdit, SIGNAL("returnPressed()"), self.doSearch)
        self.connect(self.ui.searchButton, SIGNAL("clicked()"), self.doSearch)
        
        self.adjustSize()
        self.resize(400, self.height())
        self.ui.searchEdit.setFocus()
    
    def doSearch(self):
        self.completion.preventSuggest()
        str = self.ui.searchEdit.text()
        url = QString(GSEARCH_URL).arg(str)
        QDesktopServices.openUrl(QUrl(url))
    
    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Escape:
            self.close()


if __name__ == "__main__":

    app = QApplication(sys.argv)

    suggest = SearchBox()
    suggest.setWindowFlags(Qt.FramelessWindowHint)
    suggest.show()

    charm = DragMoveCharm()
    charm.activateOn(suggest)

    sys.exit(app.exec_())
