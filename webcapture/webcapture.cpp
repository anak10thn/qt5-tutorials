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

#include <iostream>

#include <QtGui>
#include <QtWebKitWidgets>

class WebCapture : public QObject
{
    Q_OBJECT

public:
    QImage image;
    WebCapture();
    void load(const QUrl &url, int zoom, const QString &outputFileName, int width);

signals:
    void finished();

private slots:
    void showProgress(int percent);
    void saveResult(bool ok);

private:
    QWebPage m_page;
    QString m_fileName;
    int m_zoom;
    int m_percent;
};

WebCapture::WebCapture(): QObject(), m_zoom(100), m_percent(0)
{
    connect(&m_page, SIGNAL(loadProgress(int)), this, SLOT(showProgress(int)));
    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));
}

void WebCapture::load(const QUrl &url, int zoom, const QString &outputFileName, int width)
{
    std::cout << "Loading " << qPrintable(url.toString()) << std::endl;
    m_zoom = zoom;
    m_percent = 0;
    m_fileName = outputFileName;
    image = QImage();
    m_page.mainFrame()->load(url);
    m_page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    m_page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    qreal factor = static_cast<qreal>(m_zoom) / 100.0;
#if QT_VERSION < 0x040500
    m_page.setViewportSize(QSize(width, 3 * width / 4) / factor);
#else
    m_page.mainFrame()->setZoomFactor(factor);
    m_page.setViewportSize(QSize(width, 3 * width / 4));
#endif
}

void WebCapture::showProgress(int percent)
{
    if (m_percent >= percent)
        return;

    while (m_percent++ < percent)
        std::cout << "#" << std::flush;
}

void WebCapture::saveResult(bool ok)
{
    std::cout << std::endl;

    // crude error-checking
    if (!ok) {
        std::cerr << "Failed loading " << qPrintable(m_page.mainFrame()->url().toString()) << std::endl;
        emit finished();
        return;
    }

    // create the image for the buffer
    QSize size = m_page.mainFrame()->contentsSize();
    image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    // render the web page
    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    m_page.setViewportSize(m_page.mainFrame()->contentsSize());
    m_page.mainFrame()->render(&p);
    p.end();

#if QT_VERSION < 0x040500
    // scale, since we don't have full page zoom
    qreal factor = static_cast<qreal>(m_zoom) / 100.0;
    image = image.scaled(size * factor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#endif

    image.save(m_fileName);
    emit finished();
}

// shamelessly copied from Qt Demo Browser
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

#include "webcapture.moc"

int main(int argc, char * argv[])
{
    if ((argc < 4) || (argc > 5)) {
        std::cout << "Capture a web page and save it as an image" << std::endl << std::endl;
        std::cout << "  webcapture url zoom outputfile [viewport width]" << std::endl << std::endl;
        std::cout << "Notes: " << std::endl;
        std::cout << "  'zoom' specifies the zoom factor in percent, default is 100%" << std::endl;
        std::cout << "  'viewport width' defaults to 1024 (pixels)" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples: " << std::endl;
        std::cout << "  webcapture www.trolltech.com 50 trolltech.png 800" << std::endl;
        std::cout << "  webcapture www.nokia.com 25 nokia.png" << std::endl;
        std::cout << std::endl;
        return 0;
    }


    QUrl url = guessUrlFromString(QString::fromLatin1(argv[1]));
    int zoom = qMax(10, QString::fromLatin1(argv[2]).toInt());
    QString fileName = QString::fromLatin1(argv[3]);
    int width = (argc == 5) ? QString::fromLatin1(argv[4]).toInt() : 1024;

    QApplication a(argc, argv);
    WebCapture capture;
    QObject::connect(&capture, SIGNAL(finished()), QApplication::instance(), SLOT(quit()));
    capture.load(url, zoom, fileName, width);

    return a.exec();
}

