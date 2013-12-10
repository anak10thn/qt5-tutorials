/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <QtGui/QApplication>
#include <QWebView>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QDir>

class RendererReply : public QNetworkReply
{
    Q_OBJECT
public:
    RendererReply(QObject* object, const QNetworkRequest& request)
        : QNetworkReply(object)
        , position(0)
    {
        setRequest(request);
        setOperation(QNetworkAccessManager::GetOperation);
        setHeader(QNetworkRequest::ContentTypeHeader,QVariant("image/png"));
        open(ReadOnly|Unbuffered);
        setUrl(request.url());
        QString pattern = request.url().queryItemValue("pattern");
        Qt::BrushStyle brushStyle = Qt::SolidPattern;
        if (pattern == "dense")
            brushStyle = Qt::Dense2Pattern;
        else if (pattern == "cross")
            brushStyle = Qt::CrossPattern;
        else if (pattern == "diagonal")
            brushStyle = Qt::FDiagPattern;
        const QString radiusString = request.url().queryItemValue("radius");
        const QString widthString = request.url().queryItemValue("width");
        const QString heightString = request.url().queryItemValue("height");
        const QString colorString = request.url().queryItemValue("color");
        QColor color(colorString);
        bool ok;
        int radius = radiusString.toInt(&ok);
        if (!ok)
            radius = 5;
        QSize size;
        size.setWidth(widthString.toInt(&ok));
        if (!ok)
            size.setWidth(100);
        size.setHeight(heightString.toInt(&ok));
        if (!ok)
            size.setHeight(100);
        connect(&watcher, SIGNAL(finished()), SLOT(generateDone()));
        QFuture<QByteArray> future = QtConcurrent::run<QByteArray>(generate, size, brushStyle, radius, color);
        watcher.setFuture(future);
    }

    qint64 readData(char* data, qint64 maxSize)
    {
        const qint64 readSize = qMin(maxSize, (qint64)(buffer.size() - position));
        memcpy(data, buffer.constData() + position, readSize);
        position += readSize;
        return readSize;
    }

    virtual qint64 bytesAvailable() const
    {
        return buffer.size() - position;
    }

    virtual qint64 pos () const
    {
        return position;
    }

    virtual bool seek( qint64 pos )
    {
        if (pos < 0 || pos >= buffer.size())
            return false;
        position = pos;
        return true;
    }

    virtual qint64 size () const
    {
        return buffer.size();
    }

    static QByteArray generate(const QSize& size, const Qt::BrushStyle style, int radius, const QColor& color)
    {
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter painter(&image);
        QPainterPath path;
        path.addRoundedRect(QRectF(QPoint(0, 0), size), radius, radius);
        painter.fillPath(path, QBrush(color, style));
        QByteArray saveData;
        QBuffer b(&saveData);
        image.save(&b, "PNG");
        return saveData;
    }

public slots:
    void generateDone()
    {
        setHeader(QNetworkRequest::ContentTypeHeader, "image/png");
        position = 0;
        buffer = watcher.result();
        emit readyRead();
        emit finished();
    }

    void abort()
    {
    }
public:
    QNetworkReply* rawReply;
private:
    QFutureWatcher<QByteArray> watcher;
    QByteArray buffer;
    int position;
};

class UrlBasedRenderer : public QNetworkAccessManager
{
public:
    UrlBasedRenderer(QObject* parent = 0) : QNetworkAccessManager(parent)
    {
    }

    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
    {
        if (op != GetOperation || request.url().scheme() != "qt-render" || request.url().host() != "button")
            return QNetworkAccessManager::createRequest(op, request, outgoingData);
        return new RendererReply(this, request);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWebView webView;
    UrlBasedRenderer urlBasedRenderer;
    webView.page()->setNetworkAccessManager(&urlBasedRenderer);
    webView.setHtml("<HTML><BODY><IMG src=\"qt-render://button?width=100&height=100&radius=5&color=green&pattern=dense\" /></BODY></HTML>");
    webView.show();
    return app.exec();
}

#include <main.moc>
