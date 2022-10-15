#ifndef complementary_filter_h
#define complementary_filter_h

#include "common.h"

void complementary_filter(quat_t *q, const float ax, const float ay, const float az, const float gx, const float gy, const float gz, const float mx, const float my, const float mz, const float dt);

#endif