#pragma once

/**
 * Fan.h
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this does: declares the Fan model - a 3D table fan (base, pole, hub,
 * four blades) built from a single 8-vertex cube drawn seven times with a
 * different model matrix composed on the Transform push/pop stack, tinted
 * per-part through the PARTCOLOR uniform. Geometry/hierarchy carried over
 * unchanged from Quiz 2.
 *
 * Edits for Quiz 3 ("Fan Dashboard HUD"):
 *   - Quiz 2's TouchEventDown/Move/Release tap+drag gesture handling
 *     (dragBoost, kTapThreshold, fling decay) is removed outright - speed is
 *     now driven only by Scene3D::SetFanSpeed(), itself fed by SceneHUD's
 *     +SPD/-SPD buttons.
 *   - The camera translate/rotate that used to be baked into the first two
 *     MODEL_MATRIX ops of Render() moved to its own VIEW_MATRIX stack (see
 *     SceneCamera::ApplyView()), driven by an adjustable cameraDistance
 *     instead of a hard-coded -8 offset.
 *   - Per-corner Shade is replaced by a per-corner Normal attribute and real
 *     per-fragment Phong lighting (Part 2). The cube's 8 shared corners keep
 *     one normal each (normalize(position)) rather than duplicating corners
 *     per face - a softer, rounded-cube look, one of the two options the
 *     quiz explicitly allows.
 *   - Each part's world matrix is cached every Render() call and exposed for
 *     Scene3D::PickAt() (Part 8) to ray-test against.
 *   - Blade count raised from 4 to kBladeCount (16), still the same shared
 *     cube mesh just scaled thinner/longer and spread evenly around 360deg,
 *     for a fuller sunburst look. Blade indices/colours/names are generated
 *     from kBladeCount instead of being hand-enumerated.
 *   - Added an independent slow turntable spin (modelSpinAngle) rotating the
 *     whole assembly (base+pole+hub+blades together) around the vertical
 *     axis, layered underneath the existing per-blade spin - purely a
 *     presentation flourish, not tied to the HUD speed/zoom controls.
 *
 * Platform-agnostic - shared by Android, Desktop, and WebGL builds.
 */

#include "Model.h"
#include "Platform.h"
#include "SceneCamera.h"
#include "Transform.h"
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model
{
public:
    // Index of each pickable part, also used to index worldMatrix[] below.
    enum PartId
    {
        PART_BASE        = 0,
        PART_POLE        = 1,
        PART_HUB         = 2,
        PART_BLADE_FIRST = 3
    };

    static constexpr int kBladeCount = 16;
    static constexpr int PART_COUNT  = PART_BLADE_FIRST + kBladeCount;

    static const char* PartName(int partId);

#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else
    Fan();
#endif
    ~Fan() override;

    void InitModel()          override;
    void Render()             override;
    void Resize(int w, int h) override;

    // Driven by Scene3D, which itself is driven by SceneHUD's buttons.
    void SetSpeed(float unitsPerFrame);
    void SetCameraDistance(float distance);
    void SetLight(const glm::vec3& worldPos, const glm::vec3& color);

    // Part 8 - 3D picking. rayOrigin/rayDir are world-space (same space as
    // worldMatrix[], i.e. before the camera's VIEW_MATRIX is applied).
    // Returns the index of the nearest hit part, or -1 for no hit.
    int PickPart(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const;

    void SetHighlightedPart(int partId) { highlightedPart = partId; }
    int  GetHighlightedPart() const     { return highlightedPart; }

    // Raw pointers into this Fan's own Transform, valid after Render() has
    // run at least once this frame - Scene3D::PickAt() unprojects against
    // these so the picking ray uses exactly the matrices the fan was drawn
    // with.
    glm::mat4* GetViewMatrixPtr()       { return transform.TransformGetViewMatrix(); }
    glm::mat4* GetProjectionMatrixPtr() { return transform.TransformGetProjectionMatrix(); }

private:
    void drawPart(int partId, const glm::vec3& baseColor);

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLuint ibo     = 0;

    GLint uMVP          = -1;
    GLint uModel        = -1;
    GLint uNormalMatrix = -1;
    GLint uPartColor    = -1;
    GLint uLightPos     = -1;
    GLint uLightColor   = -1;
    GLint uViewPos      = -1;

    Transform transform;

    float spinAngle      = 0.0f;
    float speed          = 8.0f;   // 0..20, set by Scene3D each frame
    float cameraDistance = SceneCamera::kDefaultDistance;

    // Independent turntable spin for the whole assembly - not driven by the
    // HUD speed slider, always advances.
    float modelSpinAngle = 0.0f;
    static constexpr float kModelSpinDegPerFrame = 0.25f;

    glm::vec3 lightPos   { 6.0f, 10.0f, 4.0f };
    glm::vec3 lightColor { 1.0f, 1.0f, 1.0f };
    glm::vec3 eyeWorldPos{ 0.0f, 0.0f, 0.0f };

    glm::mat4 worldMatrix[PART_COUNT];
    int highlightedPart = -1;

    // Degrees of spin per frame per unit of HUD speed - tuned so the old
    // Quiz 2 default (kBaseSpeed = 1.5 deg/frame) lands near the middle of
    // the new 0..20 HUD range.
    static constexpr float kSpeedToAngle = 0.1875f;
};
