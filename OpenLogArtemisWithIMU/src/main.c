#define STB_IMAGE_IMPLEMENTATION
// C library headers
#include <stdio.h>   // printf
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <stdbool.h> // bool

#include "bus.h"
#include "util.h"
#include "config.h"
#include "parser.h"
#include "state.h"
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include "sensor_fusion.h"
#include <pthread.h>

static volatile int keepRunning = 1;

void intHandler(int dummy)
{
    (void)(dummy);
    keepRunning = 0;
}


int main()
{
    signal(SIGINT, intHandler);

    
    char const *device = "/dev/ttyUSB0";

    serial_bus_t *bus = create_serial_bus(device);

    parser_t *parser = create_parser();

    // FILE *csv_file = create_csv_file(); // when commented results in SEG FAULT

    next_log_data(bus, parser);

    while (keepRunning)
    {
        next_log_data(bus, parser);
        quaternion_t q = getQuat(parser->state->q1, parser->state->q2, parser->state->q3);
        printQuat(q);
        
        euler_angles_t euler = quatToEuler(q);
        printEuler(euler);
    }

    delete_bus(bus);
    delete_parser(parser);
    // fclose(csv_file);

    printf("Exit OK!\n");

    return 0;
}
