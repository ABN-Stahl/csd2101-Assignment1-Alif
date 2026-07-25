/**
 * Ground.cpp
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this does: uploads one flat quad (4 verts, 2 triangles) spanning
 * [-kHalfSize, kHalfSize] on X/Z at y = kGroundY, and draws it once per
 * frame through the same view/projection formula Fan uses (SceneCamera).
 * The checkerboard itself lives entirely in GroundFragment.glsl - this file
 * never touches per-tile colour.
 *
 * Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.
 */

#define LOG_TAG "Ground"
#include "Ground.h"
#include "SceneCamera.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace
{
    constexpr GLuint ATTRIB_POSITION = 0;
    constexpr GLuint ATTRIB_NORMAL   = 1;
}

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager* assetMgr) : mgr(assetMgr)
{
#else
Ground::Ground()
{
#endif
}

Ground::~Ground()
{
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (ibo)
    {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
}

void Ground::InitModel()
{
    LOGI("Ground::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(mgr, "shader/GroundVertex.glsl", "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("GroundVertex.glsl", "GroundFragment.glsl");
#endif

    if (!program)
    {
        LOGE("Ground: failed to build shader program");
        return;
    }

    uMVP          = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uModel        = glGetUniformLocation(program, "MODEL");
    uNormalMatrix = glGetUniformLocation(program, "NORMALMATRIX");
    uLightPos     = glGetUniformLocation(program, "LIGHTPOS");
    uLightColor   = glGetUniformLocation(program, "LIGHTCOLOR");
    uViewPos      = glGetUniformLocation(program, "VIEWPOS");

    const GLfloat positions[4][3] = {
        { -kHalfSize, kGroundY, -kHalfSize },  // V0 - back-left
        { -kHalfSize, kGroundY,  kHalfSize },  // V1 - front-left
        {  kHalfSize, kGroundY,  kHalfSize },  // V2 - front-right
        {  kHalfSize, kGroundY, -kHalfSize },  // V3 - back-right
    };
    const GLfloat normals[4][3] = {
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
    };
    // Wound CCW as seen from above (+Y), matching glFrontFace(GL_CCW) set by
    // Transform::TransformInit() - the original {0,2,1, 0,3,2} order actually
    // faced -Y (the underside), which got backface-culled from a camera
    // sitting above the floor and made the ground invisible.
    const GLushort indices[6] = { 0, 1, 2, 0, 2, 3 };

    const GLsizeiptr posSize    = sizeof(positions);
    const GLsizeiptr normalSize = sizeof(normals);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + normalSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize,    positions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, normalSize, normals);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIB_NORMAL,   3, GL_FLOAT, GL_FALSE, 0, (void*)posSize);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    transform.TransformInit();

    LOGI("Ground::InitModel done (VAO=%u, VBO=%u, IBO=%u)", vao, vbo, ibo);
}

void Ground::Resize(int w, int h)
{
    float aspect = (h > 0) ? static_cast<float>(w) / static_cast<float>(h) : 1.0f;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformSetPerspective(glm::radians(SceneCamera::kFovYDegrees), aspect,
                                       SceneCamera::kNearPlane, SceneCamera::kFarPlane, 0.0f);
}

void Ground::SetCameraDistance(float distance)
{
    cameraDistance = distance;
}

void Ground::SetLight(const glm::vec3& worldPos, const glm::vec3& color)
{
    lightPos   = worldPos;
    lightColor = color;
}

void Ground::Render()
{
    if (!program || !vao)
    {
        return;
    }

    glUseProgram(program);
    glBindVertexArray(vao);

    SceneCamera::ApplyView(transform, cameraDistance);
    glm::vec3 eyeWorldPos = SceneCamera::EyeWorldPos(transform);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    glm::mat4  world        = *transform.TransformGetModelMatrix();
    glm::mat3  normalMatrix = glm::inverseTranspose(glm::mat3(world));
    glm::mat4* mvp          = transform.TransformGetModelViewProjectionMatrix();

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(uLightPos,   1, glm::value_ptr(lightPos));
    glUniform3fv(uLightColor, 1, glm::value_ptr(lightColor));
    glUniform3fv(uViewPos,    1, glm::value_ptr(eyeWorldPos));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);

    glBindVertexArray(0);
}
