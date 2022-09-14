#ifndef VEC3_H
#define VEC3_H

typedef struct {
    float x, y, z;
} vec3;

vec3 create_vec3(const float x, const float y, const float z);
float dot_vec3(const vec3 a, const vec3 b);

vec3 add_vec3(const vec3 a, const vec3 b);
vec3 sub_vec3(const vec3 a, const vec3 b);
vec3 scale_vec3(const float scale, vec3 vec);

#endif