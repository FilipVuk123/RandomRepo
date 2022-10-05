#ifndef __Madgwick_h__
#define __Madgwick_h__

extern volatile float beta;				// algorithm gain
extern volatile float q0, q1, q2, q3;	// quaternion of sensor frame relative to auxiliary frame

void MadgwickUpdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MadgwickUpdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

#endif

