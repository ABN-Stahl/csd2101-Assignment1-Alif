#pragma once

/**
 * Ground.h
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this owns: a single large quad positioned under the fan. The
 * checkerboard pattern has no CPU-side geometry or texture behind it at all
 * - it is computed per-fragment from the fragment's world-space XZ position
 * (see GroundFragment.glsl), then lit with the same Phong formula as Fan.
 *
 * Shares Scene3D's camera (SceneCamera::ApplyView) and light, set each frame
 * by Scene3D before Render() is called, exactly like Fan.
 *
 * Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.
 */

#include "Model.h"
#include "Platform.h"
#include "SceneCamera.h"
#include "Transform.h"
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model
{
public:
#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* assetMgr);
#else
    Ground();
#endif
    ~Ground() override;

    void InitModel()          override;
    void Render()             override;
    void Resize(int w, int h) override;

    void SetCameraDistance(float distance);
    void SetLight(const glm::vec3& worldPos, const glm::vec3& color);

private:
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
    GLint uLightPos     = -1;
    GLint uLightColor   = -1;
    GLint uViewPos      = -1;

    Transform transform;

    float cameraDistance = SceneCamera::kDefaultDistance;
    glm::vec3 lightPos   { 6.0f, 10.0f, 4.0f };
    glm::vec3 lightColor { 1.0f, 1.0f, 1.0f };

    // Sits just under the fan base (base bottom is at y = -2.6 - 0.25 = -2.85).
    static constexpr float kGroundY   = -2.85f;
    static constexpr float kHalfSize  = 60.0f;
};
