#ifndef KALMAN_H
#define KALMAN_h
#include <math.h>

typedef struct
{
    
} kalman_data_t;

void KalmanInit(kalman_data_t *kalman);
void KalmanPredict(kalman_data_t *kalman);
void KalmanUpdate(kalman_data_t *kalman);

#endif