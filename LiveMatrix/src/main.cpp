#include <iostream>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QtGlobal>

#include <gst/gst.h>

#include "StreamController.hpp"

static void log(const char* msg)
{
    std::cout << msg << std::endl;
    std::cout.flush();
}

/// Attach to parent console (when run from cmd/PowerShell) so stdout/stderr are visible.
static void attachConsoleIfNeeded()
{
#ifdef _WIN32
    // 1. הגדרת נתיב הפלאגינים
    _putenv_s("GST_PLUGIN_PATH", "C:\\Program Files\\gstreamer\\1.0\\msvc_x86_64\\lib\\gstreamer-1.0");

    // 2. הוספת תיקיית ה-bin של GStreamer ל-PATH כדי שהפלאגינים ימצאו DLLs חסרים
    std::string currentPath = getenv("PATH");
    std::string gstBin = "C:\\Program Files\\gstreamer\\1.0\\msvc_x86_64\\bin";
    std::string newPath = gstBin + ";" + currentPath;
    _putenv_s("PATH", newPath.c_str());

    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        FILE* dummy = nullptr;
        (void)freopen_s(&dummy, "CONOUT$", "w", stdout);
        (void)freopen_s(&dummy, "CONOUT$", "w", stderr);
        (void)freopen_s(&dummy, "CONIN$", "r", stdin);
    }
#endif
}

int main(int argc, char* argv[])
{
    attachConsoleIfNeeded();
    log("[LiveMatrix] Application start (entry point).");

    log("[LiveMatrix] Setting High DPI attribute...");
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    log("[LiveMatrix] Creating QGuiApplication...");
    QGuiApplication app(argc, argv);
    log("[LiveMatrix] QGuiApplication created.");

    log("[LiveMatrix] Initializing GStreamer...");
    int gstArgc = 0;
    char** gstArgv = nullptr;
    gst_init(&gstArgc, &gstArgv);
    log("[LiveMatrix] GStreamer initialized.");

    log("[LiveMatrix] Creating QML engine and controller...");
    QQmlApplicationEngine engine;
    // Ensure QML imports are found when running from build/Release (e.g. qml/QtQuick, qml/QtMultimedia)
    const QString qmlImportPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QStringLiteral("/qml"));
    engine.addImportPath(qmlImportPath);
    qDebug() << "[LiveMatrix] QML import path added:" << qmlImportPath;

    StreamController controller;
    engine.rootContext()->setContextProperty(QStringLiteral("streamController"), &controller);

    const QUrl url(QStringLiteral("qrc:/LiveMatrixQml/ui/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl)
        {
            if (!obj && url == objUrl)
            {
                qCritical() << "LiveMatrix: failed to create QML root from" << objUrl;
                QCoreApplication::exit(-1);
            }
            else if (obj && url == objUrl)
            {
                qDebug() << "LiveMatrix: QML root loaded, window should be visible.";
            }
        },
        Qt::QueuedConnection);

    log("[LiveMatrix] Loading QML...");
    engine.load(url);
    log("[LiveMatrix] QML load returned.");

    if (engine.rootObjects().isEmpty())
    {
        qCritical() << "LiveMatrix: no root objects after load. Check QML errors above.";
        return -1;
    }

    log("[LiveMatrix] Entering event loop.");
    return app.exec();
}
