#ifndef CONFIG_H
#define CONFIG_H

#include "bus.h"

typedef struct {
  /* Once, when starting, parse the active sensors. */ 
	bool parsed_sensor_info;

  /* Is certain logging mode active? */
	bool accelerometer_active;
	bool gyroscope_active;
	bool magnetometer_active;
	bool temperature_active;
} imu_config_t;

imu_config_t *create_config(void);

/*
 * Activates some of the logging modes based on info passed.
 *
 * Possible info cases (arbitrarily selected) are:
 * -- aX       -> accelerometer
 * -- gX       -> gyroscope
 * -- mX       -> magnetometer
 * -- imu_degC -> temperature
 */
void activate_info(char const *const info, imu_config_t *config);

/*
 * Flip (if its active then deactivate it, if its deactive then activate it) 
 * selected option in IMU logging.
 *
 * Possible option cases are:
 * -- 2 -> accelerometer
 * -- 3 -> gyroscope
 * -- 4 -> magnetometer
 * -- 5 -> temperature
 * 
 * These seemingly arbitrary numbers are defined by the 
 * main menu of the IMU with whom we interact.
 */
void flip_imu_logging(int option, serial_bus_t *bus);

/*
 * Functions to disable logging for each of the sensor in the IMU. 
 */
void disable_accelometer(imu_config_t *config, serial_bus_t *bus);
void disable_gyroscope(imu_config_t *config, serial_bus_t *bus);
void disable_magnetometer(imu_config_t *config, serial_bus_t *bus);
void disable_temperature(imu_config_t *config, serial_bus_t *bus);

/*
 * Functions to enable logging for each of the sensor in the IMU. 
 */
void enable_accelometer(imu_config_t *config, serial_bus_t *bus);
void enable_gyroscope(imu_config_t *config, serial_bus_t *bus);
void enable_magnetometer(imu_config_t *config, serial_bus_t *bus);
void enable_temperature(imu_config_t *config, serial_bus_t *bus);

/*
 * Functions to flip logging for each of the sensor in the IMU. 
 */
void flip_accelometer(imu_config_t *config, serial_bus_t *bus);
void flip_gyroscope(imu_config_t *config, serial_bus_t *bus);
void flip_magnetometer(imu_config_t *config, serial_bus_t *bus);
void flip_temperature(imu_config_t *config, serial_bus_t *bus); 

/*
 * Frees up memory.
 */
void delete_config(imu_config_t *config);

#endif // CONFIG_H
