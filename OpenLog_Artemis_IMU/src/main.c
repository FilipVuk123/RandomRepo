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


static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main()
{
	signal(SIGINT, intHandler);
	char const *device = "/dev/ttyUSB0";

	serial_bus_t *bus = create_serial_bus(device);

	parser_t *parser = create_parser();

	FILE *csv_file = create_csv_file();

	while (keepRunning)
	{
		/* Parse sensor logged data */
		next_log_data(bus, parser);

		/* Save it to csv file */
		// log_line_to_csv(csv_file, parser);
		
	}

	delete_bus(bus);
	delete_parser(parser);
	fclose(csv_file);

	printf("Exit OK!\n");

	return 0;
}
