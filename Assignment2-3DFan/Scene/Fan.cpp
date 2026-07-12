/**
 * Fan.cpp
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-12
 *
 * What this does: implements the Fan model declared in Fan.h - uploads the
 * one shared cube (VBO with position + shade sub-regions, IBO, single VAO),
 * draws it seven times with a different Transform push/pop block and
 * PARTCOLOR per part, advances spinAngle, and handles tap/swipe gestures
 * (velocity-based drag boost, fling-with-decay on release).
 *
 * Edits: new file written for Assignment 2. Cube geometry/VAO setup pattern
 * reused from the Chapter3 Attribute_Layouts / VertexArrayObject tutorials;
 * the fan hierarchy transform values were tuned twice after the first build
 * (pole length/position and camera distance) to fix visible clipping.
 */

#define LOG_TAG "Fan"
#include "Fan.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

// One shared 8-vertex cube used for every part (base, pole, hub, 4 blades).
// Shade is a per-corner grayscale factor: front corners (z = +1) are bright,
// back corners (z = -1) are dim, giving the cube a cheap "lit" look without
// a lighting pass. Actual part color comes from the PARTCOLOR uniform.
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

static const GLfloat kShades[8] = {
    1.0f, 1.0f, 1.0f, 1.0f,   // V0-V3: front
    0.6f, 0.6f, 0.6f, 0.6f,   // V4-V7: back
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
static constexpr GLuint ATTRIB_SHADE    = 1;
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

    uMVP       = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    uPartColor = glGetUniformLocation(program, "PARTCOLOR");

    const GLsizeiptr posSize   = sizeof(kPositions);
    const GLsizeiptr shadeSize = sizeof(kShades);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + shadeSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize,   kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, shadeSize, kShades);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_SHADE);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIB_SHADE,    1, GL_FLOAT, GL_FALSE, 0, (void*)posSize);

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
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0.0f);
}

void Fan::drawPart(const glm::vec3& color)
{
    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniform3fv(uPartColor, 1, glm::value_ptr(color));
    glDrawElements(GL_TRIANGLES, INDEX_COUNT, GL_UNSIGNED_SHORT, (void*)0);
}

void Fan::Render()
{
    if (!program || !vao)
    {
        return;
    }

    if (fanOn)
    {
        spinAngle += kBaseSpeed + dragBoost;
    }
    if (spinAngle >= 360.0f)
    {
        spinAngle -= 360.0f;
    }

    // Bonus (fling with decay): once the finger lifts, dragBoost is not reset
    // to zero - it decays a little every frame instead, so the fan winds down.
    if (!dragging)
    {
        dragBoost *= kBoostDecay;
        if (dragBoost < 0.01f)
        {
            dragBoost = 0.0f;
        }
    }

    glUseProgram(program);
    glBindVertexArray(vao);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformTranslate(0.0f, 0.3f, -11.0f);
    transform.TransformRotate(glm::radians(20.0f), 0.0f, 1.0f, 0.0f);

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -2.6f, 0.0f);
    transform.TransformScale(1.6f, 0.25f, 0.8f);
    drawPart(kBaseColor);
    transform.TransformPopMatrix();

    // Pole spans from just inside the base top (-2.35) to just inside the
    // hub bottom (1.0), so it neither pokes through the base nor sticks up
    // past the hub/blades.
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, -0.625f, 0.0f);
    transform.TransformScale(0.15f, 1.775f, 0.15f);
    drawPart(kPoleColor);
    transform.TransformPopMatrix();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.0f, 1.3f, 0.0f);
    transform.TransformScale(0.3f, 0.3f, 0.3f);
    drawPart(kHubColor);
    transform.TransformPopMatrix();

    for (int i = 0; i < 4; ++i)
    {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 1.3f, 0.15f);
        transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.55f, 0.0f);
        transform.TransformScale(0.22f, 0.8f, 0.05f);
        drawPart(kBladeColors[i]);
        transform.TransformPopMatrix();
    }

    glBindVertexArray(0);
}

void Fan::TouchEventDown(float x, float y)
{
    lastX         = x;
    lastY         = y;
    movedDistance = 0.0f;
    dragging      = true;
    lastMoveTime  = std::chrono::steady_clock::now();
}

void Fan::TouchEventMove(float x, float y)
{
    float dx   = x - lastX;
    float dy   = y - lastY;
    float dist = std::sqrt(dx * dx + dy * dy);
    movedDistance += dist;

    auto  now  = std::chrono::steady_clock::now();
    float dtMs = std::chrono::duration<float, std::milli>(now - lastMoveTime).count();
    lastMoveTime = now;

    if (dtMs > 0.0f)
    {
        float velocity = dist / dtMs;
        dragBoost = std::min(velocity * kBoostScale, kMaxBoost);
        LOGI("Fan: drag velocity=%.3f px/ms -> boost=%.2f", velocity, dragBoost);
    }

    lastX = x;
    lastY = y;
}

void Fan::TouchEventRelease(float /*x*/, float /*y*/)
{
    dragging = false;

    if (movedDistance < kTapThreshold)
    {
        fanOn = !fanOn;
        LOGI("Fan %s", fanOn ? "ON" : "OFF");
    }
}
