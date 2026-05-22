#pragma once

#include <QPoint>
#include <QWidget>
#include <QUrl>

#include "helpbrowser.h"

class QHelpEngineCore;
class QTabWidget;
class QTimer;

class DocumentPane : public QWidget
{
    Q_OBJECT

public:
    explicit DocumentPane(QHelpEngineCore *helpEngine, QWidget *parent = nullptr);

    HelpBrowser *currentBrowser() const;
    HelpBrowser *createPage(const QUrl &url = QUrl());
    void closePage(int index);
    void clearPages();
    QList<HelpBrowser *> browsers() const;
    QStringList openUrls() const;
    int currentTabIndex() const;
    void setCurrentTabIndex(int index);
    void restoreTabs(const QStringList &urls, int currentIndex);
    QTabWidget *tabWidget() const { return m_tabs; }
    void adoptPage(QWidget *page, const QString &title);
    bool ensureHasPage();
    void setPaneActive(bool active);

signals:
    void activated(DocumentPane *pane);
    void linkClicked(const QUrl &url, bool newTab, HelpBrowser *browser);
    void stateChanged();
    void tabDragStarted(int index);
    void tabDragMoved(const QPoint &globalPos);
    void tabDragFinished(const QPoint &globalPos);

public slots:
    void onBrowserNavigate(const QUrl &url, HelpBrowser::NavMode mode);
    void onBrowserSourceChanged(const QUrl &url);
    void onBrowserHistoryChanged();

protected:
    void focusInEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void attachBrowser(HelpBrowser *browser);
    void detachBrowser(HelpBrowser *browser);
    void activateFromBrowser(HelpBrowser *browser);
    void scheduleStateChanged();

    QHelpEngineCore *m_helpEngine = nullptr;
    QTabWidget *m_tabs = nullptr;
    QTimer *m_stateNotifyTimer = nullptr;
    bool m_restoring = false;
    bool m_paneActive = false;
};
