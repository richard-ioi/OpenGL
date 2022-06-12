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

    friend bool operator== (const Vertex& v1, const Vertex& v2) {
        return (v1.position.x==v2.position.x && v1.position.y==v2.position.y && v1.position.z==v2.position.z
            && v1.normal.x == v2.normal.x && v1.normal.y == v2.normal.y && v1.normal.z == v2.normal.y
            && v1.uv.x == v2.uv.x && v1.uv.y == v2.uv.y);
    }
};

