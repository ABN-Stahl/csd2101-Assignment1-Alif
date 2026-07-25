#version 300 es

// GroundVertex.glsl
// What this does: same pattern as FanVertex.glsl - passes world-space
// position/normal to the fragment stage for Phong, plus gl_Position for
// rasterization. WorldPos.xz is what GroundFragment.glsl's checker()
// function reads to decide each fragment's tile.
//
// Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.

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
