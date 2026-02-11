# 🎥 LiveMatrix  
### *Professional RTSP Desktop Viewer for Windows*

---

🖥️ **LiveMatrix** is a **Windows RTSP viewer** built with **C++20, Qt 6 (Qt Quick / QML / Qt Multimedia)** and **GStreamer**.  
It is designed to look and behave like a professional, real-world video monitoring tool, focusing on:

✔ **Smooth RTSP playback** using a dedicated GStreamer pipeline  
✔ **Clear separation** between UI (QML) and streaming logic (C++)  
✔ **Robust error reporting** and simple, predictable behavior for junior-level review  

👩‍💻 **Developer**: **Elisheva Cohen**

---

## 🧩 Technology Overview

### 🔹 Language & Frameworks
- **C++20** (MSVC, Windows only)  
- **Qt 6.6+** – Quick, QML, Multimedia  
- **Qt Quick (QML)** – modern, responsive UI layer  

### 🔹 Streaming Engine
- **GStreamer 1.22+** (or compatible 1.x series) – `rtspsrc` with H.264 decoding  
- Uses an `appsink` to feed frames into `QVideoSink` / `VideoOutput` in QML  

### 🔹 Build System
- **CMake 3.21+**  
- Project is structured as a root CMake project with a `LiveMatrix` subdirectory  

---

## 🖥️ Dependencies & Installation (Windows)

### ✅ Supported Platform
- **OS**: Windows 10 / 11, 64-bit  
- **Compiler & Tools**:
  - Visual Studio 2022 (MSVC, x64 toolset)  
  - CMake **3.21+**

---

### 📦 Required Components

- **Qt 6.6+ with:**
  - `Qt Quick`
  - `Qt Qml`
  - `Qt Multimedia`

- **GStreamer 1.22+ (or compatible 1.x) – MSVC 64-bit build**
  - Install both **runtime** and **development** packages.

- **pkg-config**
  - Used by CMake to locate GStreamer  
    (`gstreamer-1.0`, `gstreamer-video-1.0`, `gstreamer-app-1.0`)
  - Typically included with the official GStreamer SDK or available via MSYS2.

---

## ⚙️ Installing Qt (recommended workflow)

1. Download and run the **Qt Online Installer**.
2. Select:
   - **Qt 6.6.x (or newer)** for **MSVC 2019/2022 64-bit**
   - Ensure **Qt Multimedia** is selected.
3. After installation, note the Qt path, for example:
   ```text
   C:\Qt\6.6.0\msvc2019_64
מעולה.
להלן אותו תוכן בדיוק, מסודר ומעוצב מחדש ברמת README יוקרתי וברור — עם כותרות, היררכיה, אייקונים, קופסאות קוד והפרדות.
❗ לא שיניתי מילים, רק תצוגה.

## 🎞️ Installing GStreamer on Windows

### 1️⃣ Download
- Download the **64-bit MSVC GStreamer SDK** (**runtime + development**).

### 2️⃣ Install (recommended)
Install to the default location:
```text
C:\Program Files\gstreamer\1.0\msvc_x86_64
3️⃣ Ensure environment readiness
🎞️ LiveMatrix - Environment Readiness
1️⃣ Installing GStreamer on Windows
Download the 64-bit MSVC GStreamer SDK (runtime + development).

Default Location (Recommended): C:\Program Files\gstreamer\1.0\msvc_x86_64

Ensure:

The bin directory is available at runtime.

The lib\pkgconfig directory is reachable by pkg-config.

Environment Configuration:

PowerShell
$env:PKG_CONFIG_PATH = "C:\Program Files\gstreamer\1.0\msvc_x86_64\lib\pkgconfig"
Note: The application’s main.cpp assumes the default GStreamer path and will automatically set GST_PLUGIN_PATH and prepend the bin directory to PATH at startup. Adjust paths in main.cpp if installed elsewhere.

🛠️ Building the Project (Windows)
1. Clone the Repository
Bash
git clone LiveMatrix
cd LiveMatrix
2. Configure with CMake
From a Developer PowerShell for VS 2022:

PowerShell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
-DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\msvc2019_64"
📌 Note: CMAKE_PREFIX_PATH must point to your Qt installation root.

3. Build
PowerShell
cmake --build build --config Release
📂 Output: build\LiveMatrix\Release\LiveMatrix.exe

4. Optional: Build inside Qt Creator
Open Qt Creator.

File → Open File or Project…

Select the root CMakeLists.txt.

Choose a MSVC 64-bit kit with Qt 6.6+.

▶️ Running LiveMatrix on Windows
Navigate and Run:

PowerShell
cd build\LiveMatrix\Release
.\LiveMatrix.exe
UI Features:

Modern, Dark-themed Window: Titled “LiveMatrix RTSP Professional”.

Controls: RTSP URL input field, PLAY button, STOP button.

Display: Central black video canvas.

Status Indicators: * ● READY

● Connecting…

● PLAYING

● Stopped

● Error

🧪 Testing & Verifying RTSP
✔️ Check your RTSP source: Test first using VLC to ensure the stream works.

▶️ Play the stream: Paste RTSP URL → Click PLAY.

Errors: Appear clearly in the console.

🗂️ Project Structure
Plaintext
LiveMatrix/
├─ CMakeLists.txt
├─ LiveMatrix/
│  ├─ src/
│  ├─ include/
│  ├─ ui/
│  └─ CMakeLists.txt
└─ docs/
Each module has a clear, single responsibility and mirrors real-world desktop architecture.

💡 Notes & Tips for New Users
Start with a known-good RTSP URL.

Prefer local / wired networks.

Ensure 64-bit consistency.

Enable Verbose Logs:

PowerShell
$env:GST_DEBUG = "3"
.\LiveMatrix.exe
🏷️ Credits
👩‍💻 Developer: Elisheva Cohen