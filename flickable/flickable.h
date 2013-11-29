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

#ifndef FLICKABLE_H
#define FLICKABLE_H

class QMouseEvent;
class QPoint;
class QWidget;

class FlickableTicker;
class FlickablePrivate;

class Flickable
{
public:

    Flickable();
    virtual ~Flickable();

    void setThreshold(int threshold);
    int threshold() const;

    void setAcceptMouseClick(QWidget *target);

    void handleMousePress(QMouseEvent *event);
    void handleMouseMove(QMouseEvent *event);
    void handleMouseRelease(QMouseEvent *event);

protected:
    virtual QPoint scrollOffset() const = 0;
    virtual void setScrollOffset(const QPoint &offset) = 0;

private:
    void tick();

private:
    FlickablePrivate *d;
    friend class FlickableTicker;
};

#endif // FLICKABLE_H
