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
#include <QtWidgets>

#include "ui_parameters.h"

class ImageTint: public QWidget
{
Q_OBJECT

public:
    ImageTint();
    void loadImage(const QString &file);
    void loadImage(const QImage &image);

protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent *event);
    void paintEvent(QPaintEvent*);

public slots:
    void handleNetworkData(QNetworkReply*);

private:
    QString m_fileName;
    QImage m_image;
    Ui::ParametersForm parameters;
    QNetworkAccessManager m_manager;
};

ImageTint::ImageTint(): QWidget()
{
    setAcceptDrops(true);

    setAttribute(Qt::WA_StaticContents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));

    QWidget* toolWidget = new QWidget(this);
    toolWidget->setWindowFlags(Qt::Tool | Qt::WindowTitleHint);
    parameters.setupUi(toolWidget);
    toolWidget->show();
    toolWidget->adjustSize();

    connect(parameters.grayscaleButton, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(parameters.tintButton, SIGNAL(toggled(bool)), this, SLOT(update()));
    connect(parameters.redSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.greenSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.blueSlider, SIGNAL(valueChanged(int)), this, SLOT(update()));
    connect(parameters.compositionModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

    parameters.redSlider->setValue(0);
    parameters.greenSlider->setValue(0);
    parameters.blueSlider->setValue(96);
}

void ImageTint::handleNetworkData(QNetworkReply *networkReply)
{
    QImage image;

    qDebug() << "Received" << networkReply->size() << "bytes";
    QUrl url = networkReply->url();
    if (networkReply->error()) {
        m_fileName = QString();
        qDebug() << "Can't download" << url.toString()
                 << ":" << networkReply->errorString();
    } else {
        m_fileName = url.toString();
        if (url.scheme() == "file")
            m_fileName = url.toLocalFile();
        image.load(networkReply, 0);
    }

    networkReply->deleteLater();
    loadImage(image);
}

void ImageTint::loadImage(const QImage &image)
{
    m_image = image;
    if (m_image.isNull()) {
        setFixedSize(512, 256);
        setWindowTitle(QString("Can not load %1").arg(m_fileName));
    } else {
        QString title = "Image Tint ";
        if ((m_image.width() > 1024) || (m_image.height() > 800)) {
            qDebug() << "Image is too large. Rescaling....";
            int w = m_image.width();
            m_image = m_image.scaled(640, 480, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            title += QString("[Zoom %1%] ").arg(m_image.width() * 100 / w);
        }
        setWindowTitle(QString("%1: %2 (%3 x %4)").arg(title).arg(m_fileName).
                       arg(m_image.width()).arg(m_image.height()));
        setFixedSize(m_image.width(), m_image.height());
    }
    update();
}

void ImageTint::loadImage(const QString &fileName)
{
    m_fileName = QFileInfo(fileName).fileName();
    loadImage(QImage(fileName));
}

void ImageTint::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void ImageTint::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.count()) {
        QUrl url = urls[0];
        if (event->mimeData()->hasImage()) {
            QImage img = qvariant_cast<QImage>(event->mimeData()->imageData());
            m_fileName = url.toString();
            loadImage(img);
        } else {
            m_manager.get(QNetworkRequest(url));
            setWindowTitle(QString("Loading %1...").arg(url.toString()));
        }

        event->acceptProposedAction();
    }
}

// Convert an image to grayscale and return it as a new image
QImage grayscaled(const QImage &image)
{
    QImage img = image;
    int pixels = img.width() * img.height();
    unsigned int *data = (unsigned int *)img.bits();
    for (int i = 0; i < pixels; ++i) {
        int val = qGray(data[i]);
        data[i] = qRgba(val, val, val, qAlpha(data[i]));
    }
    return img;
}

// Tint an image with the specified color and return it as a new image
QImage tinted(const QImage &image, const QColor &color, QPainter::CompositionMode mode = QPainter::CompositionMode_Screen)
{
    QImage resultImage(image.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&resultImage);
    painter.drawImage(0, 0, grayscaled(image));
    painter.setCompositionMode(mode);
    painter.fillRect(resultImage.rect(), color);
    painter.end();
    resultImage.setAlphaChannel(image.alphaChannel());

    return resultImage;
}

void ImageTint::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if (parameters.noEffectButton->isChecked())
        painter.drawImage(0, 0, m_image);

    if (parameters.grayscaleButton->isChecked())
        painter.drawImage(0, 0, grayscaled(m_image));

    if (parameters.tintButton->isChecked()) {
        QColor tintColor;
        tintColor.setRed(parameters.redSlider->value());
        tintColor.setGreen(parameters.greenSlider->value());
        tintColor.setBlue(parameters.blueSlider->value());

        QPainter::CompositionMode table[] = {
            QPainter::CompositionMode_Screen,
            QPainter::CompositionMode_Overlay,
            QPainter::CompositionMode_Multiply
        };
        QPainter::CompositionMode mode = table[parameters.compositionModeBox->currentIndex()];

        painter.drawImage(0, 0, tinted(m_image, tintColor, mode));
    }
}

#include "imagetint.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ImageTint widget;
    widget.show();
    if (argc > 1)
        widget.loadImage(argv[1]);
    else
        widget.loadImage(":/berry.jpg");

    return app.exec();
}
