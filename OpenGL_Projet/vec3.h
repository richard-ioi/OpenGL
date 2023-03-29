#pragma once
struct vec3 {
    float x, y, z;
    friend vec3 operator*(const vec3& v1, const vec3& v2) {
        return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
    }
    friend vec3 operator+(const vec3& v1, const vec3& v2) {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }
    friend vec3 operator-(const vec3& v1, const vec3& v2) {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }
    friend vec3 operator^(const vec3& v1, const vec3& v2) {
        vec3 t;
        t.x = v1.y * v2.z - v1.z * v2.y;
        t.y = v1.z * v2.x - v1.x * v2.z;
        t.z = v1.x * v2.y - v1.y * v2.x;
        return t;
    }
    friend vec3 operator- (const vec3& v1) {
        return { -v1.x,-v1.y,-v1.z };
    }
};

float dot(vec3 v1, vec3 v2) {
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

void normalize(vec3* v)
{
    float w = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    v->x /= w;
    v->y /= w;
    v->z /= w;
}