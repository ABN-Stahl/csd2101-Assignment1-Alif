#pragma once

/**
 * Fan.h
 *
 * A 3D table fan (base, pole, hub, four blades) built from a single 8-vertex
 * cube drawn seven times with a different model matrix composed on the
 * Transform push/pop stack, tinted per-part through the PARTCOLOR uniform.
 *
 * Gestures:
 *   TAP (down+up, little movement)  - toggle fanOn and log Fan ON / Fan OFF
 *   SWIPE / DRAG                    - drag velocity (px/ms, via std::chrono)
 *                                     is converted into a clamped dragBoost
 *                                     added on top of the base spin speed
 *
 * Bonus (fling with decay): on release dragBoost is not reset to zero
 * immediately. Instead it decays a little every frame (dragBoost *= kBoostDecay)
 * so the fan winds down like a real one, instead of snapping back to base speed.
 *
 * Platform-agnostic - shared by Android, Desktop, and WebGL builds.
 */

#include "Model.h"
#include "Platform.h"
#include "Transform.h"
#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel()          override;
    void Render()             override;
    void Resize(int w, int h) override;

    void TouchEventDown(float x, float y)    override;
    void TouchEventMove(float x, float y)    override;
    void TouchEventRelease(float x, float y) override;

private:
    void drawPart(const glm::vec3& color);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLuint ibo     = 0;

    GLint uMVP        = -1;
    GLint uPartColor  = -1;

    Transform transform;

    float spinAngle = 0.0f;
    bool  fanOn     = true;
    float dragBoost = 0.0f;
    bool  dragging  = false;

    static constexpr float kBaseSpeed    = 1.5f;
    static constexpr float kMaxBoost     = 20.0f;
    static constexpr float kBoostScale   = 8.0f;
    static constexpr float kTapThreshold = 12.0f;
    static constexpr float kBoostDecay   = 0.98f;

    float lastX = 0.0f;
    float lastY = 0.0f;
    float movedDistance = 0.0f;
    std::chrono::steady_clock::time_point lastMoveTime;
};
