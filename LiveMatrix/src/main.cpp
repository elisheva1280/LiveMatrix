#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QtGlobal>

#include <gst/gst.h>

#include "StreamController.hpp"

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qDebug() << "LiveMatrix: starting, initializing GStreamer...";
    int gstArgc = 0;
    char** gstArgv = nullptr;
    gst_init(&gstArgc, &gstArgv);

    QQmlApplicationEngine engine;

    StreamController controller;
    engine.rootContext()->setContextProperty(QStringLiteral("streamController"), &controller);

    const QUrl url(QStringLiteral("qrc:/LiveMatrixQml/Main.qml"));
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

    engine.load(url);

    if (engine.rootObjects().isEmpty())
    {
        qCritical() << "LiveMatrix: no root objects after load. Check QML errors above.";
        return -1;
    }

    qDebug() << "LiveMatrix: entering event loop.";
    return app.exec();
}
