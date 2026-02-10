#include "StreamController.hpp"
#include "RtspPipeline.hpp"

#include <QDebug>
#include <QVideoSink>
#include <QVideoFrame>
#include <QMetaObject>

StreamController::StreamController(QObject* parent)
    : QObject(parent)
{
    emit statusChanged(QStringLiteral("Idle"));
}

StreamController::~StreamController() = default;

void StreamController::setVideoSink(QVideoSink* sink)
{
    if (!sink || m_videoSink == sink)
    {
        return;
    }
    if (m_pipeline)
    {
        m_pipeline->stop();
        delete m_pipeline;
        m_pipeline = nullptr;
    }
    m_videoSink = sink;
    m_pipeline = new RtspPipeline(m_videoSink, this);
    connect(m_pipeline, &RtspPipeline::pipelineError,
            this, &StreamController::handlePipelineError);
    connect(m_pipeline, &RtspPipeline::pipelineStateChanged,
            this, &StreamController::handlePipelineStateChanged);
}

void StreamController::play(const QString& url)
{
    if (!m_pipeline || !m_videoSink)
    {
        qWarning() << "StreamController::play called before setVideoSink; ignoring.";
        emit errorOccurred(-2, QStringLiteral("Video output not ready."));
        return;
    }
    // UI reset on every play attempt
    m_videoSink->setVideoFrame(QVideoFrame());
    emit statusChanged(QStringLiteral("Connecting..."));
    emit errorOccurred(0, QString());

    if (url.trimmed().isEmpty())
    {
        qWarning() << "StreamController::play called with empty URL";
        emit errorOccurred(-1, QStringLiteral("RTSP URL cannot be empty."));
        emit statusChanged(QStringLiteral("Idle"));
        return;
    }

    m_pipeline->stop();

    if (!m_pipeline->start(url))
    {
        emit statusChanged(QStringLiteral("Idle"));
        // Detailed error already emitted by RtspPipeline
        return;
    }
}

void StreamController::stop()
{
    if (m_pipeline)
    {
        m_pipeline->stop();
    }
    if (m_videoSink)
    {
        m_videoSink->setVideoFrame(QVideoFrame());
    }
    emit statusChanged(QStringLiteral("Stopped"));
}

void StreamController::handlePipelineError(int code, const QString& message)
{
    qWarning() << "Pipeline error:" << code << message;
    emit errorOccurred(code, message);
    emit statusChanged(QStringLiteral("Error"));
}

void StreamController::handlePipelineStateChanged(const QString& state)
{
    qDebug() << "Pipeline state changed:" << state;
    emit statusChanged(state);
}
