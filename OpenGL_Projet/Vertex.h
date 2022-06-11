#pragma once

#include <cstdint>

struct vec2 {
    float x, y;
};

struct vec3 {
    float x, y, z;
};

struct Color {
    uint8_t r, g, b, a;
};

struct Vertex
{
    vec3 position;  //x,y,z
    vec3 normal;    //nx,ny,nz
    vec2 uv;        //u,v
};
