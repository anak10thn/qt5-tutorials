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
#include "ui_form.h"

#define PANEL_WIDTH 150
#define PANEL_MARGIN 20

class CustomTimeLine: public QTimeLine
{
private:
    qreal m_factor;
    qreal m_offset;
    qreal m_range;

public:
    bool useLogisticFunction;

    CustomTimeLine(): QTimeLine(), useLogisticFunction(false) {
        setFactor(4);
    }

    void setFactor(qreal factor) {
        m_factor = factor;
        qreal max = 1 / (1 + exp(-factor));
        m_offset = 1 / (1 + exp(factor));
        m_range = max - m_offset;
    }

    virtual qreal valueForTime(int msec) const {
        if (useLogisticFunction) {
            // this is normalized x
            qreal x = static_cast<qreal>(msec) / duration();

            // make it two-sided, 0 is the center
            qreal xs = x * 2 * m_factor - m_factor;

            // the simple logistic function
            qreal y = 1 / (1 + exp(-xs));

            // normalize to 0..1
            y = (y - m_offset) / m_range;

            return y;
        }
        else
            return QTimeLine::valueForTime(msec);
    }
};

class Panel: public QWidget
{
Q_OBJECT

private:
    QListWidget *m_list;

public:
    Panel(): QWidget() {
        m_list = new QListWidget(this);
        m_list->addItems(QColor::colorNames());

        // fixed position, no layout nor container
        m_list->move(0, 0);
    }

protected:
    void resizeEvent(QResizeEvent*) {
        if (m_list->height() != height())
            m_list->resize(PANEL_WIDTH, height());
    }
};

class TimeLineDemo: public QWidget
{
Q_OBJECT

private:
    QWidget *m_panel;
    CustomTimeLine *m_timeLine;
    QHash<QRadioButton*, int> m_curveMap;
    QGraphicsScene *m_scene;
    QGraphicsEllipseItem *m_curveDot;
    QGraphicsPathItem *m_curvePath;
    Ui::Form form;

public:
    TimeLineDemo(): QWidget() {

        form.setupUi(this);
        form.hideButton->hide();

        m_panel = new Panel;
        m_panel->setWindowFlags(Qt::FramelessWindowHint |
                                Qt::X11BypassWindowManagerHint |
                                Qt::Tool);
        m_panel->hide();
        m_panel->resize(0, 0);

        m_timeLine = new CustomTimeLine;
        m_timeLine->setCurveShape(QTimeLine::LinearCurve);
        m_timeLine->setFrameRange(0, PANEL_WIDTH);
        connect(m_timeLine, SIGNAL(frameChanged(int)), SLOT(setSlidePosition(int)));
        connect(m_timeLine, SIGNAL(stateChanged(QTimeLine::State)), SLOT(changeState()));
        connect(m_timeLine, SIGNAL(finished()), m_timeLine, SLOT(toggleDirection()));

        connect(form.hideButton, SIGNAL(clicked()), m_timeLine, SLOT(start()));
        connect(form.showButton, SIGNAL(clicked()), m_timeLine, SLOT(start()));
        connect(form.easeInButton, SIGNAL(toggled(bool)), SLOT(adjustCurveShape()));
        connect(form.easeOutButton, SIGNAL(toggled(bool)), SLOT(adjustCurveShape()));
        connect(form.easeInOutButton, SIGNAL(toggled(bool)), SLOT(adjustCurveShape()));
        connect(form.linearButton, SIGNAL(toggled(bool)), SLOT(adjustCurveShape()));
        connect(form.sCurveButton, SIGNAL(toggled(bool)), SLOT(adjustCurveShape()));
        connect(form.curveSlider, SIGNAL(valueChanged(int)), SLOT(changeCurveFactor(int)));

        m_curveMap[form.easeInButton] = static_cast<int>(QTimeLine::EaseInCurve);
        m_curveMap[form.easeOutButton] = static_cast<int>(QTimeLine::EaseOutCurve);
        m_curveMap[form.easeInOutButton] = static_cast<int>(QTimeLine::EaseInOutCurve);
        m_curveMap[form.linearButton] = static_cast<int>(QTimeLine::LinearCurve);

        m_scene = new QGraphicsScene(this);
        form.graphicsView->setScene(m_scene);
        form.graphicsView->setTransform(QTransform().scale(1, -1));
        form.graphicsView->setRenderHints(QPainter::Antialiasing);
        m_scene->addRect(0, 0, PANEL_WIDTH, PANEL_WIDTH, QPen(Qt::blue, 0.5));
        m_curveDot = m_scene->addEllipse(0, 0, 10, 10, QPen(), QBrush(Qt::red));
        m_curvePath = m_scene->addPath(QPainterPath(), QPen(Qt::black, 1.5));
        m_timeLine->setFactor(form.curveSlider->value());
        adjustCurvePath();
    }

    ~TimeLineDemo() {
        delete m_panel;
    }

private:

    void adjustCurvePath() {
        QPainterPath path;
        path.moveTo(0, 0);
        for (qreal i = 0.0; i <= 32.0; ++i) {
            qreal x = i / 32.0;
            int t = static_cast<int>(x * m_timeLine->duration());
            qreal y = m_timeLine->valueForTime(t);
            path.lineTo(PANEL_WIDTH * x, PANEL_WIDTH * y);
        }
        m_curvePath->setPath(path);
    }

public slots:

    void changeState() {
        int speed = form.speedSlider->value();
        m_timeLine->setDuration(1200 - speed*10);
        bool notrunning = m_timeLine->state() == QTimeLine::NotRunning;
        form.showButton->setEnabled(notrunning);
        form.hideButton->setEnabled(notrunning);
        form.showButton->setVisible(m_timeLine->direction() == QTimeLine::Backward);
        form.hideButton->setVisible(!form.showButton->isVisible());
        m_panel->show();
        m_panel->raise();
    }

private slots:

    void setSlidePosition(int pos) {
        m_panel->resize(pos, m_panel->height());
        m_panel->move(this->pos() - QPoint(pos, -PANEL_MARGIN - 20));
        qreal t = static_cast<qreal>(m_timeLine->currentTime());
        qreal x = t * PANEL_WIDTH / m_timeLine->duration();
        m_curveDot->setPos(x - 5, pos - 5);
    }

    void adjustCurveShape() {
        QRadioButton *button = qobject_cast<QRadioButton*>(sender());
        int c = m_curveMap[button];
        QTimeLine::CurveShape shape = static_cast<QTimeLine::CurveShape>(c);
        m_timeLine->useLogisticFunction = (button == form.sCurveButton);
        if (button)
            m_timeLine->setCurveShape(shape);
        adjustCurvePath();
        form.curveSlider->setEnabled(button == form.sCurveButton);
        form.curveLabel->setEnabled(button == form.sCurveButton);
    }

    void changeCurveFactor(int factor) {
        m_timeLine->setFactor(factor);
        adjustCurvePath();
    }

protected:
    void moveEvent(QMoveEvent *event) {
        m_panel->move(m_panel->pos() + event->pos() - event->oldPos());
    }

    void resizeEvent(QResizeEvent*) {
        m_panel->resize(m_panel->width(), height() - 2 * PANEL_MARGIN);
    }
};

#include "s-curve.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TimeLineDemo demo;
    demo.show();

    return app.exec();
}
