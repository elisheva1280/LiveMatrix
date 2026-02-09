Technical Decisions & Architecture:

Separation of Concerns: Used a dedicated StreamController to bridge the GStreamer pipeline with the Qt Scene Graph, keeping the UI logic-free.

Performance: Integrated QVideoSink with GstVideoFrame mapping for low-overhead rendering.

Robustness: Implemented a fail-closed error handling mechanism via the GStreamer Bus to ensure UI stability during network drops.

Memory Management: Leveraged C++20 RAII patterns with custom deleters for all GStreamer resources to ensure zero memory leaks.