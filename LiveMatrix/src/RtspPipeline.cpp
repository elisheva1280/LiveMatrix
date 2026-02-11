#include "RtspPipeline.hpp"

#include <QDebug>
#include <QImage>
#include <QVideoFrame>
#include <QMetaObject>

namespace
{
QString gstStateToString(GstState state)
{
    switch (state)
    {
    case GST_STATE_NULL:    return QStringLiteral("NULL");
    case GST_STATE_READY:   return QStringLiteral("READY");
    case GST_STATE_PAUSED:  return QStringLiteral("PAUSED");
    case GST_STATE_PLAYING: return QStringLiteral("PLAYING");
    default:                return QStringLiteral("UNKNOWN");
    }
}
}

RtspPipeline::RtspPipeline(QVideoSink* videoSink, QObject* parent)
    : QObject(parent)
    , m_videoSink(videoSink)
{
}

RtspPipeline::~RtspPipeline()
{
    stop();
}

void RtspPipeline::stop()
{
    if (m_pipeline) {
        gst_element_set_state(m_pipeline.get(), GST_STATE_NULL);
        clearPipeline();
    }
}

void RtspPipeline::clearPipeline()
{
    m_bus.reset();
    m_pipeline.reset();
}

bool RtspPipeline::start(const QString& url)
{
    stop();

    if (url.trimmed().isEmpty()) {
        emit pipelineError(-1, QStringLiteral("RTSP URL is empty."));
        return false;
    }

    GstElement* pipeline = gst_pipeline_new("rtsp-pipeline");
    if (!pipeline) {
        emit pipelineError(-2, QStringLiteral("Failed to create GStreamer pipeline."));
        return false;
    }

    GstElement* src       = gst_element_factory_make("rtspsrc", "source");
    GstElement* depay     = gst_element_factory_make("rtph264depay", "depay");
    GstElement* parse     = gst_element_factory_make("h264parse", "parse");
    GstElement* decodebin = gst_element_factory_make("decodebin", "decodebin");
    GstElement* convert   = gst_element_factory_make("videoconvert", "convert");
    GstElement* appsink   = gst_element_factory_make("appsink", "videoSink");

    if (!src || !depay || !parse || !decodebin || !convert || !appsink) {
        emit pipelineError(-3, QStringLiteral("Failed to create elements."));
        gst_object_unref(pipeline);
        return false;
    }

    g_object_set(src,
                 "location", url.toUtf8().constData(),
                 "latency", 0, 
                 "buffer-mode", 0,
                 "drop-on-latency", TRUE,
                 "protocols", 7,
                 nullptr);

    g_object_set(appsink,
                 "emit-signals", TRUE,
                 "sync", FALSE,
                 "max-buffers", 1,
                 "drop", TRUE,
                 nullptr);

    GstCaps* caps = gst_caps_new_simple("video/x-raw",
                                        "format", G_TYPE_STRING, "RGBA",
                                        nullptr);
    g_object_set(appsink, "caps", caps, nullptr);
    gst_caps_unref(caps);

    gst_bin_add_many(GST_BIN(pipeline), src, depay, parse, decodebin, convert, appsink, nullptr);

    if (!gst_element_link_many(depay, parse, decodebin, nullptr)) {
        gst_object_unref(pipeline);
        return false;
    }
    if (!gst_element_link_many(convert, appsink, nullptr)) {
        gst_object_unref(pipeline);
        return false;
    }

    g_signal_connect(src, "pad-added", G_CALLBACK(+[](GstElement*, GstPad* newPad, gpointer userData) {
        GstElement* depayElem = static_cast<GstElement*>(userData);
        GstPad* sinkPad = gst_element_get_static_pad(depayElem, "sink");
        if (sinkPad) {
            if (!gst_pad_is_linked(sinkPad)) gst_pad_link(newPad, sinkPad);
            gst_object_unref(sinkPad);
        }
    }), depay);

    g_signal_connect(decodebin, "pad-added", G_CALLBACK(+[](GstElement*, GstPad* newPad, gpointer userData) {
        GstElement* convertElem = static_cast<GstElement*>(userData);
        GstPad* sinkPad = gst_element_get_static_pad(convertElem, "sink");
        if (sinkPad) {
            if (!gst_pad_is_linked(sinkPad)) gst_pad_link(newPad, sinkPad);
            gst_object_unref(sinkPad);
        }
    }), convert);

    g_signal_connect(appsink, "new-sample", G_CALLBACK(&RtspPipeline::onNewSampleStatic), this);

    GstBus* bus = gst_element_get_bus(pipeline);
    gst_bus_set_sync_handler(bus, &RtspPipeline::busSyncHandler, this, nullptr);

    m_pipeline.reset(pipeline);
    m_bus.reset(bus);

    if (gst_element_set_state(m_pipeline.get(), GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        clearPipeline();
        return false;
    }

    return true;
}

GstBusSyncReply RtspPipeline::busSyncHandler(GstBus*, GstMessage* message, gpointer user_data)
{
    auto* self = static_cast<RtspPipeline*>(user_data);
    if (!self || !message) return GST_BUS_PASS;

    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        GError* err = nullptr;
        gst_message_parse_error(message, &err, nullptr);
        QString text = err ? QString::fromUtf8(err->message) : "Unknown error";
        int code = err ? err->code : -1;
        if (err) g_error_free(err);

        QMetaObject::invokeMethod(self, [self, code, text]() {
            emit self->pipelineError(code, text);
            self->stop();
        }, Qt::QueuedConnection);
        return GST_BUS_DROP;
    }
    return GST_BUS_PASS;
}

GstFlowReturn RtspPipeline::onNewSampleStatic(GstElement* sink, gpointer user_data)
{
    auto* self = static_cast<RtspPipeline*>(user_data);
    return self ? self->onNewSample(sink) : GST_FLOW_ERROR;
}

GstFlowReturn RtspPipeline::onNewSample(GstElement* sink)
{
    GstSample* sample = nullptr;
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (!sample) return GST_FLOW_ERROR;

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstCaps* caps     = gst_sample_get_caps(sample);
    GstVideoInfo info;

    if (!buffer || !caps || !gst_video_info_from_caps(&info, caps)) {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstVideoFrame videoFrame;
    if (!gst_video_frame_map(&videoFrame, &info, buffer, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    // Wrap buffer for zero-copy where possible
    QImage image(
        static_cast<const uchar*>(GST_VIDEO_FRAME_PLANE_DATA(&videoFrame, 0)),
        GST_VIDEO_INFO_WIDTH(&info),
        GST_VIDEO_INFO_HEIGHT(&info),
        GST_VIDEO_INFO_PLANE_STRIDE(&info, 0),
        QImage::Format_RGBA8888);

    QImage copy = image.copy(); // Required as buffer is unmapped immediately

    gst_video_frame_unmap(&videoFrame);
    gst_sample_unref(sample);

    QVideoFrame video(copy);
    QPointer<QVideoSink> sinkPtr = m_videoSink;
    
    if (sinkPtr) {
        QMetaObject::invokeMethod(sinkPtr, [sinkPtr, video]() {
            if (sinkPtr) sinkPtr->setVideoFrame(video);
        }, Qt::QueuedConnection);
    }

    return GST_FLOW_OK;
}