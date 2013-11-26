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
/*
 *modify :
 *add header QApplication
 *change toASCII()
 *add index.html to qrc
 */
#include <QtGui>
#include <QApplication>
#include <QWebView>
#include <QWebElement>
#include <QWebPage>
#include <QWebFrame>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QImage>
namespace {
    class RendererNetworkReply : public QNetworkReply
    {
        Q_OBJECT
        QBuffer buffer;
        public:
            RendererNetworkReply(QWebFrame* frame,const QNetworkRequest & r)
                   : QNetworkReply(frame)
            {
                setRequest(r);
                QString selector = r.url().toString(QUrl::RemoveScheme|QUrl::RemoveQuery);
                selector = QUrl::fromPercentEncoding(selector.toUtf8());
                setOperation(QNetworkAccessManager::GetOperation);
                setHeader(QNetworkRequest::ContentTypeHeader,QVariant("image/png"));
                open(ReadOnly|Unbuffered);
                setUrl(r.url());
                QWebElement element = frame->findFirstElement(selector);
                QImage img(element.geometry().size(),QImage::Format_ARGB32);
                img.fill(0);
                {
                    QPainter painter(&img);
                    element.render(&painter);
                }
                buffer.open(QIODevice::WriteOnly);
                img.save(&buffer,"png");
                buffer.close();
                buffer.open(QIODevice::ReadOnly);
                QTimer::singleShot(0,this,SIGNAL(readyRead()));
                QTimer::singleShot(0,this,SIGNAL(finished()));
            }

            virtual qint64 readData(char* data, qint64 maxSize)
            {
                return buffer.read(data,maxSize);
            }

            virtual qint64 bytesAvailable() const
            {
                return buffer.bytesAvailable();
            }

            void setData(const QByteArray & data)
            {
                buffer.setData(data);
                buffer.open(QIODevice::ReadOnly);
            }


            void abort()
            {
            }
    };

    class RendererNetworkAccessManager : public QNetworkAccessManager
    {
        QWebPage* webPage;
        public:
        RendererNetworkAccessManager(QWebPage* page) : QNetworkAccessManager(page),webPage(page)
        {
        }
        QNetworkReply* createRequest(Operation op, const QNetworkRequest & req, QIODevice* outgoingData)
        {
            if (req.url().scheme() == "render" && op == GetOperation) {
                return new RendererNetworkReply(webPage->currentFrame(),req);
            } else
                return QNetworkAccessManager::createRequest(op,req,outgoingData);
        }
    };

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
    QWebView w;
    w.page()->setNetworkAccessManager(new RendererNetworkAccessManager(w.page()));
    QFile f(":/example.html");
    f.open(QIODevice::ReadOnly);
    w.setHtml(f.readAll());
    f.close();
    w.show();
    return a.exec();
}


#include <main.moc>
