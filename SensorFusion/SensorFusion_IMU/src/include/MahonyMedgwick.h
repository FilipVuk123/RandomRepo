
#ifndef __Mahony_Madgwick_h__
#define __Mahony_Madgwick_h__

extern volatile float twoKp;          // 2 * proportional gain (Kp)
extern volatile float twoKi;          // 2 * integral gain (Ki)
extern volatile float q0, q1, q2, q3; // quaternion of sensor frame relative to auxiliary frame

extern volatile float beta; // algorithm gain

void MadgwickUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MadgwickUpdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

void MahonyUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MahonyUpdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

#endif