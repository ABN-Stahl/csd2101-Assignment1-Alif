#version 300 es

// HUDVertex.glsl
// What this does: maps pixel-space vertices straight to clip space through
// an orthographic PROJECTION matrix (built with TransformOrtho(0, w, h, 0,
// -1, 1) in SceneHUD::Resize) - no model or view matrix, since HUD widgets
// never move relative to the screen.
//
// Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.

layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec3 VertexColor;

out vec3 Color;

uniform mat4 PROJECTION;

void main()
{
    gl_Position = PROJECTION * vec4(VertexPosition, 0.0, 1.0);
    Color = VertexColor;
}
