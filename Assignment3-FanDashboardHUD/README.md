# Fan Dashboard HUD

Quiz 3, built on top of `Assignment2-3DFan/`. The renderer now draws two
scenes per frame:

- **Scene3D** - the Fan (per-fragment Phong lighting) and a new Phong-lit
  checkerboard **Ground**, through a perspective camera at an adjustable
  distance.
- **SceneHUD** - four buttons (`+SPD`/`-SPD`/`+ZOOM`/`-ZOOM`) and a 20-block
  speed bar, drawn last through an orthographic projection.

## Controls

- **+SPD / -SPD** - fan speed, `[0, 20]`, mirrored in the speed bar.
- **+ZOOM / -ZOOM** - camera distance.
- **Click the fan directly** - picks/highlights the part under the cursor;
  click again (or empty space) to clear.

## Project layout

```
CMakeLists.txt              Desktop (GLFW + GLEW + GLM) entry point
build_web.bat / .sh         Web (Emscripten) entry point
Scene/                      Shared by all three platforms
  Fan.h, Fan.cpp             fan model: geometry, transforms, Phong, picking
  Ground.h, Ground.cpp       checkerboard floor, Phong-lit
  Scene3D.h, Scene3D.cpp     owns Fan + Ground, shared camera/light, PickAt()
  SceneHUD.h, SceneHUD.cpp   HUD buttons + speed bar, screen-space hit-testing
  SceneCamera.h              camera formula shared by Fan/Ground/Scene3D::PickAt
  main.cpp                   Desktop/Web entry point
android/                    Android Studio / Gradle project
  app/src/main/assets/shader/   all shaders (Desktop/Web copy this folder at build time)
```

## Desktop

```
cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
cmake --build build_desktop --config Debug
build_desktop\Debug\Fan3D.exe
```

## Android

Open `android/` in Android Studio, or from the command line:

```
cd android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

## Web (WebGL / Emscripten)

Requires the Emscripten SDK activated in the shell.

```
./build_web.sh        # build_web.bat on Windows
cd web
python -m http.server 8080
# open http://localhost:8080/
```

## CI/CD

No workflow file here - the repo-root `.github/workflows/` pipelines
(`android.yml`, `desktop.yml`, `pages.yml`) build this project via a matrix
entry alongside Quiz 1 and Quiz 2.
