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
from PyQt4.QtSvg import *

try:
    import weatherinfo_rc
except ImportError:
    sys.stderr.write(
        "Please process the weatherinfo.qrc file before running this program. "
        "For example with:\n\n"
        "pyrcc4 -o weatherinfo_rc.py weatherinfo.qrc\n\n"
        )
    sys.exit(1)

# Left in for future reference:
#if defined (Q_OS_SYMBIAN)
#include "sym_iap_util.h"
#endif

class WeatherInfo(QMainWindow):

    def __init__(self, parent = None):
    
        QMainWindow.__init__(self, parent)

        self.m_view = QGraphicsView()
        self.m_scene = QGraphicsScene()
        self.city = u""
        self.m_statusItem = QGraphicsRectItem()
        self.m_temperatureItem = QGraphicsTextItem()
        self.m_conditionItem = QGraphicsTextItem()
        self.m_iconItem = QGraphicsSvgItem()
        self.m_forecastItems = []
        self.m_dayItems = []
        self.m_conditionItems = []
        self.m_rangeItems = []
        self.m_timeLine = QTimeLine()
        self.m_icons = {}

        self.m_view = QGraphicsView(self)
        self.setCentralWidget(self.m_view)

        self.setupScene()
        self.m_view.setScene(self.m_scene)
        self.m_view.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.m_view.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

        self.m_view.setFrameShape(QFrame.NoFrame)
        self.setWindowTitle(self.tr("Weather Info"))

        cities = []
        for city in "Oslo", "Berlin", "Brisbane", "San Diego":
            action = QAction(city, self)
            self.connect(action, SIGNAL("triggered()"), self.chooseCity)
            self.addAction(action)
            if sys.platform == "symbian_s60":
                self.menuBar().addAction(action)
        self.setContextMenuPolicy(Qt.ActionsContextMenu)

        QTimer.singleShot(0, self.delayedInit)

    def delayedInit(self):
        if sys.platform == "symbian_s60":
            #qt_SetDefaultIap()
            pass
        else:
            self.request("Oslo")

    def chooseCity(self):
        action = self.sender()
        self.request(action.text())

    def handleNetworkData(self, networkReply):
        url = networkReply.url()
        if not networkReply.error():
            self.digest(networkReply.readAll())
        networkReply.deleteLater()
        networkReply.manager().deleteLater()

    def animate(self, frame):
        progress = frame / 100.0
        self.m_iconItem.setOpacity(progress)
        hw = self.width() / 2.0
        self.m_statusItem.setPos(-hw + hw * progress, 0)
        for i in range(len(self.m_forecastItems)):
            ofs = i * 0.5 / len(self.m_forecastItems)
            alpha = min(max(0.0, 2 * (progress - ofs)), 1.0)
            self.m_conditionItems[i].setOpacity(alpha)
            pos = self.m_forecastItems[i].pos()
            if self.width() > self.height():
                fx = self.width() - self.width() * 0.4 * alpha
                self.m_forecastItems[i].setPos(fx, pos.y())
            else:
                fx = self.height() - self.height() * 0.5 * alpha
                self.m_forecastItems[i].setPos(pos.x(), fx)

    def setupScene(self):

        textColor = self.palette().color(QPalette.WindowText)
        textFont = self.font()
        textFont.setBold(True)
        textFont.setPointSize(textFont.pointSize() * 2)

        self.m_temperatureItem = self.m_scene.addText(u"", textFont)
        self.m_temperatureItem.setDefaultTextColor(textColor)

        self.m_conditionItem = self.m_scene.addText(QString(), textFont)
        self.m_conditionItem.setDefaultTextColor(textColor)

        self.m_iconItem = QGraphicsSvgItem()
        self.m_scene.addItem(self.m_iconItem)

        self.m_statusItem = self.m_scene.addRect(0, 0, 10, 10)
        self.m_statusItem.setPen(QPen(Qt.NoPen))
        self.m_statusItem.setBrush(QBrush(Qt.NoBrush))
        self.m_temperatureItem.setParentItem(self.m_statusItem)
        self.m_conditionItem.setParentItem(self.m_statusItem)
        self.m_iconItem.setParentItem(self.m_statusItem)

        self.connect(self.m_timeLine, SIGNAL("frameChanged(int)"), self.animate)
        self.m_timeLine.setDuration(1100)
        self.m_timeLine.setFrameRange(0, 100)
        self.m_timeLine.setCurveShape(QTimeLine.EaseInCurve)

    def request(self, location):
        url = QUrl("http://www.google.com/ig/api")
        url.addEncodedQueryItem("hl", "en")
        url.addEncodedQueryItem("weather", QUrl.toPercentEncoding(location))

        manager = QNetworkAccessManager(self)
        self.connect(manager, SIGNAL("finished(QNetworkReply*)"),
                     self.handleNetworkData)
        manager.get(QNetworkRequest(url))

        self.city = u""
        self.setWindowTitle(self.tr("Loading..."))

    def extractIcon(self, data):
        if not self.m_icons:
            self.m_icons["mostly_cloudy"]    = "weather-few-clouds"
            self.m_icons["cloudy"]           = "weather-overcast"
            self.m_icons["mostly_sunny"]     = "weather-sunny-very-few-clouds"
            self.m_icons["partly_cloudy"]    = "weather-sunny-very-few-clouds"
            self.m_icons["sunny"]            = "weather-sunny"
            self.m_icons["flurries"]         = "weather-snow"
            self.m_icons["fog"]              = "weather-fog"
            self.m_icons["haze"]             = "weather-haze"
            self.m_icons["icy"]              = "weather-icy"
            self.m_icons["sleet"]            = "weather-sleet"
            self.m_icons["chance_of_sleet"]  = "weather-sleet"
            self.m_icons["snow"]             = "weather-snow"
            self.m_icons["chance_of_snow"]   = "weather-snow"
            self.m_icons["mist"]             = "weather-showers"
            self.m_icons["rain"]             = "weather-showers"
            self.m_icons["chance_of_rain"]   = "weather-showers"
            self.m_icons["storm"]            = "weather-storm"
            self.m_icons["chance_of_storm"]  = "weather-storm"
            self.m_icons["thunderstorm"]     = "weather-thundershower"
            self.m_icons["chance_of_tstorm"] = "weather-thundershower"
       
        regex = QRegExp("([\\w]+).gif$")
        if regex.indexIn(data) != -1:
            i = regex.cap()
            i = i[:-4]
            name = self.m_icons[str(i)]
            if name:
                name = ":/icons/" + name + ".svg"
                return name

        return ""

    def toCelcius(self, t, unit):
        try:
            degree = int(t)
        except ValueError:
            return u""
        if unit != "SI":
            degree = ((degree - 32) * 5 + 8)/ 9
        return unicode(degree) + u"\u0260"

    def GET_DATA_ATTR(self, xml):
        return xml.attributes().value("data").toString()

    def removeAll(self, items):
        while items:
            self.m_scene.removeItem(items.pop())

    def digest(self, data):

        textColor = self.palette().color(QPalette.WindowText)
        unitSystem = u""

        del self.m_iconItem
        self.m_iconItem = QGraphicsSvgItem()
        self.m_scene.addItem(self.m_iconItem)
        self.m_iconItem.setParentItem(self.m_statusItem)
        self.removeAll(self.m_dayItems)
        self.removeAll(self.m_conditionItems)
        self.removeAll(self.m_rangeItems)
        self.removeAll(self.m_forecastItems)
        self.removeAll(self.m_dayItems)
        self.removeAll(self.m_conditionItems)
        self.removeAll(self.m_rangeItems)
        self.removeAll(self.m_forecastItems)

        xml = QXmlStreamReader(data)
        while not xml.atEnd():
            xml.readNext()
            if xml.tokenType() == QXmlStreamReader.StartElement:
                if xml.name() == "city":
                    city = self.GET_DATA_ATTR(xml)
                    self.setWindowTitle(city)

                elif xml.name() == "unit_system":
                    unitSystem = xml.attributes().value("data").toString()
                # Parse current weather conditions
                elif xml.name() == "current_conditions":
                    while not xml.atEnd():
                        xml.readNext()
                        if xml.name() == "current_conditions":
                            break
                        if xml.tokenType() == QXmlStreamReader.StartElement:
                            if xml.name() == "condition":
                                self.m_conditionItem.setPlainText(self.GET_DATA_ATTR(xml))

                            if xml.name() == "icon":
                                name = self.extractIcon(self.GET_DATA_ATTR(xml))
                                if name:
                                    del self.m_iconItem
                                    self.m_iconItem = QGraphicsSvgItem(name)
                                    self.m_scene.addItem(self.m_iconItem)
                                    self.m_iconItem.setParentItem(self.m_statusItem)

                            if xml.name() == "temp_c":
                                s = self.GET_DATA_ATTR(xml) + QChar(176)
                                self.m_temperatureItem.setPlainText(s)

                # Parse and collect the forecast conditions
                if xml.name() == "forecast_conditions":
                    dayItem = None
                    statusItem = None
                    lowT, highT = u"", u""
                    while not xml.atEnd():
                        xml.readNext()
                        if xml.name() == "forecast_conditions":
                            if dayItem and statusItem and lowT and highT:
                                self.m_dayItems.append(dayItem)
                                self.m_conditionItems.append(statusItem)
                                txt = highT + '/' + lowT
                                rangeItem = self.m_scene.addText(txt)
                                rangeItem.setDefaultTextColor(textColor)
                                self.m_rangeItems.append(rangeItem)
                                box = self.m_scene.addRect(0, 0, 10, 10)
                                box.setPen(QPen(Qt.NoPen))
                                box.setBrush(QBrush(Qt.NoBrush))
                                self.m_forecastItems.append(box)
                                dayItem.setParentItem(box)
                                statusItem.setParentItem(box)
                                rangeItem.setParentItem(box)
                            else:
                                if dayItem:
                                    scene.removeItem(dayItem)
                                if statusItem:
                                    scene.removeItem(statusItem)

                            break

                        if xml.tokenType() == QXmlStreamReader.StartElement:
                            if xml.name() == "day_of_week":
                                s = self.GET_DATA_ATTR(xml)
                                dayItem = self.m_scene.addText(s[:3])
                                dayItem.setDefaultTextColor(textColor)

                            if xml.name() == "icon":
                                name = self.extractIcon(self.GET_DATA_ATTR(xml))
                                if name:
                                    statusItem = QGraphicsSvgItem(name)
                                    self.m_scene.addItem(statusItem)

                            if xml.name() == "low":
                                lowT = self.toCelcius(self.GET_DATA_ATTR(xml), unitSystem)
                            if xml.name() == "high":
                                highT = self.toCelcius(self.GET_DATA_ATTR(xml), unitSystem)

        self.m_timeLine.stop()
        self.layoutItems()
        self.animate(0)
        self.m_timeLine.start()

    def layoutItems(self):
        self.m_scene.setSceneRect(0, 0, self.width() - 1, self.height() - 1)
        self.m_view.centerOn(self.width() / 2, self.height() / 2)
        if self.width() > self.height():
            self.layoutItemsLandscape()
        else:
            self.layoutItemsPortrait()

    def layoutItemsLandscape(self):
        self.m_statusItem.setRect(0, 0, self.width() / 2 - 1, self.height() - 1)

        if not self.m_iconItem.boundingRect().isEmpty():
            dim = min(self.width() * 0.6, self.height() * 0.8)
            pad = (self.height()  - dim) / 2
            sw = dim / self.m_iconItem.boundingRect().width()
            sh = dim / self.m_iconItem.boundingRect().height()
            self.m_iconItem.setTransform(QTransform().scale(sw, sh))
            self.m_iconItem.setPos(1, pad)

        self.m_temperatureItem.setPos(2, 2)
        h = self.m_conditionItem.boundingRect().height()
        self.m_conditionItem.setPos(10, self.height() - h)

        if self.m_dayItems:
            left = self.width() * 0.6
            h = self.height() / len(self.m_dayItems)
            textFont = self.font()
            textFont.setPixelSize(int(h * 0.3))
            statusWidth = 0
            rangeWidth = 0
            for i in range(len(self.m_dayItems)):
                self.m_dayItems[i].setFont(textFont)
                brect = self.m_dayItems[i].boundingRect()
                statusWidth = max(statusWidth, brect.width())
                brect = self.m_rangeItems[i].boundingRect()
                rangeWidth = max(rangeWidth, brect.width())
           
            space = self.width() - left - statusWidth - rangeWidth
            dim = min(h, space)
            pad = statusWidth + (space  - dim) / 2
            for i in range(len(self.m_dayItems)):
                base = h * i
                self.m_forecastItems[i].setPos(left, base)
                self.m_forecastItems[i].setRect(0, 0, self.width() - left, h)
                brect = self.m_dayItems[i].boundingRect()
                ofs = (h - brect.height()) / 2
                self.m_dayItems[i].setPos(0, ofs)
                brect = self.m_rangeItems[i].boundingRect()
                ofs = (h - brect.height()) / 2
                self.m_rangeItems[i].setPos(self.width() - rangeWidth - left, ofs)
                brect = self.m_conditionItems[i].boundingRect()
                ofs = (h - dim) / 2
                self.m_conditionItems[i].setPos(pad, ofs)
                if brect.isEmpty():
                    continue
                sw = dim / brect.width()
                sh = dim / brect.height()
                self.m_conditionItems[i].setTransform(QTransform().scale(sw, sh))

    def layoutItemsPortrait(self):

        self.m_statusItem.setRect(0, 0, self.width() - 1, self.height() / 2 - 1)

        if not self.m_iconItem.boundingRect().isEmpty():
            dim = min(self.width() * 0.8, self.height() * 0.4)
            ofsy = (self.height() / 2  - dim) / 2
            ofsx = (self.width() - dim) / 3
            sw = dim / self.m_iconItem.boundingRect().width()
            sh = dim / self.m_iconItem.boundingRect().height()
            self.m_iconItem.setTransform(QTransform().scale(sw, sh))
            self.m_iconItem.setPos(ofsx, ofsy)

        self.m_temperatureItem.setPos(2, 2)
        ch = self.m_conditionItem.boundingRect().height()
        cw = self.m_conditionItem.boundingRect().width()
        self.m_conditionItem.setPos(self.width() - cw , self.height() / 2 - ch - 20)

        if self.m_dayItems:
            top = self.height() * 0.5
            w = self.width() / len(self.m_dayItems)
            statusHeight = 0
            rangeHeight = 0
            for i in range(len(self.m_dayItems)):
                self.m_dayItems[i].setFont(self.font())
                brect = self.m_dayItems[i].boundingRect()
                statusHeight = max(statusHeight, brect.height())
                brect = self.m_rangeItems[i].boundingRect()
                rangeHeight = max(rangeHeight, brect.height())
           
            space = self.height() - top - statusHeight - rangeHeight
            dim = min(w, space)

            boxh = statusHeight + rangeHeight + dim
            pad = (self.height() - top - boxh) / 2

            for i in range(len(self.m_dayItems)):
                base = w * i
                self.m_forecastItems[i].setPos(base, top)
                self.m_forecastItems[i].setRect(0, 0, w, boxh)
                brect = self.m_dayItems[i].boundingRect()
                ofs = (w - brect.width()) / 2
                self.m_dayItems[i].setPos(ofs, pad)

                brect = self.m_rangeItems[i].boundingRect()
                ofs = (w - brect.width()) / 2
                self.m_rangeItems[i].setPos(ofs, pad + statusHeight + dim)

                brect = self.m_conditionItems[i].boundingRect()
                ofs = (w - dim) / 2
                self.m_conditionItems[i].setPos(ofs, pad + statusHeight)
                if brect.isEmpty():
                    continue
                sw = dim / brect.width()
                sh = dim / brect.height()
                self.m_conditionItems[i].setTransform(QTransform().scale(sw, sh))

    def resizeEvent(self, event):
        self.layoutItems()


if __name__ == "__main__":

    app = QApplication(sys.argv)

    w = WeatherInfo()
    if sys.platform == "symbian_s60":
        w.showMaximized()
    else:
        w.resize(520, 288)
        w.show()

    sys.exit(app.exec_())
