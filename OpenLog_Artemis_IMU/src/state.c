#include <stdlib.h> // malloc
#include <stdbool.h>

#include "state.h"
#include "parser.h"

state_t *create_state(void) {
  state_t *state = malloc(sizeof(state_t));

  state->line_len = 0;

  state->aX = state->aY = state->aZ = 0.0;
  state->gX = state->gY = state->gZ = 0.0;
  state->mX = state->mY = state->mZ = 0.0;
  state->imu_degC = 0.0;

  state->gps_Lat = state->gps_Long = 0.0;
  state->gps_Alt = 0.0;
  //  float gps_AltMSL;
  state->gps_SIV = 0;
  state->gps_FixType = 0;
  //  int gps_CarrierSolution;
  state->gps_GroundSpeed = 0.0;
  state->gps_Heading = 0.0;
  state->gps_pDOP = 0.0;

  return state;
}

void delete_state(state_t *state) {
  free(state);
}