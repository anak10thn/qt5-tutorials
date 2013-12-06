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

#include <QtWebKit>
#include <QtNetwork>
#include <QtGui>
#include <QtXmlPatterns>

#include "twitter.h"

const QUrl Twitter::LOGIN_URL("http://twitter.com/account/verify_credentials.xml");
const QUrl Twitter::GET_FRIENDS_URL("http://twitter.com/statuses/friends.xml");
const QUrl Twitter::GET_FOLLOWERS_URL("http://twitter.com/statuses/followers.xml");
const QUrl Twitter::GET_PUBLIC_STATUS_URL("http://twitter.com/statuses/public_timeline.xml");
const QUrl Twitter::GET_FRIENDS_STATUS_URL("http://twitter.com/statuses/friends_timeline.xml"); // same as /home
const QUrl Twitter::GET_MY_STATUS_URL("http://twitter.com/statuses/user_timeline.xml");
const QString Twitter::GET_USER_STATUS_URL("http://twitter.com/statuses/user_timeline/%1.xml");
const QUrl Twitter::UPDATE_STATUS_URL("http://twitter.com/statuses/update.xml");

Twitter::Twitter(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, SIGNAL(finished(QNetworkReply *)), this, SIGNAL(finished(QNetworkReply *)));
}

Twitter::~Twitter()
{
}

QString Twitter::userName() const
{
    return m_userName;
}

void Twitter::setUserName(const QString &userName)
{
    m_userName = userName;
}

QString Twitter::password() const
{
    return m_password;
}

void Twitter::setPassword(const QString &password)
{
    m_password = password;
}

void Twitter::applyCredentials(QNetworkRequest *request)
{
    QString userpass = QString("%1:%2").arg(m_userName).arg(m_password);
    QByteArray auth("Basic " + userpass.toUtf8().toBase64());
    request->setRawHeader("Authorization", auth);
}

QNetworkReply *Twitter::get(const QUrl &url, const QString &requestType)
{
    QNetworkRequest request(url);
    applyCredentials(&request);
    QNetworkReply *reply = m_nam->get(request);
    reply->setProperty("requestType", requestType);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
    return reply;
}

QByteArray Twitter::credentialsXml() const
{
    return m_credentialsXml;
}

void Twitter::verifyCredentials()
{
    connect(get(LOGIN_URL, "login"), SIGNAL(finished()), this, SLOT(handleLoginReply()));
}

void Twitter::handleLoginReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    m_credentialsXml = reply->readAll();
    QString error;
    if (!m_credentialsXml.isEmpty()) {
        QBuffer buffer(&m_credentialsXml);
        buffer.open(QIODevice::ReadOnly);

        QXmlQuery query;
        query.setFocus(&buffer);
        query.setQuery("data(/hash/error)");
        QString error;
        query.evaluateTo(&error);
    } else {
        error = reply->errorString();
    }
    emit credentialsVerified(reply->error() == QNetworkReply::NoError, error);
}

QNetworkReply *Twitter::getFriends()
{
    return get(GET_FRIENDS_URL, "getFriends");
}

QNetworkReply *Twitter::getFollowers()
{
    return get(GET_FOLLOWERS_URL, "getFollowers");
}

QNetworkReply *Twitter::getPublicStatus()
{
    return get(GET_PUBLIC_STATUS_URL, "getPublicStatus");
}

QNetworkReply *Twitter::getFriendsStatus()
{
    return get(GET_FRIENDS_STATUS_URL, "getFriendsStatus");
}

QNetworkReply *Twitter::getMyStatus()
{
    return get(GET_MY_STATUS_URL, "getMyStatus");
}

QNetworkReply *Twitter::getUserStatus(const QString &id)
{
    return get(QUrl(GET_USER_STATUS_URL.arg(id)), "getUserStatus_" + id);
}

QNetworkReply *Twitter::setStatus(const QString &status)
{
    QNetworkRequest request(UPDATE_STATUS_URL);
    applyCredentials(&request);
    QNetworkReply *reply = m_nam->post(request, QByteArray("status=" + QUrl::toPercentEncoding(status.left(140))));
    reply->setProperty("requestType", "setStatus");
    return reply;
}

