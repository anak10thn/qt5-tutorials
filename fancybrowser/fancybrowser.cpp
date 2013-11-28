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

// for Qt 4.5, we use JQuery
// for Qt >= 4.6, we use the QWebElement API
#if QT_VERSION < 0x040600
#define USE_JQUERY
#endif

#if QT_VERSION < 0x0040500
#error You need Qt 4.5 or newer
#endif

class FancyBrowser : public QMainWindow
{
    Q_OBJECT

private:

    QString jQuery;
    QWebView *view;
    QLineEdit *locationEdit;
    QString title;
    int progress;

public:
    FancyBrowser(): QMainWindow(0), progress(0) {

#ifdef USE_JQUERY
        QFile file;
        file.setFileName(":/jquery.min.js");
        file.open(QIODevice::ReadOnly);
        jQuery = file.readAll();
        file.close();
#endif

        view = new QWebView(this);
        view->load(QUrl("http://www.google.com/ncr"));
        connect(view, SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
        connect(view, SIGNAL(titleChanged(const QString&)), SLOT(adjustTitle(const QString&)));
        connect(view, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
        connect(view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));

        locationEdit = new QLineEdit(this);
        locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
        connect(locationEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

        QToolBar *toolBar = addToolBar("Navigation");
        toolBar->addAction(view->pageAction(QWebPage::Back));
        toolBar->addAction(view->pageAction(QWebPage::Forward));
        toolBar->addAction(view->pageAction(QWebPage::Reload));
        toolBar->addAction(view->pageAction(QWebPage::Stop));

        QMenu *toolsMenu = new QMenu(this);
        toolsMenu->addAction("Highlight all links", this, SLOT(highlightAllLinks()));
        toolsMenu->addSeparator();
        toolsMenu->addAction("Remove GIF images", this, SLOT(removeGifImages()));
        toolsMenu->addAction("Remove all inline frames", this, SLOT(removeInlineFrames()));
        toolsMenu->addAction("Remove all object elements", this, SLOT(removeObjectElements()));
        toolsMenu->addAction("Remove all embedded elements", this, SLOT(removeEmbeddedElements()));

        QAction *rotateAction = new QAction(this);
        rotateAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
        rotateAction->setCheckable(true);
        rotateAction->setText("Turn images upside down");
        connect(rotateAction, SIGNAL(toggled(bool)), this, SLOT(rotateImages(bool)));

        QToolButton *btn = new QToolButton(this);
        btn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
        btn->setMenu(toolsMenu);
        btn->setPopupMode(QToolButton::InstantPopup);
        toolBar->addWidget(btn);
        toolBar->addAction(rotateAction);
        toolBar->addWidget(locationEdit);

        setCentralWidget(view);
        title = "Fancy Browser";
    }


protected slots:

    void adjustLocation() {
        locationEdit->setText(view->url().toString());
    }

    void adjustTitle(const QString &t) {
        title = t;
        if (progress <= 0 || progress >= 100)
            setWindowTitle(title);
        else
            setWindowTitle(QString("%1 (%2%)").arg(title).arg(progress));
    }

    void setProgress(int p) {
        progress = p;
        adjustTitle(title);
    }

    void finishLoading(bool) {
        progress = 100;
        adjustTitle(title);
#ifdef USE_JQUERY
        view->page()->mainFrame()->evaluateJavaScript(jQuery);
#endif
    }

    void changeLocation() {
        QUrl url = QUrl(locationEdit->text());
        locationEdit->setText(url.toString());
        view->load(url);
        view->setFocus();
    }

#ifdef USE_JQUERY

    void highlightAllLinks() {
        QString code = "$('a').each( function () { $(this).css('background-color', 'yellow') } )";
        view->page()->mainFrame()->evaluateJavaScript(code);
    }

    void rotateImages(bool toggle) {
        QString code = "$('img').each( function () { $(this).css('-webkit-transition', '-webkit-transform 2s') } )";
        view->page()->mainFrame()->evaluateJavaScript(code);
        if (toggle)
            code = "$('img').each( function () { $(this).css('-webkit-transform', 'rotate(180deg)') } )";
        else
            code = "$('img').each( function () { $(this).css('-webkit-transform', 'rotate(0deg)') } )";
        view->page()->mainFrame()->evaluateJavaScript(code);
    }

    void removeGifImages() {
        QString code = "$('[src*=gif]').remove()";
        view->page()->mainFrame()->evaluateJavaScript(code);
    }

    void removeInlineFrames() {
        QString code = "$('iframe').remove()";
        view->page()->mainFrame()->evaluateJavaScript(code);
    }

    void removeObjectElements() {
        QString code = "$('object').remove()";
        view->page()->mainFrame()->evaluateJavaScript(code);
    }

    void removeEmbeddedElements() {
        QString code = "$('embed').remove()";
        view->page()->mainFrame()->evaluateJavaScript(code);
    }

#else

    void highlightAllLinks() {
        foreach (QWebElement element, view->page()->mainFrame()->findAllElements("a"))
            element.setStyleProperty("background-color", "yellow");
    }

    void rotateImages(bool toggle) {
        foreach (QWebElement element, view->page()->mainFrame()->findAllElements("img")) {
            element.setStyleProperty("-webkit-transition", "-webkit-transform 2s");
            if (toggle)
                element.setStyleProperty("-webkit-transform", "rotate(180deg)");
            else
                element.setStyleProperty("-webkit-transform", "rotate(0deg)");
        }
    }

    void removeGifImages() {
        foreach (QWebElement element, view->page()->mainFrame()->findAllElements("[src*=gif]"))
            element.removeFromDocument();
    }

    void removeInlineFrames() {
        foreach (QWebElement element, view->page()->mainFrame()->findAllElements("iframe"))
            element.removeFromDocument();
    }

    void removeObjectElements() {
        foreach (QWebElement element, view->page()->mainFrame()->findAllElements("object"))
            element.removeFromDocument();
    }

    void removeEmbeddedElements() {
        foreach (QWebElement element, view->page()->mainFrame()->findAllElements("embed"))
            element.removeFromDocument();
    }

#endif

};

#include "fancybrowser.moc"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);

    FancyBrowser browser;
    browser.setWindowTitle("Fancy Browser");
    browser.show();

    return app.exec();
}
