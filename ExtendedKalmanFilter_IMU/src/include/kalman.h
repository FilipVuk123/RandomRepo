#ifndef KALMAN_H
#define KALMAN_h
#include <math.h>

typedef struct
{
    float phi_rad, theta_rad;
    float P[4], Q[2], R[3];
} kalman_data_t;

void KalmanInit(kalman_data_t *kalman, float Pinit, float *Q, float *R);
void KalmanPredict(kalman_data_t *kalman, float gx, float gy, float gz, float T);
void KalmanUpdate(kalman_data_t *kalman, float ax, float ay, float az);

#endif