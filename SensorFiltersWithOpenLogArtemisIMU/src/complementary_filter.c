#include "include/complementary_filter.h"
#include <math.h>

#define g ((float)9.81f)
#define COMP_FILT_ALPHA ((float)0.90f)

void ComplementaryFilterPitchRoll(comp_filter_t *comp_filter, double ax, double ay, double az, double gx, double gy, double gz, float T)
{
    float phiHat_acc_rad = atan(ay / az);
    float thetaHat_acc_rad = asin(ax / g / 1000);
    float phiDot_rps = gx + tan(comp_filter->thetaHat_rad) * (sin(comp_filter->phiHat_rad) * gy + cos(comp_filter->phiHat_rad) * gz);
    float thetaDot_rps = cos(comp_filter->phiHat_rad) * gx - sin(comp_filter->phiHat_rad) * gz;
    comp_filter->phiHat_rad = COMP_FILT_ALPHA * phiHat_acc_rad + (1.0f - COMP_FILT_ALPHA) * (comp_filter->phiHat_rad + T * phiDot_rps);
    comp_filter->thetaHat_rad = COMP_FILT_ALPHA * thetaHat_acc_rad + (1.0f - COMP_FILT_ALPHA) * (comp_filter->thetaHat_rad + T * thetaDot_rps);
}