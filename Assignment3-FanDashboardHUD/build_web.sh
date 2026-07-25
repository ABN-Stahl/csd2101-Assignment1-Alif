#!/usr/bin/env sh
# =============================================================================
# build_web.sh - Build the Fan Dashboard HUD assignment for WebGL using Emscripten
#
# Prerequisites:
#   1. Install the Emscripten SDK:  https://emscripten.org/docs/getting_started/
#   2. Activate it:   source /path/to/emsdk/emsdk_env.sh
#   3. Run from the project root:  ./build_web.sh
#
# Output: web/index.html  (+ index.js, index.wasm)
# Serve:  cd web && python3 -m http.server 8080
# =============================================================================

set -e

mkdir -p web

# GLM is header-only; fetch it once into external/ (not committed - see README)
if [ ! -d external/glm/glm ]; then
    mkdir -p external
    git clone --depth 1 --branch 1.0.1 https://github.com/g-truc/glm.git external/glm
fi

emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Transform.cpp \
    Scene/Fan.cpp \
    Scene/Ground.cpp \
    Scene/Scene3D.cpp \
    Scene/SceneHUD.cpp \
    -IScene \
    -Iexternal/glm \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

echo ""
echo "Build succeeded."
echo "Run:  cd web && python3 -m http.server 8080"
echo "Open: http://localhost:8080/"
