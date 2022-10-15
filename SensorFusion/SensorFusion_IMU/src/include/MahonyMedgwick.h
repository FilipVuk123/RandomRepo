
#ifndef __Mahony_Madgwick_h__
#define __Mahony_Madgwick_h__

#include "common.h"

extern volatile float twoKp;          // 2 * proportional gain (Kp)
extern volatile float twoKi;          // 2 * integral gain (Ki)
extern volatile float beta; // algorithm gain

void MadgwickUpdate(quat_t *quat, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MadgwickUpdateIMU(quat_t *quat, float gx, float gy, float gz, float ax, float ay, float az);

void MahonyUpdate(quat_t *quat, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MahonyUpdateIMU(quat_t *quat, float gx, float gy, float gz, float ax, float ay, float az);

#endif
