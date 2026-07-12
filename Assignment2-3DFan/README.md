# 3D Fan

A 3D table fan (base, pole, hub, four blades) built from a single cube drawn
seven times on a push/pop matrix stack, running on Desktop, Android, and
WebGL from one shared C++ source tree (`Scene/`).

This project lives as the `Assignment2-3DFan/` subfolder of the shared
`csd2101-Assignment1-Alif` course repo, alongside `Assignment1-Android/` and
the `Chapter2/` tutorials. All commands below are run from inside this
folder (`cd Assignment2-3DFan` first if you've just cloned the repo root).

- **Tap** (click, or touch down+up with little movement) toggles the fan ON/OFF and logs `Fan ON` / `Fan OFF`.
- **Swipe / drag** (click-drag) spins the fan faster, proportional to drag speed. Releasing lets the boost decay back to base speed.

## Project layout

```
CMakeLists.txt              Desktop (GLFW + GLEW + GLM) entry point
script_build_and_run.bat    Desktop convenience wrapper
build_web.bat / .sh         Web (Emscripten) entry point
cmake/ImportDependencies.cmake
Scene/                      Shared by all three platforms
  Fan.h, Fan.cpp             the fan model - geometry, transforms, gestures
  Transform.h, Transform.cpp provided matrix stack (unmodified)
  Model.h, Renderer.h/.cpp, ShaderHelper.h, Platform.h
  main.cpp                   Desktop/Web entry point (mouse -> touch mapping)
android/                    Android Studio / Gradle project
```

## Desktop (Windows)

Prerequisites: CMake 3.15+, Visual Studio 2022 (or another VS 2022-compatible
generator), and an internet connection (CMake fetches GLFW, GLEW, and GLM the
first time via `FetchContent`).

```
script_build_and_run.bat
```

This wipes `build_desktop/`, configures, builds the Debug config, and runs
`build_desktop\Debug\Fan3D.exe`. Click and drag with the left mouse button;
press ESC to quit.

To build manually:

```
cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
cmake --build build_desktop --config Debug
build_desktop\Debug\Fan3D.exe
```

## Android

Prerequisites: Android Studio (current stable), Android SDK with API 34+ and
NDK, a device or emulator with OpenGL ES 3.0 support.

1. Open the `android/` folder in Android Studio as a project.
2. Let Gradle sync (this also fetches GLM via `FetchContent` inside
   `android/app/src/main/cpp/CMakeLists.txt`).
3. Run the `app` configuration on a device or emulator.

From the command line, once a Gradle wrapper is available (Android Studio
generates `gradlew` on first sync, or run `gradle wrapper` once if you have a
system Gradle installed):

```
cd android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb logcat -s Fan3D
```

Tap the screen to toggle the fan; drag to spin it faster. `Fan ON` / `Fan OFF`
appears in `adb logcat -s Fan3D`.

## Web (WebGL / Emscripten)

Prerequisites: [Emscripten SDK](https://emscripten.org/docs/getting_started/)
activated in the current shell (`source /path/to/emsdk/emsdk_env.sh` on
Linux/macOS, `emsdk_env.bat` on Windows), and `git` in `PATH` (used once to
fetch the header-only GLM library into `external/glm/`).

```
./build_web.sh          # Linux / macOS
build_web.bat            # Windows
```

This produces `web/index.html`, `web/index.js`, and `web/index.wasm`. Serve
the folder locally and open it in a browser:

```
cd web
python -m http.server 8080
# open http://localhost:8080/
```

Click and drag with the left mouse button; `Fan ON` / `Fan OFF` appears in
the browser's developer console.

## CI/CD

This project doesn't carry its own workflow file - GitHub Actions only reads
workflows from the repo root's `.github/workflows/`, so it's wired into the
three shared, matrix-based pipelines that cover every project in this repo:

- **`android.yml`** - builds a debug APK via Gradle (`Assignment2-3DFan`
  entry in the matrix). Installs the pinned NDK/CMake, reads the Gradle
  version straight out of `android/gradle/wrapper/gradle-wrapper.properties`,
  and uploads the APK as a build artifact.
- **`desktop.yml`** - configures and builds the GLFW/GLEW/GLM target on both
  `ubuntu-latest` and `windows-latest` (`Assignment2-3DFan` entries in the
  matrix).
- **`pages.yml`** - builds the Emscripten target (`bash build_web.sh`) and
  deploys it alongside the repo's other web demos. The live URL is nested
  under this project's name, e.g. `https://<user>.github.io/<repo>/Assignment2-3DFan/`,
  not the Pages root - that's what belongs in the class tracker's *web build
  output* column.

All three run automatically on every push to `main` (and on pull requests,
for `android.yml`/`desktop.yml`). Publishing Pages from a private repository
requires GitHub Pro (included in the GitHub Student Developer Pack) - in the
repository settings, set **Pages -> Source** to **GitHub Actions**.
