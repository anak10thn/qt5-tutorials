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
#include <QtWebKit>
#include <QtXmlPatterns>

#include "twitterview.h"
#include "twitter.h"

TwitterView::TwitterView(Twitter *twitter, QWidget *parent)
    : QWebView(parent),
      m_twitter(twitter)
{
    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(registerObjects()));

    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(page(), SIGNAL(linkClicked(QUrl)), this, SLOT(handleLinkClicked(QUrl)));

    connect(m_twitter, SIGNAL(finished(QNetworkReply *)), this, SLOT(handleReply(QNetworkReply *)));

    showTwitter();
}

TwitterView::~TwitterView()
{
}

void TwitterView::showTwitter()
{
    QXmlQuery query(QXmlQuery::XSLT20);
    QByteArray ba(m_twitter->credentialsXml());
    QBuffer xml(&ba);
    xml.open(QIODevice::ReadOnly);
    query.setFocus(&xml);
    query.setQuery(QUrl("qrc:transforms/twitterview.xsl"));

    QByteArray html;
    QBuffer buffer(&html);
    buffer.open(QIODevice::WriteOnly);
    query.evaluateTo(&buffer);
    buffer.close();

    setHtml(html);

    m_twitter->getFriendsStatus();
    m_twitter->getFriends();
    m_twitter->getFollowers();
}

void TwitterView::setStatus(const QString &status)
{
    connect(m_twitter->setStatus(status), SIGNAL(finished()), m_twitter, SLOT(getFriendsStatus()));
}

void TwitterView::registerObjects()
{
    page()->mainFrame()->addToJavaScriptWindowObject("twitter", m_twitter);
    page()->mainFrame()->addToJavaScriptWindowObject("view", this);
}

void TwitterView::handleLinkClicked(const QUrl &url)
{
    QString anchor = QString::fromLatin1(url.encodedFragment());
    if (anchor == "meAndMyFriendsStatus") {
        m_twitter->getFriendsStatus();
    } else if (anchor == "myStatus") {
        m_twitter->getMyStatus();
    } else if (anchor == "publicStatus") {
        m_twitter->getPublicStatus();
    } else if (anchor.startsWith("friend_")) {
        m_twitter->getUserStatus(anchor.mid(7));
    } else if (anchor.startsWith("follower_")) {
        m_twitter->getUserStatus(anchor.mid(9));
    } else if (url.toString().startsWith("http://")) {
        QDesktopServices::openUrl(url);
    }

    page()->mainFrame()->evaluateJavaScript("setCentralHtml('Please wait...', 'Loading...')");
}

static QString jsEscape(const QString &str, bool escapeSingleQuote = true) 
{
    QString string = str;

    string.replace('\\', "\\\\");
    if (escapeSingleQuote) {
        string.replace('\'', "\\\'");
    } else {
        string.replace('"', "\\\"");
    }
    string.replace('\t', "\\t");
    string.replace('\f', "\\f");
    string.replace('\r', "\\r");
    string.replace('\b', "\\b");
    string.replace('\n', "\\n");

    return string;
}

void TwitterView::handleReply(QNetworkReply *reply)
{
    const QString requestType = reply->property("requestType").toString();

    QString html;
    QByteArray response(reply->readAll());
    QBuffer xml(&response);
    xml.open(QIODevice::ReadOnly);

    if (reply->error() != QNetworkReply::NoError) {
        QXmlQuery query;
        query.setFocus(&xml);
        query.setQuery("data(/hash/error)");
        QString error;
        query.evaluateTo(&error);
        html = QString("Error. Request:%1- NetworkError:%2 ServerError:%3")
                .arg(requestType).arg(reply->errorString()).arg(error.trimmed()).toUtf8();
    } else {
        QXmlQuery query(QXmlQuery::XSLT20);
        query.setFocus(&xml);
        QUrl input;
        if (requestType == "getMyStatus") {
            input = QUrl("qrc:transforms/singleuser.xsl");
        } else if (requestType == "getPublicStatus") {
            input = QUrl("qrc:transforms/public.xsl");
        } else if (requestType == "getFriends") {
            input = QUrl("qrc:transforms/friends.xsl");
        } else if (requestType == "getFollowers") {
            input = QUrl("qrc:transforms/followers.xsl");
        } else if (requestType == "getFriendsStatus") {
            input = QUrl("qrc:transforms/multiuser.xsl");
        } else if (requestType.startsWith("getUserStatus_")) {
            input = QUrl("qrc:transforms/singleuser.xsl");
        }
        query.setQuery(input);

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        query.evaluateTo(&buffer);
        buffer.close();
        html = QString::fromUtf8(ba);
    }

    html = jsEscape(html);

    if (requestType == "getMyStatus") {
        page()->mainFrame()->evaluateJavaScript("setCentralHtml('" + html + "', 'Messages from Me')");
    } else if (requestType == "getPublicStatus") {
        page()->mainFrame()->evaluateJavaScript("setCentralHtml('" + html + "', 'Messages from Everyone')");
    } else if (requestType == "getFriends") {
        page()->mainFrame()->evaluateJavaScript("setFriendsHtml('" + html + "')");
    } else if (requestType == "getFollowers") {
        page()->mainFrame()->evaluateJavaScript("setFollowersHtml('" + html + "')");
    } else if (requestType == "getFriendsStatus") {
        page()->mainFrame()->evaluateJavaScript("setCentralHtml('" + html + "', 'Messages from Me and My Friends')");
    } else if (requestType.startsWith("getUserStatus_")) {
        QString user = requestType.mid(14);
        page()->mainFrame()->evaluateJavaScript("setCentralHtml(\'" + html + "', '" + user + "')");
    }
}

