#pragma once

/**
 * SceneCamera.h
 *
 * What this owns: the one camera formula shared by every Scene3D occupant
 * (Fan, Ground) and by Scene3D::PickAt(). Quiz 2 baked
 * Translate(0,0.8,-8)->Rotate(20,Y) straight into Fan::Render()'s
 * MODEL_MATRIX; Quiz 3 needs that same transform applied identically to two
 * separate Model objects (so the ground and the fan agree on where the
 * camera is) and reproduced exactly by Scene3D's picking ray, so it is
 * pulled out here instead of copy-pasted three times.
 *
 * Transform::TransformSetView() (Transform.cpp) is an empty stub ("Reserved
 * for future use"), so a precomputed view matrix cannot be injected into a
 * Model's own Transform instance that way - each Model calls ApplyView()
 * itself, on its own VIEW_MATRIX stack, every frame.
 *
 * Edits: new file written for Assignment 3, no Quiz 2 equivalent existed.
 */

#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SceneCamera
{
    constexpr float kFovYDegrees = 60.0f;
    constexpr float kNearPlane   = 0.01f;
    constexpr float kFarPlane    = 1000.0f;

    // Clamp range for Scene3D's adjustable camera distance (Part 7).
    constexpr float kMinDistance = 4.0f;
    constexpr float kMaxDistance = 20.0f;
    constexpr float kDefaultDistance = 11.0f; // matches Quiz 2's tuned -8 z-offset... see Render()

    // Builds the shared view matrix (camera translate + tilt) on whichever
    // Transform is passed in. Caller must have already selected VIEW_MATRIX
    // mode is done here for them.
    inline void ApplyView(Transform& transform, float cameraDistance)
    {
        transform.TransformSetMatrixMode(VIEW_MATRIX);
        transform.TransformLoadIdentity();
        transform.TransformTranslate(0.0f, 0.3f, -cameraDistance);
        transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);
    }

    // World-space eye position, derived from the view matrix just built by
    // ApplyView() - needed for the Phong specular term (Part 2).
    inline glm::vec3 EyeWorldPos(Transform& transform)
    {
        glm::mat4 invView = glm::inverse(*transform.TransformGetViewMatrix());
        return glm::vec3(invView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}
