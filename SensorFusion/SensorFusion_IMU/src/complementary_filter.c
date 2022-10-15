#include "complementary_filter.h"

#include <math.h>

void complementary_filter(quat_t *q, const float ax, const float ay, const float az, const float gx, const float gy, const float gz, const float mx, const float my, const float mz, const float dt){
    float qgx = 0.0, qgy = 0.0, qgz = 0.0, qgw = 1.0;
    float qax = 0.0, qay = 0.0, qaz = 0.0, qaw = 1.0;
    float pitch = 0.0, roll = 0.0, yaw = 0.0;
    
    // First, the current orientation is estimated at time t, from a previous orientation at time t−1, and a given angular velocity, g_xyz, in rad/s.
    qgw = q->w - dt/2 * gx * q->x - dt/2 * gy * q->y - dt/2 * gz * q->z;
    qgx = q->x - dt/2 * gx * q->w - dt/2 * gy * q->z - dt/2 * gz * q->y;
    qgy = q->y - dt/2 * gx * q->z - dt/2 * gy * q->w - dt/2 * gz * q->x;
    qgz = q->z - dt/2 * gx * q->y - dt/2 * gy * q->x - dt/2 * gz * q->w;

    // Secondly, the tilt is computed from the accelerometer measurements as:
    pitch = atan2(ay, az);
    roll = atan2(-ax, sqrt(ay*ay + az*az));

    // yaw = atan(sqrt(ay*ay + ax*ax) / az); this is a meme
    
    if (mx != 0.0, my != 0.0, mz != 0.0){
        yaw = atan2(mz*sin(pitch) - my*cos(pitch), mx*cos(roll) + my*sin(roll)*sin(pitch) + mz*sin(roll)*cos(pitch));
    }
    // We transform the roll-pitch-yaw angles to a quaternion representation
    qaw = cos(pitch/2)*cos(roll/2)*cos(yaw/2) + sin(pitch/2)*sin(roll/2)*sin(yaw/2);
    qax = sin(pitch/2)*cos(roll/2)*cos(yaw/2) - cos(pitch/2)*sin(roll/2)*sin(yaw/2);
    qay = cos(pitch/2)*sin(roll/2)*cos(yaw/2) + sin(pitch/2)*cos(roll/2)*sin(yaw/2);
    qaz = cos(pitch/2)*cos(roll/2)*sin(yaw/2) - sin(pitch/2)*sin(roll/2)*cos(yaw/2);

    // Finally, after each orientation is estimated independently, they are fused with the complementary filter.
    q->w = (1.0 - comp_filter_alpha)*qgw + comp_filter_alpha*qaw;
    q->x = (1.0 - comp_filter_alpha)*qgx + comp_filter_alpha*qax;
    q->y = (1.0 - comp_filter_alpha)*qgy + comp_filter_alpha*qay;
    q->z = (1.0 - comp_filter_alpha)*qgz + comp_filter_alpha*qaz;
}