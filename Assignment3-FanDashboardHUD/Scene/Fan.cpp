/**
 * Fan.cpp
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this does: implements the Fan model declared in Fan.h - uploads the
 * one shared cube (VBO with position + normal sub-regions, IBO, single VAO),
 * draws it seven times with a different Transform push/pop block and
 * PARTCOLOR per part, advances spinAngle from the HUD-driven speed, and
 * ray-tests its own cached per-part world matrices for Scene3D's picking.
 *
 * Edits for Quiz 3: replaced the per-corner Shade attribute with a per-corner
 * Normal attribute + real Phong lighting (Part 2); moved the camera
 * translate/rotate off MODEL_MATRIX and onto its own VIEW_MATRIX stack via
 * SceneCamera::ApplyView(), driven by an adjustable cameraDistance (Part 7);
 * removed Quiz 2's tap/drag TouchEvent* handlers and dragBoost/kTapThreshold
 * fields entirely; added per-part world matrix caching + ray-vs-OBB picking
 * (Part 8). Cube geometry/VAO setup pattern and the base/pole/hub/blade
 * hierarchy values are otherwise unchanged from Quiz 2.
 */

#define LOG_TAG "Fan"
#include "Fan.h"
#include "SceneCamera.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

// One shared 8-vertex cube used for every part (base, pole, hub, 4 blades).
// Normal is the normalized corner position - the cube's 8 shared corners
// keep one normal each instead of duplicating corners per face, giving a
// softer, rounded-cube lit look (one of the two options Part 2 allows).
// Actual part color comes from the PARTCOLOR uniform.
static const GLfloat kPositions[8][3] = {
    {-1.0f, -1.0f,  1.0f},  // V0
    {-1.0f,  1.0f,  1.0f},  // V1
    { 1.0f,  1.0f,  1.0f},  // V2
    { 1.0f, -1.0f,  1.0f},  // V3
    {-1.0f, -1.0f, -1.0f},  // V4
    {-1.0f,  1.0f, -1.0f},  // V5
    { 1.0f,  1.0f, -1.0f},  // V6
    { 1.0f, -1.0f, -1.0f},  // V7
};

static const GLfloat kNormals[8][3] = {
    {-0.57735f, -0.57735f,  0.57735f},  // V0
    {-0.57735f,  0.57735f,  0.57735f},  // V1
    { 0.57735f,  0.57735f,  0.57735f},  // V2
    { 0.57735f, -0.57735f,  0.57735f},  // V3
    {-0.57735f, -0.57735f, -0.57735f},  // V4
    {-0.57735f,  0.57735f, -0.57735f},  // V5
    { 0.57735f,  0.57735f, -0.57735f},  // V6
    { 0.57735f, -0.57735f, -0.57735f},  // V7
};

static const GLushort kIndices[36] = {
    0,3,1, 3,2,1,   // front
    7,4,6, 4,5,6,   // back
    4,0,5, 0,1,5,   // left
    3,7,2, 7,6,2,   // right
    1,2,5, 2,6,5,   // top
    3,0,7, 0,4,7    // bottom
};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_NORMAL   = 1;
static constexpr GLsizei INDEX_COUNT    = 36;

static const glm::vec3 kBaseColor(0.45f, 0.28f, 0.12f);
static const glm::vec3 kPoleColor(0.55f, 0.55f, 0.58f);
static const glm::vec3 kHubColor(0.20f, 0.20f, 0.22f);
static const glm::vec3 kBladeColors[4] = {
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.5f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
};
static const glm::vec3 kHighlightColor(1.0f, 0.75f, 0.0f); // amber, Part 8

const char* Fan::PartName(int partId)
{
    static char buf[16];

    if (partId < 0 || partId >= PART_COUNT)
    {
        return "None";
    }
    if (partId == Fan::PART_BASE) return "Base";
    if (partId == Fan::PART_POLE) return "Pole";
    if (partId == Fan::PART_HUB)  return "Hub";

    std::snprintf(buf, sizeof(buf), "Blade%d", partId - Fan::PART_BLADE_FIRST);
    return buf;
}

// Ray-vs-oriented-box test: transforms the world-space ray into the part's
// local unit-cube space (via the inverse of its world matrix) and runs a
// standard slab test against [-1,1]^3. The exact geometry is already known
// (every part is the same unit cube, just scaled/rotated/translated), so an
// exact OBB test is used here rather than the sphere/segment approximation
// the Scene Graph PickAt() examples fall back to for thin parts.
static bool RayIntersectsOBB(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                              const glm::mat4& worldMatrix, float& outT)
{
    glm::mat4 invWorld  = glm::inverse(worldMatrix);
    glm::vec3 localOrig = glm::vec3(invWorld * glm::vec4(rayOrigin, 1.0f));
    glm::vec3 localDir  = glm::vec3(invWorld * glm::vec4(rayDir, 0.0f));

    float tmin = -std::numeric_limits<float>::max();
    float tmax =  std::numeric_limits<float>::max();

    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::fabs(localDir[axis]) < 1e-6f)
        {
            if (localOrig[axis] < -1.0f || localOrig[axis] > 1.0f)
            {
                return false;
            }
            continue;
        }

        float t1 = (-1.0f - localOrig[axis]) / localDir[axis];
        float t2 = ( 1.0f - localOrig[axis]) / localDir[axis];
        if (t1 > t2)
        {
            std::swap(t1, t2);
        }
        tmin = std::max(tmin, t1);
        tmax = std::min(tmax, t2);
        if (tmin > tmax)
        {
            return false;
        }
    }

    if (tmax < 0.0f)
    {
        return false; // box is entirely behind the ray origin
    }
    outT = (tmin >= 0.0f) ? tmin : tmax;
    return true;
}

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* assetMgr) : mgr(assetMgr)
{
#else
Fan::Fan()
{
#endif
    modelType = FanType;
}

Fan::~Fan()
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

void Fan::InitModel()
{
    LOGI("Fan::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(mgr, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("FanVertex.glsl", "FanFragment.glsl");
#endif

    if (!program)
    {
        LOGE("Fan: failed to build shader program");
        return;
    }

    uMVP          = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uModel        = glGetUniformLocation(program, "MODEL");
    uNormalMatrix = glGetUniformLocation(program, "NORMALMATRIX");
    uPartColor    = glGetUniformLocation(program, "PARTCOLOR");
    uLightPos     = glGetUniformLocation(program, "LIGHTPOS");
    uLightColor   = glGetUniformLocation(program, "LIGHTCOLOR");
    uViewPos      = glGetUniformLocation(program, "VIEWPOS");

    const GLsizeiptr posSize    = sizeof(kPositions);
    const GLsizeiptr normalSize = sizeof(kNormals);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + normalSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize,    kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, normalSize, kNormals);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

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

    LOGI("Fan::InitModel done (VAO=%u, VBO=%u, IBO=%u)", vao, vbo, ibo);
}

void Fan::Resize(int w, int h)
{
    float aspect = (h > 0) ? static_cast<float>(w) / static_cast<float>(h) : 1.0f;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformSetPerspective(glm::radians(SceneCamera::kFovYDegrees), aspect,
                                       SceneCamera::kNearPlane, SceneCamera::kFarPlane, 0.0f);
}

void Fan::SetSpeed(float unitsPerFrame)
{
    speed = unitsPerFrame;
}

void Fan::SetCameraDistance(float distance)
{
    cameraDistance = distance;
}

void Fan::SetLight(const glm::vec3& worldPos, const glm::vec3& color)
{
    lightPos   = worldPos;
    lightColor = color;
}

void Fan::drawPart(int partId, const glm::vec3& baseColor)
{
    worldMatrix[partId] = *transform.TransformGetModelMatrix();

    glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(worldMatrix[partId]));
    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();

    const glm::vec3& color = (partId == highlightedPart) ? kHighlightColor : baseColor;

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(worldMatrix[partId]));
    glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(uPartColor, 1, glm::value_ptr(color));
    glDrawElements(GL_TRIANGLES, INDEX_COUNT, GL_UNSIGNED_SHORT, (void*)0);
}

void Fan::Render()
{
    if (!program || !vao)
    {
        return;
    }

    spinAngle += speed * kSpeedToAngle;
    if (spinAngle >= 360.0f)
    {
        spinAngle -= 360.0f;
    }

    modelSpinAngle += kModelSpinDegPerFrame;
    if (modelSpinAngle >= 360.0f)
    {
        modelSpinAngle -= 360.0f;
    }

    glUseProgram(program);
    glBindVertexArray(vao);

    SceneCamera::ApplyView(transform, cameraDistance);
    eyeWorldPos = SceneCamera::EyeWorldPos(transform);

    glUniform3fv(uLightPos,   1, glm::value_ptr(lightPos));
    glUniform3fv(uLightColor, 1, glm::value_ptr(lightColor));
    glUniform3fv(uViewPos,    1, glm::value_ptr(eyeWorldPos));

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // Independent turntable spin for the whole assembly - applied before any
    // part is pushed, so every part below inherits it as part of its own
    // model matrix, on top of (not instead of) its own local placement.
    transform.TransformRotate(glm::radians(modelSpinAngle), 0.0f, 1.0f, 0.0f);

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(PART_BASE, kBaseColor);
    transform.TransformPopMatrix();

    // Pole spans from just inside the base top (-2.35) to just inside the
    // hub bottom (1.0), so it neither pokes through the base nor sticks up
    // past the hub/blades.
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -0.625f, 0.0f);
    transform.TransformScale(0.15f, 1.775f, 0.15f);
    drawPart(PART_POLE, kPoleColor);
    transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 1.3f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    drawPart(PART_HUB, kHubColor);
    transform.TransformPopMatrix();

    const float kBladeStepDeg = 360.0f / static_cast<float>(kBladeCount);
    for (int i = 0; i < kBladeCount; ++i)
    {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 1.3f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + i * kBladeStepDeg), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.55f, 0.0f);
        transform.TransformScale(0.09f, 0.85f, 0.05f);
        drawPart(PART_BLADE_FIRST + i, kBladeColors[i % 4]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

int Fan::PickPart(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const
{
    int   best   = -1;
    float bestT  = std::numeric_limits<float>::max();

    for (int i = 0; i < PART_COUNT; ++i)
    {
        float t;
        if (RayIntersectsOBB(rayOrigin, rayDir, worldMatrix[i], t) && t < bestT)
        {
            bestT = t;
            best  = i;
        }
    }
    return best;
}
