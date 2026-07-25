#pragma once

/**
 * Scene3D.h
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this owns: the Fan (Quiz 2) plus the new Ground (Part 3), the shared
 * Phong light, and the adjustable camera distance (Part 7) both of them
 * render with. Also owns PickAt() (Part 8): unprojects a screen click into a
 * world-space ray and asks the Fan which of its cached part world matrices
 * that ray hits nearest.
 *
 * Edits: new file written for Quiz 3, no Quiz 2 equivalent existed - this is
 * the class that used to be "Renderer owns one Fan directly" before the
 * Scene3D/SceneHUD split (Part 1).
 */

#include "Model.h"
#include "Platform.h"
#include "SceneCamera.h"
#include "Transform.h"
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan;
class Ground;

class Scene3D : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* assetMgr);
#else
    Scene3D();
#endif
    ~Scene3D() override;

    void InitModel()          override;
    void Render()             override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float distance); // called by Renderer, fed from SceneHUD's zoom buttons
    void SetFanSpeed(float unitsPerFrame);  // called by Renderer, fed from SceneHUD's speed buttons

    // Part 8 - 3D picking. screenX/screenY are top-left-origin pixel
    // coordinates (the same convention mouse/touch already delivers).
    // Returns true if a fan part was hit (highlight state also toggles).
    bool PickAt(float screenX, float screenY);

private:
    Fan*    fan    = nullptr;
    Ground* ground = nullptr;

    // Used only to invoke Transform::TransformUnproject(), which takes its
    // matrices/viewport as explicit parameters and does not depend on this
    // instance's own stack state.
    Transform pickTransform;

    float cameraDistance = SceneCamera::kDefaultDistance;
    int   screenWidth    = 0;
    int   screenHeight   = 0;

    glm::vec3 lightPos   { 6.0f, 10.0f, 4.0f };
    glm::vec3 lightColor { 1.0f, 1.0f, 1.0f };
};
