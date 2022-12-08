#include "sensor_fusion.h"
#include <math.h>
#include <stdio.h>

double toRadians(double x){
    return x * M_PI/180.0f;
}

double toDegrees(double x){
    return x * 180.f/M_PI;
}

quaternion_t initQuat()
{
	quaternion_t to_return;
	to_return.x = 0.0;
	to_return.y = 0.0;
	to_return.z = 0.0;
	to_return.w = 1.0;
	return to_return;
}

quaternion_t createQuat(double x, double y, double z, double w){
	quaternion_t to_return;
	to_return.x = x;
	to_return.y = y;
	to_return.z = z;
	to_return.w = w;
	return to_return;
}

euler_angles_t initEuler()
{
	euler_angles_t to_return;
	to_return.yaw = 0.0;
	to_return.pitch = 0.0;
	to_return.roll = 0.0;
	return to_return;
}

euler_angles_t createEuler(double yaw, double pitch, double roll)
{
	euler_angles_t to_return;
	to_return.yaw = yaw;
	to_return.pitch = pitch;
	to_return.roll = roll;
	return to_return;
}

euler_angles_t quatToEuler(const quaternion_t q)
{
	euler_angles_t to_return;
	double q2sqr = q.y * q.y;

	// roll (x-axis rotation)
	double t0 = +2.0 * (q.w * q.x + q.y * q.z);
	double t1 = +1.0 - 2.0 * (q.x * q.x + q2sqr);
	to_return.pitch = atan2(t0, t1) * 180.0 / M_PI;

	// pitch (y-axis rotation)
	double t2 = +2.0 * (q.w * q.y - q.z * q.x);
	t2 = t2 > 1.0 ? 1.0 : t2;
	t2 = t2 < -1.0 ? -1.0 : t2;
	to_return.roll = asin(t2) * 180.0 / M_PI;

	// yaw (z-axis rotation)
	double t3 = +2.0 * (q.w * q.z + q.x * q.y);
	double t4 = +1.0 - 2.0 * (q2sqr + q.z * q.z);
	to_return.yaw = atan2(t3, t4) * 180.0 / M_PI;

	return to_return;
}

quaternion_t getQuat(const float q1, const float q2, const float q3)
{
	quaternion_t to_return;
	to_return.x = q1;
	to_return.y = q2;
	to_return.z = q3;
	double tmp = 1.0f - q1 * q1 - q2 * q2 - q3 * q3;
	to_return.w = tmp > 0.0 ? sqrt(tmp) : 1.0;
	return to_return;
}

void printQuat(const quaternion_t q)
{
	printf("%f, %f, %f, %f\n", q.x, q.y, q.z, q.w);
}

void printEuler(const euler_angles_t euler)
{
	printf("%f, %f, %f\n", euler.yaw, euler.pitch, euler.roll);
}

void set_zero_point(quaternion_t *q_zero, quaternion_t *q)
{
	q_zero->w = q->w;
	q_zero->y = -q->y;
	q_zero->z = -q->z;
	q_zero->x = -q->x;
}

quaternion_t conjugate_quaternion(const quaternion_t quat)
{
	quaternion_t to_return;
	to_return.w = quat.w;
	to_return.x = -quat.x;
	to_return.y = -quat.y;
	to_return.z = -quat.z;
	return to_return;
}

quaternion_t hamilton_quaternions(const quaternion_t a, const quaternion_t b)
{
	quaternion_t to_return;
	to_return.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	to_return.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	to_return.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	to_return.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	return to_return;
}
