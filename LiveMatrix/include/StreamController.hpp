/// StreamController.hpp
/// Controller layer: bridges QML UI and RtspPipeline engine.

#pragma once

#include <QObject>
#include <QString>
#include <QPointer>
#include <QVideoSink>

class RtspPipeline;

class StreamController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVideoSink* videoSink READ videoSink CONSTANT)

public:
    explicit StreamController(QObject* parent = nullptr);
    ~StreamController() override;

    QVideoSink* videoSink() const noexcept { return m_videoSink; }

    Q_INVOKABLE void play(const QString& url);
    Q_INVOKABLE void stop();

signals:
    void statusChanged(const QString& status);
    void errorOccurred(int code, const QString& message);

private:
    void handlePipelineError(int code, const QString& message);
    void handlePipelineStateChanged(const QString& state);

    QPointer<QVideoSink> m_videoSink;
    RtspPipeline* m_pipeline; // owned
};
