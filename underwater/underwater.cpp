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
#include <QtWidgets>
#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

void applyUnderwater(const QImage* img, QImage* result1, QImage *result2, int amp, qreal tick)
{
    Q_ASSERT(img);
    Q_ASSERT(result1);
    Q_ASSERT(result2);
    Q_ASSERT(img->size() == result1->size());
    Q_ASSERT(img->size() == result2->size());
    Q_ASSERT(img->format() == QImage::Format_ARGB32);

    int height = img->height();
    int width = img->width();
    int stride =  img->bytesPerLine();

    // shift horizontally
    for (int y = 0; y < height; ++y) {
        const uchar* src = img->scanLine(y);
        uchar* dst = result1->scanLine(y);
        int shift = amp / 2 * (1 + sin((4 * y / qreal(height) + tick) * 2 * M_PI));
        int offset = 4 * shift;
        memcpy(dst + offset, src, img->bytesPerLine() - offset);
    }

    // shift vertically
    for (int x = 0; x < width; ++x) {
        int shift = amp / 2 * (1 + sin((4 * x / qreal(width) + tick) * 2 * M_PI));
        const uchar* src = result1->scanLine(0) + x * 4;
        uchar* dst = result2->scanLine(shift) + x * 4;
        for (int c = 0; c < height - shift; ++c, dst += stride, src += stride) {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = src[3];
        }
    }

}

class UnderwaterDemo: public QWidget
{
public:
    UnderwaterDemo();
    void loadImage(const QString& file);
    void loadImage(const QImage& image);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent* event);
    void paintEvent(QPaintEvent*);

private:
    QString m_fileName;
    QImage m_image;
    QImage m_modifiedImage1;
    QImage m_modifiedImage2;
    QTimeLine* m_timeLine;
    QGroupBox* m_enableEffect;
    QSlider* m_amplitudeSlider;
    QSlider* m_speedSlider;
};

UnderwaterDemo::UnderwaterDemo()
{
    setAcceptDrops(true);

    setAttribute(Qt::WA_StaticContents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    m_timeLine = new QTimeLine(1000, this);
    m_timeLine->setFrameRange(0, 4096);
    m_timeLine->setCurveShape(QTimeLine::LinearCurve);
    m_timeLine->setLoopCount(0);
    connect(m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(update()));

    QWidget* toolWidget = new QWidget(this);
    toolWidget->setWindowTitle("Parameters");
    toolWidget->setWindowFlags(Qt::Tool | Qt::WindowTitleHint);
    toolWidget->show();
    QVBoxLayout* toolLayout = new QVBoxLayout;
    toolWidget->setLayout(toolLayout);
    toolLayout->setSizeConstraint(QLayout::SetFixedSize);

    m_enableEffect = new QGroupBox(toolWidget);
    m_enableEffect->setCheckable(true);
    m_enableEffect->setTitle("Enable");
    m_enableEffect->setFlat(true);
    connect(m_enableEffect, SIGNAL(toggled(bool)), SLOT(update()));
    toolLayout->addWidget(m_enableEffect);

    m_amplitudeSlider = new QSlider(toolWidget);
    m_amplitudeSlider->setRange(5, 16);
    m_amplitudeSlider->setOrientation(Qt::Horizontal);
    connect(m_amplitudeSlider, SIGNAL(sliderMoved(int)), SLOT(update()));

    m_speedSlider = new QSlider(toolWidget);
    m_speedSlider->setRange(1000, 4000);
    m_speedSlider->setOrientation(Qt::Horizontal);
    connect(m_speedSlider, SIGNAL(sliderMoved(int)), SLOT(update()));

    QGridLayout* layout = new QGridLayout;
    m_enableEffect->setLayout(layout);
    layout->addWidget(new QLabel("Amplitude"), 0, 0);
    layout->addWidget(m_amplitudeSlider, 0, 1);
    layout->addWidget(new QLabel("Speed"), 1, 0);
    layout->addWidget(m_speedSlider, 1, 1);

    toolWidget->adjustSize();

    m_enableEffect->setChecked(true);
    m_amplitudeSlider->setValue(6);
    m_speedSlider->setValue(1000);
}

void UnderwaterDemo::loadImage(const QImage& image)
{
    m_image = image;
    if (m_image.isNull()) {
        setFixedSize(512, 256);
        setWindowTitle(QString("Can not load %1").arg(m_fileName));
    } else {
        QString title = "Underwater Effect ";
        if ((m_image.width() > 640) || (m_image.height() > 480)) {
            qDebug() << "Image is too large. Rescaling....";
            int w = m_image.width();
            m_image = m_image.scaled(640, 480, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            title += QString("[Zoom %1%] ").arg(m_image.width() * 100 / w);
        }
        setWindowTitle(QString("%1: %2 (%3 x %4)").arg(title).arg(m_fileName).
                       arg(m_image.width()).arg(m_image.height()));
        setFixedSize(m_image.width(), m_image.height());
        m_image = m_image.convertToFormat(QImage::Format_ARGB32);
    }

    m_modifiedImage1 = m_image;
    m_modifiedImage2 = m_image;
    update();
}

void UnderwaterDemo::loadImage(const QString& fileName)
{
    m_fileName = QFileInfo(fileName).fileName();
    loadImage(QImage(fileName));
}

void UnderwaterDemo::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void UnderwaterDemo::dropEvent(QDropEvent* event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.count()) {
        QString fname = urls[0].toLocalFile();
        if (event->mimeData()->hasImage()) {
            QImage img = qvariant_cast<QImage>(event->mimeData()->imageData());
            m_fileName = QFileInfo(fname).fileName();
            loadImage(img);
        } else
            loadImage(fname);
        event->acceptProposedAction();
    }
}

void UnderwaterDemo::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (!m_enableEffect->isChecked()) {
        m_timeLine->setPaused(true);
        painter.drawImage(0, 0, m_image);
    } else {
        if (m_timeLine->state() != QTimeLine::Running)
            m_timeLine->start();
        int speed = m_speedSlider->value();
        int duration = m_speedSlider->maximum() - speed + m_speedSlider->minimum();
        m_timeLine->setDuration(duration);

        int amp = m_amplitudeSlider->value();
        qreal tick = m_timeLine->currentValue();
        if (!m_image.isNull())
            if (!m_modifiedImage1.isNull())
                if (!m_modifiedImage2.isNull())
                    applyUnderwater(&m_image, &m_modifiedImage1, &m_modifiedImage2, amp, tick);
        painter.drawImage(0, 0, m_modifiedImage2);
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    UnderwaterDemo widget;
    widget.show();
    if (argc > 1)
        widget.loadImage(argv[1]);
    else
        widget.loadImage("://bridge.jpg");


    return app.exec();
}
