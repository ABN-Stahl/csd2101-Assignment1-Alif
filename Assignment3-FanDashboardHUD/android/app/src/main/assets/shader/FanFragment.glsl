#version 300 es

// FanFragment.glsl
// What this does: real per-fragment Phong shading (ambient + diffuse +
// specular) using PARTCOLOR as the material's diffuse/ambient colour, a
// single point light (LIGHTPOS/LIGHTCOLOR), and the world-space eye position
// (VIEWPOS) for the specular half.
//
// Edits for Quiz 3: replaces Quiz 2's `PARTCOLOR * Shade` baked-gradient
// output entirely - Shade was a fixed per-vertex factor, not lighting.

precision mediump float;

in vec3 WorldPos;
in vec3 WorldNormal;
layout(location = 0) out vec4 outColor;

uniform vec3 PARTCOLOR;
uniform vec3 LIGHTPOS;
uniform vec3 LIGHTCOLOR;
uniform vec3 VIEWPOS;

void main()
{
    vec3 N = normalize(WorldNormal);
    vec3 L = normalize(LIGHTPOS - WorldPos);
    vec3 V = normalize(VIEWPOS  - WorldPos);
    vec3 R = reflect(-L, N);

    vec3  ambient  = 0.18 * PARTCOLOR;
    float diff     = max(dot(N, L), 0.0);
    vec3  diffuse  = diff * PARTCOLOR * LIGHTCOLOR;
    float spec     = pow(max(dot(V, R), 0.0), 32.0);
    vec3  specular = spec * 0.6 * LIGHTCOLOR;

    outColor = vec4(ambient + diffuse + specular, 1.0);
}
