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
#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// comment out to disable reflection
#define PIE_REFLECTION

static qreal shiftHue(qreal hue, qreal shift)
{
    hue += shift;
    while (hue >= 1.0) hue -= 1.0;
    while (hue < 0.0) hue += 1.0;
    return hue;
}

void drawHsvPie(QImage* img, int radius, int depth, qreal ratio, qreal limit, qreal init)
{
    Q_ASSERT(img);

    int width = img->width();
    int height = img->height();
#ifdef PIE_REFLECTION
    int center = (height - 2 * depth) / 2;
#else
    int center = (height - depth) / 2;
#endif

    for (int i = 0; i < radius; i++) {
        qreal hue = 1 - init;
        qreal sat = 1 - qreal(i) / radius;
        for (int d = 0; d < depth; d++) {
            qreal value = 1 - qreal(d) / depth;
            QColor color = QColor::fromHsvF(hue, sat, value);
            img->setPixel(width / 2 - radius + i + 1, center + d, color.rgb());
        }
    }

    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            qreal rx = x - width / 2;
            qreal ry = y - center;
            ry = ry / ratio;
            qreal d = rx * rx + ry * ry;
            if (d < radius*radius) {
                qreal hue = (atan2(ry, rx) + M_PI) / (2 * M_PI);
                if (hue < limit) {
                    hue = 1 - shiftHue(hue, init);
                    qreal sat = sqrt(d) / radius;
                    QColor color = QColor::fromHsvF(hue, sat, 1);
                    img->setPixel(x, y, color.rgb());
                }
            }
        }


    if ((limit > 0.25) || (limit < 0.75)) {
        qreal hue = shiftHue(limit, init);
        qreal angle = (-0.5 + limit) * 2 * M_PI;
        for (int x = 0; x < radius*cos(angle); x++) {
            qreal yy = tan(angle) * x;
            int y = int(yy * ratio);
            qreal sat = sqrt(yy * yy + x * x) / radius;
            for (int d = 0; d < depth; d++) {
                qreal value = 1 - qreal(d) / depth;
                QColor color = QColor::fromHsvF(hue, sat, value);
                img->setPixel(width / 2 + x, center + y + d, color.rgb());
            }
        }
    }

    if (limit > 0.5) {
        for (int x = radius - 1; x > -radius; x--) {
            qreal yy = sqrt(radius * radius - x * x);
            qreal hue = (atan2(yy, qreal(x)) + M_PI) / (2 * M_PI);
            if (hue > limit)
                break;
            int y = int(yy * ratio);
            hue = 1 - shiftHue(hue, init);
            for (int d = 0; d < depth; d++) {
                qreal value = 1 - qreal(d) / depth;
                QColor color = QColor::fromHsvF(hue, 1, value);
                img->setPixel(width / 2 + x, center + y + d, color.rgb());
            }
        }
    }
}

static QRgb blendColor(QRgb c1, QRgb c2, qreal blend)
{
    qreal r = qRed(c1) * blend + qRed(c2) * (1 - blend);
    qreal g = qGreen(c1) * blend + qGreen(c2) * (1 - blend);
    qreal b = qBlue(c1) * blend + qBlue(c2) * (1 - blend);
    return qRgb(int(r), int(g), int(b));
}

void drawReflection(const QImage& img, QImage* result, qreal opacity, int offset, int shift)
{
    QRgb bgpixel = qRgb(255, 255, 255);

    for (int x = 0; x < img.width(); x++) {
        int y;
        for (y = img.height() - 1; y >= 0; y--)
            if (img.pixel(x, y) != bgpixel)
                break;
        if (y > 0) {
            int ys = y  - shift;
            int yp = y;
            for (int h = 0; h < offset; h++)
                if ((ys < img.height()) && (yp >= 0)) {
                    QRgb c = img.pixel(x, yp--);
                    qreal a =  1 - qreal(h) / offset;
                    a *= opacity;
                    result->setPixel(x, ys++, blendColor(c, bgpixel, a));
                } else
                    break;
        }
    }
}

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

class QImage createHsvPie(int radius, int depth, qreal ratio, qreal limit, qreal init)
{
    int width = int(radius * 2.4);
    int height = depth + int(radius * ratio * 2.4);
#ifdef PIE_REFLECTION
    height += depth;
#endif

    QImage img(width, height, QImage::Format_ARGB32_Premultiplied);
    img.fill(qRgb(255, 255, 255));

    drawHsvPie(&img, radius, depth, ratio, limit, init);

#ifdef PIE_REFLECTION
    int blur = radius / 50;
    QImage imgr = img;
    imgr.fill(qRgb(255, 255, 255));
    if (limit > 0.75) {
        QImage half = img;
        half.fill(qRgb(255, 255, 255));
        drawHsvPie(&half, radius, depth, ratio, 0.5, init);
        drawReflection(half, &imgr, 0.85, depth * 3 / 4, 2 * blur);
    }
    drawReflection(img, &imgr, 0.85, depth * 3 / 4, 2 * blur);
    img = blurred(imgr, img.rect(), blur);
    drawHsvPie(&img, radius, depth, ratio, limit, init);
#endif

    return img;
}

class HsvPie: public QWidget
{
public:
    HsvPie();
    void paintEvent(QPaintEvent*);

private:
    QPixmap m_pixmap;
};

HsvPie::HsvPie()
{
    setWindowTitle("HSV Pie");

    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

#if 0
    QImage pie = createHsvPie(200, 75, 0.7, 0.67, 0.33);
#else
    // smoother result by oversampling
    QImage pie = createHsvPie(4 * 200, 4 * 75, 0.7, 0.67, 0.33);
    pie = pie.scaled(pie.size() / 4, Qt::KeepAspectRatio, Qt::SmoothTransformation);
#endif

    m_pixmap = QPixmap::fromImage(pie);
    resize(m_pixmap.size());
}

void HsvPie::paintEvent(QPaintEvent*)
{
    int x = (width() - m_pixmap.width()) / 2;
    int y = (height() - m_pixmap.height()) / 2;
    QPainter painter(this);
    painter.drawPixmap(x, y, m_pixmap);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    HsvPie widget;
    widget.show();

    return app.exec();
}

