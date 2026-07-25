#version 300 es

// GroundFragment.glsl
// What this does: computes a black/white checkerboard purely from the
// fragment's world-space XZ position (no texture, no UV attribute needed -
// the quad is large enough, and floor()/mod() repeats the pattern past it
// anyway), then feeds that cell colour through the same ambient+diffuse+
// specular formula as FanFragment.glsl so the floor is lit by the same
// light as the fan.
//
// Edits: new file written for Quiz 3, no Quiz 2 equivalent existed.

precision mediump float;

in vec3 WorldPos;
in vec3 WorldNormal;
layout(location = 0) out vec4 outColor;

uniform vec3 LIGHTPOS;
uniform vec3 LIGHTCOLOR;
uniform vec3 VIEWPOS;

const float TILE_SIZE  = 2.0;
const vec3  COLOR_DARK  = vec3(0.08, 0.08, 0.09);
const vec3  COLOR_LIGHT = vec3(0.82, 0.82, 0.85);

float checker(vec2 worldXZ, float tileSize)
{
    vec2  cell   = floor(worldXZ / tileSize);
    float parity = mod(cell.x + cell.y, 2.0);
    return parity; // 0.0 or 1.0 -> mix() between two colours
}

void main()
{
    vec3 materialColor = mix(COLOR_DARK, COLOR_LIGHT, checker(WorldPos.xz, TILE_SIZE));

    vec3 N = normalize(WorldNormal);
    vec3 L = normalize(LIGHTPOS - WorldPos);
    vec3 V = normalize(VIEWPOS  - WorldPos);
    vec3 R = reflect(-L, N);

    vec3  ambient  = 0.18 * materialColor;
    float diff     = max(dot(N, L), 0.0);
    vec3  diffuse  = diff * materialColor * LIGHTCOLOR;
    float spec     = pow(max(dot(V, R), 0.0), 32.0);
    vec3  specular = spec * 0.4 * LIGHTCOLOR;

    outColor = vec4(ambient + diffuse + specular, 1.0);
}
