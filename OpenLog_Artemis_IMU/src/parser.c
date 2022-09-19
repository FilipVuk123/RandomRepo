#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // calloc
#include <string.h>
#include <math.h>

#include "parser.h"
#include "state.h"

parser_t *create_parser(void)
{
  parser_t *parser = malloc(sizeof(parser_t));
  parser->state = create_state();
  parser->config = create_config();

  return parser;
}

int parse_line(parser_t *parser)
{
  state_t *state = parser->state;
  imu_config_t *config = parser->config;

  if (state->line_len >= 3 && state->line[state->line_len - 3] == ',')
  {
    if (!config->parsed_sensor_info)
    {
      state->line[state->line_len] = '\0';
      config->parsed_sensor_info = true;
      parse_sensor_info(parser, state->line);
    }
    else
    {
      // state->line is log data
      // printf("%s", state->line);
      parse_log_data(parser, state->line);

      return 1;
    }
  }

  return 0;
}

void parse_sensor_info(parser_t *parser,
                       char const *const sensor_info)
{
  char info[30] = "\n";
  int j = 0;
  for (int i = 0; sensor_info[i] != '\0'; i++)
  {
    if (sensor_info[i] == ',')
    {
      activate_info(info, parser->config);
      memset(info, 0, sizeof info);
      j = 0;
    }
    else
      info[j++] = sensor_info[i];
  }
}

void parse_log_data(parser_t *parser, char *data)
{
  state_t *state = parser->state;

  char *end;

  // parsing date and time MM/DD/YYYY + HH:MM:SS.SSS -> 6 numbers
  for (int i = 0; i < 6; i++)
  {
    strtof(data, &end);
    data = end + 1;
  }

  state->qx = strtof(data, &end);
  data = end + 1;
  state->qy = strtof(data, &end);
  data = end + 1;
  state->qz = strtof(data, &end);
  data = end + 1;

  // IMU part
  if (parser->config->accelerometer_active)
  {
    state->aX = strtof(data, &end);
    data = end + 1;
    state->aY = strtof(data, &end);
    data = end + 1;
    state->aZ = strtof(data, &end);
    data = end + 1;
  }

  if (parser->config->gyroscope_active)
  {
    state->gX = strtof(data, &end);
    data = end + 1;
    state->gY = strtof(data, &end);
    data = end + 1;
    state->gZ = strtof(data, &end);
    data = end + 1;
  }

  if (parser->config->magnetometer_active)
  {
    state->mX = strtof(data, &end);
    data = end + 1;
    state->mY = strtof(data, &end);
    data = end + 1;
    state->mZ = strtof(data, &end);
    data = end + 1;
  }

  if (parser->config->temperature_active)
  {
    state->imu_degC = strtof(data, &end);
    data = end + 1;
  }

  // GPS part
  // parsing date and time MM/DD/YYYY + HH:MM:SS.SSS -> 6 numbers
  for (int i = 0; i < 6; i++)
  {
    strtof(data, &end);
    data = end + 1;
  }

  // parse_float(data, &i, &state->gps_Lat);
  state->gps_Lat = strtof(data, &end);
  data = end + 1;
  state->gps_Lat /= powf(10, 7);
  // parse_float(data, &i, &state->gps_Long);
  state->gps_Long = strtof(data, &end);
  data = end + 1;
  state->gps_Long /= powf(10, 7);
  // parse_float(data, &i, &state->gps_Alt);
  state->gps_Alt = strtof(data, &end);
  state->gps_Alt /= 1000;
  data = end + 1;
  // parse_int(data, &i, &state->gps_SIV);
  state->gps_SIV = strtol(data, &end, 10);
  data = end + 1;
  // parse_int(data, &i, &state->gps_FixType);
  state->gps_FixType = strtol(data, &end, 10);
  data = end + 1;
  // parse_float(data, &i, &state->gps_GroundSpeed);
  state->gps_GroundSpeed = strtof(data, &end);
  state->gps_GroundSpeed /= 1000;
  data = end + 1;
  // parse_float(data, &i, &state->gps_Heading);
  state->gps_Heading = strtof(data, &end);
  state->gps_Heading /= powf(10, 5);
  data = end + 1;
  // parse_float(data, &i, &state->gps_pDOP);
  state->gps_pDOP = strtof(data, &end);
  data = end + 1;

  //
}

void next_log_data(serial_bus_t *bus, parser_t *parser)
{
  state_t *state = parser->state;
  bool log_data_parsed = false;

  while (!log_data_parsed)
  {
    int num_bytes = read_from_bus(bus);

    for (int i = 0; i < num_bytes; i++)
    {
      state->line[state->line_len++] = bus->read_buf[i];

      if (bus->read_buf[i] == '\n')
      {
        if (parse_line(parser))
          log_data_parsed = true;

        memset(state->line, 0, sizeof state->line);
        state->line_len = 0;
      }
    }
  }
}

void log_line_to_csv(FILE *file, parser_t *parser)
{
  imu_config_t *config = parser->config;
  state_t *state = parser->state;

  if (config->accelerometer_active)
    fprintf(file, "%f, %f, %f, ", state->aX, state->aY, state->aZ);
  else
    fprintf(file, "%s, %s, %s, ", "", "", "");

  if (config->gyroscope_active)
    fprintf(file, "%f, %f, %f, ", state->gX, state->gY, state->gZ);
  else
    fprintf(file, "%s, %s, %s, ", "", "", "");

  if (config->magnetometer_active)
    fprintf(file, "%f, %f, %f, ", state->mX, state->mY, state->mZ);
  else
    fprintf(file, "%s, %s, %s, ", "", "", "");

  if (config->temperature_active)
    fprintf(file, "%f, ", state->imu_degC);
  else
    fprintf(file, "%s, ", "");

  fprintf(file, "%s, %s, %f, %f, %f, %d, %d, %f, %f, %f, ",
          state->gps_Date, state->gps_Time, state->gps_Lat, state->gps_Long,
          state->gps_Alt, state->gps_SIV, state->gps_FixType, state->gps_GroundSpeed,
          state->gps_Heading, state->gps_pDOP);

  // category and LOI
  fprintf(file, "%s, %s\n", "unknown", "unknown");

  printf("Writing to csv done\n");
}

void delete_parser(parser_t *parser)
{
  delete_config(parser->config);
  delete_state(parser->state);
  free(parser);
}
