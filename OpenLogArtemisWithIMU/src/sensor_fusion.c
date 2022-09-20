#include "sensor_fusion.h"
#include <math.h>
#include <stdio.h>

#define GyroMeasError		M_PI * (180.0f / 180.0f)							// gyroscope measurement error in rads/s (start at 40 deg/s)
#define GyroMeasDrift		M_PI * (0.0f  / 180.0f);							// gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
#define beta 				1 // sqrt(3.0f / 4.0f) *GyroMeasError   					// compute beta

quaternion_t createQuat(){
	quaternion_t to_return;
	to_return.x=0.0;
	to_return.y=0.0; 
	to_return.z=0.0; 
	to_return.w=1.0;
	return to_return;
}
euler_angles_t createEular(){
	euler_angles_t to_return;
	to_return.yaw = 0.0;
	to_return.pitch = 0.0;
	to_return.roll = 0.0;
	return to_return;
}


euler_angles_t quatToEuler(const quaternion_t q){
    euler_angles_t to_return;
    to_return.yaw   = atan2(2.0f * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z);
	to_return.pitch = -asin(2.0f * (q.x * q.z - q.w * q.y));
	to_return.roll  = atan2(2.0f * (q.w * q.x + q.y * q.z), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);
	to_return.pitch *= 180.0f / M_PI;
	to_return.yaw   *= (180.0f) / M_PI;
	to_return.roll  *= (180.0f) / M_PI;

    return to_return;
}


void printQuat(const quaternion_t q){
	printf("%f, %f, %f, %f\n", q.x, q.y, q.z, q.w);
}

void printEuler(const euler_angles_t euler){
	printf("%f, %f, %f\n", euler.yaw, euler.pitch, euler.roll);
}

void set_zero_point(quaternion_t *q_zero, quaternion_t* q){
	q_zero->w = q->w;
	q_zero->y = -q->y;
	q_zero->z = -q->z;
	q_zero->x = -q->x;
}

quaternion_t conjugate_quaternion(const quaternion_t quat){
	quaternion_t to_return;
	to_return.w = quat.w;
	to_return.x = -quat.x;
	to_return.y = -quat.y;
	to_return.z = -quat.z;
	return to_return;
}

quaternion_t hamilton_quaternions(const quaternion_t a, const quaternion_t b){
	quaternion_t to_return;
	to_return.w=a.w*b.w	-a.x*b.x	-a.y*b.y	-a.z*b.z;
	to_return.x=a.w*b.x	+a.x*b.w	+a.y*b.z	-a.z*b.y;
	to_return.y=a.w*b.y	-a.x*b.z	+a.y*b.w	+a.z*b.x;
	to_return.z=a.w*b.z	+a.x*b.y	-a.y*b.x	+a.z*b.w;
	return to_return;
}

void MadgwickQuaternionUpdate(quaternion_t *q, quaternion_t *q_zero, const float delta_t, float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz)
{
	float q1 = q->w, q2 = q->x, q3 = q->y, q4 = q->z;   						// short name local variable for readability
	float norm;
	float hx, hy, _2bx, _2bz;
	float s1, s2, s3, s4;
	float qDot1, qDot2, qDot3, qDot4;

	// Auxiliary variables to avoid repeated arithmetic
	float _2q1mx;
	float _2q1my;
	float _2q1mz;
	float _2q2mx;
	float _4bx;
	float _4bz;
	float _2q1 = 2.0f * q1;
	float _2q2 = 2.0f * q2;
	float _2q3 = 2.0f * q3;
	float _2q4 = 2.0f * q4;
	float _2q1q3 = 2.0f * q1 * q3;
	float _2q3q4 = 2.0f * q3 * q4;
	float q1q1 = q1 * q1;
	float q1q2 = q1 * q2;
	float q1q3 = q1 * q3;
	float q1q4 = q1 * q4;
	float q2q2 = q2 * q2;
	float q2q3 = q2 * q3;
	float q2q4 = q2 * q4;
	float q3q3 = q3 * q3;
	float q3q4 = q3 * q4;
	float q4q4 = q4 * q4;

	// Normalise accelerometer measurement
	norm = sqrt(ax * ax + ay * ay + az * az);
	
	if (norm == 0.0f) return; // handle NaN
	norm = 1.0f/norm;
	ax *= norm;
	ay *= norm;
	az *= norm;

	// Normalise magnetometer measurement
	norm = sqrt(mx * mx + my * my + mz * mz);
	if (norm == 0.0f) return; // handle NaN
	norm = 1.0f/norm;
	mx *= norm;
	my *= norm;
	mz *= norm;

	// Reference direction of Earth's magnetic field
	_2q1mx = 2.0f * q1 * mx;
	_2q1my = 2.0f * q1 * my;
	_2q1mz = 2.0f * q1 * mz;
	_2q2mx = 2.0f * q2 * mx;
	hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
	hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
	_2bx = sqrt(hx * hx + hy * hy);
	_2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
	_4bx = 2.0f * _2bx;
	_4bz = 2.0f * _2bz;

	// Gradient decent algorithm corrective step
	s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
	s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
	s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
	s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
	norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
	norm = 1.0f/norm;
	s1 *= norm;
	s2 *= norm;
	s3 *= norm;
	s4 *= norm;

	// Compute rate of change of quaternion
	qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
	qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy)  - beta * s2;
	qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx)  - beta * s3;
	qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx)  - beta * s4;

	// Integrate to yield quaternion
	q1 += qDot1 * delta_t;
	q2 += qDot2 * delta_t;
	q3 += qDot3 * delta_t;
	q4 += qDot4 * delta_t;
	norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
	norm = 1.0f/norm;
	
	printf("Before change ");
	printQuat(*q);
	q->w = q1 * norm;
	q->x = q2 * norm;
	q->y = q3 * norm;
	q->z = q4 * norm;
	printf("After change ");
	printQuat(*q);

	*q = hamilton_quaternions(*q_zero, *q);
}

