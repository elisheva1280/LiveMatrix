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
} // namespace

RtspPipeline::RtspPipeline(QVideoSink* videoSink, QObject* parent)
    : QObject(parent)
    , m_videoSink(videoSink)
{
}

RtspPipeline::~RtspPipeline()
{
    stop();
}

bool RtspPipeline::start(const QString& url)
{
    stop();

    if (url.trimmed().isEmpty())
    {
        emit pipelineError(-1, QStringLiteral("RTSP URL is empty."));
        return false;
    }

    qDebug() << "RtspPipeline::start with URL:" << url;

    GstElement* pipeline = gst_pipeline_new("rtsp-pipeline");
    if (!pipeline)
    {
        emit pipelineError(-2, QStringLiteral("Failed to create GStreamer pipeline."));
        return false;
    }

    GstElement* src       = gst_element_factory_make("rtspsrc", "source");
    GstElement* depay     = gst_element_factory_make("rtph264depay", "depay");
    GstElement* parse     = gst_element_factory_make("h264parse", "parse");
    GstElement* decodebin = gst_element_factory_make("decodebin", "decodebin");
    GstElement* convert   = gst_element_factory_make("videoconvert", "convert");
    GstElement* appsink   = gst_element_factory_make("appsink", "videoSink");

    if (!src || !depay || !parse || !decodebin || !convert || !appsink)
    {
        emit pipelineError(-3, QStringLiteral("Failed to create one or more GStreamer elements."));
        gst_object_unref(pipeline);
        return false;
    }

    g_object_set(src,
                 "location", url.toUtf8().constData(),
                 "latency", 200,
                 "protocols", 7, // GST_RTSP_LOWER_TRANS_TCP
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

    gst_bin_add_many(GST_BIN(pipeline),
                     src, depay, parse, decodebin, convert, appsink,
                     nullptr);

    if (!gst_element_link_many(depay, parse, decodebin, nullptr))
    {
        emit pipelineError(-4, QStringLiteral("Failed to link depay -> parse -> decodebin."));
        gst_object_unref(pipeline);
        return false;
    }

    if (!gst_element_link_many(convert, appsink, nullptr))
    {
        emit pipelineError(-5, QStringLiteral("Failed to link videoconvert -> appsink."));
        gst_object_unref(pipeline);
        return false;
    }

    g_signal_connect(src, "pad-added",
                     G_CALLBACK(+[](GstElement* srcElem, GstPad* newPad, gpointer userData)
                     {
                         GstElement* depayElem = static_cast<GstElement*>(userData);
                         GstPad* sinkPad = gst_element_get_static_pad(depayElem, "sink");
                         if (!sinkPad)
                         {
                             qWarning() << "Failed to get depay sink pad.";
                             return;
                         }

                         if (gst_pad_is_linked(sinkPad))
                         {
                             gst_object_unref(sinkPad);
                             return;
                         }

                         if (gst_pad_link(newPad, sinkPad) != GST_PAD_LINK_OK)
                         {
                             qWarning() << "Failed to link rtspsrc to depay.";
                         }

                         gst_object_unref(sinkPad);
                     }),
                     depay);

    g_signal_connect(decodebin, "pad-added",
                     G_CALLBACK(+[](GstElement* decode, GstPad* newPad, gpointer userData)
                     {
                         GstElement* convertElem = static_cast<GstElement*>(userData);
                         GstPad* sinkPad = gst_element_get_static_pad(convertElem, "sink");
                         if (!sinkPad)
                         {
                             qWarning() << "Failed to get convert sink pad.";
                             return;
                         }

                         if (gst_pad_is_linked(sinkPad))
                         {
                             gst_object_unref(sinkPad);
                             return;
                         }

                         if (gst_pad_link(newPad, sinkPad) != GST_PAD_LINK_OK)
                         {
                             qWarning() << "Failed to link decodebin to videoconvert.";
                         }

                         gst_object_unref(sinkPad);
                     }),
                     convert);

    g_signal_connect(appsink, "new-sample",
                     G_CALLBACK(&RtspPipeline::onNewSampleStatic),
                     this);

    GstBus* bus = gst_element_get_bus(pipeline);
    gst_bus_set_sync_handler(bus, &RtspPipeline::busSyncHandler, this, nullptr);

    m_pipeline.reset(pipeline);
    m_bus.reset(bus);

    GstStateChangeReturn ret = gst_element_set_state(m_pipeline.get(), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        emit pipelineError(-6, QStringLiteral("Failed to set pipeline to PLAYING state."));
        clearPipeline();
        return false;
    }

    qDebug() << "GStreamer pipeline started.";
    return true;
}

void RtspPipeline::stop()
{
    if (!m_pipeline)
    {
        return;
    }

    qDebug() << "Stopping GStreamer pipeline.";
    gst_element_set_state(m_pipeline.get(), GST_STATE_NULL);
    clearPipeline();
}

void RtspPipeline::clearPipeline()
{
    m_bus.reset();
    m_pipeline.reset();
}

GstBusSyncReply RtspPipeline::busSyncHandler(GstBus* /*bus*/, GstMessage* message, gpointer user_data)
{
    auto* self = static_cast<RtspPipeline*>(user_data);
    if (!self || !message)
    {
        return GST_BUS_PASS;
    }

    switch (GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_ERROR:
        self->handleErrorMessage(message);
        return GST_BUS_DROP;

    case GST_MESSAGE_STATE_CHANGED:
        self->handleStateChangedMessage(message);
        break;

    default:
        break;
    }

    return GST_BUS_PASS;
}

void RtspPipeline::handleErrorMessage(GstMessage* message)
{
    GError* err = nullptr;
    gchar* debugInfo = nullptr;
    gst_message_parse_error(message, &err, &debugInfo);

    int code = err ? err->code : -100;
    QString text = QStringLiteral("GStreamer error.");

    if (err)
    {
        text = QString::fromUtf8(err->message);
    }

    if (debugInfo)
    {
        qWarning() << "GStreamer debug info:" << debugInfo;
    }

    if (err)
    {
        g_error_free(err);
    }
    g_free(debugInfo);

    QMetaObject::invokeMethod(
        this,
        [this, code, text]()
        {
            emit pipelineError(code, text);
        },
        Qt::QueuedConnection);

    // Fail-closed: stop the pipeline on error
    QMetaObject::invokeMethod(
        this,
        [this]()
        {
            stop();
        },
        Qt::QueuedConnection);
}

void RtspPipeline::handleStateChangedMessage(GstMessage* message)
{
    if (!m_pipeline)
    {
        return;
    }

    if (GST_MESSAGE_SRC(message) != GST_OBJECT(m_pipeline.get()))
    {
        return;
    }

    GstState oldState = GST_STATE_NULL;
    GstState newState = GST_STATE_NULL;
    GstState pending  = GST_STATE_NULL;
    gst_message_parse_state_changed(message, &oldState, &newState, &pending);

    QString oldStr = gstStateToString(oldState);
    QString newStr = gstStateToString(newState);

    qDebug() << "Pipeline state transition:" << oldStr << "->" << newStr;

    QMetaObject::invokeMethod(
        this,
        [this, newStr]()
        {
            emit pipelineStateChanged(newStr);
        },
        Qt::QueuedConnection);
}

GstFlowReturn RtspPipeline::onNewSampleStatic(GstElement* sink, gpointer user_data)
{
    auto* self = static_cast<RtspPipeline*>(user_data);
    if (!self)
    {
        return GST_FLOW_ERROR;
    }
    return self->onNewSample(sink);
}

GstFlowReturn RtspPipeline::onNewSample(GstElement* sink)
{
    GstSample* sample = nullptr;
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (!sample)
    {
        return GST_FLOW_ERROR;
    }

    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstCaps* caps     = gst_sample_get_caps(sample);

    if (!buffer || !caps)
    {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    GstVideoInfo info;
    if (!gst_video_info_from_caps(&info, caps))
    {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    if (GST_VIDEO_INFO_FORMAT(&info) != GST_VIDEO_FORMAT_RGBA)
    {
        // Unexpected format; ignore this frame but keep pipeline running.
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    GstVideoFrame videoFrame;
    if (!gst_video_frame_map(&videoFrame, &info, buffer, GST_MAP_READ))
    {
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    const int width  = static_cast<int>(GST_VIDEO_FRAME_WIDTH(&videoFrame));
    const int height = static_cast<int>(GST_VIDEO_FRAME_HEIGHT(&videoFrame));
    const int stride = static_cast<int>(GST_VIDEO_FRAME_PLANE_STRIDE(&videoFrame, 0));

    QImage image(
        static_cast<const uchar*>(GST_VIDEO_FRAME_PLANE_DATA(&videoFrame, 0)),
        width,
        height,
        stride,
        QImage::Format_RGBA8888);

    // Deep copy so we are independent of the GStreamer buffer lifetime.
    QImage copy = image.copy();
    gst_video_frame_unmap(&videoFrame);
    gst_sample_unref(sample);

    if (copy.isNull())
    {
        return GST_FLOW_OK;
    }

    QVideoFrame video(copy);

    QPointer<QVideoSink> sinkPtr = m_videoSink;
    if (!sinkPtr)
    {
        return GST_FLOW_OK;
    }

    QMetaObject::invokeMethod(
        sinkPtr,
        [sinkPtr, video]()
        {
            if (sinkPtr)
            {
                sinkPtr->setVideoFrame(video);
            }
        },
        Qt::QueuedConnection);

    return GST_FLOW_OK;
}
