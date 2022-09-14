#include "vec3.h"

vec3 create_vec3(const float x, const float y, const float z){
    vec3 to_return;
    to_return.x = x;
    to_return.y = y;
    to_return.z = z;
    return to_return;
}
float dot_vec3(const vec3 a, const vec3 b){ 
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 add_vec3(const vec3 a, const vec3 b){
    vec3 to_return;
    to_return.x = a.x + b.x;
    to_return.y = a.y + b.y;
    to_return.z = a.z + b.z;
    return to_return;
}


vec3 sub_vec3(const vec3 a, const vec3 b){
    vec3 to_return;
    to_return.x = a.x - b.x;
    to_return.y = a.y - b.y;
    to_return.z = a.z - b.z;
    return to_return;
}
vec3 scale_vec3(const float scale, vec3 vec){
    vec3 to_return;
    to_return.x = scale * vec.x;
    to_return.y = scale * vec.y;
    to_return.z = scale * vec.z;
    return to_return;
}