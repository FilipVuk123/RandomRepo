#ifndef STATE_H
#define STATE_H

#include "bus.h"

typedef struct
{
  /*
   * How much bytes of data has been parsed so far for current line.
   * It resets back to 0 when parsing of new line starts.
   */
  int line_len;

  /* Line being parsed. */
  char line[1000];

  /*
   * IMU logging values, for more details see:
   * https://github.com/sparkfun/OpenLog_Artemis/blob/main/SENSOR_UNITS.md#ICM-20948-IMU
   */

  /* Accelerometer */
  float aX, aY, aZ;

  /* Gyroscope */
  float gX, gY, gZ;

  /* Magnetometer */
  float mX, mY, mZ;

  /* Temperature */
  float imu_degC;

  float qx, qy, qz;

  /*
   * GPS logging values, for more details see:
   * https://github.com/sparkfun/OpenLog_Artemis/blob/main/SENSOR_UNITS.md#u-blox-GNSS-boards
   */

  /*
   * Date string in the format of MM/DD/YYYY.
   * Size is 11 because of an additional '\0' character.
   */
  char gps_Date[11];

  /*
   * Time string in the format of HH:MM:SS.SSS.
   * Size is 13 beause of an additional null-terminating character
   */
  char gps_Time[13];

  /*
   * Latitude and Longitude.
   * It is stored as an angle with 7 digits of precision.
   */
  float gps_Lat, gps_Long;

  /* Altitude */
  float gps_Alt;

  /* This value is not being logged by default. */
  /* float gps_AltMSL; */

  /* Number of satellites in view */
  int gps_SIV;

  /* Fix type */
  int gps_FixType;

  /* This value is not being logged by default. */
  /* int gps_CarrierSolution; */

  /* Ground speed */
  float gps_GroundSpeed;

  /* Heading */
  float gps_Heading;

  /* Positional dilution of precision */
  float gps_pDOP;

  /* This value is not being logged by default. */
  /* float gps_iTOW; */
} state_t;

/*
 * Create the state and set the default values to 0.
 */
state_t *create_state(void);

/*
 * Frees up memory.
 */
void delete_state(state_t *state);

#endif // STATE_H