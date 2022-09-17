#ifndef SENSOR_FUSION_H
#define SENSOR_FUSION_H

typedef struct
{
    double x, y, z, w;
} quaternion_t;

typedef struct
{
    double yaw, pitch, roll;
} euler_angles_t;


void MadgwickQuaternionUpdate(quaternion_t *q, const float delta_t, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);

euler_angles_t quatToEuler(const quaternion_t q);

void printQuat(const quaternion_t q);

void printEuler(const euler_angles_t euler);

quaternion_t createQuat();

euler_angles_t createEular();

#endif