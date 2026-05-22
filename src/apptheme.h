#pragma once

#include <QColor>
#include <QIcon>
#include <QString>
#include <QStringList>

class QWidget;

namespace AppTheme {

QStringList ids();
QStringList names();
QString currentId();
void setCurrentId(const QString &id);
bool isDarkCode(const QString &id = QString());

QString scrollBarStyleSheet(const QString &id = QString());
QString appStyleSheet(const QString &id = QString());
QString dialogStyleSheet(const QString &id = QString());
QString documentStyle(int zoomPercent, const QString &id = QString());
QString codePanelBackground(const QString &id = QString());
QString metaPanelBackground(const QString &id = QString());
QString tableBorderColor(const QString &id = QString());
QString tableCellBackground(const QString &id = QString());
QString toolbarIconColor(const QString &id = QString());
QString toolbarAccentColor(const QString &id = QString());
QIcon toolbarIcon(const char *name, const QColor &color = QColor());
void applyWindowFrameTheme(QWidget *window, const QString &id = QString());

} // namespace AppTheme
