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

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtScript>
#include <QtWidgets>

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

static QScriptValue drawPolygons(QScriptContext *context, QScriptEngine *engine);

class QSMonster : public QWidget
{
    Q_OBJECT

private:

    QScriptEngine *engine;
    QNetworkAccessManager *manager;
#ifdef MONSTER_OFFSCREEN
    QPixmap pixmap;
#endif

public:

    QVector<PaintCommand> commandBuffer;
    int fps;

    QSMonster(QWidget *parent = 0): QWidget(parent), fps(0) {

        engine = new QScriptEngine(this);
        engine->globalObject().setProperty("drawPolygons", engine->newFunction(drawPolygons, 2));

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
            engine->evaluate(magicCode);

            QString scriptCode = networkReply->readAll();

            // Qt Script parser does not accept this
#if QT_VERSION < 0x040500
            scriptCode.replace(",}", "}");
#else
            scriptCode.replace("break}", "break;}");
            scriptCode.replace("continue}", "continue;}");
#endif

            engine->evaluate(scriptCode);
            engine->evaluate("window.onLoad()");
            startTimer(33);
        }
        networkReply->deleteLater();
    }

protected:

    void timerEvent(QTimerEvent*) {
        engine->evaluate("window.onTimer()");
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

static QScriptValue drawPolygons(QScriptContext *context, QScriptEngine *engine)
{
    QSMonster *qsmonster = static_cast<QSMonster*>(engine->parent());

    int polygonCount = context->argument(0).toInt32();
    QScriptValue polygonBuffer = context->argument(1);

    qsmonster->commandBuffer.resize(polygonCount);

    for (int c = 0; c < polygonCount; ++c) {

        QScriptValue cmd = polygonBuffer.property(c);
        qreal alpha = cmd.property(0).toNumber();
        QString strokeColor = cmd.property(1).toString();
        QString fillColor = cmd.property(2).toString();
        int pointCount = cmd.property(3).toInt32();
        QScriptValue pointArray = cmd.property(4);

        QPolygonF polygon(pointCount / 2);
        for (int p = 0; p < pointCount / 2; ++p) {
            polygon[p] = QPointF(pointArray.property(2 * p).toNumber(),
                                 pointArray.property(2 * p + 1).toNumber());
        }

        PaintCommand *paint = qsmonster->commandBuffer.data() + c;
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

    qsmonster->update();

    static QTime frameTick;
    static int totalFrame = 0;
    if (!(totalFrame & 15)) {
        int elapsed = frameTick.elapsed();
        frameTick.start();
        qsmonster->fps = 16 * 1000 / (1 + elapsed);
    }
    totalFrame++;

    return QScriptValue();
}

#include "qsmonster.moc"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(qsmonster);

    QApplication app(argc, argv);

    QSMonster qsmonster;
    qsmonster.setFixedSize(800, 600);
    qsmonster.show();

    return app.exec();
}
