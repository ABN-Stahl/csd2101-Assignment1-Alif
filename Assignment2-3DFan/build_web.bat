@echo off
REM =============================================================================
REM build_web.bat - Build the 3D Fan assignment for WebGL (Emscripten/Windows)
REM
REM Prerequisites:
REM   1. Install Emscripten SDK: https://emscripten.org/docs/getting_started/
REM   2. Activate emsdk in this shell before running, OR edit EMSDK_PATH below.
REM   3. Run from the project root.
REM
REM Output: web\index.html (+ index.js, index.wasm)
REM Serve:  cd web && python -m http.server 8080
REM =============================================================================

REM SET EMSDK_PATH=C:\emsdk
REM CALL "%EMSDK_PATH%\emsdk_env.bat"

IF NOT EXIST web MKDIR web

REM GLM is header-only; fetch it once into external/ (not committed - see README)
IF NOT EXIST external\glm\glm (
    IF NOT EXIST external MKDIR external
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git external/glm
)

emcc -std=c++17 ^
     Scene/main.cpp ^
     Scene/Renderer.cpp ^
     Scene/Transform.cpp ^
     Scene/Fan.cpp ^
     -IScene ^
     -Iexternal/glm ^
     -DUSE_GLFW ^
     -s USE_GLFW=3 ^
     -s USE_WEBGL2=1 ^
     -s FULL_ES3=1 ^
     -s WASM=1 ^
     -s ALLOW_MEMORY_GROWTH=1 ^
     --preload-file android/app/src/main/assets/shader@assets/shader ^
     -o web/index.html

IF %ERRORLEVEL% NEQ 0 (
    echo Build FAILED.
    pause
    exit /b 1
)

echo.
echo Build succeeded.
echo Run:  cd web ^&^& python -m http.server 8080
echo Open: http://localhost:8080/
pause
