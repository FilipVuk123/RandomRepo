#include "imu.h"
#include "vec3.h"
#include <math.h>


double computeHeading(const double mag_x, const double mag_y, const double mag_z, const double accel_x, const double accel_y, const double accel_z)
{
	double scale;
	const vec3 vector_mag = create_vec3(mag_x, mag_y, mag_z);
	const vec3 vector_down = create_vec3(accel_x, accel_y, accel_z);

	double tmp = dot_vec3(vector_down, vector_down);
	if (tmp != 0.0)
		scale = dot_vec3(vector_mag, vector_down) / tmp;
	else
		scale = dot_vec3(vector_mag, vector_down);
	
	const vec3 vector_north = sub_vec3(vector_mag, scale_vec3(scale, vector_down));
	
	return atan2(vector_north.x, vector_north.y) * 180 / M_PI;
}


double getRoll(const double accel_x, const double accel_y, const double accel_z){
    return atan(accel_x / sqrt(accel_y * accel_y + accel_z * accel_z));
}


double getPitch(const double accel_x, const double accel_y, const double accel_z){
    return atan(accel_y / sqrt(accel_x * accel_x + accel_z * accel_z));
}


double getYaw(const double mag_x, const double mag_y, const double mag_z, const double roll, const double pitch){
    return atan2((mag_y * cos(roll)) - (mag_z * sin(roll)),
					(mag_x * cos(pitch)) + (mag_y * sin(roll) * sin(pitch)) + (mag_z * cos(roll) * sin(pitch)));
}


double toDegrees(const double x){
    return 180.0 * x / M_PI;
}


double toRadians(const double x){
    return M_PI * x / 180.0;
}