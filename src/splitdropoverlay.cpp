#include "splitdropoverlay.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace {

constexpr int kBtn = 40;
constexpr int kEdgeGap = 4;
constexpr int kGap = kBtn / 2 + kEdgeGap;

void paintGuideButton(QPainter &p, const QRect &r, bool active, const QColor &accent)
{
    const QColor bg = active ? QColor(18, 131, 116, 220) : QColor(255, 255, 255, 235);
    const QColor border = active ? accent : QColor(160, 160, 160);
    p.setPen(QPen(border, active ? 2 : 1));
    p.setBrush(bg);
    p.drawRoundedRect(r.adjusted(1, 1, -1, -1), 4, 4);
}

void paintCenterIcon(QPainter &p, const QRect &r)
{
    const QRect ir = r.adjusted(10, 10, -10, -10);
    p.setPen(QPen(QColor(80, 80, 80), 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRect(ir);
}

void paintArrowIcon(QPainter &p, const QRect &r, int dir)
{
    QPainterPath path;
    const QPoint c = r.adjusted(10, 10, -10, -10).center();
    const int s = 10;
    switch (dir) {
    case 0:
        path.moveTo(c.x(), c.y() - s);
        path.lineTo(c.x() - s, c.y() + s / 2);
        path.lineTo(c.x() + s, c.y() + s / 2);
        break;
    case 1:
        path.moveTo(c.x(), c.y() + s);
        path.lineTo(c.x() - s, c.y() - s / 2);
        path.lineTo(c.x() + s, c.y() - s / 2);
        break;
    case 2:
        path.moveTo(c.x() - s, c.y());
        path.lineTo(c.x() + s / 2, c.y() - s);
        path.lineTo(c.x() + s / 2, c.y() + s);
        break;
    default:
        path.moveTo(c.x() + s, c.y());
        path.lineTo(c.x() - s / 2, c.y() - s);
        path.lineTo(c.x() - s / 2, c.y() + s);
        break;
    }
    path.closeSubpath();
    p.fillPath(path, QColor(90, 90, 90));
}

} // namespace

SplitDropOverlay::SplitDropOverlay(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::NoFocus);
}

void SplitDropOverlay::setTargetRect(const QRect &rectInOverlay)
{
    if (m_targetRect == rectInOverlay)
        return;
    m_targetRect = rectInOverlay;
    update();
}

QPoint SplitDropOverlay::guideCenter() const
{
    if (m_targetRect.isValid())
        return m_targetRect.center();
    return rect().center();
}

QRect SplitDropOverlay::zoneButtonRect(Zone zone) const
{
    const QPoint c = guideCenter();
    switch (zone) {
    case Zone::Center:
        return QRect(c.x() - kBtn / 2, c.y() - kBtn / 2, kBtn, kBtn);
    case Zone::Top:
        return QRect(c.x() - kBtn / 2, c.y() - kGap - kBtn, kBtn, kBtn);
    case Zone::Bottom:
        return QRect(c.x() - kBtn / 2, c.y() + kGap, kBtn, kBtn);
    case Zone::Left:
        return QRect(c.x() - kGap - kBtn, c.y() - kBtn / 2, kBtn, kBtn);
    case Zone::Right:
        return QRect(c.x() + kGap, c.y() - kBtn / 2, kBtn, kBtn);
    default:
        return {};
    }
}

SplitDropOverlay::Zone SplitDropOverlay::zoneAt(const QPoint &globalPos) const
{
    const QPoint local = mapFromGlobal(globalPos);
    if (!rect().contains(local))
        return Zone::None;

    static const Zone zones[] = {Zone::Center, Zone::Top, Zone::Bottom, Zone::Left, Zone::Right};
    for (Zone z : zones) {
        if (zoneButtonRect(z).adjusted(-6, -6, 6, 6).contains(local))
            return z;
    }
    return Zone::None;
}

void SplitDropOverlay::setActiveZone(Zone zone)
{
    if (m_active == zone)
        return;
    m_active = zone;
    update();
}

QRect SplitDropOverlay::previewRect() const
{
    if (!m_targetRect.isValid() || m_active == Zone::None || m_active == Zone::Center)
        return {};

    const int w = m_targetRect.width();
    const int h = m_targetRect.height();
    const int halfW = qMax(80, w / 2);
    const int halfH = qMax(80, h / 2);

    switch (m_active) {
    case Zone::Left:
        return QRect(m_targetRect.left(), m_targetRect.top(), halfW, h);
    case Zone::Right:
        return QRect(m_targetRect.right() - halfW + 1, m_targetRect.top(), halfW, h);
    case Zone::Top:
        return QRect(m_targetRect.left(), m_targetRect.top(), w, halfH);
    case Zone::Bottom:
        return QRect(m_targetRect.left(), m_targetRect.bottom() - halfH + 1, w, halfH);
    default:
        return {};
    }
}

void SplitDropOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_targetRect.isValid())
        p.fillRect(m_targetRect.adjusted(-1, -1, 1, 1), QColor(0, 0, 0, 25));

    const QRect preview = previewRect();
    if (preview.isValid()) {
        p.fillRect(preview, QColor(18, 131, 116, 70));
        p.setPen(QPen(QColor(18, 131, 116, 200), 2));
        p.drawRect(preview.adjusted(1, 1, -1, -1));
    } else if (m_active == Zone::Center && m_targetRect.isValid()) {
        p.fillRect(m_targetRect, QColor(18, 131, 116, 45));
        p.setPen(QPen(QColor(18, 131, 116, 180), 2));
        p.drawRect(m_targetRect.adjusted(1, 1, -1, -1));
    }

    const QColor accent(18, 131, 116);
    const Zone buttons[] = {Zone::Center, Zone::Top, Zone::Bottom, Zone::Left, Zone::Right};
    const int arrows[] = {-1, 0, 1, 2, 3};
    for (int i = 0; i < 5; ++i) {
        const Zone z = buttons[i];
        const QRect br = zoneButtonRect(z);
        paintGuideButton(p, br, m_active == z, accent);
        if (arrows[i] < 0)
            paintCenterIcon(p, br);
        else
            paintArrowIcon(p, br, arrows[i]);
    }
}
