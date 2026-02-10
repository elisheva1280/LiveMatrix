/// StreamController.hpp
/// Controller layer: bridges QML UI and RtspPipeline engine.
/// Uses the VideoOutput's read-only videoSink (Qt 6): QML passes it via setVideoSink().

#pragma once

#include <QObject>
#include <QString>
#include <QPointer>
#include <QVideoSink>

class RtspPipeline;

class StreamController : public QObject
{
    Q_OBJECT

public:
    explicit StreamController(QObject* parent = nullptr);
    ~StreamController() override;

    /// Called from QML with VideoOutput.videoSink (read-only property from Qt 6 Multimedia).
    Q_INVOKABLE void setVideoSink(QVideoSink* sink);
    Q_INVOKABLE void play(const QString& url);
    Q_INVOKABLE void stop();

signals:
    void statusChanged(const QString& status);
    void errorOccurred(int code, const QString& message);

private:
    void handlePipelineError(int code, const QString& message);
    void handlePipelineStateChanged(const QString& state);

    QPointer<QVideoSink> m_videoSink;
    RtspPipeline* m_pipeline = nullptr; // owned, created when setVideoSink is called
};
