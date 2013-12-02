/* This file is part of mobile_hybrid_demo (HTML5 demonstration)

Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
All rights reserved.

Contact:  Nokia Corporation qt-info@nokia.com

You may use this file under the terms of the BSD license as follows:

"Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: *
Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer. * Redistributions in binary
form must reproduce the above copyright notice, this list of conditions and the
following disclaimer in the documentation and/or other materials provided with
the distribution. * Neither the name of Nokia Corporation and its
Subsidiary(-ies) nor the names of its contributors may be used to endorse or
promote products derived from this software without specific prior written
permission. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
*/

#include "webwidget.h"
#include <QPainter>
#include <QWebSettings>
#include <QWebFrame>
#include <QAccelerometer>
#include <QDebug>

// #include <QGLWidget>

WebWidget::WebWidget()
{
    setFrameShape(NoFrame);
    setScene(new QGraphicsScene(this));
    setContextMenuPolicy(Qt::NoContextMenu);

    m_webview.setResizesToContents(false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::TiledBackingStoreEnabled, false);
    scene()->addItem(&m_webview);
    m_webview.focusWidget();

    accelerometerSensor = new QAccelerometer(this);
    inactiveTimer.setSingleShot(true);

    if (!Timer.isActive())
        Timer.start(20, this);

    // start the sensor
    if (!accelerometerSensor->isActive())
        accelerometerSensor->start();

    if (!accelerometerSensor->isActive())
    {
        //qDebug() << "accelerometer sensor didn't start!" << endl;
    }
}

void WebWidget::loadUrl(const QUrl &url)
{
    m_webview.load(url);
    QWebFrame *frame = m_webview.page()->mainFrame();
    frame->addToJavaScriptWindowObject("testObject", this->parent());
}

void WebWidget::resizeEvent(QResizeEvent *event)
{
    m_webview.resize(event->size());
    scene()->setSceneRect(QRect(QPoint(), event->size()));
    QGraphicsView::resizeEvent(event);
}

bool WebWidget::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Timer:
        if (static_cast<QTimerEvent*>(event)->timerId() == Timer.timerId())
            updateXYZPosition(); // update the xyz position
        break;
    default:
        break;
    }
    return QGraphicsView::event(event);
}

void WebWidget::updateXYZPosition()
{
    QAccelerometerReading *reading = accelerometerSensor->reading();
    qreal xacceleration = 0.0f;
    qreal yacceleration = 0.0f;
    qreal zacceleration = 0.0f;
    if (reading) {
        xacceleration = reading->x();
        yacceleration = reading->y();
        zacceleration = reading->z();
        m_webview.page()->mainFrame()->evaluateJavaScript("accelerometer("+QString::number(qRound(xacceleration*10))+","+QString::number(qRound(yacceleration*10))+","+QString::number(qRound(zacceleration*10))+")");

     }
}
