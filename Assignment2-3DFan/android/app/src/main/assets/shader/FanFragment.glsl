#version 300 es

precision mediump float;

in float Shade;
layout(location = 0) out vec4 outColor;

uniform vec3 PARTCOLOR;

void main()
{
    outColor = vec4(PARTCOLOR * Shade, 1.0);
}
