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

#include "dragmovecharm.h"

#include <QEvent>
#include <QHash>
#include <QMouseEvent>
#include <QWidget>

struct DragMoveData {
    bool isMoving;
    QPoint startDrag;
};

class DragMoveCharmPrivate
{
public:
    QHash<QWidget*, DragMoveData*> dragMoveData;
};

DragMoveCharm::DragMoveCharm(QObject *parent): QObject(parent)
{
    d = new DragMoveCharmPrivate;
}

DragMoveCharm::~DragMoveCharm()
{
    delete d;
}

void DragMoveCharm::activateOn(QWidget *widget)
{
    if (d->dragMoveData.contains(widget))
        return;

    DragMoveData *data = new DragMoveData;
    data->startDrag = QPoint(0, 0);
    data->isMoving = false;
    d->dragMoveData[widget] = data;

    widget->installEventFilter(this);
}

void DragMoveCharm::deactivateFrom(QWidget *widget)
{
    delete d->dragMoveData[widget];
    d->dragMoveData.remove(widget);
    widget->removeEventFilter(this);
}

bool DragMoveCharm::eventFilter(QObject *object, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget*>(object);
    if (!widget)
        return false;

    QEvent::Type type = event->type();
    if (type != QEvent::MouseButtonPress &&
            type != QEvent::MouseButtonRelease &&
            type != QEvent::MouseMove)
        return false;

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (!mouseEvent || mouseEvent->modifiers() != Qt::NoModifier)
        return false;
    Qt::MouseButton button = mouseEvent->button();

    DragMoveData *data = d->dragMoveData.value(widget);
    if (!widget || !data)
        return false;

    bool consumed = false;

    if (type == QEvent::MouseButtonPress && button == Qt::LeftButton) {
        data->startDrag = mouseEvent->globalPos();
        data->isMoving = true;
        event->accept();
        consumed = true;
    }

    if (type == QEvent::MouseButtonRelease) {
        data->startDrag = QPoint(0, 0);
        data->isMoving = false;
    }

    if (type == QEvent::MouseMove && data->isMoving) {
        QPoint pos = mouseEvent->globalPos();
        widget->move(widget->pos() + pos - data->startDrag);
        data->startDrag = pos;
        consumed = true;
    }

    return consumed;
}
