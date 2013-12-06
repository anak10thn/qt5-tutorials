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
#include <QtNetwork>

#include "mainwindow.h"
#include "twitterview.h"
#include "twitter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.actionAbout_TwitterView, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), this, SLOT(aboutQt()));
    connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(login()));

    m_movie = new QMovie(this);
    m_movie->setFileName(":/images/loading.gif");
    m_movie->start();
    ui.progressLabel->setText(QString());

    m_twitter = new Twitter(this);
    connect(m_twitter, SIGNAL(credentialsVerified(bool, QString)),
            this, SLOT(credentialsVerified(bool, QString)));

    statusBar()->showMessage(tr("Ready."));
}

MainWindow::~MainWindow()
{
}

void MainWindow::login()
{
    ui.loginButton->setEnabled(false);
    ui.progressLabel->setText(QString());
    ui.progressLabel->setMovie(m_movie);
    m_twitter->setUserName(ui.userNameEdit->text());
    m_twitter->setPassword(ui.passwordEdit->text());
    m_twitter->verifyCredentials();
    statusBar()->showMessage(tr("Logging in"));
}

void MainWindow::credentialsVerified(bool success, const QString &msg)
{
    if (success) {
        statusBar()->showMessage(tr("Logged in"));
        setCentralWidget(new TwitterView(m_twitter));
    } else {
        ui.progressLabel->setText(tr("Login failed!"));
        statusBar()->showMessage(tr("Login failed - %1").arg(msg));
        ui.loginButton->setEnabled(true);
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("TwitterView Demo"),
                       tr("Demonstrates the use of QtXmlPatterns/XSLT with WebKit"));
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

