#pragma once

#include <QRect>
#include <QWidget>

class SplitDropOverlay : public QWidget
{
    Q_OBJECT

public:
    enum class Zone { None, Center, Left, Right, Top, Bottom };

    explicit SplitDropOverlay(QWidget *parent = nullptr);

    void setTargetRect(const QRect &rectInOverlay);
    Zone zoneAt(const QPoint &globalPos) const;
    void setActiveZone(Zone zone);
    Zone activeZone() const { return m_active; }
    QRect previewRect() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPoint guideCenter() const;
    QRect zoneButtonRect(Zone zone) const;

    QRect m_targetRect;
    Zone m_active = Zone::None;
};
