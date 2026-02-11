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
   ```

---

## 🎞️ Installing GStreamer on Windows

**Download**: Obtain the **64-bit MSVC GStreamer SDK** (Runtime + Development).

**Install**: Use the default recommended path:
```text
C:\Program Files\gstreamer\1.0\msvc_x86_64
```

### Configuration Checklist
- [x] **Runtime**: Ensure the `bin` directory is available  
- [x] **pkg-config**: The `lib\pkgconfig` directory must be reachable  

Set environment variable:

```powershell
$env:PKG_CONFIG_PATH = "C:\Program Files\gstreamer\1.0\msvc_x86_64\lib\pkgconfig"
```

> **Important**  
> `main.cpp` assumes the default GStreamer path to set `GST_PLUGIN_PATH` and prepend the `bin` directory to `PATH`.  
> If you use a custom installation path, adjust it in `main.cpp` or configure your environment manually.

---

## 🛠️ Building the Project

### 1️⃣ Clone the Repository
```bash
git clone LiveMatrix
cd LiveMatrix
```

### 2️⃣ Configure with CMake
Run from a **Developer PowerShell for VS 2022**:

```powershell
cmake -S . -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\msvc2019_64"
```

> `CMAKE_PREFIX_PATH` must point to your Qt installation root.  
> CMake will automatically locate GStreamer via `pkg-config`.

### 3️⃣ Build
```powershell
cmake --build build --config Release
```

📂 **Output**
```text
build\LiveMatrix\Release\LiveMatrix.exe
```

### 4️⃣ Optional: Build inside Qt Creator
1. Open **Qt Creator**
2. `File` → `Open File or Project...`
3. Select the root `CMakeLists.txt`
4. Choose an **MSVC 64-bit** kit with Qt 6.6+
5. **Build** and **Run**

---

## 🏷️ Credits

👩‍💻 **Developer**: **Elisheva Cohen**
