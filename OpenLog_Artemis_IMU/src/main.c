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

void intHandler(int dummy) {
	(void)(dummy);
    keepRunning = 0;
}

double computeHeading(double mag_x, double mag_y, double mag_z, double accel_x, double accel_y, double accel_z)
    {
        const vec3 vector_mag = create_vec3(mag_x, mag_y, mag_z);
        const vec3 vector_down = create_vec3(accel_x, accel_y, accel_z);
        const vec3 vector_north = sub_vec3 (vector_mag , scale_vec3((dot_vec3(vector_mag,vector_down) / dot_vec3(vector_down, vector_down)), vector_down));
        return atan2(vector_north.x, vector_north.y) * 180 / M_PI;
    }

int main()
{
	signal(SIGINT, intHandler);
	char const *device = "/dev/ttyUSB0";

	serial_bus_t *bus = create_serial_bus(device);

	parser_t *parser = create_parser();

	// FILE *csv_file = create_csv_file(); // when commented results in SEG FAULT
	
	while (keepRunning)
	{
		/* Parse sensor logged data */
		next_log_data(bus, parser);
		printf("IMU stuff: a: %f, %f, %f, g: %f, %f, %f, m: %f, %f, %f \n", 
			parser->state->aX, parser->state->aY, parser->state->aZ, 
			parser->state->gX, parser->state->gY, parser->state->gZ, 
			parser->state->mX, parser->state->mY, parser->state->mZ);
		printf("GPS stuff: Alt %f m, Lat %f°, Long %f°, Speed %fm/s, %f°, SIV %d, FixPoint %d\n", 
			parser->state->gps_Alt, parser->state->gps_Lat, parser->state->gps_Long, 
			parser->state->gps_GroundSpeed, parser->state->gps_Heading,
			parser->state->gps_SIV, parser->state->gps_FixType);

		printf("\n");
		/* Save it to csv file */
		// log_line_to_csv(csv_file, parser);

		// Where is north?
		
		// computing just from Magnetometer
		// float heading = atan2(parser->state->mY, parser->state->mX) * 180 / M_PI;
		
		// computing from both Magnetometer and Accelerometer
		float newHeading = computeHeading(parser->state->mX,parser->state->mY, parser->state->mZ, parser->state->aX, parser->state->aY, parser->state->aZ);
		printf("%f -> ", newHeading);
		if(newHeading >= -45 && newHeading < 45){
			printf("N\n");
		}else if(newHeading >= 45 && newHeading < 135){
			printf("E\n");
		}else if(newHeading <= -45 && newHeading > -135){
			printf("W\n");
		}else{
			printf("S\n");
		}
		
		
		// Euler angles?
	}

	delete_bus(bus);
	delete_parser(parser);
	// mfclose(csv_file);

	printf("Exit OK!\n");

	return 0;
}
