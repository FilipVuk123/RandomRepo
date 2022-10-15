#ifndef common_h
#define common_h

#define comp_filter_alpha ((float) 0.1)
#define MilliGtoMetarsPerSquareSecond ((float)0.000980665f)

typedef struct 
{
    double x, y, z, w;
} quat_t;

typedef struct 
{
    double yaw, pitch, roll;
} euler_t;



#endif