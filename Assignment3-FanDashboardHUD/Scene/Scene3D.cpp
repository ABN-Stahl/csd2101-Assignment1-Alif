/**
 * Scene3D.cpp
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this does: constructs/owns the Fan + Ground, forwards InitModel/
 * Render/Resize to both, pushes the shared camera distance + light into each
 * before drawing them, and implements PickAt() (Part 8): unproject the click
 * at the near/far planes into a world-space ray using the exact view/
 * projection matrices the Fan was just drawn with, then ask the Fan for the
 * nearest part that ray hits.
 *
 * Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.
 */

#define LOG_TAG "Scene3D"
#include "Scene3D.h"
#include "Fan.h"
#include "Ground.h"
#include "SceneCamera.h"

#ifdef PLATFORM_ANDROID
Scene3D::Scene3D(AAssetManager* assetMgr)
{
    fan    = new Fan(assetMgr);
    ground = new Ground(assetMgr);
}
#else
Scene3D::Scene3D()
{
    fan    = new Fan();
    ground = new Ground();
}
#endif

Scene3D::~Scene3D()
{
    delete fan;
    delete ground;
}

void Scene3D::InitModel()
{
    LOGI("Scene3D::InitModel");
    pickTransform.TransformInit();
    fan->InitModel();
    ground->InitModel();
}

void Scene3D::Resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;
    fan->Resize(w, h);
    ground->Resize(w, h);
}

void Scene3D::SetCameraDistance(float distance)
{
    cameraDistance = glm::clamp(distance, SceneCamera::kMinDistance, SceneCamera::kMaxDistance);
}

void Scene3D::SetFanSpeed(float unitsPerFrame)
{
    fan->SetSpeed(unitsPerFrame);
}

void Scene3D::Render()
{
    fan->SetCameraDistance(cameraDistance);
    ground->SetCameraDistance(cameraDistance);
    fan->SetLight(lightPos, lightColor);
    ground->SetLight(lightPos, lightColor);

    ground->Render();
    fan->Render();
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    if (screenWidth <= 0 || screenHeight <= 0)
    {
        return false;
    }

    // TransformUnproject follows the standard GL viewport convention (origin
    // at the bottom-left, Y growing up), but screenX/screenY arrive in the
    // top-left-origin, Y-down convention every mouse/touch API on this
    // project uses - flip Y here or picking is vertically mirrored.
    float glY = static_cast<float>(screenHeight) - screenY;

    int viewport[4] = { 0, 0, screenWidth, screenHeight };
    glm::mat4* view = fan->GetViewMatrixPtr();
    glm::mat4* proj = fan->GetProjectionMatrixPtr();

    float nx, ny, nz, fx, fy, fz;
    pickTransform.TransformUnproject(screenX, glY, 0.0f, view, proj, viewport, &nx, &ny, &nz);
    pickTransform.TransformUnproject(screenX, glY, 1.0f, view, proj, viewport, &fx, &fy, &fz);

    glm::vec3 nearPoint(nx, ny, nz);
    glm::vec3 farPoint(fx, fy, fz);
    glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

    int hit     = fan->PickPart(nearPoint, rayDir);
    int current = fan->GetHighlightedPart();
    int next    = (hit == current) ? -1 : hit;

    fan->SetHighlightedPart(next);
    LOGI("Scene3D::PickAt (%.1f,%.1f) -> %s", screenX, screenY, Fan::PartName(next));

    return hit != -1;
}
