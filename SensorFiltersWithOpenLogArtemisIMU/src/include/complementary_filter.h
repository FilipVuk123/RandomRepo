#ifndef COMPLEMENTARY_FILTER_H
#define COMPLEMENTARY_FILTER_H

typedef struct
{
    float phiHat_rad;
    float thetaHat_rad;
} comp_filter_t;

void ComplementaryFilterPitchRoll(comp_filter_t *comp_filter, double ax, double ay, double az, double gx, double gy, double gz, float T);

#endif