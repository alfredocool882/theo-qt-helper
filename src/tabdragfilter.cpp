#include "tabdragfilter.h"
#include "documentpane.h"

#include <QApplication>
#include <QMouseEvent>
#include <QStyle>
#include <QTabBar>

namespace {

bool tabBarCloseHit(QTabBar *bar, const QPoint &pos)
{
    const int index = bar->tabAt(pos);
    if (index < 0 || !bar->tabsClosable())
        return false;
    const QRect tabRect = bar->tabRect(index);
    const QStyle *style = bar->style();
    const int w = style->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, bar);
    const int h = style->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, bar);
    const QRect closeRect(tabRect.right() - w - 14, tabRect.center().y() - h / 2 - 4, w + 18, h + 8);
    return closeRect.contains(pos);
}

} // namespace

TabDragFilter::TabDragFilter(DocumentPane *pane, QTabBar *bar, QObject *parent)
    : QObject(parent), m_pane(pane), m_bar(bar)
{
    m_bar->installEventFilter(this);
}

bool TabDragFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_bar)
        return QObject::eventFilter(watched, event);

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() == Qt::LeftButton) {
            if (tabBarCloseHit(m_bar, mouse->pos()))
                return false;
            m_pressIndex = m_bar->tabAt(mouse->pos());
            m_pressPos = mouse->globalPosition().toPoint();
            m_dragging = false;
        }
        break;
    }
    case QEvent::MouseMove: {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (m_pressIndex < 0 || !(mouse->buttons() & Qt::LeftButton))
            break;

        const QPoint global = mouse->globalPosition().toPoint();
        if (!m_dragging) {
            if ((global - m_pressPos).manhattanLength() <= QApplication::startDragDistance())
                break;
            const QPoint local = m_bar->mapFromGlobal(global);
            if (m_bar->rect().contains(local))
                break;
            m_dragging = true;
            emit tabDragStarted(m_pressIndex);
        }
        if (m_dragging) {
            emit tabDragMoved(global);
            return true;
        }
        break;
    }
    case QEvent::MouseButtonRelease: {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (m_dragging && mouse->button() == Qt::LeftButton) {
            emit tabDragFinished(mouse->globalPosition().toPoint());
            m_dragging = false;
            m_pressIndex = -1;
            return true;
        }
        m_pressIndex = -1;
        m_dragging = false;
        break;
    }
    default:
        break;
    }
    return false;
}
