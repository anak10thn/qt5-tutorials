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

#ifndef TWITTER_H
#define TWITTER_H

#include <QObject>
#include <QUrl>
#include <QtWebKitWidgets>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class Twitter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString userName READ userName WRITE setUserName)
    Q_PROPERTY(QString password READ password WRITE setPassword)

public:
    Twitter(QObject *parent = 0);
    ~Twitter();

    static const QUrl LOGIN_URL;
    static const QUrl GET_FRIENDS_URL;
    static const QUrl GET_FOLLOWERS_URL;
    static const QUrl GET_PUBLIC_STATUS_URL;
    static const QUrl GET_FRIENDS_STATUS_URL;
    static const QUrl GET_MY_STATUS_URL;
    static const QUrl UPDATE_STATUS_URL;
    static const QString GET_USER_STATUS_URL;

    QString userName() const;
    void setUserName(const QString &userName);

    QString password() const;
    void setPassword(const QString &password);

    QByteArray credentialsXml() const;

public slots:
    void verifyCredentials();
    QNetworkReply *getFriends();
    QNetworkReply *getFollowers();
    QNetworkReply *getPublicStatus();
    QNetworkReply *getFriendsStatus();
    QNetworkReply *getMyStatus();
    QNetworkReply *getUserStatus(const QString &userId);

    QNetworkReply *setStatus(const QString &userId);

signals:
    void credentialsVerified(bool success, const QString &message);
    void finished(QNetworkReply *reply);

private slots:
    void handleLoginReply();

private:
    QNetworkReply *get(const QUrl &url, const QString &requestType);
    void applyCredentials(QNetworkRequest *request);

    QNetworkAccessManager *m_nam;
    QString m_userName;
    QString m_password;
    QByteArray m_credentialsXml;
};

#endif // TWITTER_H

