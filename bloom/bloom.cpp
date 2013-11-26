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
// Blur the image according to the blur radius
// Based on exponential blur algorithm by Jani Huhtanen
// (maximum radius is set to 16)
QImage blurred(const QImage& image, const QRect& rect, int radius)
{
    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

    QImage result = image;
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char* p;

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r1) + col * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p += bpl;
        for (int j = r1; j < r2; j++, p += bpl)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c1 * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p += 4;
        for (int j = c1; j < c2; j++, p += 4)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r2) + col * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p -= bpl;
        for (int j = r1; j < r2; j++, p -= bpl)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c2 * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p -= 4;
        for (int j = c1; j < c2; j++, p -= 4)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    return result;
}

// Change brightness (positive integer) of each pixel
QImage brightened(const QImage& image, int brightness)
{
    int tab[ 256 ];
    for (int i = 0; i < 256; ++i)
        tab[i] = qMin(i + brightness, 255);

    QImage img = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < img.height(); y++) {
        QRgb* line = (QRgb*)(img.scanLine(y));
        for (int x = 0; x < img.width(); x++)
            line[x] = qRgb(tab[qRed(line[x])], tab[qGreen(line[x])], tab[qBlue(line[x])]);
    }

    return img;
}

// Composite two QImages using given composition mode and opacity
QImage composited(const QImage& img1, const QImage& img2, int opacity, QPainter::CompositionMode mode)
{
    QImage result = img1.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&result);
    painter.setCompositionMode(mode);
    painter.setOpacity((qreal)(opacity) / 256.0);
    painter.drawImage(0, 0, img2);
    painter.end();
    return result;
}

// Apply Bloom effect with the 4 parameters
QImage bloomed(const QImage& img, int blurRadius, int brightness, int opacity,
               QPainter::CompositionMode mode)
{
    // (1) blur the original image
    QImage step1 = blurred(img, img.rect(), blurRadius);

    // (2) increase the brightness of the blurred image
    QImage step2 = brightened(step1, brightness);

    // (3) finally overlay with the original image
    QImage step3 = composited(img, step2, opacity, mode);

    return step3;
}

class BloomDemo: public QWidget
{
public:
    BloomDemo();
    void loadImage(const QString& file);
    void loadImage(const QImage& image);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent* event);
    void paintEvent(QPaintEvent*);

private:
    QString m_fileName;
    QImage m_image;
    QGroupBox* m_enableEffect;
    QSlider* m_blurSlider;
    QSlider* m_brightnessSlider;
    QSlider* m_opacitySlider;
    QComboBox* m_modeSelector;
};

BloomDemo::BloomDemo()
{
    setAcceptDrops(true);

    setAttribute(Qt::WA_StaticContents, true);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

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

    m_blurSlider = new QSlider(toolWidget);
    m_blurSlider->setRange(0, 20);
    m_blurSlider->setOrientation(Qt::Horizontal);
    connect(m_blurSlider, SIGNAL(sliderMoved(int)), SLOT(update()));

    m_brightnessSlider = new QSlider(toolWidget);
    m_brightnessSlider->setRange(0, 50);
    m_brightnessSlider->setOrientation(Qt::Horizontal);
    connect(m_brightnessSlider, SIGNAL(sliderMoved(int)), SLOT(update()));

    m_opacitySlider = new QSlider(toolWidget);
    m_opacitySlider->setRange(0, 255);
    m_opacitySlider->setOrientation(Qt::Horizontal);
    connect(m_opacitySlider, SIGNAL(sliderMoved(int)), SLOT(update()));

    m_modeSelector = new QComboBox(toolWidget);
    m_modeSelector->setEditable(false);
    m_modeSelector->addItem("Multiply");
    m_modeSelector->addItem("Screen");
    m_modeSelector->addItem("Overlay");
    m_modeSelector->addItem("Darken");
    m_modeSelector->addItem("Lighten");
    m_modeSelector->addItem("Hard Light");
    m_modeSelector->addItem("Soft Light");
    connect(m_modeSelector, SIGNAL(currentIndexChanged(int)), SLOT(update()));

    QGridLayout* layout = new QGridLayout;
    m_enableEffect->setLayout(layout);
    layout->addWidget(new QLabel("Blur radius"), 0, 0);
    layout->addWidget(m_blurSlider, 0, 1);
    layout->addWidget(new QLabel("Brightness"), 1, 0);
    layout->addWidget(m_brightnessSlider, 1, 1);
    layout->addWidget(new QLabel("Opacity"), 2, 0);
    layout->addWidget(m_opacitySlider, 2, 1);
    layout->addWidget(new QLabel("Composition mode"), 3, 0);
    layout->addWidget(m_modeSelector, 3, 1);

    toolWidget->adjustSize();

    // sane default values
    m_enableEffect->setChecked(true);
    m_blurSlider->setValue(6);
    m_brightnessSlider->setValue(30);
    m_opacitySlider->setValue(192);
    m_modeSelector->setCurrentIndex(2);
}

void BloomDemo::loadImage(const QImage& image)
{
    m_image = image;
    if (m_image.isNull()) {
        setFixedSize(512, 256);
        setWindowTitle(QString("Can not load %1").arg(m_fileName));
    } else {
        QString title = "Bloom Effect ";
        if ((m_image.width() > 640) || (m_image.height() > 480)) {
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

void BloomDemo::loadImage(const QString& fileName)
{
    m_fileName = QFileInfo(fileName).fileName();
    loadImage(QImage(fileName));
}

void BloomDemo::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void BloomDemo::dropEvent(QDropEvent* event)
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

void BloomDemo::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (!m_enableEffect->isChecked())
        painter.drawImage(0, 0, m_image);
    else {
        int blurRadius = m_blurSlider->value();
        int brightness = m_brightnessSlider->value();
        int opacity = m_opacitySlider->value();

        QPainter::CompositionMode table[] = {
            QPainter::CompositionMode_Multiply,
            QPainter::CompositionMode_Screen,
            QPainter::CompositionMode_Overlay,
            QPainter::CompositionMode_Darken,
            QPainter::CompositionMode_Lighten,
            QPainter::CompositionMode_HardLight,
            QPainter::CompositionMode_SoftLight
        };
        QPainter::CompositionMode mode = table[m_modeSelector->currentIndex()];

        painter.drawImage(0, 0, bloomed(m_image, blurRadius, brightness, opacity, mode));
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    BloomDemo widget;
    widget.show();
    if (argc > 1)
        widget.loadImage(argv[1]);
    else
        widget.loadImage(":/scene.jpg");


    return app.exec();
}
