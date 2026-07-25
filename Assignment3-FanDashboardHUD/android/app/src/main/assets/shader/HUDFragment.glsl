#version 300 es

// HUDFragment.glsl
// What this does: outputs the per-vertex flat colour unchanged - no
// lighting, no texture, no font rendering, per the HUD spec.
//
// Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.

precision mediump float;

in vec3 Color;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(Color, 1.0);
}
