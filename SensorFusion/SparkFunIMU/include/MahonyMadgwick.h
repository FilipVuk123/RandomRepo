
#ifndef __Mahony_Madgwick_h__
#define __Mahony_Madgwick_h__

extern volatile float twoKp;          // 2 * proportional gain (Kp)
extern volatile float twoKi;          // 2 * integral gain (Ki)
extern volatile float beta; // algorithm gain

#include "sensor_fusion.h"

/**
 * The function takes in the current quaternion, gyroscope, accelerometer, and magnetometer data, and
 * returns an updated quaternion
 * 
 * @param quat The quaternion to update.
 * @param gx gyroscope x-axis measurement in radians/s
 * @param gy gyroscope y-axis measurement in radians/s
 * @param gz gyroscope z-axis measurement in radians/s.
 * @param ax Accelerometer x-axis measurement in m/s^2
 * @param ay accelerometer y-axis measurement in m/s^2
 * @param az Accelerometer z-axis measurement in m/s^2
 * @param mx Magnetometer x-axis measurement in uT
 * @param my magnetometer y-axis measurement in uT
 * @param mz magnetometer z-axis measurement in uT
 * @param sample_freq The sample frequency in Hz.
 * 
 * @return The quaternion is being returned.
 */
void MadgwickUpdate(quaternion_t *quat, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, const float sample_freq);
void MadgwickUpdateIMU(quaternion_t *quat, float gx, float gy, float gz, float ax, float ay, float az, const float sample_freq);

/**
 * The function takes in the current quaternion, the gyroscope and accelerometer data, and the
 * magnetometer data. It then uses the gyroscope data to calculate the rate of change of the
 * quaternion, and uses the accelerometer and magnetometer data to calculate the error in the
 * quaternion. It then uses the error to calculate the rate of change of the quaternion, and adds this
 * to the rate of change calculated from the gyroscope data
 * 
 * @param quat The quaternion to update.
 * @param gx gyroscope x-axis measurement in radians/s
 * @param gy gyroscope y-axis measurement in radians/s
 * @param gz gyroscope z-axis measurement in radians/s.
 * @param ax Accelerometer x-axis measurement in m/s^2
 * @param ay accelerometer y-axis measurement in m/s^2
 * @param az Accelerometer z-axis measurement in m/s^2
 * @param mx Magnetometer x-axis measurement in uT
 * @param my magnetometer y-axis measurement in uT
 * @param mz magnetometer z-axis measurement in uT
 * @param sample_freq The sample frequency in Hz.
 * 
 * @return The quaternion is being returned.
 */
void MahonyUpdate(quaternion_t *quat, float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, const float sample_freq);
void MahonyUpdateIMU(quaternion_t *quat, float gx, float gy, float gz, float ax, float ay, float az, const float sample_freq);

#endif
