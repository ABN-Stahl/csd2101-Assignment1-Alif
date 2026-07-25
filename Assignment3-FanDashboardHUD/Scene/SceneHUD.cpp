/**
 * SceneHUD.cpp
 *
 * Author: Alif Bin Norazhar (alif.binnorazhar@digipen.edu)
 * Date:   2026-07-21
 *
 * What this does: builds and hit-tests the four dashboard buttons and the
 * 20-block speed bar entirely in screen space (pixels, top-left origin, Y
 * growing downward - matching mouse/touch coordinates directly, so hit
 * testing never needs unprojection). All geometry is one dynamic
 * position+colour VBO, rebuilt on every RebuildGeometry() call (Resize(),
 * or whenever a button press changes speed/zoom) since <= 24 quads is too
 * small to bother dirty-tracking.
 *
 * Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.
 */

#define LOG_TAG "SceneHUD"
#include "SceneHUD.h"
#include "SceneCamera.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD(AAssetManager* assetMgr) : mgr(assetMgr)
{
#else
SceneHUD::SceneHUD()
{
#endif
}

SceneHUD::~SceneHUD()
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
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
}

void SceneHUD::InitModel()
{
    LOGI("SceneHUD::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(mgr, "shader/HUDVertex.glsl", "shader/HUDFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile("HUDVertex.glsl", "HUDFragment.glsl");
#endif

    if (!program)
    {
        LOGE("SceneHUD: failed to build shader program");
        return;
    }

    uProjection = glGetUniformLocation(program, "PROJECTION");

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const GLsizei stride = 5 * sizeof(GLfloat); // vec2 position + vec3 color
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    transform.TransformInit();

    LOGI("SceneHUD::InitModel done (VAO=%u, VBO=%u)", vao, vbo);
}

void SceneHUD::Resize(int w, int h)
{
    screenWidth  = w;
    screenHeight = h;

    const float margin = 24.0f;
    const float btnW   = 120.0f;
    const float btnH   = 48.0f;
    const float gapY   = 16.0f;
    const float topY   = 56.0f;

    float leftX  = margin;
    float rightX = static_cast<float>(w) - margin - btnW;

    buttons[BTN_SPD_UP]   = { leftX,  topY,             btnW, btnH };
    buttons[BTN_SPD_DOWN] = { leftX,  topY + btnH+gapY, btnW, btnH };
    buttons[BTN_ZOOM_IN]  = { rightX, topY,             btnW, btnH };
    buttons[BTN_ZOOM_OUT] = { rightX, topY + btnH+gapY, btnW, btnH };

    float barTotalW = kSpeedSteps * (kBlockW + kBlockGap) - kBlockGap;
    barOrigin.x = (static_cast<float>(w) - barTotalW) * 0.5f;
    barOrigin.y = static_cast<float>(h) - 60.0f;
    barOrigin.w = kBlockW;
    barOrigin.h = kBlockH;

    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformOrtho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f, -1.0f, 1.0f);

    RebuildGeometry();
}

bool SceneHUD::PointInRect(float x, float y, const Rect& r)
{
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

void SceneHUD::AppendQuad(std::vector<GLfloat>& verts, const Rect& r, const glm::vec3& color)
{
    const float x0 = r.x, y0 = r.y, x1 = r.x + r.w, y1 = r.y + r.h;
    const float positions[6][2] = {
        { x0, y0 }, { x1, y0 }, { x1, y1 },
        { x0, y0 }, { x1, y1 }, { x0, y1 },
    };

    for (const auto& p : positions)
    {
        verts.push_back(p[0]);
        verts.push_back(p[1]);
        verts.push_back(color.r);
        verts.push_back(color.g);
        verts.push_back(color.b);
    }
}

void SceneHUD::RebuildGeometry()
{
    static const glm::vec3 kButtonColors[BTN_COUNT] = {
        { 0.25f, 0.75f, 0.35f }, // +SPD
        { 0.65f, 0.20f, 0.20f }, // -SPD
        { 0.25f, 0.45f, 0.85f }, // +ZOOM
        { 0.60f, 0.45f, 0.15f }, // -ZOOM
    };
    static const glm::vec3 kBlockLit    = { 0.25f, 0.80f, 0.30f };
    static const glm::vec3 kBlockUnlit  = { 0.35f, 0.35f, 0.38f };

    std::vector<GLfloat> verts;
    verts.reserve((BTN_COUNT + kSpeedSteps) * 6 * 5);

    for (int i = 0; i < BTN_COUNT; ++i)
    {
        glm::vec3 color = kButtonColors[i];
        if (i == activeButton)
        {
            color = glm::min(color + glm::vec3(0.2f), glm::vec3(1.0f));
        }
        AppendQuad(verts, buttons[i], color);
    }

    int litBlocks = static_cast<int>(speed + 0.5f);
    for (int i = 0; i < kSpeedSteps; ++i)
    {
        Rect block;
        block.x = barOrigin.x + i * (kBlockW + kBlockGap);
        block.y = barOrigin.y;
        block.w = kBlockW;
        block.h = kBlockH;
        AppendQuad(verts, block, (i < litBlocks) ? kBlockLit : kBlockUnlit);
    }

    vertexCount = static_cast<GLsizei>(verts.size() / 5);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(GLfloat)),
                 verts.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool SceneHUD::TouchDown(float x, float y)
{
    for (int i = 0; i < BTN_COUNT; ++i)
    {
        if (!PointInRect(x, y, buttons[i]))
        {
            continue;
        }

        activeButton = i;
        switch (i)
        {
            case BTN_SPD_UP:
                speed = std::min(20.0f, speed + 1.0f);
                LOGI("+SPD pressed (speed=%.0f)", speed);
                break;
            case BTN_SPD_DOWN:
                speed = std::max(0.0f, speed - 1.0f);
                LOGI("-SPD pressed (speed=%.0f)", speed);
                break;
            case BTN_ZOOM_IN:
                zoom = std::max(SceneCamera::kMinDistance, zoom - 1.0f);
                LOGI("+ZOOM pressed (zoom=%.0f)", zoom);
                break;
            case BTN_ZOOM_OUT:
                zoom = std::min(SceneCamera::kMaxDistance, zoom + 1.0f);
                LOGI("-ZOOM pressed (zoom=%.0f)", zoom);
                break;
        }

        RebuildGeometry();
        return true;
    }
    return false;
}

void SceneHUD::TouchRelease()
{
    if (activeButton != -1)
    {
        activeButton = -1;
        RebuildGeometry();
    }
}

void SceneHUD::Render()
{
    if (!program || !vao || vertexCount == 0)
    {
        return;
    }

    glUseProgram(program);
    glBindVertexArray(vao);
    glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(*transform.TransformGetProjectionMatrix()));
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}
