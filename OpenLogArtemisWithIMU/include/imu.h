#ifndef IMU_H
#define IMU_H

/**
 * > The function computes the heading of the device by projecting the magnetic field vector onto the
 * plane perpendicular to the gravity vector and then computing the angle between the projected vector
 * and the north vector
 * 
 * @param mag_x The x-axis magnetic field reading from the magnetometer.
 * @param mag_y The y-axis of the magnetometer
 * @param mag_z The z-axis of the magnetometer
 * @param accel_x Accelerometer X axis
 * @param accel_y Acceleration in the y direction (in g's)
 * @param accel_z The z-axis of the accelerometer.
 * 
 * @return The heading in degrees.
 */
double computeHeading(const double mag_x, const double mag_y, const double mag_z, const double accel_x, const double accel_y, const double accel_z);


/**
 * > The roll angle is the angle between the x-axis and the projection of the x-axis on the y-z plane
 * (the plane perpendicular to the x-axis)
 * 
 * @param accel_x The acceleration in the x-axis.
 * @param accel_y The acceleration in the y-axis.
 * @param accel_z The acceleration in the z-axis.
 * 
 * @return The roll angle of the device.
 */
double getRoll(const double accel_x, const double accel_y, const double accel_z);

/**
 * > The pitch is the angle between the x-axis and the projection of the x-y plane onto the x-z plane
 * 
 * @param accel_x The acceleration in the x-axis.
 * @param accel_y The acceleration in the y-axis.
 * @param accel_z The acceleration in the z-axis.
 * 
 * @return The pitch of the device.
 */
double getPitch(const double accel_x, const double accel_y, const double accel_z);

/**
 * > The function takes in the magnetometer readings and the roll and pitch angles and returns the yaw
 * angle
 * 
 * @param mag_x The x-axis value of the magnetometer
 * @param mag_y The y-axis value of the magnetometer
 * @param mag_z The z-axis value of the magnetometer
 * @param roll The roll of the IMU in radians.
 * @param pitch The pitch of the IMU in radians.
 * 
 * @return The yaw angle.
 */
double getYaw(const double mag_x, const double mag_y, const double mag_z, const double roll, const double pitch);

/**
 * It converts radians to degrees
 * 
 * @param x The angle in radians
 * 
 * @return the value of x in degrees.
 */
double toDegrees(const double x);

/**
 * It converts degrees to radians
 * 
 * @param x The angle in degrees
 * 
 * @return the value of x in radians.
 */
double toRadians(const double x);

#endif