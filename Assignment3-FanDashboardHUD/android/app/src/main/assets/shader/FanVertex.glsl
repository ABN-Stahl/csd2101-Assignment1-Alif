#version 300 es

// FanVertex.glsl
// What this does: transforms each cube vertex by MODELVIEWPROJECTIONMATRIX
// for gl_Position, and separately computes the world-space position/normal
// the fragment stage needs for per-fragment Phong lighting.
//
// Edits for Quiz 3: replaced the per-corner Shade attribute with a Normal
// attribute; the normal is transformed by NORMALMATRIX (inverse-transpose of
// MODEL's upper 3x3, computed on the CPU in Fan::drawPart) rather than
// passed straight through, so lighting stays correct once the model matrix
// rotates a part with non-uniform scale (every stretched "bone" in this
// fan's hierarchy has one).

layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

out vec3 WorldPos;
out vec3 WorldNormal;

uniform mat4 MODELVIEWPROJECTIONMATRIX;
uniform mat4 MODEL;
uniform mat3 NORMALMATRIX;

void main()
{
    vec4 worldPos = MODEL * VertexPosition;
    WorldPos      = worldPos.xyz;
    WorldNormal   = normalize(NORMALMATRIX * VertexNormal);

    gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
}
