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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QActionGroup>

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~MainWindow();

    struct FeedInfo {
        QString name;
        QUrl uri;
        QString query;
        QString embedDiv;
        QString titleFilter;
    };

    FeedInfo currentFeedInfo() const;

private slots:
    void about();
    void aboutQt();
    
    void clearHistory();
    void changeFeed();
    void loadFinished(bool ok);
    void loadVideo(const QModelIndex &index);

private:
    QActionGroup *m_feedGroup;
    QModelIndex m_loadingIndex;

    void populateFeedsMenu();
    void loadFeed(const FeedInfo &fi) ;

    Ui::MainWindow ui;
    QWebView *m_view;
    QStandardItemModel *m_rssModel;

    enum {
        TitleRole = Qt::UserRole + 1,
        AuthorRole,
        DescriptionRole,
        SubTitleRole,
        PageLinkRole,
        DownloadRole,
        PublishedDateRole,
        DurationRole,
        SearchRole
    };
};

Q_DECLARE_METATYPE(MainWindow::FeedInfo);

#endif // MAINWINDOW_H

