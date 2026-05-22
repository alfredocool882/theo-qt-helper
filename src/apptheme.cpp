#include "apptheme.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QStandardPaths>
#include <QSvgRenderer>
#include <QWidget>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <dwmapi.h>
#endif

namespace AppTheme {

static const char *kSettingKey = "ui/theme";

struct DocPalette {
    const char *pageBg;
    const char *pageFg;
    const char *link;
    const char *linkHover;
    const char *h1Border;
    const char *fnFg;
    const char *fnBorder;
    const char *inlineCodeBg;
    const char *inlineCodeFg;
    const char *inlineCodeBorder;
    const char *preBg;
    const char *preFg;
    const char *preBorder;
    const char *kw;
    const char *type;
    const char *str;
    const char *comment;
    const char *number;
    const char *op;
    const char *pp;
    const char *preLink;
    const char *tableBorder;
    const char *thBg;
    const char *cellBg;
    const char *cellAltBg;
    const char *memName;
    const char *memTypeLink;
};

// doc.qt.io/style/lightmode.css + online2.css
static const DocPalette kLightDoc = {
    "#ffffff", "#000000", "#12834b", "#21be2b", "#eeeeee",
    "#26282a", "#eeeeee",
    "#f9f9f9", "#000000", "#eeeeee",
    "#f3f4f5", "#000000", "#d1d5da",
    "#6730c5", "#4f9d08", "#085d6c", "#7f4707", "#912583", "#080808", "#6730c5", "#21be2b",
    "#eeeeee", "#f9f9f9", "#ffffff", "#f9f9f9", "#12834b", "#21be2b"
};

// doc.qt.io/style/darkmode.css
static const DocPalette kDarkDoc = {
    "#1f1f1f", "#e3e3e3", "#1ec974", "#7ae8b0", "#323232",
    "#e3e3e3", "#323232",
    "#262626", "#e3e3e3", "#323232",
    "#262626", "#e3e3e3", "#323232",
    "#c586c0", "#4f9d08", "#6ec1b0", "#d9a066", "#b5cea8", "#e3e3e3", "#c586c0", "#1ec974",
    "#323232", "#262626", "#121212", "#262626", "#1ec974", "#1ec974"
};

QStringList ids()
{
    return {QStringLiteral("light"), QStringLiteral("dark")};
}

QStringList names()
{
    return {QStringLiteral("浅色"), QStringLiteral("深色")};
}

QString currentId()
{
    const QString saved = QSettings().value(QLatin1String(kSettingKey)).toString();
    return saved == QStringLiteral("dark") ? QStringLiteral("dark") : QStringLiteral("light");
}

void setCurrentId(const QString &id)
{
    QSettings().setValue(QLatin1String(kSettingKey), id == QStringLiteral("dark") ? QStringLiteral("dark") : QStringLiteral("light"));
}

bool isDarkCode(const QString &id)
{
    const QString theme = id.isEmpty() ? currentId() : id;
    return theme == QStringLiteral("dark");
}

QString codePanelBackground(const QString &id)
{
    return isDarkCode(id) ? QStringLiteral("#262626") : QStringLiteral("#f3f4f5");
}

QString metaPanelBackground(const QString &id)
{
    return tableCellBackground(id);
}

QString tableBorderColor(const QString &id)
{
    return isDarkCode(id) ? QStringLiteral("#323232") : QStringLiteral("#eeeeee");
}

QString tableCellBackground(const QString &id)
{
    return isDarkCode(id) ? QStringLiteral("#121212") : QStringLiteral("#ffffff");
}

QString toolbarIconColor(const QString &id)
{
    return isDarkCode(id) ? QStringLiteral("#e3e3e3") : QStringLiteral("#333333");
}

QString toolbarAccentColor(const QString &id)
{
    return isDarkCode(id) ? QStringLiteral("#1ec974") : QStringLiteral("#12834b");
}

static QString resolveToolbarSvgPath(const char *name)
{
    const QString fileName = QString::fromLatin1(name) + QStringLiteral(".svg");
#ifndef THEO_NO_EMBEDDED_ASSETS
    const QString qrcPath = QStringLiteral(":/icons/") + fileName;
    if (QSvgRenderer(qrcPath).isValid())
        return qrcPath;
#endif
    const QString diskPath = QCoreApplication::applicationDirPath() + QStringLiteral("/icons/") + fileName;
    if (QFile::exists(diskPath))
        return diskPath;
    return QString();
}

static QIcon renderToolbarSvg(const QString &path, const QColor &color, int size)
{
    QSvgRenderer renderer(path);
    if (!renderer.isValid())
        return QIcon();
    const QColor c = color.isValid() ? color : QColor(toolbarIconColor());
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&p);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pix.rect(), c);
    p.end();
    return QIcon(pix);
}

QIcon toolbarIcon(const char *name, const QColor &color)
{
    const QString path = resolveToolbarSvgPath(name);
    if (path.isEmpty())
        return QIcon();
    return renderToolbarSvg(path, color, 24);
}

void applyWindowFrameTheme(QWidget *window, const QString &id)
{
    if (!window)
        return;
#ifdef Q_OS_WIN
    const HWND hwnd = reinterpret_cast<HWND>(window->winId());
    if (!hwnd)
        return;
    const BOOL dark = isDarkCode(id) ? TRUE : FALSE;
    const int immersiveDark = 20;
    DwmSetWindowAttribute(hwnd, immersiveDark, &dark, sizeof(dark));
    const int immersiveDarkOld = 19;
    DwmSetWindowAttribute(hwnd, immersiveDarkOld, &dark, sizeof(dark));
#else
    Q_UNUSED(window);
    Q_UNUSED(id);
#endif
}

static QString scrollBarLight()
{
    return QStringLiteral(
        "QScrollBar:vertical { background: #f2f2f2; width: 10px; margin: 2px 1px 2px 0; border: none; }"
        "QScrollBar:horizontal { background: #f2f2f2; height: 10px; margin: 0 1px 2px 2px; border: none; }"
        "QScrollBar::handle:vertical { background: #c4c8cc; min-height: 40px; border-radius: 5px; margin: 2px 1px; }"
        "QScrollBar::handle:vertical:hover { background: #a8adb2; }"
        "QScrollBar::handle:vertical:pressed { background: #969ba0; }"
        "QScrollBar::handle:horizontal { background: #c4c8cc; min-width: 40px; border-radius: 5px; margin: 1px 2px; }"
        "QScrollBar::handle:horizontal:hover { background: #a8adb2; }"
        "QScrollBar::handle:horizontal:pressed { background: #969ba0; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; background: transparent; border: none; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; background: transparent; border: none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical,"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }");
}

static QString scrollBarDark()
{
    return QStringLiteral(
        "QScrollBar:vertical { background: #121212; width: 10px; margin: 2px 1px 2px 0; border: none; }"
        "QScrollBar:horizontal { background: #121212; height: 10px; margin: 0 1px 2px 2px; border: none; }"
        "QScrollBar::handle:vertical { background: #4a4a4a; min-height: 40px; border-radius: 5px; margin: 2px 1px; }"
        "QScrollBar::handle:vertical:hover { background: #5e5e5e; }"
        "QScrollBar::handle:vertical:pressed { background: #707070; }"
        "QScrollBar::handle:horizontal { background: #4a4a4a; min-width: 40px; border-radius: 5px; margin: 1px 2px; }"
        "QScrollBar::handle:horizontal:hover { background: #5e5e5e; }"
        "QScrollBar::handle:horizontal:pressed { background: #707070; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; background: transparent; border: none; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; background: transparent; border: none; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical,"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }");
}

QString scrollBarStyleSheet(const QString &id)
{
    return isDarkCode(id) ? scrollBarDark() : scrollBarLight();
}

static QString tabCloseStyleSheet(const QString &id)
{
    const QString hover = isDarkCode(id) ? QStringLiteral("#3e3e42") : QStringLiteral("#e8e8e8");
    const QString pressed = isDarkCode(id) ? QStringLiteral("#505050") : QStringLiteral("#d1d5da");
    const QString closeSvg = resolveToolbarSvgPath("x");
    if (closeSvg.isEmpty()) {
        return QStringLiteral(
            "QTabBar::close-button { width: 12px; height: 12px; background: transparent; border: none; }"
            "QTabBar::close-button:hover { background: %1; border-radius: 2px; }"
            "QTabBar::close-button:pressed { background: %2; border-radius: 2px; }")
            .arg(hover, pressed);
    }

    const QColor color = toolbarIconColor(id);
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/TheoQtHelper");
    QDir().mkpath(cacheDir);
    const QString filePath = cacheDir + QStringLiteral("/tab-close-")
        + (isDarkCode(id) ? QStringLiteral("dark") : QStringLiteral("light"))
        + QStringLiteral(".png");

    QSvgRenderer renderer(closeSvg);
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    if (renderer.isValid()) {
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&p);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), color);
        p.end();
        pix.save(filePath);
    }

    const QString url = QDir::toNativeSeparators(filePath).replace(QLatin1Char('\\'), QLatin1Char('/'));
    return QStringLiteral(
        "QTabBar::close-button { image: url(%1); background: transparent; border: none; width: 12px; height: 12px; padding: 1px; }"
        "QTabBar::close-button:hover { background: %2; border-radius: 2px; }"
        "QTabBar::close-button:pressed { background: %3; border-radius: 2px; }")
        .arg(url, hover, pressed);
}

static QString appLight()
{
    return QStringLiteral(
        "QMainWindow { background: #f2f2f2; color: #000000; }"
        "QMenuBar { background: #f2f2f2; color: #000000; border-bottom: 1px solid #eeeeee; padding: 2px 0; }"
        "QMenuBar::item { padding: 6px 12px; }"
        "QMenuBar::item:selected { background: #f9f9f9; color: #12834b; }"
        "QMenu { background: #ffffff; border: 1px solid #eeeeee; padding: 4px; }"
        "QMenu::item { padding: 7px 24px 7px 14px; }"
        "QMenu::item:selected { background: #f9f9f9; color: #12834b; }"
        "QToolBar { background: #f2f2f2; border-bottom: 1px solid #eeeeee; spacing: 4px; padding: 4px 6px; }"
        "QToolButton { background: transparent; border: 1px solid transparent; padding: 4px 6px; color: #000000; }"
        "QToolButton:hover { background: #ffffff; border: 1px solid #eeeeee; }"
        "QToolButton:pressed, QToolButton:checked { background: #f9f9f9; border: 1px solid #d1d5da; }"
        "QDockWidget::title { background: #f2f2f2; border-bottom: 1px solid #eeeeee; padding: 7px 9px; font-weight: 600; color: #26282a; }"
        "QTabWidget::pane { border: 1px solid #eeeeee; background: #ffffff; top: -1px; }"
        "QTabBar::tab { background: #f2f2f2; border: 1px solid #eeeeee; padding: 6px 12px; margin-right: 1px; color: #26282a; }"
        "QTabBar::tab:selected { background: #ffffff; color: #12834b; font-weight: 600; border-bottom: 2px solid #12834b; }"
        "QTabBar::tab:hover:!selected { background: #f9f9f9; }"
        "QLineEdit { background: #ffffff; border: 1px solid #d1d5da; padding: 5px 8px; color: #000000; selection-background-color: #d1e8f6; selection-color: #000000; }"
        "QLineEdit:focus { border-color: #12834b; }"
        "QTreeView, QListWidget { background: #ffffff; border: 1px solid #eeeeee; outline: none; color: #000000; font-size: 13px; }"
        "QTreeView::item, QListWidget::item { padding: 4px 6px; }"
        "QTreeView::item:selected, QListWidget::item:selected { background: #d1e8f6; color: #12834b; }"
        "QTreeView::item:hover:!selected, QListWidget::item:hover:!selected { background: #f9f9f9; }"
        "QStatusBar { background: #f2f2f2; border-top: 1px solid #eeeeee; color: #26282a; }"
        "QLabel#docVersionBadge { color: #12834b; font-weight: 600; padding: 2px 8px; }"
        "QLabel#indexHint { color: #606366; font-size: 12px; }"
        "QLabel#openPagesHdr { padding: 6px 8px; border-top: 1px solid #eeeeee; background: #f2f2f2; font-weight: 600; color: #26282a; }"
        "DocumentPane[activePane=\"true\"] QTabBar::tab:selected { background: #ffffff; color: #12834b; font-weight: 700; }"
        "DocumentPane[activePane=\"false\"] QTabBar::tab:selected { color: #606366; font-weight: 500; }")
        + scrollBarLight();
}

static QString appDark()
{
    return QStringLiteral(
        "QMainWindow { background: #1f1f1f; color: #e3e3e3; }"
        "QMenuBar { background: #1f1f1f; color: #e3e3e3; border-bottom: 1px solid #323232; padding: 2px 0; }"
        "QMenuBar::item { padding: 6px 12px; }"
        "QMenuBar::item:selected { background: #262626; color: #1ec974; }"
        "QMenu { background: #121212; border: 1px solid #323232; padding: 4px; color: #e3e3e3; }"
        "QMenu::item { padding: 7px 24px 7px 14px; }"
        "QMenu::item:selected { background: #262626; color: #1ec974; }"
        "QToolBar { background: #1f1f1f; border-bottom: 1px solid #323232; spacing: 4px; padding: 4px 6px; }"
        "QToolButton { background: transparent; border: 1px solid transparent; padding: 4px 6px; color: #e3e3e3; }"
        "QToolButton:hover { background: #262626; border: 1px solid #323232; }"
        "QToolButton:pressed, QToolButton:checked { background: #262626; border: 1px solid #1ec974; color: #1ec974; }"
        "QDockWidget::title { background: #1f1f1f; border-bottom: 1px solid #323232; padding: 7px 9px; font-weight: 600; color: #e3e3e3; }"
        "QTabWidget::pane { border: 1px solid #323232; background: #121212; top: -1px; }"
        "QTabBar::tab { background: #1f1f1f; border: 1px solid #323232; padding: 6px 12px; margin-right: 1px; color: #969696; }"
        "QTabBar::tab:selected { background: #121212; color: #1ec974; font-weight: 600; border-bottom: 2px solid #1ec974; }"
        "QTabBar::tab:hover:!selected { background: #262626; color: #e3e3e3; }"
        "QLineEdit { background: #262626; border: 1px solid #323232; padding: 5px 8px; color: #e3e3e3; selection-background-color: #323232; selection-color: #1ec974; }"
        "QLineEdit:focus { border-color: #1ec974; }"
        "QTreeView, QListWidget { background: #121212; border: 1px solid #323232; outline: none; color: #e3e3e3; font-size: 13px; }"
        "QTreeView::item, QListWidget::item { padding: 4px 6px; }"
        "QTreeView::item:selected, QListWidget::item:selected { background: #262626; color: #1ec974; }"
        "QTreeView::item:hover:!selected, QListWidget::item:hover:!selected { background: #262626; }"
        "QStatusBar { background: #1f1f1f; border-top: 1px solid #323232; color: #969696; }"
        "QLabel#docVersionBadge { color: #1ec974; font-weight: 600; padding: 2px 8px; }"
        "QLabel#indexHint { color: #969696; font-size: 12px; }"
        "QLabel#openPagesHdr { padding: 6px 8px; border-top: 1px solid #323232; background: #1f1f1f; font-weight: 600; color: #e3e3e3; }"
        "DocumentPane[activePane=\"true\"] QTabBar::tab:selected { background: #121212; color: #1ec974; font-weight: 700; }"
        "DocumentPane[activePane=\"false\"] QTabBar::tab:selected { color: #969696; font-weight: 500; }")
        + scrollBarDark();
}

QString appStyleSheet(const QString &id)
{
    return (isDarkCode(id) ? appDark() : appLight()) + tabCloseStyleSheet(id);
}

static QString dialogLight()
{
    return QStringLiteral(
        "QDialog { background: #f2f2f2; color: #000000; }"
        "QLabel#currentDocLabel { background: #12834b; color: #ffffff; font-size: 14px; font-weight: 600; padding: 10px 14px; }"
        "QListWidget { background: #ffffff; border: 1px solid #eeeeee; color: #000000; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:selected { background: #d1e8f6; color: #12834b; }"
        "QTextEdit { background: #ffffff; color: #000000; border: 1px solid #eeeeee; font-family: Consolas; font-size: 12px; }"
        "QPushButton { background: #ffffff; border: 1px solid #d1d5da; padding: 7px 14px; color: #000000; }"
        "QPushButton:hover { background: #f9f9f9; }"
        "QPushButton#primaryBtn { background: #12834b; color: #ffffff; border: none; font-weight: 600; }"
        "QPushButton#primaryBtn:hover { background: #21be2b; }");
}

static QString dialogDark()
{
    return QStringLiteral(
        "QDialog { background: #1f1f1f; color: #e3e3e3; }"
        "QLabel#currentDocLabel { background: #262626; color: #1ec974; font-size: 14px; font-weight: 600; padding: 10px 14px; border: 1px solid #323232; }"
        "QListWidget { background: #121212; border: 1px solid #323232; color: #e3e3e3; }"
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:selected { background: #262626; color: #1ec974; }"
        "QTextEdit { background: #121212; color: #e3e3e3; border: 1px solid #323232; font-family: Consolas; font-size: 12px; }"
        "QPushButton { background: #262626; border: 1px solid #323232; padding: 7px 14px; color: #e3e3e3; }"
        "QPushButton:hover { background: #323232; }"
        "QPushButton#primaryBtn { background: #12834b; color: #ffffff; border: none; font-weight: 600; }"
        "QPushButton#primaryBtn:hover { background: #1ec974; }");
}

QString dialogStyleSheet(const QString &id)
{
    return isDarkCode(id) ? dialogDark() : dialogLight();
}

QString documentStyle(int zoomPercent, const QString &id)
{
    const DocPalette &p = isDarkCode(id) ? kDarkDoc : kLightDoc;
    const double scale = zoomPercent / 100.0;
    return QStringLiteral(R"(
        body { background: %6; color: %7; font-family: "Titillium Web","Segoe UI","Microsoft YaHei UI","Microsoft YaHei",Arial,sans-serif; font-size: %1px; line-height: 1.6; margin: 0; }
        main { max-width: 1120px; margin: 0; padding: 24px 36px 48px 36px; }
        h1 { color: %7; font-size: %2px; font-weight: 700; margin: 4px 0 18px 0; border-bottom: 1px solid %10; padding-bottom: 12px; }
        h2 { color: %7; font-size: %3px; font-weight: 600; margin: 28px 0 12px 0; padding: 0 0 10px 0; border-bottom: 2px solid %10; }
        h3 { color: %11; font-size: %4px; font-weight: 600; margin: 18px 0 6px 0; }
        h3.fn { margin: 18px 0 0 0; padding: 15px 0 12px 0; border: 0; border-bottom: 2px solid %12; line-height: 1.4; }
        p { margin: 8px 0; color: %7; font-size: %1px; }
        a { color: %8; text-decoration: none; font-size: %1px; }
        a:hover { color: %9; text-decoration: underline; }
        code { font-family: "Droid Sans Mono","Cascadia Code",Consolas,monospace; font-size: %5px; background: %13; color: %14; border: 0; padding: 1px 4px; }
        tt { font-family: "Droid Sans Mono","Cascadia Code",Consolas,monospace; font-size: %5px; }
        .name { font-family: "Droid Sans Mono","Cascadia Code",Consolas,monospace; font-size: %5px; }
        div.ide-code, div.pre { margin: 12px 0 20px 0; }
        table.code-panel { border-collapse: collapse; width: 100%; margin: 0; border: 1px solid %16; background: %15; }
        table.code-panel td.code-panel-body { background: %15; padding: 0; border: 0; vertical-align: top; }
        pre, pre.ide-pre { background: %15; color: %17; border: 0; margin: 0; padding: 16px 20px; white-space: pre-wrap; line-height: 1.55; font-size: %5px; font-family: "Droid Sans Mono","Cascadia Code",Consolas,monospace; }
        table.code-panel pre, table.code-panel td, table.code-panel tr { background: %15; }
        table.api-meta-panel { border-collapse: collapse; width: 100%; margin: 16px 0 22px 0; border: 1px solid %18; background: %19; }
        table.api-meta-panel tr, table.api-meta-panel td, table.api-meta-panel th { background: %19; border: 0; padding: 5px 20px; vertical-align: top; color: %7; line-height: 1.3; }
        table.api-meta-panel code, table.api-meta-panel tt { background: transparent; border: 0; padding: 0; color: %7; }
        table.api-valuelist { border-collapse: collapse; width: 100%; margin: 12px 0 22px 0; border: 1px solid %18; background: %19; }
        table.api-valuelist tr { background: %19; }
        table.api-valuelist th { background: %20; color: %7; font-weight: 600; padding: 5px 20px; border: 1px solid %18; text-align: left; }
        table.api-valuelist td { background: %19; padding: 5px 20px; border: 1px solid %18; vertical-align: top; color: %7; line-height: 1.3; }
        table.api-valuelist td.tblval { font-family: "Droid Sans Mono","Cascadia Code",Consolas,monospace; font-size: %5px; }
        table.api-valuelist code, table.api-valuelist tt { background: transparent; border: 0; padding: 0; color: %7; }
        pre span.keyword, table.code-panel span.keyword { color: %21; font-weight: 600; }
        pre span.type, table.code-panel span.type { color: %22; }
        pre span.string, table.code-panel span.string { color: %23; }
        pre span.comment, table.code-panel span.comment { color: %24; font-style: italic; }
        pre span.number, table.code-panel span.number { color: %25; }
        pre span.operator, table.code-panel span.operator { color: %26; }
        pre span.preprocessor, table.code-panel span.preprocessor { color: %27; }
        pre a, table.code-panel a { color: %28; text-decoration: none; }
        table { border-collapse: collapse; margin: 12px 0 18px 0; }
        th { background: %20; color: %7; font-weight: 600; font-size: %1px; padding: 5px 12px; border: 1px solid %18; }
        td { background: %19; color: %7; font-size: %1px; padding: 5px 12px; border: 1px solid %18; vertical-align: top; line-height: 1.35; }
        table.api-table { border: 1px solid %18; }
        td.memItemLeft { font-family: "Droid Sans Mono","Cascadia Code",Consolas,monospace; font-size: %5px; width: 32%; }
        td.memItemRight { font-size: %1px; }
        .memItemLeft { font-family: "Droid Sans Mono","Cascadia Code",Consolas,monospace; font-size: %5px; color: %7; }
        .memItemRight { font-size: %1px; color: %7; }
        div.ide-code span, pre span, table.code-panel span { background: transparent; }
        .memItemRight b, .memItemRight .name { color: %29; font-weight: 600; }
        .memItemRight .type a { color: %30; }
        .header, .footer, .b-sidebar__sidebar, .b-sidebar__topbar, .b-sidebar__content__right, .qds-page-header, .c-sidebar-navigation, .admonition-title, .toc, #sidebar-toctree { display: none; }
        .landing { font-size: %1px; color: %7; }
        .landing a { color: %8; font-size: %1px; }
        img { max-width: 100%; }
    )")
        .arg(14 * scale, 0, 'f', 1)
        .arg(28 * scale, 0, 'f', 1)
        .arg(20 * scale, 0, 'f', 1)
        .arg(16 * scale, 0, 'f', 1)
        .arg(13 * scale, 0, 'f', 1)
        .arg(QLatin1String(p.pageBg), QLatin1String(p.pageFg), QLatin1String(p.link), QLatin1String(p.linkHover),
             QLatin1String(p.h1Border), QLatin1String(p.fnFg), QLatin1String(p.fnBorder),
             QLatin1String(p.inlineCodeBg), QLatin1String(p.inlineCodeFg),
             QLatin1String(p.preBg), QLatin1String(p.preBorder), QLatin1String(p.preFg),
             QLatin1String(p.tableBorder), QLatin1String(p.cellBg), QLatin1String(p.thBg),
             QLatin1String(p.kw), QLatin1String(p.type), QLatin1String(p.str), QLatin1String(p.comment),
             QLatin1String(p.number), QLatin1String(p.op), QLatin1String(p.pp), QLatin1String(p.preLink),
             QLatin1String(p.memName), QLatin1String(p.memTypeLink));
}

} // namespace AppTheme
