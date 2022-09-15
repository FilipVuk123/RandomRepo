#ifndef VEC3_H
#define VEC3_H

typedef struct
{
    double x, y, z;
} vec3;

vec3 create_vec3(const double x, const double y, const double z);

double dot_vec3(const vec3 a, const vec3 b);

vec3 add_vec3(const vec3 a, const vec3 b);

vec3 sub_vec3(const vec3 a, const vec3 b);

vec3 scale_vec3(const double scale, const vec3 vec);
#endif