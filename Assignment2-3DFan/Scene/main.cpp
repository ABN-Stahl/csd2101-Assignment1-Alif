/**
 * main.cpp
 *
 * Entry point for Desktop (Windows/GLFW) and WebGL (Emscripten/GLFW) builds.
 * Android uses NativeTemplate.cpp + JNI instead of this file.
 *
 * The left mouse button mirrors the touch gestures from the assignment:
 *   press   -> TouchEventDown
 *   drag    -> TouchEventMove
 *   release -> TouchEventRelease
 *
 * Edits: ported from Chapter2/4-TouchEvents-Modern's main.cpp. Removed the
 * tutorial's unused SDL2 backend branch (this project only ever builds the
 * GLFW path), and renamed window title / log strings from TouchEvents to
 * 3D Fan.
 */

#include "Platform.h"
#include "Renderer.h"

#ifdef PLATFORM_EMSCRIPTEN

static GLFWwindow* g_window    = nullptr;
static bool        g_mouseDown = false;

static void mouseButtonCB(GLFWwindow* win, int button, int action, int /*mods*/)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
    {
        return;
    }

    double x, y;
    glfwGetCursorPos(win, &x, &y);

    if (action == GLFW_PRESS)
    {
        g_mouseDown = true;
        Renderer::Instance().TouchEventDown(static_cast<float>(x), static_cast<float>(y));
    }
    else if (action == GLFW_RELEASE)
    {
        g_mouseDown = false;
        Renderer::Instance().TouchEventRelease(static_cast<float>(x), static_cast<float>(y));
    }
}

static void cursorPosCB(GLFWwindow* /*win*/, double x, double y)
{
    if (g_mouseDown)
    {
        Renderer::Instance().TouchEventMove(static_cast<float>(x), static_cast<float>(y));
    }
}

static void fbsizeCB(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

static void main_loop()
{
    if (glfwWindowShouldClose(g_window))
    {
        emscripten_cancel_main_loop();
        glfwDestroyWindow(g_window);
        glfwTerminate();
        return;
    }

    glfwPollEvents();
    Renderer::Instance().render();
    glfwSwapBuffers(g_window);
}

int main()
{
    LOGI("3D Fan - WebGL / Emscripten");

    if (!glfwInit())
    {
        LOGE("glfwInit failed");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    g_window = glfwCreateWindow(800, 600, "3D Fan", nullptr, nullptr);
    if (!g_window)
    {
        LOGE("glfwCreateWindow failed");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(g_window);
    glfwSetMouseButtonCallback(g_window, mouseButtonCB);
    glfwSetCursorPosCallback(g_window, cursorPosCB);
    glfwSetFramebufferSizeCallback(g_window, fbsizeCB);

    Renderer::Instance().initializeRenderer();

    int w, h;
    glfwGetFramebufferSize(g_window, &w, &h);
    Renderer::Instance().resize(w, h);

    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}

#elif defined(PLATFORM_WINDOWS)

#include <iostream>

static bool g_mouseDown = false;

static void mouseButtonCB(GLFWwindow* win, int button, int action, int /*mods*/)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT)
    {
        return;
    }

    double x, y;
    glfwGetCursorPos(win, &x, &y);

    if (action == GLFW_PRESS)
    {
        g_mouseDown = true;
        Renderer::Instance().TouchEventDown(static_cast<float>(x), static_cast<float>(y));
    }
    else if (action == GLFW_RELEASE)
    {
        g_mouseDown = false;
        Renderer::Instance().TouchEventRelease(static_cast<float>(x), static_cast<float>(y));
    }
}

static void cursorPosCB(GLFWwindow* /*win*/, double x, double y)
{
    if (g_mouseDown)
    {
        Renderer::Instance().TouchEventMove(static_cast<float>(x), static_cast<float>(y));
    }
}

static void framebufferSizeCB(GLFWwindow* /*win*/, int w, int h)
{
    Renderer::Instance().resize(w, h);
}

int main()
{
    std::cout << "3D Fan - Desktop (GLFW)\n";
    std::cout << "Click: tap (toggle ON/OFF) | Drag: spin faster | ESC: quit\n";

    if (!glfwInit())
    {
        std::cerr << "glfwInit failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Fan", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "glewInit failed\n";
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouseButtonCB);
    glfwSetCursorPosCallback(window, cursorPosCB);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCB);

    Renderer::Instance().initializeRenderer();

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    Renderer::Instance().resize(w, h);

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        Renderer::Instance().render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#endif // PLATFORM_WINDOWS
