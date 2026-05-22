#pragma once

#include <QObject>
#include <QPoint>

class DocumentPane;
class QTabBar;

class TabDragFilter : public QObject
{
    Q_OBJECT

public:
    TabDragFilter(DocumentPane *pane, QTabBar *bar, QObject *parent = nullptr);

signals:
    void tabDragStarted(int index);
    void tabDragMoved(const QPoint &globalPos);
    void tabDragFinished(const QPoint &globalPos);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    DocumentPane *m_pane = nullptr;
    QTabBar *m_bar = nullptr;
    int m_pressIndex = -1;
    QPoint m_pressPos;
    bool m_dragging = false;
};
