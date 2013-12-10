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
#include <QtWebKit>

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


class WebMonster : public QWidget
{
    Q_OBJECT

private:

    QWebPage *page;
    QNetworkAccessManager *manager;
    QVector<PaintCommand> commandBuffer;
    int fps;
#ifdef MONSTER_OFFSCREEN
    QPixmap pixmap;
#endif

public:

    WebMonster(QWidget *parent = 0): QWidget(parent), fps(0) {

        page = new QWebPage(this);
        page->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
        page->mainFrame()->addToJavaScriptWindowObject("webmonster", this);

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
            page->mainFrame()->evaluateJavaScript(magicCode);

            QString scriptCode = networkReply->readAll();
            page->mainFrame()->evaluateJavaScript(scriptCode);
            page->mainFrame()->evaluateJavaScript("window.onLoad()");
        }
        networkReply->deleteLater();
    }

    void drawPolygons(int polygonCount, QVariantList polygonArray) {

        commandBuffer.resize(polygonCount);

        PaintCommand *paint = commandBuffer.data();

        for (int i = 0; i < polygonCount; ++i, ++paint) {

            QVariantList cmd = polygonArray[i].toList();
            qreal alpha = cmd[0].toDouble();
            QString strokeColor = cmd[1].toString();
            QString fillColor = cmd[2].toString();
            int pointCount = cmd[3].toInt();
            QVariantList pointArray = cmd[4].toList();

            QPolygonF polygon(pointCount / 2);
            for (int p = 0; p < pointCount / 2; ++p)
                polygon[p] = QPointF(pointArray[2 * p].toDouble(),
                                     pointArray[2 * p + 1].toDouble());

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

        update();

        static QTime frameTick;
        static int totalFrame = 0;
        if (!(totalFrame & 15)) {
            int elapsed = frameTick.elapsed();
            frameTick.start();
            fps = 16 * 1000 / (1 + elapsed);
        }
        totalFrame++;
    }


protected:

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


#include "webmonster.moc"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(webmonster);

    QApplication a(argc, argv);

    WebMonster monster;
    monster.setFixedSize(800, 600);
    monster.show();

    return a.exec();
}
