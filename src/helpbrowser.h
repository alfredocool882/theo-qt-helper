#pragma once

#include <QTextBrowser>

class QContextMenuEvent;
class DocumentPane;
class QHelpEngineCore;
class QFocusEvent;
class QMouseEvent;
class QWheelEvent;
class QUrl;

class HelpBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    enum class NavMode { SameTab, NewTab };
    Q_ENUM(NavMode)

    explicit HelpBrowser(QHelpEngineCore *helpEngine, QWidget *parent = nullptr);
    ~HelpBrowser() override;

    static int globalZoomPercent();
    static bool canZoomInGlobal();
    static bool canZoomOutGlobal();
    static void zoomInGlobal(int range = 1);
    static void zoomOutGlobal(int range = 1);
    static void resetZoomGlobal();
    static void flushVisibleZoom();

    int zoomPercent() const { return m_zoomPercent; }
    void syncGlobalZoom();
    void refreshStyle();
    static void clearRenderCache();

    void attachToPane(DocumentPane *pane);
    void detachFromPane();
    DocumentPane *ownerPane() const { return m_ownerPane; }

signals:
    void linkNavigateRequested(const QUrl &url, HelpBrowser::NavMode mode);

private:
    static void setGlobalZoomPercent(int percent);

    void applyScrollBarStyle();
    void applyZoomStyle();
    void completeZoomStyle(int attempt = 0);

    QUrl pageSource() const;
    QUrl canonicalPageUrl(const QUrl &url) const;
    void restoreScrollAfterZoom();

    QUrl linkAtViewportPos(const QPoint &viewportPos) const;
    bool isNonLinkViewportClick(const QPoint &viewportPos) const;
    void requestNavigation(const QUrl &url, NavMode mode);

protected:
    QVariant loadResource(int type, const QUrl &name) override;
    void doSetSource(const QUrl &name, QTextDocument::ResourceType type = QTextDocument::UnknownResource) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    QUrl resolveLink(const QUrl &name) const;
    void scrollToIndexAnchor(const QString &fragment);
    bool scrollToNamedAnchor(const QString &name);
    void scheduleAnchorScroll(const QString &fragment);

    QHelpEngineCore *m_helpEngine = nullptr;
    DocumentPane *m_ownerPane = nullptr;
    int m_zoomPercent = 100;
    int m_lastAppliedZoomPercent = 100;
    QUrl m_helpSource;
    int m_zoomScrollValue = 0;
    int m_zoomScrollOldMax = 0;
    bool m_pendingScrollRestore = false;
    int m_focusScrollValue = 0;
};
