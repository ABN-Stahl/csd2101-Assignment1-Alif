#define LOG_TAG "Square"

/*!***************************************************************************
 * \file   Square.cpp
 * \author Alif Bin Norazhar(alif.binnorazhar@digipen.edu)
 * \par    CSD2101
 * \date   2026
 * \brief  Concrete Model subclass that renders a solid yellow square, offset
 *         to the upper-right of clip space so it does not overlap the
 *         triangle.  Shader source is loaded from .glsl asset files via
 *         ShaderHelper.  Owns its own VAO, VBO, and shader program.
 *****************************************************************************/

#include "Square.h"
#include "ShaderHelper.h"

// ---------------------------------------------------------------------------
// Vertex data — two triangles forming a square, offset to upper-right
// ---------------------------------------------------------------------------

static const GLfloat kPositions[] = {
     0.45f, 0.45f,
     0.85f, 0.45f,
     0.85f, 0.85f,

     0.45f, 0.45f,
     0.85f, 0.85f,
     0.45f, 0.85f
};

// ---------------------------------------------------------------------------
// Square
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Square::Square(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Square::Square() = default;
#endif

Square::~Square()
{
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

void Square::InitModel()
{
    LOGI("Square::InitModel");

#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr,
                    "shader/YellowSquareVertex.glsl",
                    "shader/YellowSquareFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile(
                    "YellowSquareVertex.glsl",
                    "YellowSquareFragment.glsl");
#endif
    if (!programID) { LOGE("Square: could not create program"); return; }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Square::InitModel done");
}

void Square::Render()
{
    glUseProgram(programID);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Square::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
}