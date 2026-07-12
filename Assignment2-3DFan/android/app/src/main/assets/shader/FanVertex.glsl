#version 300 es

// FanVertex.glsl
// What this does: transforms each cube vertex by MODELVIEWPROJECTIONMATRIX
// and passes the per-corner grayscale Shade through to the fragment stage.
// Edits: new file written for Assignment 2 - attribute-layout style follows
// the Chapter3 VAOVertex.glsl pattern, but Shade + PARTCOLOR tinting (see
// FanFragment.glsl) replaces that tutorial's baked-in per-vertex color.

layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in float VertexShade;

out float Shade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

void main()
{
    gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
    Shade = VertexShade;
}
