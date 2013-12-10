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

#include "v8.h"

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#define MONSTER_JS_URL "http://deanm.github.com/pre3d/monster.js"

#if defined(Q_WS_MAC)
#define MONSTER_OFFSCREEN
#endif

struct PaintCommand {
    qreal opacity;
    QPen stroke;
    QBrush fill;
    QPolygonF polygon;
};

static inline QColor parseColor(const QString &name)
{
    if (name == "none")
        return QColor();

    if (name.startsWith("rgba(")) {
        int start = name.indexOf('(');
        int end = name.indexOf(')', start + 2);
        QString substr = name.mid(start + 1, end - start - 1);
        QStringList colors = substr.trimmed().split(',');
        if (colors.count() == 4) {
            int r = colors[0].toInt();
            int g = colors[1].toInt();
            int b = colors[2].toInt();
            int a = qMax(0, static_cast<int>(colors[3].toDouble() * 255));
            return QColor(r, g, b, a);
        }
    }

    return QColor(name);
}

static v8::Handle<v8::Value> drawPolygons(const v8::Arguments& args);

class CyMonster : public QWidget
{
    Q_OBJECT

private:

    v8::Handle<v8::ObjectTemplate> global;
    v8::Handle<v8::Context> context;
    QNetworkAccessManager *manager;
#ifdef MONSTER_OFFSCREEN
    QPixmap pixmap;
#endif

public:

    QVector<PaintCommand> commandBuffer;
    int fps;

    CyMonster(QWidget *parent = 0): QWidget(parent), fps(0) {

        v8::HandleScope handle_scope;
        global = v8::ObjectTemplate::New();
        global->Set(v8::String::New("drawPolygons"), v8::FunctionTemplate::New(drawPolygons));
        global->Set(v8::String::New("parent"), v8::External::Wrap((void*)(this)));
        context = v8::Context::New(NULL, global);

        manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(start(QNetworkReply*)));

        QUrl url(MONSTER_JS_URL);
        QNetworkReply *reply = manager->get(QNetworkRequest(url));
        connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(progress(qint64, qint64)));
        setWindowTitle(QString("About to download %1").arg(MONSTER_JS_URL));

#ifdef MONSTER_OFFSCREEN
        pixmap = QPixmap(800, 600);
        pixmap.fill(Qt::black);
#endif
    }

private:

    void engineEvaluate(const QString &scriptCode) {
        v8::HandleScope handle_scope;
        v8::Context::Scope context_scope(context);
        v8::Handle<v8::String> source;
        source = v8::String::New((const uint16_t*)(scriptCode.constData()), scriptCode.length());
        v8::Handle<v8::Script> script = v8::Script::Compile(source);
        if (!script.IsEmpty())
            v8::Handle<v8::Value> result = script->Run();
    }

public slots:

    void progress(qint64 received, qint64 total) {
        int percent = received * 100 / total;
        setWindowTitle(QString("Downloading %1: %2%").arg(MONSTER_JS_URL).arg(percent));
    }

    void start(QNetworkReply *networkReply) {
        QUrl url = networkReply->url();
        if (networkReply->error()) {
            setWindowTitle(QString("Can't download %1: %2").arg(url.toString())
                           .arg(networkReply->errorString()));
        } else {

            setWindowTitle("Monster Evolution in Qt");
            setAttribute(Qt::WA_OpaquePaintEvent, true);

            QFile file;
            file.setFileName(":/magic.js");
            file.open(QIODevice::ReadOnly);
            QString magicCode = file.readAll();
            file.close();
            engineEvaluate(magicCode);

            QString scriptCode = networkReply->readAll();
            engineEvaluate(scriptCode);
            engineEvaluate("window.onLoad()");

            // capped at 50 fps
            startTimer(20);
        }
        networkReply->deleteLater();
    }

protected:

    void timerEvent(QTimerEvent*) {
        v8::HandleScope handle_scope;
        v8::Context::Scope context_scope(context);

        v8::Handle<v8::Object> windowObject = context->Global()->Get(v8::String::New("window"))->ToObject();
        v8::Handle<v8::Object> onTimerObject = windowObject->Get(v8::String::New("onTimer"))->ToObject();
        v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(onTimerObject);

        func->Call(windowObject, 0, 0);
    }

    void paintEvent(QPaintEvent*) {
        QPainter p;
#ifdef MONSTER_OFFSCREEN
        p.begin(&pixmap);
#else
        p.begin(this);
#endif
        p.setRenderHint(QPainter::Antialiasing, true);
        const PaintCommand *cmd = commandBuffer.constData();
        int count = commandBuffer.count();
        for (int i = 0; i < count; ++i, ++cmd) {
            p.setPen(cmd->stroke);
            p.setOpacity(cmd->opacity);
            p.setBrush(cmd->fill);
            p.drawPolygon(cmd->polygon);
        }
        p.setPen(Qt::white);
        p.drawText(10, 600 - 20, QString("%1 FPS").arg(fps));
        p.end();

#ifdef MONSTER_OFFSCREEN
        p.begin(this);
        p.drawPixmap(0,0, pixmap);
        p.end();
#endif
    }

};

static QString toQString(v8::Local<v8::String> str)
{
    QString result;
    result.resize(str->Length());
    str->Write(reinterpret_cast<uint16_t*>(result.data()));
    return result;
}

v8::Handle<v8::Value> drawPolygons(const v8::Arguments& args)
{
    v8::HandleScope handle_scope;
    v8::Local<v8::Value> parentValue = args.This()->Get(v8::String::New("parent"));
    CyMonster *cymonster = static_cast<CyMonster*>(v8::External::Unwrap(parentValue));

    int polygonCount = args[0]->Int32Value();
    v8::Local<v8::Object> polygonBuffer = args[1]->ToObject();

    cymonster->commandBuffer.resize(polygonCount);

    for (int c = 0; c < polygonCount; ++c) {
        v8::Local<v8::Object> cmd = polygonBuffer->Get(v8::Int32::New(c))->ToObject();

        qreal alpha = cmd->Get(v8::Int32::New(0))->NumberValue();
        QString strokeColor = toQString(cmd->Get(v8::Int32::New(1))->ToString());
        QString fillColor = toQString(cmd->Get(v8::Int32::New(2))->ToString());
        int pointCount = cmd->Get(v8::Int32::New(3))->Int32Value();

        v8::Local<v8::Object> pointArray = cmd->Get(v8::Int32::New(4))->ToObject();

        QPolygonF polygon(pointCount / 2);
        for (int p = 0; p < pointCount / 2; ++p) {
            qreal x =  pointArray->Get(v8::Int32::New(2 * p))->NumberValue();
            qreal y =  pointArray->Get(v8::Int32::New(2 * p + 1))->NumberValue();
            polygon[p] = QPointF(x, y);
        }

        PaintCommand *paint = cymonster->commandBuffer.data() + c;
        paint->opacity = alpha;
        paint->stroke = QPen(Qt::NoPen);
        if (strokeColor != "none") {
            paint->stroke.setStyle(Qt::SolidLine);
            paint->stroke.setColor(parseColor(strokeColor));
        }
        paint->fill = QBrush(Qt::NoBrush);
        if (fillColor != "none")
            paint->fill = QBrush(parseColor(fillColor));
        paint->polygon = polygon;


    }

    cymonster->update();

    static QTime frameTick;
    static int totalFrame = 0;
    if (!(totalFrame & 15)) {
        int elapsed = frameTick.elapsed();
        frameTick.start();
        cymonster->fps = 16 * 1000 / (1 + elapsed);
    }
    totalFrame++;

    return v8::Undefined();
}

#include "cymonster.moc"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(cymonster);

    QApplication app(argc, argv);

    CyMonster cymonster;
    cymonster.setFixedSize(800, 600);
    cymonster.show();

    return app.exec();
}
