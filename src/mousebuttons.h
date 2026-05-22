#pragma once

#include <QEvent>
#include <QMouseEvent>

inline bool isMouseBackButton(Qt::MouseButton button)
{
    return button == Qt::BackButton || button == Qt::XButton1;
}

inline bool isMouseForwardButton(Qt::MouseButton button)
{
    return button == Qt::ForwardButton || button == Qt::XButton2;
}

inline bool isMouseHistoryPress(QEvent *event)
{
    return event->type() == QEvent::MouseButtonPress;
}
