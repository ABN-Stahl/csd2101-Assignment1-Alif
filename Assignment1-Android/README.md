# AndroidProgrammingQuiz-CrossPlatformCI

A renderer built on a `Model` + `Renderer`architecture. 
A rotating RGB triangle and a solid yellow square render
together on three targets: Desktop, Android, and Web.

## Architecture

- `Scene/Model.h` — abstract base class defining the shape lifecycle
  (`InitModel`, `Render`, `Resize`).
- `Scene/Renderer.h` / `.cpp` — Owns a collection of
  `Model*` and driving the frame lifecycle.
- `Scene/ShaderHelper.h` — reusable shader compilation, supporting inline
  strings and file-based `.glsl` loading (disk on Desktop/Web, APK assets on
  Android).
- `Scene/Triangle.h` / `.cpp`, `Scene/Square.h` / `.cpp` — `Model`
  subclasses, each owning its own shader program, VAO, and VBOs.

Shader source in `android/app/src/main/assets/shader/`.

## Build & Run

### Desktop

Run from the project root:

```
script_build_and_run.bat
```

This configures and builds viaw `CMakeLists.txt` (GLFW + GLEW),
copies the shader assets next to the executable, and launches a window
showing both shapes. Press `ESC` to quit.

### Android

1. Open the `android/` folder in Android Studio.
2. Let Gradle sync complete.
3. Build and run on an emulator or device.

The native library is built via `android/app/src/main/cpp/CMakeLists.txt`.
Shaders are read from the APK assets through `AAssetManager`.

### Web (Emscripten)

Ensure Emscripten is installed and activated, check via "emcc --version", then
run from the project root:

```
build_web.bat for Windows
bash build_web.sh for Linux or WSL
```

Re-enter the output:

```
e.g:
cd web && python -m http.server 8080
```

Open `http://localhost:8080/` in a browser, to view the output.