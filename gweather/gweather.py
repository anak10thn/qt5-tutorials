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
from PyQt4.uic import loadUi

if QT_VERSION < 0x0040500:
    sys.stderr.write("You need Qt 4.5 or newer to run this example.\n")
    sys.exit(1)

__this_dir__ = os.path.split(__file__)[0]

class GWeatherWidget(QWidget):

    def __init__(self):
    
        QWidget.__init__(self)
        
        self.form = loadUi(os.path.join(__this_dir__, "form.ui"), self)
        self.form.progressBar.hide()
        self.form.weatherLabel.hide()
        
        self.form.locationEdit.setFocus()
        self.form.locationEdit.returnPressed.connect(self.start)
        
        self.webPage = QWebPage()
        self.webPage.setViewportSize(QSize(1024, 768))
        self.webPage.loadStarted.connect(self.form.progressBar.show)
        self.webPage.loadProgress.connect(self.form.progressBar.setValue)
        self.webPage.loadFinished.connect(self.finish)
        
        self.setWindowTitle("Weather Information - provided by Google")
    
    def start(self):
        self.form.locationLabel.hide()
        self.form.locationEdit.hide()

        url = QUrl("http://www.google.com/search")
        query = "Weather in " + self.form.locationEdit.text()
        url.addEncodedQueryItem("q", QUrl.toPercentEncoding(query))
        self.webPage.mainFrame().setUrl(url)
    
    def evalJavaScript(self, x):
        return self.webPage.mainFrame().evaluateJavaScript((x)).toInt()[0]
    
    def finish(self, ok):
        self.form.progressBar.hide()
        if not ok:
            self.form.weatherLabel.setText("Can't connect to Google")
            self.form.weatherLabel.show()
        else:
            frame = self.webPage.mainFrame()
            
            # insert jQuery framework
            file = open(os.path.join(__this_dir__, "jquery.min.js"))
            self.evalJavaScript(file.read())
            file.close()
            
            # this hides the "Add to iGoogle" link
            self.evalJavaScript("$('div#res.med > div.e > table.ts.std > tbody > tr > td > "
                 "div > a').css('visibility','hidden');")
            
            # this grabs the element for the weather info
            self.evalJavaScript("var e = $('div#res.med > div.e > table.ts.std');")
            
            # locate the element
            x = self.evalJavaScript("e.position().left;")
            y = self.evalJavaScript("e.position().top;")
            w = self.evalJavaScript("e.outerWidth(true);")
            h = self.evalJavaScript("e.outerHeight(true);")
            
            if not w or not h or y <= 0 or y > 250:
                str = QString("No weather information found for %1").arg(self.form.locationEdit.text())
                self.form.weatherLabel.setText(str)
                self.form.weatherLabel.show()
            else:
                # render to a pixmap and then show it
                pixmap = QPixmap(w, h)
                pixmap.fill(Qt.transparent)
                p = QPainter()
                p.begin(pixmap)
                p.translate(-x, -y)
                frame.render(p)
                p.end()
                
                self.setStyleSheet("background: white;")
                self.form.weatherLabel.setPixmap(pixmap)
                self.form.weatherLabel.show()
                self.setWindowTitle(frame.title())


if __name__ == "__main__":

    app = QApplication(sys.argv)
    weather = GWeatherWidget()
    weather.show()
    sys.exit(app.exec_())
