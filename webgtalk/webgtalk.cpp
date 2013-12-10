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

#include <QtGui>
#include <QtWebKitWidgets>

#if QT_VERSION < 0x040500
#error You must use Qt >= 4.5.0!
#endif

#include "ui_form.h"
#include "flickcharm.h"

#define GTALK_URL "http://talkgadget.google.com/talkgadget/m"

class GTalkWidget: public QWidget
{
    Q_OBJECT

private:
    Ui::Form form;
    FlickCharm flickCharm;
    QString userName;
    QString password;

public:
    GTalkWidget(): QWidget() {
        form.setupUi(this);
        setFixedSize(320, 480);

        form.userNameEdit->setFocus();
        connect(form.userNameEdit, SIGNAL(textChanged(QString)), SLOT(adjustLoginButton()));
        connect(form.userNameEdit, SIGNAL(returnPressed()), SLOT(inputPassword()));

        connect(form.passwordEdit, SIGNAL(textChanged(QString)), SLOT(adjustLoginButton()));
        connect(form.passwordEdit, SIGNAL(returnPressed()), SLOT(doLogin()));

        form.loginButton->setEnabled(false);
        connect(form.loginButton, SIGNAL(clicked()), SLOT(doLogin()));

        connect(form.webView, SIGNAL(loadFinished(bool)), SLOT(initialPage(bool)));
        connect(form.webView, SIGNAL(loadProgress(int)),
                form.progressBar, SLOT(setValue(int)));
        form.webView->setUrl((QUrl(GTALK_URL)));
        form.webView->setContextMenuPolicy(Qt::PreventContextMenu);
        flickCharm.activateOn(form.webView);

        showStatus("Wait...");
    }

    void showStatus(const QString &msg) {
        form.statusLabel->setText(msg);
        form.stackedWidget->setCurrentIndex(0);
    }

    void showError(const QString &msg) {
        form.progressBar->hide();
        showStatus(QString("Error: %1").arg(msg));
    }

    QString evalJS(const QString &js) {
        QWebFrame *frame = form.webView->page()->mainFrame();
        return frame->evaluateJavaScript(js).toString();
    }

private slots:

    void adjustLoginButton() {
        userName = form.userNameEdit->text();
        password = form.passwordEdit->text();
        bool ok = !userName.isEmpty() && !password.isEmpty();
        form.loginButton->setEnabled(ok);
    }

    void inputPassword() {
        if (!form.userNameEdit->text().isEmpty())
            form.passwordEdit->setFocus();
    }

    void doLogin() {
        userName = form.userNameEdit->text();
        password = form.passwordEdit->text();
        bool ok = !userName.isEmpty() && !password.isEmpty();
        if (!ok)
            return;

        form.progressBar->setValue(0);
        form.progressBar->show();
        connect(form.webView, SIGNAL(loadFinished(bool)), SLOT(loginPage(bool)));
        connect(form.webView, SIGNAL(loadProgress(int)),
                form.progressBar, SLOT(setValue(int)));
        showStatus("Logging in...");

        QString userEmail = userName + "@gmail.com";
        evalJS(QString("document.getElementById('Email').value = \"%1\";").arg(userEmail));
        evalJS(QString("document.getElementById('Passwd').value = \"%1\";").arg(password));
        evalJS("document.getElementById('gaia_loginform').submit();");
    }

    void initialPage(bool ok) {
        if (ok) {
            QString s1 = evalJS("document.getElementById('Email').name");
            QString s2 = evalJS("document.getElementById('Passwd').name");
            QString s3 = evalJS("document.getElementById('gaia_loginform').id");
            if (s1 == "Email" && s2 == "Passwd" && s3 == "gaia_loginform") {
                form.stackedWidget->setCurrentIndex(1);
                form.webView->disconnect();
                return;
            }
        }

        showError("SERVICE unavailable.");
    }

    void loginPage(bool ok) {
        QString location = form.webView->url().toString();
        if (!ok) {
            if (location.indexOf("CheckCookie"))
                return;
            showError("Service unavailable");
        } else {
            // check for any error message
            QString c = evalJS("document.getElementsByClassName('errormsg').length");
            if (c == "0") {
                // we hide the header and footer on the master window
                evalJS("document.getElementsByClassName('footer-footer')[0]"
                       ".style.visibility = 'hidden';");
                evalJS("document.getElementsByClassName('title-bar-bg title-bar')[0]"
                       ".style.visibility = 'hidden';");

                form.stackedWidget->setCurrentIndex(2);
                return;
            }

            QString err = "Unknown login failure.";
            if (c == "1") {
                err = evalJS("document.getElementsByClassName('errormsg')[0].textContent");
                err = err.simplified();
            }
            showError(err);
        }
    }


};

#include "webgtalk.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GTalkWidget gtalk;
    gtalk.show();

    return app.exec();
}
