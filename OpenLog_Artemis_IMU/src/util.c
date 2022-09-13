#include <stdbool.h> // bool
#include <stdio.h> // printf
#include <string.h> // strerror
#include <errno.h> // errno
#include <stdlib.h> // calloc
#include <time.h>

#include <termios.h> // Contains POSIX terminal control definitions
#include <sys/ioctl.h> // ioctl
											 
#include "util.h"

void log_error(char const *const func_name) {
	printf("Error %d from %s: %s.", errno, func_name, strerror(errno));
  exit(0);
} 

int _kbhit() {
	static const int STDIN = 0;
	static bool initialized = false;

	if (! initialized) {
		// Use termios to turn off line buffering
		struct termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = true;
	}

	int bytesWaiting;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

FILE *create_csv_file() {
  time_t mytime = time(NULL);
  char *time_str = ctime(&mytime);

  char *csv_file_name = malloc(strlen("Logging/") + strlen(time_str) - 1 + strlen(".csv")); 
  
  memcpy(csv_file_name, "Logging/", strlen("Logging/"));
  memcpy(csv_file_name + strlen("Logging/"), time_str, strlen(time_str) - 1);
  memcpy(csv_file_name + strlen("Logging/") + strlen(time_str) - 1, ".csv", strlen(".csv"));

  FILE *file = fopen(csv_file_name, "w");
  char const *categories = "aX, aY, aZ, gX, gY, gZ, mX, mY, mZ, imu_degC, gps_Date, \
                            gps_time, gps_Lat, gps_Long, gps_Alt, gps_SIV, gps_FixType, \
                            gps_GroundSpeed, gps_Heading, gps_pDOP, category, LOI";
  fprintf(file, "%s\n", categories);

  free(csv_file_name);

  return file;
}
