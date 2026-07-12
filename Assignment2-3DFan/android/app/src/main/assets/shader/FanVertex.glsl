#version 300 es

layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in float VertexShade;

out float Shade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

void main()
{
    gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
    Shade = VertexShade;
}
