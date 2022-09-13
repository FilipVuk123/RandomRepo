#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

/*
 * Prints func_name to the console with error code and exits program.
 */ 
void log_error(char const *const func_name);
int _kbhit();
FILE *create_csv_file();

#endif // UTIL_H
