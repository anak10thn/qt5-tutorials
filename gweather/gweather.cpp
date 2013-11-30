/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtWebKit>

#include "ui_form.h"

#if QT_VERSION < 0x0040500
#error You need Qt 4.5 or newer
#endif


#define EVAL(x) webPage->mainFrame()->evaluateJavaScript((x)).toInt()

class GWeatherWidget: public QWidget
{
    Q_OBJECT

private:
    Ui::Form form;
    QWebPage *webPage;

public:
    GWeatherWidget(): QWidget() {
        form.setupUi(this);
        form.progressBar->hide();
        form.weatherLabel->hide();

        form.locationEdit->setFocus();
        connect(form.locationEdit, SIGNAL(returnPressed()), SLOT(start()));

        webPage = new QWebPage(this);
        webPage->setViewportSize(QSize(1024, 768));
        connect(webPage, SIGNAL(loadStarted()), form.progressBar, SLOT(show()));
        connect(webPage, SIGNAL(loadProgress(int)), form.progressBar, SLOT(setValue(int)));
        connect(webPage, SIGNAL(loadFinished(bool)), SLOT(finish(bool)));

        setWindowTitle("Weather Information - provided by Google");
    }

private slots:

    void start() {
        form.locationLabel->hide();
        form.locationEdit->hide();

        QUrl url("http://www.google.com/search");
        QString query = "Weather in " + form.locationEdit->text();
        url.addEncodedQueryItem("q", QUrl::toPercentEncoding(query));
        webPage->mainFrame()->setUrl(url);
    }


    void finish(bool ok) {
        form.progressBar->hide();
        if (!ok) {
            form.weatherLabel->setText("Can't connect to Google");
            form.weatherLabel->show();
        } else {
            QWebFrame *frame = webPage->mainFrame();

            // insert jQuery framework
            QFile file(":/jquery.min.js");
            file.open(QIODevice::ReadOnly);
            EVAL(file.readAll());
            file.close();

            // this hides the "Add to iGoogle" link
            EVAL("$('div#res.med > div.e > table.ts.std > tbody > tr > td > "
                 "div > a').css('visibility','hidden');");

            // this grabs the element for the weather info
            EVAL("var e = $('div#res.med > div.e > table.ts.std');");

            // locate the element
            int x = EVAL("e.position().left;");
            int y = EVAL("e.position().top;");
            int w = EVAL("e.outerWidth(true);");
            int h = EVAL("e.outerHeight(true);");

            if (!w || !h || y <= 0 || y > 250) {
                QString str = QString("No weather information found for %1").arg(form.locationEdit->text());
                form.weatherLabel->setText(str);
                form.weatherLabel->show();
            } else {
                // render to a pixmap and then show it
                QPixmap pixmap(w, h);
                pixmap.fill(Qt::transparent);
                QPainter p;
                p.begin(&pixmap);
                p.translate(-x, -y);
                frame->render(&p);
                p.end();

                setStyleSheet("background: white;");
                form.weatherLabel->setPixmap(pixmap);
                form.weatherLabel->show();
                setWindowTitle(frame->title());
            }
        }
    }

};

#include "gweather.moc"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    GWeatherWidget weather;
    weather.show();
    return app.exec();
}
