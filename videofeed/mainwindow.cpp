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
#include <QtNetwork>
#include <QtWebKitWidgets>
#include <QtXmlPatterns>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), m_view(0)
{
    statusBar()->showMessage(tr("Ready"));

    ui.setupUi(this);
    connect(ui.action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui.action_About, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.actionAbout_Qt, SIGNAL(triggered()), this, SLOT(aboutQt()));

    QWebSettings *defaultSettings = QWebSettings::globalSettings();
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::setOfflineStoragePath(QStandardPaths::standardLocations(QStandardPaths::DataLocation).join("/"));
    QWebSettings::setOfflineStorageDefaultQuota(500000);

    m_rssModel = new QStandardItemModel(this);

    populateFeedsMenu();
    m_feedGroup->actions().last()->setChecked(true);
    QTimer::singleShot(0, this, SLOT(changeFeed()));

    m_view = new QWebView(this);
    m_view->move(-1000, -1000);

    ui.webView->page()->mainFrame()->load(QUrl("qrc:/html/videoplayer.html"));

    QSortFilterProxyModel *filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(m_rssModel);
    filterModel->setDynamicSortFilter(true);
    filterModel->setFilterRole(SearchRole);
    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filterModel->setFilterKeyColumn(0);
    connect(ui.searchEdit, SIGNAL(textChanged(QString)), filterModel, SLOT(setFilterFixedString(QString)));

    ui.treeView->setModel(filterModel);
    ui.treeView->header()->hide();
    connect(ui.treeView, SIGNAL(activated(QModelIndex)), this, SLOT(loadVideo(QModelIndex)));
    
    QShortcut *clearHistoryShortcut = new QShortcut(QKeySequence(tr("Ctrl+Alt+X", "Clear history")), this);
    connect(clearHistoryShortcut, SIGNAL(activated()), this, SLOT(clearHistory()));
}

MainWindow::~MainWindow()
{
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About %1").arg(windowTitle()), 
                       tr("Demo of Qt/WebKit flash support"));
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::clearHistory()
{
    ui.webView->page()->mainFrame()->evaluateJavaScript("clearHistory()");
    statusBar()->showMessage(tr("History cleared"));
}

void MainWindow::populateFeedsMenu()
{
    QDirIterator it(":/rss", QStringList("*.ini"));
    m_feedGroup = new QActionGroup(this);
    m_feedGroup->setExclusive(true);
    while (it.hasNext()) {
        QString fileName = it.next();
        QSettings settings(fileName, QSettings::IniFormat);
        QAction *action = new QAction(m_feedGroup);
        action->setText(settings.value("feed/name").toString());
        action->setCheckable(true);
        FeedInfo fi;
        fi.name = settings.value("feed/name").toString();
        fi.uri = settings.value("feed/uri").toString();
        fi.titleFilter = settings.value("feed/titleFilter").toString();
        QFile query(it.path() + "/" + settings.value("feed/query").toString());
        query.open(QFile::ReadOnly);
        fi.query = query.readAll();
        fi.embedDiv = settings.value("feed/embedDiv").toString();
        QVariant variant;
        variant.setValue<FeedInfo>(fi);
        action->setData(variant);
        m_feedGroup->addAction(action);
    }
    ui.menu_Feeds->addActions(m_feedGroup->actions());
    connect(m_feedGroup, SIGNAL(triggered(QAction *)), this, SLOT(changeFeed()));
}

MainWindow::FeedInfo MainWindow::currentFeedInfo() const
{
    QAction *action = m_feedGroup->checkedAction();
    return action->data().value<FeedInfo>();
}

void MainWindow::changeFeed()
{
    loadFeed(currentFeedInfo());
}

void MainWindow::loadFeed(const MainWindow::FeedInfo &fi) 
{
    m_rssModel->clear();
    QStandardItem *root = m_rssModel->invisibleRootItem();
    QStandardItem *item = new QStandardItem(tr("Loading feed, please wait..."));
    item->setFlags(Qt::NoItemFlags);
    root->appendRow(item);

    QXmlQuery query;
    const QString uri = fi.uri.toString();
    const QString DELIMITER = " %%QT_DEMO_DELIM%% ";
    query.bindVariable("uri", QVariant(uri));
    query.setQuery(fi.query);
    QStringList result;
    // ##: evaluteTo loads the URI with its own event loop. This means that we will
    // hit some rescursion with signals/slots. We ignore such bugs in this example.
    // To avoid it we will have to load the uro ourselves.
    query.evaluateTo(&result);
    statusBar()->showMessage(tr("Feed has %1 items").arg(result.count()));

    m_rssModel->clear();
    root = m_rssModel->invisibleRootItem();

    for (int i = 0; i < result.size(); i++) {
        QStandardItem *item = new QStandardItem;

        QStringList arr = result.at(i).split(DELIMITER);
        QString titleStr = arr.at(0);
        titleStr.remove(QRegExp(fi.titleFilter));
        item->setText(titleStr);

        item->setData(arr.at(0), TitleRole);
        item->setData(arr.at(1), AuthorRole);
        item->setData(arr.at(2), DescriptionRole);
        item->setData(arr.at(3), SubTitleRole);
        item->setData(arr.at(4), PageLinkRole);
        item->setData(arr.at(5), DownloadRole);
        item->setData(arr.at(6), PublishedDateRole);
        item->setData(arr.at(7), DurationRole);
        item->setData(arr.at(0) + " " + arr.at(1) + " " + arr.at(2) + "  " + arr.at(3), SearchRole);

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        root->appendRow(item);
    }

    setWindowTitle(tr("VideoFeed - %1").arg(fi.name));
}

static QString jsEscape(const QString &str, bool escapeSingleQuote = true) 
{
    QString string = str;

    string.replace('\\', "\\\\");
    if (escapeSingleQuote) {
        string.replace('\'', "\\\'");
    } else {
        string.replace('"', "\\\"");
    }
    string.replace('\t', "\\t");
    string.replace('\f', "\\f");
    string.replace('\r', "\\r");
    string.replace('\b', "\\b");
    string.replace('\n', "\\n");

    return string;
}

void MainWindow::loadVideo(const QModelIndex &index)
{
    m_loadingIndex = index;
    statusBar()->showMessage(tr("Loading video - %1").arg(index.data(TitleRole).toString()));
    QString desc = index.data(DescriptionRole).toString();
    ui.webView->page()->mainFrame()->evaluateJavaScript("showLoading('" + jsEscape(desc) + "')");
    m_view->page()->mainFrame()->setUrl(QUrl(index.data(PageLinkRole).toString()));
    disconnect(m_view, 0, this, 0);
    connect(m_view, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
}

void MainWindow::loadFinished(bool ok)
{
    disconnect(m_view, 0, this, 0);

    if (ok) {
        statusBar()->showMessage(tr("Showing video"));
        QString script = QString("document.getElementById('%1').innerHTML").arg(currentFeedInfo().embedDiv);
        QString videoHtml = jsEscape(m_view->page()->mainFrame()->evaluateJavaScript(script).toString());
        m_view->page()->mainFrame()->evaluateJavaScript("window.location.href='about:blank'");

        QString title = jsEscape(m_loadingIndex.data(TitleRole).toString());
        QString author = jsEscape(m_loadingIndex.data(AuthorRole).toString());
        QString desc = jsEscape(m_loadingIndex.data(DescriptionRole).toString());
        QString pubDate = jsEscape(m_loadingIndex.data(PublishedDateRole).toString());
        QString duration = jsEscape(m_loadingIndex.data(DurationRole).toString());

#if 0
        qDebug() << title;
        qDebug() << author;
        qDebug() << desc;
        qDebug() << pubDate;
        qDebug() << duration;
        qDebug() << videoHtml;
#endif
        ui.webView->page()->mainFrame()->evaluateJavaScript( // ###: pass JSON
                QString("addVideo('%1', '%2', '%3', '%4', '%5', '%6')")
                        .arg(title)
                        .arg(author)
                        .arg(desc)
                        .arg(pubDate)
                        .arg(duration)
                        .arg(videoHtml)
                       );
    } else {
        statusBar()->showMessage(tr("Something bad happened"));
        ui.webView->page()->mainFrame()->evaluateJavaScript("showLoadError()");
    }
}

