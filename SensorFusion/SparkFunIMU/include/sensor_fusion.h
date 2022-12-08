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

double toRadians(double x);
double toDegrees(double x);

void set_zero_point(quaternion_t *q_zero, quaternion_t* q);

quaternion_t conjugate_quaternion(const quaternion_t quat);

quaternion_t hamilton_quaternions(const quaternion_t a, const quaternion_t b);

euler_angles_t quatToEuler(const quaternion_t q);

void printQuat(const quaternion_t q);

void printEuler(const euler_angles_t euler);

quaternion_t initQuat();

euler_angles_t initEuler();

quaternion_t createQuat(double x, double y, double z, double w);

euler_angles_t createEuler(double yaw, double pitch, double roll);

euler_angles_t quatToEuler(const quaternion_t q);

quaternion_t getQuat(const float q1, const float q2, const float q3);

#endif