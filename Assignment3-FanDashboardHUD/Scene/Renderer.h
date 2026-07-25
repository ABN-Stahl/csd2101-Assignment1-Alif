#pragma once

/**
 * Renderer.h
 *
 * Singleton renderer that owns a Scene3D and a SceneHUD and drives the
 * render loop in the fixed order Part 1 requires. Platform-agnostic -
 * shared by Android, Desktop, and WebGL builds.
 *
 * Edits for Quiz 3: replaced the generic std::vector<Model*> loop (used for
 * both render() and every TouchEvent*()) with explicit Scene3D / SceneHUD
 * pointer ownership - the fixed clear -> Scene3D -> disable depth -> SceneHUD ->
 * enable depth -> swap order, and HUD-first/pick-on-miss touch routing,
 * both need the Renderer to know which scene is which rather than just
 * iterating.
 */

#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Scene3D;
class SceneHUD;

class Renderer
{
public:
    static Renderer& Instance()
    {
        static Renderer instance;
        return instance;
    }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();

    void TouchEventDown(float x, float y);
    void TouchEventMove(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer() = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D*  scene3D  = nullptr;
    SceneHUD* sceneHUD = nullptr;

    int screenWidth  = 0;
    int screenHeight = 0;
};
