#pragma once

/**
 * SceneHUD.h
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this owns: the four dashboard buttons (+SPD, -SPD, +ZOOM, -ZOOM) and
 * the 20-block speed bar, all screen-space quads drawn in one orthographic
 * pass. Owns no 3D state itself - it only reads/writes the shared speed and
 * zoom values Renderer pulls out via CurrentSpeed()/CurrentZoom() and pushes
 * into Scene3D every frame. Geometry is plain position+colour triangles, no
 * textures, no font rendering (per the quiz spec), rebuilt every frame -
 * at most 24 quads, so there's nothing worth dirty-tracking.
 *
 * Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.
 */

#include "Model.h"
#include "Platform.h"
#include "SceneCamera.h"
#include "Transform.h"
#include <glm/glm.hpp>
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class SceneHUD : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* assetMgr);
#else
    SceneHUD();
#endif
    ~SceneHUD() override;

    void InitModel()          override;
    void Render()             override;
    void Resize(int w, int h) override;

    bool TouchDown(float x, float y); // HUD hit-testing - Part 5. Returns true if a button was hit.
    void TouchRelease();

    float CurrentSpeed() const { return speed; }
    float CurrentZoom()  const { return zoom;  }

private:
    struct Rect
    {
        float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    };

    enum ButtonId
    {
        BTN_SPD_UP   = 0,
        BTN_SPD_DOWN = 1,
        BTN_ZOOM_IN  = 2,
        BTN_ZOOM_OUT = 3,
        BTN_COUNT    = 4
    };

    static bool PointInRect(float x, float y, const Rect& r);
    void RebuildGeometry();
    void AppendQuad(std::vector<GLfloat>& verts, const Rect& r, const glm::vec3& color);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLint  uProjection = -1;

    Transform transform;

    Rect buttons[BTN_COUNT];
    Rect barOrigin; // top-left of block 0; each block is kBlockW x kBlockH

    int screenWidth  = 0;
    int screenHeight = 0;

    GLsizei vertexCount = 0;

    float speed  = 8.0f;   // 0..20 - Part 6
    float zoom   = SceneCamera::kDefaultDistance;  // camera distance mirror, clamped to SceneCamera's range - Part 7
    int   activeButton = -1; // which button (if any) is held

    static constexpr int   kSpeedSteps = 20;
    static constexpr float kBlockW     = 10.0f;
    static constexpr float kBlockH     = 20.0f;
    static constexpr float kBlockGap   = 2.0f;
};
