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

#include "ui_controls.h"


class ParallaxSlide : public QGraphicsView
{
    Q_OBJECT

public:
    ParallaxSlide();

public slots:
    void slideLeft();
    void slideRight();

protected:
    void slideBy(int dx);
    void setParallaxEnabled(bool);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void moveIcons(int x);
    void moveBackground(int x);
    void adjustParameters();

private:
    QGraphicsScene scene;
    QGraphicsItem *background;
    QList<QGraphicsItem*> icons;
    QTimeLine iconTimeLine;
    QTimeLine backgroundTimeLine;
    int ofs;
    qreal factor;
    Ui::ControlsForm controls;
};

ParallaxSlide::ParallaxSlide(): QGraphicsView(), ofs(0), factor(1)
{
    setScene(&scene);

    background = scene.addPixmap(QPixmap(":/background.jpg"));
    background->setZValue(0.0);
    background->setPos(0, 0);

    for (int i = 0; i < 7; i++) {
        QString str = QString(":/icon%1.png").arg(i+1);
        QGraphicsItem *icon = scene.addPixmap(QPixmap(str));
        icon->setPos(320+i*64, 400);
        icon->setZValue(1.0);
        icons += icon;
    }

    setFixedSize(320, 480);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(&iconTimeLine, SIGNAL(frameChanged(int)), this, SLOT(moveIcons(int)));
    iconTimeLine.setCurveShape(QTimeLine::EaseInOutCurve);

    connect(&backgroundTimeLine, SIGNAL(frameChanged(int)), this, SLOT(moveBackground(int)));
    connect(&backgroundTimeLine, SIGNAL(finished()), this, SLOT(adjustParameters()));
    backgroundTimeLine.setCurveShape(QTimeLine::EaseInOutCurve);

    QWidget* toolWidget = new QWidget(this);
    toolWidget->setWindowFlags(Qt::Tool | Qt::WindowTitleHint);
    controls.setupUi(toolWidget);
    toolWidget->show();

    connect(controls.speedSlider, SIGNAL(valueChanged(int)), this, SLOT(adjustParameters()));
    connect(controls.normalButton, SIGNAL(clicked()), this, SLOT(adjustParameters()));
    connect(controls.parallaxButton, SIGNAL(clicked()), this, SLOT(adjustParameters()));
    connect(controls.leftButton, SIGNAL(clicked()), this, SLOT(slideLeft()));
    connect(controls.rightButton, SIGNAL(clicked()), this, SLOT(slideRight()));

    slideBy(-320);
    adjustParameters();
}

void ParallaxSlide::slideLeft()
{
    if (iconTimeLine.state() != QTimeLine::NotRunning)
        return;

    if (ofs > -640)
        slideBy(-320);
}

void ParallaxSlide::slideRight()
{
    if (iconTimeLine.state() != QTimeLine::NotRunning)
        return;

    if (ofs < 0)
        slideBy(320);
}

void ParallaxSlide::slideBy(int dx)
{
    int iconStart = ofs;
    int iconEnd = ofs + dx;
    iconTimeLine.setFrameRange(iconStart, iconEnd);
    iconTimeLine.start();

    int backgroundStart = -320 - (int)((-320 - iconStart)/factor);
    int backgroundEnd = -320 - (int)((-320 - iconEnd)/factor);
    backgroundTimeLine.setFrameRange(backgroundStart, backgroundEnd);
    backgroundTimeLine.start();

    ofs = iconEnd;
}

void ParallaxSlide::setParallaxEnabled(bool p)
{
    factor = p ? 2 : 1;
    if (p)
        setWindowTitle("Sliding - Parallax mode");
    else
        setWindowTitle("Sliding - Normal mode");
}

void ParallaxSlide::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left)
        slideLeft();

    if (event->key() == Qt::Key_Right)
        slideRight();
}

void ParallaxSlide::moveIcons(int x)
{
    for (int i = 0; i < icons.count(); i++)
        icons[i]->setPos(320 + x+i*64, icons[i]->pos().y());
}

void ParallaxSlide::moveBackground(int x)
{
    background->setPos(x, background->pos().y());
}

void ParallaxSlide::adjustParameters()
{
    int speed = controls.speedSlider->value();
    iconTimeLine.setDuration(1200 - speed*10);
    backgroundTimeLine.setDuration(1200 - speed*10);
    setParallaxEnabled(controls.parallaxButton->isChecked());
    controls.leftButton->setEnabled(ofs > -640);
    controls.rightButton->setEnabled(ofs < 0);
}

#include "parallaxslide.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ParallaxSlide slider;
    slider.show();

    return app.exec();
}
