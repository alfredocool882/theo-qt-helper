#pragma once

#include <QTextBrowser>

class QContextMenuEvent;
class DocumentPane;
class QHelpEngineCore;
class QMouseEvent;

class HelpBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    enum class NavMode { SameTab, NewTab };
    Q_ENUM(NavMode)

    explicit HelpBrowser(QHelpEngineCore *helpEngine, QWidget *parent = nullptr);
    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    void resetZoom();
    void refreshStyle();
    static void clearRenderCache();
    void navigateToFragment(const QString &fragment);

    void attachToPane(DocumentPane *pane);
    void detachFromPane();

signals:
    void linkNavigateRequested(const QUrl &url, HelpBrowser::NavMode mode);

private:
    void applyScrollBarStyle();
    void applyZoomStyle();
    QUrl linkAtViewportPos(const QPoint &viewportPos) const;
    void requestNavigation(const QUrl &url, NavMode mode);

protected:
    QVariant loadResource(int type, const QUrl &name) override;
    void doSetSource(const QUrl &name, QTextDocument::ResourceType type = QTextDocument::UnknownResource) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    QUrl resolveLink(const QUrl &name) const;
    void scrollToIndexAnchor(const QString &fragment);
    bool scrollToNamedAnchor(const QString &name);
    void scheduleAnchorScroll(const QString &fragment);
    QByteArray renderDocument(const QByteArray &data) const;
    QString extractArticle(const QString &html) const;
    QString documentStyle() const;

    QHelpEngineCore *m_helpEngine = nullptr;
    DocumentPane *m_ownerPane = nullptr;
    int m_zoomPercent = 100;
    int m_lastAppliedZoomPercent = 100;
};
