#version 300 es

// FanFragment.glsl
// What this does: outputs PARTCOLOR tinted by the interpolated Shade value,
// so the one shared cube reads as a different-colored, lit-looking part
// depending on which uniform was uploaded before the draw call.
// Edits: new file written for Assignment 2, no tutorial equivalent existed.

precision mediump float;

in float Shade;
layout(location = 0) out vec4 outColor;

uniform vec3 PARTCOLOR;

void main()
{
    outColor = vec4(PARTCOLOR * Shade, 1.0);
}
