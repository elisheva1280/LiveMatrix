/// RtspPipeline.hpp
/// Engine layer: owns and manages the GStreamer RTSP pipeline.

#pragma once

#include <QObject>
#include <QPointer>
#include <QVideoSink>

#include <memory>
#include <string>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>

class RtspPipeline : public QObject
{
    Q_OBJECT

public:
    explicit RtspPipeline(QVideoSink* videoSink, QObject* parent = nullptr);
    ~RtspPipeline() override;

    bool start(const QString& url);
    void stop();

signals:
    void pipelineError(int code, const QString& message);
    void pipelineStateChanged(const QString& state);

private:
    struct GstElementDeleter
    {
        void operator()(GstElement* element) const noexcept
        {
            if (element != nullptr)
            {
                gst_object_unref(element);
            }
        }
    };

    struct GstBusDeleter
    {
        void operator()(GstBus* bus) const noexcept
        {
            if (bus != nullptr)
            {
                gst_object_unref(bus);
            }
        }
    };

    using GstElementPtr = std::unique_ptr<GstElement, GstElementDeleter>;
    using GstBusPtr     = std::unique_ptr<GstBus, GstBusDeleter>;

    static GstBusSyncReply busSyncHandler(GstBus* bus, GstMessage* message, gpointer user_data);
    static GstFlowReturn onNewSampleStatic(GstElement* sink, gpointer user_data);

    GstFlowReturn onNewSample(GstElement* sink);
    void handleErrorMessage(GstMessage* message);
    void handleStateChangedMessage(GstMessage* message);

    void clearPipeline();

    QPointer<QVideoSink> m_videoSink;
    GstElementPtr m_pipeline;
    GstBusPtr     m_bus;
};
