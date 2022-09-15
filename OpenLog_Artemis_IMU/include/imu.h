#ifndef IMU_H
#define IMU_H

double computeHeading(const double mag_x, const double mag_y, const double mag_z, const double accel_x, const double accel_y, const double accel_z);

double getRoll(const double accel_x, const double accel_y, const double accel_z);

double getPitch(const double accel_x, const double accel_y, const double accel_z);

double getYaw(const double mag_x, const double mag_y, const double mag_z, const double roll, const double pitch);

double toDegrees(const double x);

double toRadians(const double x);

#endif