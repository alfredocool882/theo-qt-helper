#include "documentpane.h"
#include "apptheme.h"
#include "helpbrowser.h"
#include "tabdragfilter.h"

#include <QApplication>
#include <QFocusEvent>
#include <QHelpEngineCore>
#include <QMouseEvent>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

DocumentPane::DocumentPane(QHelpEngineCore *helpEngine, QWidget *parent)
    : QWidget(parent), m_helpEngine(helpEngine)
{
    setProperty("activePane", false);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_tabs = new QTabWidget(this);
    m_tabs->setDocumentMode(true);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setUsesScrollButtons(true);
    m_tabs->setTabPosition(QTabWidget::North);
    m_tabs->tabBar()->setExpanding(false);
    m_tabs->tabBar()->setElideMode(Qt::ElideRight);
    auto *dragFilter = new TabDragFilter(this, m_tabs->tabBar(), m_tabs);
    connect(dragFilter, &TabDragFilter::tabDragStarted, this, &DocumentPane::tabDragStarted);
    connect(dragFilter, &TabDragFilter::tabDragMoved, this, &DocumentPane::tabDragMoved);
    connect(dragFilter, &TabDragFilter::tabDragFinished, this, &DocumentPane::tabDragFinished);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &DocumentPane::closePage);
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int) {
        if (HelpBrowser *browser = currentBrowser())
            browser->syncGlobalZoom();
        emit activated(this);
        if (!m_restoring)
            emit stateChanged();
    });
    m_tabs->tabBar()->installEventFilter(this);
    m_tabs->installEventFilter(this);
    layout->addWidget(m_tabs);
    setFocusPolicy(Qt::ClickFocus);
}

void DocumentPane::setPaneActive(bool active)
{
    m_paneActive = active;
    setProperty("activePane", active);
    const bool dark = AppTheme::isDarkCode();
    if (active) {
        setStyleSheet(dark ? QStringLiteral("DocumentPane { border: 3px solid #1ec974; background: #1a2e28; }")
                           : QStringLiteral("DocumentPane { border: 3px solid #12834b; background: #eef8f4; }"));
    } else {
        setStyleSheet(dark ? QStringLiteral("DocumentPane { border: 1px solid #323232; background: #1f1f1f; }")
                           : QStringLiteral("DocumentPane { border: 1px solid #d8d8d8; background: #f5f5f5; }"));
    }
}

void DocumentPane::activateFromBrowser(HelpBrowser *browser)
{
    if (browser && m_tabs->indexOf(browser) >= 0)
        m_tabs->setCurrentWidget(browser);
    if (browser)
        browser->setFocus(Qt::MouseFocusReason);
    emit activated(this);
}

bool DocumentPane::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn) {
        if (watched == m_tabs->tabBar() || watched == m_tabs) {
            emit activated(this);
            return false;
        }
        HelpBrowser *browser = qobject_cast<HelpBrowser *>(watched);
        if (!browser) {
            if (auto *vp = qobject_cast<QWidget *>(watched))
                browser = qobject_cast<HelpBrowser *>(vp->parentWidget());
        }
        if (browser && m_tabs->indexOf(browser) >= 0) {
            activateFromBrowser(browser);
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void DocumentPane::mousePressEvent(QMouseEvent *event)
{
    emit activated(this);
    if (HelpBrowser *browser = currentBrowser())
        browser->setFocus(Qt::MouseFocusReason);
    QWidget::mousePressEvent(event);
}

void DocumentPane::attachBrowser(HelpBrowser *browser)
{
    if (!browser)
        return;
    browser->installEventFilter(this);
    if (QWidget *viewport = browser->viewport())
        viewport->installEventFilter(this);
    browser->attachToPane(this);
}

void DocumentPane::detachBrowser(HelpBrowser *browser)
{
    if (!browser)
        return;
    browser->removeEventFilter(this);
    if (QWidget *viewport = browser->viewport())
        viewport->removeEventFilter(this);
    browser->detachFromPane();
}

void DocumentPane::onBrowserNavigate(const QUrl &url, HelpBrowser::NavMode mode)
{
    auto *browser = qobject_cast<HelpBrowser *>(sender());
    if (!browser)
        return;
    emit linkClicked(url, mode == HelpBrowser::NavMode::NewTab, browser);
}

void DocumentPane::scheduleStateChanged()
{
    if (!m_stateNotifyTimer) {
        m_stateNotifyTimer = new QTimer(this);
        m_stateNotifyTimer->setSingleShot(true);
        m_stateNotifyTimer->setInterval(150);
        connect(m_stateNotifyTimer, &QTimer::timeout, this, &DocumentPane::stateChanged);
    }
    m_stateNotifyTimer->start();
}

void DocumentPane::onBrowserSourceChanged(const QUrl &)
{
    scheduleStateChanged();
}

void DocumentPane::onBrowserHistoryChanged()
{
    scheduleStateChanged();
}

HelpBrowser *DocumentPane::currentBrowser() const
{
    return qobject_cast<HelpBrowser *>(m_tabs->currentWidget());
}

HelpBrowser *DocumentPane::createPage(const QUrl &url)
{
    HelpBrowser *browser = new HelpBrowser(m_helpEngine, m_tabs);
    attachBrowser(browser);
    m_tabs->addTab(browser, tr("页面"));
    m_tabs->setCurrentWidget(browser);
    if (url.isValid() && !url.isEmpty())
        browser->setSource(url);
    emit activated(this);
    if (!m_restoring)
        emit stateChanged();
    return browser;
}

void DocumentPane::closePage(int index)
{
    if (index < 0 || index >= m_tabs->count())
        return;
    QWidget *page = m_tabs->widget(index);
    if (auto *browser = qobject_cast<HelpBrowser *>(page))
        detachBrowser(browser);
    m_tabs->removeTab(index);
    page->deleteLater();
    emit stateChanged();
}

void DocumentPane::clearPages()
{
    while (m_tabs->count() > 0) {
        QWidget *page = m_tabs->widget(0);
        if (auto *browser = qobject_cast<HelpBrowser *>(page))
            detachBrowser(browser);
        m_tabs->removeTab(0);
        page->deleteLater();
    }
    emit stateChanged();
}

QList<HelpBrowser *> DocumentPane::browsers() const
{
    QList<HelpBrowser *> list;
    for (int i = 0; i < m_tabs->count(); ++i) {
        if (auto *browser = qobject_cast<HelpBrowser *>(m_tabs->widget(i)))
            list.append(browser);
    }
    return list;
}

QStringList DocumentPane::openUrls() const
{
    QStringList urls;
    for (HelpBrowser *browser : browsers()) {
        if (browser->source().isValid() && !browser->source().isEmpty())
            urls.append(browser->source().toString());
    }
    return urls;
}

int DocumentPane::currentTabIndex() const
{
    return m_tabs->currentIndex();
}

void DocumentPane::setCurrentTabIndex(int index)
{
    if (index >= 0 && index < m_tabs->count())
        m_tabs->setCurrentIndex(index);
}

void DocumentPane::restoreTabs(const QStringList &urls, int currentIndex)
{
    m_restoring = true;
    clearPages();
    if (urls.isEmpty()) {
        m_restoring = false;
        return;
    }
    for (const QString &url : urls) {
        if (!url.isEmpty())
            createPage(QUrl(url));
    }
    if (!m_tabs->count())
        m_restoring = false;
    else {
        setCurrentTabIndex(qBound(0, currentIndex, m_tabs->count() - 1));
        m_restoring = false;
    }
}

void DocumentPane::adoptPage(QWidget *page, const QString &title)
{
    if (!page)
        return;
    if (auto *browser = qobject_cast<HelpBrowser *>(page)) {
        browser->detachFromPane();
        attachBrowser(browser);
    }
    m_tabs->addTab(page, title.isEmpty() ? tr("页面") : title);
    m_tabs->setCurrentWidget(page);
    emit activated(this);
    if (!m_restoring)
        emit stateChanged();
}

bool DocumentPane::ensureHasPage()
{
    if (m_tabs->count() > 0)
        return true;
    createPage(QUrl());
    return m_tabs->count() > 0;
}

void DocumentPane::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    emit activated(this);
}
