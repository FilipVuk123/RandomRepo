// C library headers
#include <stdio.h>	 // printf
#include <stdlib.h>	 // calloc
#include <string.h>	 // strcmp
#include <stdbool.h> // bool

#include "bus.h"
#include "util.h"
#include "config.h"
#include "parser.h"
#include "state.h"
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include "imu.h"
#include "sensor_fusion.h"

static volatile int keepRunning = 1;

void intHandler(int dummy)
{
	(void)(dummy);
	keepRunning = 0;
}

#define AVG_COUNT 30

int main()
{
	signal(SIGINT, intHandler);
	char const *device = "/dev/ttyUSB0";

	serial_bus_t *bus = create_serial_bus(device);

	parser_t *parser = create_parser();

	// FILE *csv_file = create_csv_file(); // when commented results in SEG FAULT

	next_log_data(bus, parser);
#if 0
	if (!parser->config->accelerometer_active)
	{
		enable_accelometer(parser->config, bus);
	}
	if (!parser->config->magnetometer_active)
	{
		enable_magnetometer(parser->config, bus);
	}
	if (!parser->config->gyroscope_active)
	{
		enable_gyroscope(parser->config, bus);
	}
	if (!parser->config->temperature_active)
	{
		enable_temperature(parser->config, bus);
	}
#endif
	quaternion_t q = createQuat();
	quaternion_t q_zero = createQuat();

	while (keepRunning)
	{
#if 0
		float avg_aX = 0.0, avg_aY = 0.0, avg_aZ = 0.0, avg_gX = 0.0, avg_gY = 0.0, avg_gZ = 0.0, avg_mX = 0.0, avg_mY = 0.0, avg_mZ = 0.0;
		for (int i = 0; i < AVG_COUNT; i++)
		{
			if (!keepRunning)
				break;
			next_log_data(bus, parser);
			avg_aX += parser->state->aX;
			avg_aY += parser->state->aY;
			avg_aZ += parser->state->aZ;
			avg_gX += parser->state->gX;
			avg_gY += parser->state->gY;
			avg_gZ += parser->state->gZ;
			avg_mX += parser->state->mX;
			avg_mY += parser->state->mY;
			avg_mZ += parser->state->mZ;
		}
		avg_aX /= AVG_COUNT;
		avg_aY /= AVG_COUNT;
		avg_aZ /= AVG_COUNT;
		avg_gX /= AVG_COUNT;
		avg_gY /= AVG_COUNT;
		avg_gZ /= AVG_COUNT;
		avg_mX /= AVG_COUNT;
		avg_mY /= AVG_COUNT;
		avg_mZ /= AVG_COUNT;

		printf("IMU : a: %f, %f, %f, g: %f, %f, %f, m: %f, %f, %f \n",
			   avg_aX, avg_aY, avg_aZ,
			   avg_gX, avg_gY, avg_gZ,
			   avg_mX, avg_mY, avg_mZ);

#endif

		next_log_data(bus, parser);
		printf("In main. Before change ");
		printQuat(q);
		MadgwickQuaternionUpdate(&q, &q_zero, 0.1, // changed to 10Hz
								 parser->state->aX / 1000, parser->state->aY / 1000, parser->state->aZ / 1000,
								 toRadians(parser->state->gX), toRadians(parser->state->gY), toRadians(parser->state->gZ),
								 parser->state->mX, parser->state->mY, parser->state->mZ);
		printf("In main. After change ");
		printQuat(q);
		euler_angles_t euler = quatToEuler(q);
		printf("Euler: ");
		printEuler(euler);

#if 0
		// printf("GPS : Alt %f m, Lat %f°, Long %f°, Speed %fm/s, %f°, SIV %d, FixPoint %d\n",
		// 	   parser->state->gps_Alt, parser->state->gps_Lat, parser->state->gps_Long,
		// 	   parser->state->gps_GroundSpeed, parser->state->gps_Heading,
		// 	   parser->state->gps_SIV, parser->state->gps_FixType);

		printf("\n");
		/* Save it to csv file */
		// log_line_to_csv(csv_file, parser);


		/*-------------------------------------*/
		// Where is north?

		// computing from both Magnetometer and Accelerometer
		double newHeading = computeHeading(avg_mX, avg_mY, avg_mZ, avg_aX, avg_aY, avg_aZ);
		printf("%f -> ", newHeading);
		if (newHeading >= -45 && newHeading <= 45)
		{
			printf("North\n");
		}
		else if (newHeading >= 45 && newHeading <= 135)
		{
			printf("West\n");
		}
		else if (newHeading <= -45 && newHeading >= -135)
		{
			printf("East\n");
		}
		else
		{
			printf("South\n");
		}

		newHeading = atan2(avg_mY, avg_mX) * 180 / M_PI;
		printf("%f -> ", newHeading);
		if (newHeading >= -45 && newHeading <= 45)
		{
			printf("North\n");
		}
		else if (newHeading >= 45 && newHeading <= 135)
		{
			printf("West\n");
		}
		else if (newHeading <= -45 && newHeading >= -135)
		{
			printf("East\n");
		}
		else
		{
			printf("South\n");
		}

		/*-------------------------------------*/
		// Euler angles?

		double yaw, pitch, roll;

		roll = getRoll(avg_aX, avg_aY, avg_aZ);

		pitch = getPitch(avg_aX, avg_aY, avg_aZ);

		yaw = getYaw(avg_mX, avg_mY, avg_mZ, roll, pitch);

		pitch = toDegrees(pitch);
		roll = toDegrees(roll);
		yaw = toDegrees(yaw);

		printf("Roll: %f, Pitch: %f, Yaw: %f\n", roll, pitch, yaw); // gimbal lock!
#endif
	}

	delete_bus(bus);
	delete_parser(parser);
	// fclose(csv_file);

	printf("Exit OK!\n");

	return 0;
}
