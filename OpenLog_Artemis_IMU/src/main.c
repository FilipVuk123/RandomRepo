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
#include "vec3.h"
#include <signal.h>
#include <math.h>
#include <unistd.h>

static volatile int keepRunning = 1;

void intHandler(int dummy)
{
	(void)(dummy);
	keepRunning = 0;
}

float computeHeading(const float mag_x, const float mag_y, const float mag_z, const float accel_x, const float accel_y, const float accel_z)
{
	const vec3 vector_mag = create_vec3(mag_x, mag_y, mag_z);
	const vec3 vector_down = create_vec3(accel_x, accel_y, accel_z);
	const float scale = dot_vec3(vector_mag, vector_down) / dot_vec3(vector_down, vector_down) ? dot_vec3(vector_down, vector_down) != 0 : dot_vec3(vector_mag, vector_down);
	const vec3 vector_north = sub_vec3(vector_mag, scale_vec3(scale, vector_down));
	return atan2(vector_north.x, vector_north.y) * 180 / M_PI;
}

#define AVG_COUNT 30

int main()
{
	signal(SIGINT, intHandler);
	char const *device = "/dev/ttyUSB0";

	serial_bus_t *bus = create_serial_bus(device);

	parser_t *parser = create_parser();

	double f_yaw = 0.0, f_pitch = 0.0, f_roll = 0.0;
	int first = 1;

	// FILE *csv_file = create_csv_file(); // when commented results in SEG FAULT

	while (keepRunning)
	{
		float avg_aX = 0.0, avg_aY = 0.0, avg_aZ = 0.0, avg_gX = 0.0, avg_gY = 0.0, avg_gZ = 0.0, avg_mX = 0.0, avg_mY = 0.0, avg_mZ = 0.0;
		/* Parse sensor logged data */
		for (int i = 0; i < AVG_COUNT; i++)
		{
			if(!keepRunning) break;
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

		// printf("GPS : Alt %f m, Lat %f°, Long %f°, Speed %fm/s, %f°, SIV %d, FixPoint %d\n",
			//    parser->state->gps_Alt, parser->state->gps_Lat, parser->state->gps_Long,
			//    parser->state->gps_GroundSpeed, parser->state->gps_Heading,
			//    parser->state->gps_SIV, parser->state->gps_FixType);

		printf("\n");
		/* Save it to csv file */
		// log_line_to_csv(csv_file, parser);

		/*-------------------------------------*/
		// Where is north?

		// computing just from Magnetometer
		// double heading = atan2(avg_mY, avg_mX) * 180 / M_PI;
		// printf("%f -> ", heading);
		// if(heading>= -45 && heading<= 45){
		// 	printf("N\n");
		// }else if(heading>= 45 && heading<= 135){
		// 	printf("W\n");
		// }else if(heading<= -45 && heading>= -135){
		// 	printf("E\n");
		// }else{
		// 	printf("S\n");
		// }

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

		/*-------------------------------------*/
		// Euler angles?

		double yaw, pitch, roll;

		roll = atan (avg_aX/sqrt(avg_aY*avg_aY + avg_aZ*avg_aZ));
		pitch = atan (avg_aY/sqrt(avg_aX*avg_aX + avg_aZ*avg_aZ));

   		yaw = atan2((avg_mY * cos(roll)) - (avg_mZ * sin(roll)), (avg_mX * cos(pitch))+(avg_mY* sin(roll)*sin(pitch)) + (avg_mZ * cos(roll) * sin(pitch)));
		pitch = 180* pitch / M_PI;
		roll = 180* roll / M_PI;
		yaw = 180*yaw / M_PI;

		if(first){
			first = 0;
			f_yaw = yaw;
			f_roll = roll;
			f_pitch = pitch;		
		}else{
			yaw -= f_yaw;
			roll -= f_roll;
			pitch -= f_pitch;
		}


		printf("Roll: %f, Pitch: %f, Yaw: %f\n", roll, pitch, yaw);
	}

	delete_bus(bus);
	delete_parser(parser);
	// fclose(csv_file);

	printf("Exit OK!\n");

	return 0;
}
