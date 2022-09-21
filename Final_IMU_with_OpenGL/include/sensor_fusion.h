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


void set_zero_point(quaternion_t *q_zero, quaternion_t* q);

quaternion_t conjugate_quaternion(const quaternion_t quat);

quaternion_t hamilton_quaternions(const quaternion_t a, const quaternion_t b);

void MadgwickQuaternionUpdate(quaternion_t *q, quaternion_t *q_zero, const float delta_t, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);

euler_angles_t quatToEuler(const quaternion_t q);

void printQuat(const quaternion_t q);

void printEuler(const euler_angles_t euler);

quaternion_t createQuat();

euler_angles_t createEular();

euler_angles_t quatToEuler(const quaternion_t q);

quaternion_t getQuat(const float q1, const float q2, const float q3);

#endif