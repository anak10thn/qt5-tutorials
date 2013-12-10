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

class WebSnap : public QObject
{
    Q_OBJECT

public:
    WebSnap();
    QSize targetSize() const;
    void setTargetSize(const QSize &s);
    QImage image() const;
    void load(const QUrl &url, const QString &outputFileName);

signals:
    void finished();

private slots:
    void saveResult(bool ok);

private:
    QWebPage m_page;
    QString m_fileName;
    QSize m_targetSize;
    QImage m_image;
};

WebSnap::WebSnap(): QObject()
{
    m_targetSize = QSize(400, 300);
    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));
}

void WebSnap::setTargetSize(const QSize &s)
{
    m_targetSize = s;
}

QSize WebSnap::targetSize() const
{
    return m_targetSize;
}

QImage WebSnap::image() const
{
    return m_image;
}

void WebSnap::load(const QUrl &url, const QString &outputFileName)
{
    m_fileName = outputFileName;
    m_image = QImage();
    m_page.mainFrame()->load(url);
}

void WebSnap::saveResult(bool ok)
{
    // crude error-checking
    if (!ok) {
        std::cerr << "Failed loading " << qPrintable(m_page.mainFrame()->url().toString()) << std::endl;
        emit finished();
        return;
    }

    // find proper size, we stick to sensible aspect ratio
    QSize size = m_page.mainFrame()->contentsSize();
    size.setHeight(size.width() * m_targetSize.height() / m_targetSize.width());

    // create the target surface
    m_image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    m_image.fill(Qt::transparent);

    // render and rescale
    QPainter p(&m_image);
    m_page.setViewportSize(m_page.mainFrame()->contentsSize());
    m_page.mainFrame()->render(&p);
    p.end();
    m_image = m_image.scaled(m_targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    if (!m_fileName.isEmpty()) {
        if (m_image.save(m_fileName, "png"))
            std::cout << "Result saved to " << qPrintable(m_fileName) << std::endl;
        else
            std::cout << "Failed to save to " << qPrintable(m_fileName) << std::endl;
    }

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

#include "websnap.moc"

int main(int argc, char * argv[])
{
    if ((argc < 2) || (argc == 4)) {
        std::cout << "Create a thumbnail preview of a web page" << std::endl << std::endl;
        std::cout << "  websnap url [outputfile [width height]]" << std::endl << std::endl;
        std::cout << "Examples: " << std::endl;
        std::cout << "  websnap www.trolltech.com" << std::endl;
        std::cout << "  websnap www.nokia.com" << std::endl;
        std::cout << "  websnap www.google.com google.png 400 300" << std::endl;
        std::cout << std::endl;
        return 0;
    }

    QString fileName;

    QUrl url = guessUrlFromString(QString::fromLatin1(argv[1]));
    if (argc >= 3) {
        fileName = QString::fromLatin1(argv[2]);
    } else {
        fileName = QFileInfo(url.path()).completeBaseName();
        if (fileName.isEmpty())
            fileName = "result";
        fileName += ".png";
    }

    QApplication a(argc, argv);
    WebSnap websnap;
    QObject::connect(&websnap, SIGNAL(finished()), QApplication::instance(), SLOT(quit()));

    if (argc == 5) {
        int w = QString::fromLatin1(argv[3]).toInt();
        int h = QString::fromLatin1(argv[4]).toInt();
        websnap.setTargetSize(QSize(w, h));
        if (!websnap.targetSize().isValid() || w <= 0 || h <= 0) {
            std::cerr << "Please specify a valid target size !" << std::endl;
            return 0;
        }
    }

    websnap.load(url, fileName);
    return a.exec();
}

