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
#include <QtWebKitWidgets>


static QUrl guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (hasSchema) {
        QUrl url(urlStr, QUrl::TolerantMode);
        if (url.isValid())
            return url;
    }

    // Might be a file.
    if (QFile::exists(urlStr))
        return QUrl::fromLocalFile(urlStr);

    // Might be a shorturl - try to detect the schema.
    if (!hasSchema) {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if (dotIndex != -1) {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            if (url.isValid())
                return url;
        }
    }

    // Fall back to QUrl's own tolerant parser.
    return QUrl(string, QUrl::TolerantMode);
}

class PrettyView : public QWebView
{
    Q_OBJECT

private:
    QImage buffer;
    bool pretty;

public:

    PrettyView(QWidget *parent = 0): QWebView(parent), pretty(false) {}

public slots:
    void prettify(bool yes) {
        pretty = yes;
        update();
    }

protected:

    void resizeEvent(QResizeEvent *event) {
        buffer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        QWebView::resizeEvent(event);
    }

    void paintEvent(QPaintEvent *event) {

        QRect r = event->rect();
        QWebFrame *frame = page()->mainFrame();

        static QColor prettyColor = QColor(226, 63, 183);

        QPainter painter;
        painter.begin(&buffer);
        frame->render(&painter, event->region());
        painter.end();

        if (pretty)
            for (int y = r.top(); y <= r.bottom(); ++y) {
                quint32 *ptr = (quint32*)(buffer.scanLine(y));
                for (int x = r.left(); x <= r.right(); ++x) {
                    int v = 255 - qGray(ptr[x]);
                    int r = v * (255 - prettyColor.red()) / 255;
                    int g = v * (255 - prettyColor.green()) / 255;
                    int b = v * (255 - prettyColor.blue()) / 255;
                    ptr[x] = qRgb(255 - r, 255 - g, 255 - b);
                }
            }

        painter.begin(this);
        painter.drawImage(r, buffer, r);
        painter.end();
    }

};

class PrettyBrowser : public QMainWindow
{
    Q_OBJECT

private:

    QProgressBar *progress;
    QLineEdit *locationEdit;
    QCheckBox *prettyBox;
    PrettyView *view;

public:
    PrettyBrowser() {

        progress = new QProgressBar(this);
        progress->setRange(0, 100);
        progress->setMinimumSize(100, 20);
        progress->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        progress->hide();
        statusBar()->addPermanentWidget(progress);

        locationEdit = new QLineEdit(this);
        locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
        connect(locationEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

        view = new PrettyView(this);
        view->load(QUrl("http://www.google.com"));
        connect(view, SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
        connect(view, SIGNAL(titleChanged(const QString&)), SLOT(setWindowTitle(const QString&)));
        connect(view, SIGNAL(loadProgress(int)), progress, SLOT(show()));
        connect(view, SIGNAL(loadProgress(int)), progress, SLOT(setValue(int)));
        connect(view, SIGNAL(loadFinished(bool)), progress, SLOT(hide()));

        prettyBox = new QCheckBox(this);
        prettyBox->setText("Make it pretty!");
        QPalette pal = palette();
        pal.setColor(QPalette::WindowText, QColor(192, 0, 0));
        prettyBox->setPalette(pal);

        prettyBox->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
        connect(prettyBox, SIGNAL(toggled(bool)), view, SLOT(prettify(bool)));

        QToolBar *toolBar = addToolBar("Location");
        toolBar->addWidget(locationEdit);
        toolBar = addToolBar("Navigation");
        toolBar->addAction(view->pageAction(QWebPage::Back));
        toolBar->addAction(view->pageAction(QWebPage::Forward));
        toolBar->addAction(view->pageAction(QWebPage::Reload));
        toolBar->addAction(view->pageAction(QWebPage::Stop));

        QWidget *spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());

        toolBar->addWidget(spacer);
        toolBar->addWidget(prettyBox);

        setCentralWidget(view);
    }


protected slots:

    void adjustLocation() {
        locationEdit->setText(view->url().toString());
    }

    void changeLocation() {
        QUrl url = guessUrlFromString(locationEdit->text());
        locationEdit->setText(url.toString());
        view->load(url);
        view->setFocus();
    }

};

#include "main.moc"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    PrettyBrowser browser;
    browser.setWindowTitle("Pretty Browser");
    browser.show();

    return app.exec();
}
