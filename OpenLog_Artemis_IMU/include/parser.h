#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

#include "bus.h"
#include "config.h"
#include "state.h"

typedef struct
{
  /* Config used to track which IMU sensor are we parsing. */
  imu_config_t *config;

  /* Object used to store values of parsed logged data. */
  state_t *state;
} parser_t;

parser_t *create_parser(void);

/*
 * This function is called whenever a '\n' is read. This means that
 * current line (inside state_t object which is inside the parser_t object)
 * is ready to be parsed.
 *
 * Sensor logged values always have ',' at the end of the line, with exception of
 * the list of sensors that are active. This list is sent only once on startup so
 * we need to parse it only once.
 *
 * If the line thats parser are sensor logged values then this function return 1,
 * and otherwise returns 0.
 */
int parse_line(parser_t *parser);

/*
 * This function is called only once on startup to configure the IMU values that
 * are active from the last session.
 */
void parse_sensor_info(parser_t *parser, char const *const sensor_info);

/*
 * Parses sensor logged data seperated by ',' and stores values inside
 * state_t object inside parser_t that is passed as a parameter.
 *
 * First two values are not important as they are local date and time of IMU.
 * We use correct date and time of GPS.
 */
void parse_log_data(parser_t *parser, char *data);

/*
 * This function will keep reading from the bus until it parses a line
 * of sensor values. Values will then be stored inside state_t struct
 * inside parser and can be freely used.
 */
void next_log_data(serial_bus_t *bus, parser_t *parser);

/*
 * Logs a parsed line whose values are stored in parser->state.
 */
void log_line_to_csv(FILE *file, parser_t *parser);

void delete_parser(parser_t *parser);

#endif // PARSER_H
