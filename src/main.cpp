#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Theo Qt Helper"));
    QApplication::setApplicationDisplayName(QStringLiteral("Theo Qt Helper"));
    QApplication::setOrganizationName(QStringLiteral("TheoQtHelper"));
    QApplication::setOrganizationDomain(QStringLiteral("theoqthelper.local"));
#ifndef THEO_NO_EMBEDDED_ASSETS
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/app.ico")));
#endif
#ifdef Q_OS_WIN
    if (QStyle *style = QStyleFactory::create(QStringLiteral("windowsvista")))
        QApplication::setStyle(style);
#else
    if (QStyle *style = QStyleFactory::create(QStringLiteral("Fusion")))
        QApplication::setStyle(style);
#endif

    MainWindow window;
#ifndef THEO_NO_EMBEDDED_ASSETS
    window.setWindowIcon(QIcon(QStringLiteral(":/app.ico")));
#endif
    window.show();
    return app.exec();
}
