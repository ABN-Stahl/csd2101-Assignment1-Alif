#pragma once

/**
 * Platform.h
 *
 * Single-header platform detection for the 3D Fan assignment.
 * Supports three targets:
 *   PLATFORM_EMSCRIPTEN  - WebGL 2.0 via Emscripten + GLFW
 *   PLATFORM_WINDOWS     - OpenGL ES (compat) via GLFW + GLEW
 *   PLATFORM_ANDROID     - OpenGL ES 3.0 via Android NDK
 *
 * Emscripten must be detected before Windows because emcc defines both
 * __EMSCRIPTEN__ and _WIN32 on some toolchain setups.
 *
 * Edits: ported from Chapter2/1-GLPIFrameworkIntro-Modern's Platform.h.
 * Changed the default LOG_TAG to "Fan3D" and guarded each platform #define
 * against redefinition, since the desktop CMakeLists.txt already defines
 * PLATFORM_WINDOWS on the compiler command line.
 */

#if defined(__EMSCRIPTEN__) && !defined(PLATFORM_EMSCRIPTEN)
    #define PLATFORM_EMSCRIPTEN
#elif defined(_WIN32) && !defined(PLATFORM_WINDOWS)
    #define PLATFORM_WINDOWS
#elif defined(__ANDROID__) && !defined(PLATFORM_ANDROID)
    #define PLATFORM_ANDROID
#endif

#ifdef PLATFORM_EMSCRIPTEN
    #define GLFW_INCLUDE_ES3
    #include <GLFW/glfw3.h>
    #include <emscripten.h>
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#elif defined(PLATFORM_WINDOWS)
    #include <GL/glew.h>
    #include <GLFW/glfw3.h>
    #include <cstdio>
    #include <cstring>

    #define LOGI(...) do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGE(...) do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
    #define LOGD(...) do { printf("[DEBUG] " __VA_ARGS__); printf("\n"); } while(0)

#elif defined(PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
    #include <android/log.h>
    #include <cstring>

    #ifndef LOG_TAG
    #define LOG_TAG "Fan3D"
    #endif

    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif
