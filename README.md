## LiveMatrix

LiveMatrix is a **Windows RTSP viewer** built with **C++20, Qt 6 (Qt Quick / QML / Qt Multimedia)** and **GStreamer**.  
It is designed to look and behave like a professional, real‑world video monitoring tool, focusing on:

- **Smooth RTSP playback** using a dedicated GStreamer pipeline
- **Clear separation** between UI (QML) and streaming logic (C++)
- **Robust error reporting** and simple, predictable behavior for junior‑level review

Developer: **Elisheva Cohen**


### Technology Overview

- **Language & Frameworks**
  - **C++20** (MSVC, Windows only)
  - **Qt 6.6+** – Quick, QML, Multimedia
  - **Qt Quick (QML)** – modern, responsive UI layer
- **Streaming Engine**
  - **GStreamer 1.22+** (or compatible 1.x series) – `rtspsrc` with H.264 decoding
  - Uses an `appsink` to feed frames into `QVideoSink` / `VideoOutput` in QML
- **Build System**
  - **CMake 3.21+**
  - Project is structured as a root CMake project with a `LiveMatrix` subdirectory


## Dependencies & Installation (Windows)

### Supported Platform

- **OS**: Windows 10 / 11, 64‑bit  
- **Compiler & Tools**:
  - Visual Studio 2022 (MSVC, x64 toolset)
  - CMake **3.21+**


### Required Components

- **Qt 6.6+ with:**
  - `Qt Quick`
  - `Qt Qml`
  - `Qt Multimedia`
- **GStreamer 1.22+ (or compatible 1.x) – MSVC 64‑bit build**
  - Install both **runtime** and **development** packages.
- **pkg-config**
  - Used by CMake to locate GStreamer (`gstreamer-1.0`, `gstreamer-video-1.0`, `gstreamer-app-1.0`).
  - Typically included with the official GStreamer SDK or available via MSYS2.


### Installing Qt (recommended workflow)

1. Download and run the **Qt Online Installer**.
2. Select:
   - **Qt 6.6.x (or newer)** for **MSVC 2019/2022 64‑bit**.
   - Ensure **Qt Multimedia** is selected.
3. After installation, note the Qt path, for example:
   - `C:\Qt\6.6.0\msvc2019_64`


### Installing GStreamer on Windows

1. Download the **64‑bit MSVC GStreamer SDK** (runtime + development).
2. Install to the **default location** (recommended):
   - `C:\Program Files\gstreamer\1.0\msvc_x86_64`
3. Ensure the following:
   - The `bin` directory (e.g.  
     `C:\Program Files\gstreamer\1.0\msvc_x86_64\bin`) is available at runtime.
   - The `lib\pkgconfig` directory is reachable by `pkg-config`, e.g. by setting:

     ```powershell
     $env:PKG_CONFIG_PATH = "C:\Program Files\gstreamer\1.0\msvc_x86_64\lib\pkgconfig"
     ```

4. The application’s `main.cpp` assumes the **default GStreamer path** and will:
   - Set `GST_PLUGIN_PATH` to  
     `C:\Program Files\gstreamer\1.0\msvc_x86_64\lib\gstreamer-1.0`
   - Prepend  
     `C:\Program Files\gstreamer\1.0\msvc_x86_64\bin`  
     to `PATH` at startup

   If you install GStreamer elsewhere, adjust these paths in `main.cpp` or configure the environment before launching the app.


## Building the Project (Windows)

### 1. Clone the repository

```powershell
git clone <your-repo-url> LiveMatrix
cd LiveMatrix
```


### 2. Configure with CMake

From a **Developer PowerShell for VS 2022** (or x64 Native Tools prompt):

```powershell
cd C:\path\to\LiveMatrix

cmake -S . -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\msvc2019_64"
```

Notes:

- `CMAKE_PREFIX_PATH` must point to your Qt installation root.
- CMake will:
  - Verify **Qt 6.6+** (`Quick`, `Qml`, `Multimedia`).
  - Use **pkg-config** to locate **GStreamer** (via `PkgConfig::GSTREAMER`).


### 3. Build

```powershell
cmake --build build --config Release
```

The main executable will typically be produced as:

- `build\LiveMatrix\Release\LiveMatrix.exe`


### 4. Optional: Build inside Qt Creator

1. Open **Qt Creator**.
2. `File` → `Open File or Project…` → select the root `CMakeLists.txt`.
3. Choose a **MSVC 64‑bit** kit with Qt 6.6+.
4. Configure, then **Build** and **Run** directly from Qt Creator.


## Running LiveMatrix on Windows

Once built successfully:

1. Navigate to the output directory (for example):

   ```powershell
   cd build\LiveMatrix\Release
   .\LiveMatrix.exe
   ```

2. The app opens a **modern, dark‑themed window** titled  
   **“LiveMatrix RTSP Professional”**.
3. At the top, you will see:
   - A **text field** for entering the RTSP URL.
   - A **PLAY** button.
   - A **STOP** button.
4. The central area is a **black video canvas** that will display the stream.
5. A small status text in the bottom corner of the video area shows:
   - `● READY`, `● Connecting…`, `● PLAYING`, `● Stopped`, `● Error`, etc.

### Console output (optional but recommended)

When started from **cmd** or **PowerShell**, the app attaches to the console so that:

- Diagnostic messages from **Qt** and **GStreamer** are visible.
- RTSP connection errors and pipeline issues can be inspected easily.

This is particularly helpful when testing RTSP URLs or debugging environment issues.


## Testing & Verifying RTSP

### 1. Check that your RTSP source works

Before using LiveMatrix, verify that your RTSP camera/stream is reachable:

1. Open a tool such as **VLC**.
2. `Media` → `Open Network Stream…`.
3. Paste your RTSP URL, for example:
   - `rtsp://<camera-ip>/path`
4. Confirm that VLC plays the stream correctly.

If VLC cannot connect, fix the camera/network issue first (firewall, credentials, URL, etc.).


### 2. Play the stream in LiveMatrix

1. Launch `LiveMatrix.exe` (from `build\LiveMatrix\Release` or your installation folder).
2. Paste the **same RTSP URL** into the URL field.
3. Click **PLAY**.
4. Observe:
   - Status changes to **“Connecting…”**.
   - On success, the video appears and status becomes **“PLAYING”**.
   - On failure, status becomes **“Error”** and details are printed to the console.
5. Click **STOP** to stop the pipeline and clear the frame.


### 3. Suggested test scenarios

- **Valid RTSP URL**
  - Expect video to appear quickly and status to show **“PLAYING”**.
- **Invalid / empty URL**
  - The UI prevents empty URLs and reports a clear error.
- **Network / camera offline**
  - GStreamer error messages will appear in the console.
  - Status will move to **“Error”** and then **“Idle”/“Stopped”** depending on flow.
- **Mid‑stream disconnect**
  - Observe how the pipeline reports the error and stops.

These scenarios correspond to the project’s **error‑handling requirements** and are useful for reviewer evaluation.


## Project Structure

High‑level layout:

- `CMakeLists.txt`  
  - Root CMake project.  
  - Configures C++ standard, finds Qt 6 and GStreamer via `pkg-config`, and adds the `LiveMatrix` subdirectory.

- `LiveMatrix/`
  - **CMakeLists.txt**  
    - Defines the `LiveMatrix` executable.  
    - Registers the QML module (`LiveMatrixQml`) and links Qt + GStreamer.
    - Post‑build step cleans up duplicate FFmpeg DLLs when using vcpkg.

  - `src/`
    - `main.cpp`  
      - Entry point: initializes Qt and GStreamer.  
      - Sets GStreamer plugin and PATH environment (for default install path).  
      - Sets up the QML engine and exposes `StreamController` to QML as `streamController`.
    - `StreamController.cpp`  
      - High‑level **controller** between UI and RTSP pipeline.  
      - Validates URLs, manages play/stop, tracks status text, and forwards detailed errors.
    - `RtspPipeline.cpp`  
      - Low‑level **GStreamer pipeline**:  
        `rtspsrc → rtph264depay → h264parse → decodebin → videoconvert → appsink`.  
      - Converts GStreamer frames to `QVideoFrame` and feeds `QVideoSink`.  
      - Handles bus messages and error reporting.

  - `include/`
    - `StreamController.hpp` – Qt `QObject` interface used from QML.  
    - `RtspPipeline.hpp` – encapsulation of the GStreamer RTSP pipeline.

  - `ui/`
    - `Main.qml`  
      - Top‑level window and UI layout.  
      - URL entry, PLAY/STOP controls, dark themed toolbar.  
      - Central `VideoOutput` bound to the `QVideoSink` provided by `StreamController`.  
      - Status indicator overlay showing current playback state.

- `docs/`
  - `project_summary.md`  
    - High‑level architectural and planning notes used to design the implementation.


## Notes & Tips for New Users

- **Start with a known‑good RTSP URL**
  - Test your camera or stream in VLC first.
  - If LiveMatrix fails but VLC works, check console logs for pipeline details.

- **Prefer wired / local networks**
  - For smooth evaluation, use a camera on the same LAN.

- **Environment consistency matters**
  - Ensure that Qt, GStreamer, and the MSVC toolchain are all **64‑bit**.
  - Avoid mixing MSVC and MinGW builds.

- **Debugging GStreamer issues**
  - Run from PowerShell and watch the console for `GStreamer` messages.
  - You can enable more verbose logs by setting `GST_DEBUG` before launching:

    ```powershell
    $env:GST_DEBUG = "3"
    .\LiveMatrix.exe
    ```

- **Deployment to another machine**
  - Make sure to deploy:
    - `LiveMatrix.exe`
    - Required Qt libraries and plugins (Qt Quick, Qt Qml, Qt Multimedia)
    - GStreamer runtime (or install GStreamer on the target machine)
  - Qt’s deployment tools (`windeployqt`) can help package Qt dependencies.


## Credits

- **Developer**: Elisheva Cohen

