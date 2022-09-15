#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // calloc
#include <unistd.h> // STDIN_FILENO
#include <sys/ioctl.h>

#include "config.h"
#include "bus.h"

imu_config_t *create_config(void)
{
	imu_config_t *config = malloc(sizeof(imu_config_t));

	config->parsed_sensor_info = false;

	config->accelerometer_active = false;
	config->gyroscope_active = false;
	config->magnetometer_active = false;
	config->temperature_active = false;

	return config;
}

void activate_info(char const *const info, imu_config_t *config)
{
	if (!strcmp(info, "aX"))
		config->accelerometer_active = true;
	else if (!strcmp(info, "gX"))
		config->gyroscope_active = true;
	else if (!strcmp(info, "mX"))
		config->magnetometer_active = true;
	else if (!strcmp(info, "imu_degC"))
		config->temperature_active = true;
}

void flip_imu_logging(int option, serial_bus_t *bus)
{
	// interrupt senzor log and enter menu
	write_to_bus(bus, "x");
	// wait for menu to pop up
	while (read_from_bus(bus))
		;
	// enter IMU logging
	write_to_bus(bus, "3");
	// wait for menu to pop up
	while (read_from_bus(bus))
		;
	// flip selected option, need to append \n char to take effect
	char msg[2] = {'0' + option, '\n'};
	write_to_bus(bus, msg);
	// wait for menu to pop up
	while (read_from_bus(bus))
		;
	// exit IMU logging config
	write_to_bus(bus, "x");
	// wait for menu to pop up
	while (read_from_bus(bus))
		;
	// exit menu and enter logging
	write_to_bus(bus, "x");

	// wait for new data (but dont read yet) before exiting function call
	int bytes = 0;
	while (bytes == 0)
		ioctl(bus->serial_port, FIONREAD, &bytes);
}

void flip_accelometer(imu_config_t *config, serial_bus_t *bus)
{
	config->accelerometer_active = !config->accelerometer_active;

	flip_imu_logging(2, bus);
}

void flip_gyroscope(imu_config_t *config, serial_bus_t *bus)
{
	config->gyroscope_active = !config->gyroscope_active;

	flip_imu_logging(3, bus);
}

void flip_magnetometer(imu_config_t *config, serial_bus_t *bus)
{
	config->magnetometer_active = !config->magnetometer_active;

	flip_imu_logging(4, bus);
}

void flip_temperature(imu_config_t *config, serial_bus_t *bus)
{
	config->temperature_active = !config->temperature_active;

	flip_imu_logging(5, bus);
}

void disable_accelometer(imu_config_t *config, serial_bus_t *bus)
{
	if (!config->accelerometer_active)
		return;

	config->accelerometer_active = false;
	flip_imu_logging(2, bus);
}

void disable_gyroscope(imu_config_t *config, serial_bus_t *bus)
{
	if (!config->gyroscope_active)
		return;

	config->gyroscope_active = false;
	flip_imu_logging(3, bus);
}

void disable_magnetometer(imu_config_t *config, serial_bus_t *bus)
{
	if (!config->magnetometer_active)
		return;

	config->magnetometer_active = false;
	flip_imu_logging(4, bus);
}

void disable_temperature(imu_config_t *config, serial_bus_t *bus)
{
	if (!config->temperature_active)
		return;

	config->temperature_active = false;
	flip_imu_logging(5, bus);
}

void enable_accelometer(imu_config_t *config, serial_bus_t *bus)
{
	if (config->accelerometer_active)
		return;

	config->accelerometer_active = true;
	flip_imu_logging(2, bus);
}

void enable_gyroscope(imu_config_t *config, serial_bus_t *bus)
{
	if (config->gyroscope_active)
		return;

	config->gyroscope_active = true;
	flip_imu_logging(3, bus);
}

void enable_magnetometer(imu_config_t *config, serial_bus_t *bus)
{
	if (config->magnetometer_active)
		return;

	config->magnetometer_active = true;
	flip_imu_logging(4, bus);
}

void enable_temperature(imu_config_t *config, serial_bus_t *bus)
{
	if (config->temperature_active)
		return;

	config->temperature_active = true;
	flip_imu_logging(5, bus);
}

void delete_config(imu_config_t *config)
{
	free(config);
}
