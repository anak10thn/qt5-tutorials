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
#include <QtWebKit>

#define PREVIEW_WIDTH 160
#define PREVIEW_HEIGHT 120
#define TEXT_WIDTH 200

// ==== WebSnap: capture a web page and create a thumbnail preview ====

class WebSnap : public QObject
{
    Q_OBJECT

public:
    QImage result;
    WebSnap(QObject *parent = 0);
    void load(const QUrl &url);

signals:
    void progress(int percent);
    void finished();

private slots:
    void saveResult(bool ok);
    void progressiveRender(int percent);

private:
    QWebPage m_page;
    QSize m_targetSize;
    int m_lastPercent;
};


WebSnap::WebSnap(QObject *parent): QObject(parent), m_lastPercent(0)
{
    m_targetSize = QSize(PREVIEW_WIDTH, PREVIEW_HEIGHT);
    connect(&m_page, SIGNAL(loadFinished(bool)), this, SLOT(saveResult(bool)));
    connect(&m_page, SIGNAL(loadProgress(int)), this, SLOT(progressiveRender(int)));
}

void WebSnap::load(const QUrl &url)
{
    result = QImage();
    m_lastPercent = 0;
    m_page.mainFrame()->load(url);
}

void WebSnap::progressiveRender(int percent)
{
    int rounded = (int)(percent / 10) * 10;
    QSize size = m_page.mainFrame()->contentsSize();
    if (!size.isEmpty() && percent > 10 && rounded != m_lastPercent) {
        m_lastPercent = rounded;
        emit progress(rounded);

        size.setHeight(size.width() * m_targetSize.height() / m_targetSize.width());
        result = QImage(size, QImage::Format_ARGB32_Premultiplied);
        result.fill(Qt::transparent);

        // render and rescale
        QPainter p(&result);
        m_page.setViewportSize(m_page.mainFrame()->contentsSize());
        m_page.mainFrame()->render(&p);
        p.setPen(QPen(Qt::darkGray, 1));
        p.setBrush(Qt::NoBrush);
        p.drawRect(result.rect().adjusted(0, 0, -1, -1));
        p.end();
        result = result.scaledToWidth(m_targetSize.width(), Qt::SmoothTransformation);
    }
}

void WebSnap::saveResult(bool ok)
{
    // crude error-checking
    if (!ok) {
        result = QImage();
        emit finished();
        return;
    }

    m_lastPercent = 0;
    progressiveRender(100);
    emit finished();
}

// ==== SearchSnap: search using Google and display the thumbnail preview ====

class SearchSnap: public QGraphicsView
{
    Q_OBJECT

public:
    SearchSnap();

private slots:
    void startSearch();
    void processSearch(bool ok);
    void renderPreview(int percent);
    void nextPreview();

private:
    QGraphicsScene m_scene;
    QGraphicsItem *m_searchItem;
    QLineEdit *m_searchEdit;
    QWebPage m_searchPage;
    QProgressBar *m_progressBar;
    QStringList m_searchHits;
    int m_hitIndex;
    WebSnap *websnap;
    QList<QGraphicsPixmapItem*> m_previewItems;
    QList<QGraphicsTextItem*> m_textItems;
};

SearchSnap::SearchSnap(): QGraphicsView()
{
    setScene(&m_scene);
    resize(750, 550);
    setWindowTitle("Search & Snap");

    QWidget *searchWidget = new QWidget;
    searchWidget->setBackgroundRole(QPalette::Window);

    QLabel *label = new QLabel(searchWidget);
    label->setText("Search for");

    m_searchEdit = new QLineEdit(searchWidget);
    m_searchEdit->setText("trolltech qt");
    connect(m_searchEdit, SIGNAL(returnPressed()), this, SLOT(startSearch()));

    QPushButton *searchButton = new QPushButton(searchWidget);
    searchButton->setText("&Search");
    searchButton->setDefault(true);
    connect(searchButton, SIGNAL(clicked()), this, SLOT(startSearch()));

    QHBoxLayout *layout = new QHBoxLayout;
    searchWidget->setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(m_searchEdit);
    layout->addWidget(searchButton);

    m_progressBar = new QProgressBar;
    m_progressBar->hide();

    m_searchItem = m_scene.addWidget(searchWidget);
    m_scene.addWidget(m_progressBar);

    connect(&m_searchPage, SIGNAL(loadProgress(int)), m_progressBar, SLOT(setValue(int)));
    connect(&m_searchPage, SIGNAL(loadFinished(bool)), this, SLOT(processSearch(bool)));

    websnap = new WebSnap(this);
    connect(websnap, SIGNAL(progress(int)), this, SLOT(renderPreview(int)));
    connect(websnap, SIGNAL(finished()), this, SLOT(nextPreview()));

    QTimer::singleShot(0, m_searchEdit, SLOT(setFocus()));
}

void SearchSnap::startSearch()
{
    QString str = m_searchEdit->text();
    if (!str.isEmpty()) {
        setWindowTitle(QString("Searching for '%1'").arg(str));
        m_searchItem->hide();
        m_progressBar->show();

        m_searchHits.clear();
        m_hitIndex = 0;
        qDeleteAll(m_previewItems);
        qDeleteAll(m_textItems);

        QString s = str;
        s.replace(' ', '+');
        QString url = QString("http://www.google.com/search?hl=en&num=8&q=%1").arg(s);
        m_searchPage.mainFrame()->load(QUrl(url));
    }
}

#define EVAL_JS(x) m_searchPage.mainFrame()->evaluateJavaScript((x))
#define FIND_HITCOUNT  "document.getElementsByTagName('li').length"
#define FIND_CLASSNAME "document.getElementsByTagName('li')[%1].className"
#define FIND_HREF      "document.getElementsByTagName('li')[%1].childNodes[0].childNodes[0].href"
#define FIND_TITLE     "document.getElementsByTagName('li')[%1].childNodes[0].childNodes[0].text"
#define FIND_SNIPPET   "document.getElementsByTagName('li')[%1].childNodes[1].textContent"
#define FIND_CUTOFF    "document.getElementsByTagName('li')[%1].childNodes[1].getElementsByTagName('cite')[0].textContent"

void SearchSnap::processSearch(bool ok)
{
    m_progressBar->hide();

    QStringList searchTitles;
    QStringList searchSnippets;

    if (ok) {
        int count = EVAL_JS(QString(FIND_HITCOUNT)).toInt();
        for (int i = 0; i < count; i++) {
            QString className = EVAL_JS(QString(FIND_CLASSNAME).arg(i)).toString();
            QString url = EVAL_JS(QString(FIND_HREF).arg(i)).toString();
            QString title = EVAL_JS(QString(FIND_TITLE).arg(i)).toString();
            QString snippet = EVAL_JS(QString(FIND_SNIPPET).arg(i)).toString();
            QString cutoff = EVAL_JS(QString(FIND_CUTOFF).arg(i)).toString();
            if (!cutoff.isEmpty())
                if (snippet.indexOf(cutoff) > 0)
                    snippet = snippet.left(snippet.indexOf(cutoff));
            if (!url.isEmpty() && className == "g") {
                m_searchHits += url;
                searchTitles += title;
                searchSnippets += snippet;
            }
            if (m_searchHits.count() >= 8)
                break;
        }
    }

    if (m_searchHits.count() > 0) {

        QPixmap empty(PREVIEW_WIDTH, PREVIEW_HEIGHT);
        empty.fill(QColor(224, 224, 224));

        for (int i = 0; i < m_searchHits.count(); i++) {

            QGraphicsPixmapItem *preview = new QGraphicsPixmapItem;
            preview->setPixmap(empty);

            QGraphicsTextItem* item = new QGraphicsTextItem;
            QString txt = "<b>" + searchTitles[i] + "</b><br/>";
            txt += "<small>" + searchSnippets[i] + "</small>";
            item->setHtml(txt);
            item->setTextWidth(TEXT_WIDTH);

            int x = (PREVIEW_WIDTH + TEXT_WIDTH + 5) * ((int)(i / 4));
            int y = (PREVIEW_HEIGHT + 5) * (i % 4);
            preview->setPos(x, y);
            x += PREVIEW_WIDTH + 5;
            item->setPos(x, y);

            m_scene.addItem(item);
            m_scene.addItem(preview);
            m_textItems += item;
            m_previewItems += preview;
        }

        websnap->load(m_searchHits[0]);

    } else {
        setWindowTitle(QString("No hit found for '%1'").arg(m_searchEdit->text()));
        m_searchItem->show();
        m_searchEdit->clear();
        m_searchEdit->setFocus();
    }
}

void SearchSnap::renderPreview(int percent)
{
    QGraphicsPixmapItem *item = m_previewItems[m_hitIndex];
    item->setPixmap(QPixmap::fromImage(websnap->result));
}

void SearchSnap::nextPreview()
{
    renderPreview(100);
    m_hitIndex++;
    if (m_hitIndex < m_searchHits.count())
        websnap->load(m_searchHits[m_hitIndex]);
}

#include "searchsnap.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    SearchSnap search;
    search.show();

    return app.exec();
}
