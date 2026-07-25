/**
 * Renderer.cpp
 *
 * Edits for Quiz 3 (Part 1): createModels() now builds one Scene3D and one
 * SceneHUD instead of one Fan. render() implements the exact
 * clear -> Scene3D (depth ON) -> disable depth+cull -> SceneHUD ->
 * re-enable depth+cull -> swap order the quiz requires (buffer swap itself
 * happens in main.cpp/NativeTemplate.cpp, after this returns). Touch events
 * go to SceneHUD first; only a miss (no button hit) falls through to
 * Scene3D::PickAt (Part 5/8) - a HUD click must never also trigger a 3D
 * pick, and vice versa.
 */

#define LOG_TAG "Renderer"
#include "Renderer.h"
#include "Scene3D.h"
#include "SceneHUD.h"

Renderer::~Renderer()
{
    clearModels();
}

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr)
{
    assetMgr = mgr;
}
#endif

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::createModels()
{
    clearModels();
#ifdef PLATFORM_ANDROID
    scene3D  = new Scene3D(assetMgr);
    sceneHUD = new SceneHUD(assetMgr);
#else
    scene3D  = new Scene3D();
    sceneHUD = new SceneHUD();
#endif
    LOGI("Renderer: Scene3D + SceneHUD created");
}

void Renderer::initializeModels()
{
    scene3D->InitModel();
    sceneHUD->InitModel();
}

void Renderer::clearModels()
{
    delete scene3D;
    delete sceneHUD;
    scene3D  = nullptr;
    sceneHUD = nullptr;
}

void Renderer::resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;
    glViewport(0, 0, w, h);
    scene3D->Resize(w, h);
    sceneHUD->Resize(w, h);
    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::render()
{
    // HUD-driven state (Part 6/7) is pulled into the 3D scene before it draws.
    scene3D->SetFanSpeed(sceneHUD->CurrentSpeed());
    scene3D->SetCameraDistance(sceneHUD->CurrentZoom());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    scene3D->Render();

    // A HUD button must never be hidden behind "closer" 3D geometry, and its
    // screen-space winding (Y-down ortho) isn't guaranteed front-facing
    // under the 3D scene's CCW convention, so both are turned off for the
    // HUD pass and restored afterward for next frame's Scene3D pass.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    sceneHUD->Render();
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::TouchEventDown(float x, float y)
{
    if (sceneHUD->TouchDown(x, y))
    {
        return; // consumed by a HUD button - never also tested against the 3D scene
    }
    scene3D->PickAt(x, y);
}

void Renderer::TouchEventMove(float /*x*/, float /*y*/)
{
    // HUD buttons are click-based, not drag-based, and picking only fires on
    // down - nothing to do here for Quiz 3.
}

void Renderer::TouchEventRelease(float /*x*/, float /*y*/)
{
    sceneHUD->TouchRelease();
}
