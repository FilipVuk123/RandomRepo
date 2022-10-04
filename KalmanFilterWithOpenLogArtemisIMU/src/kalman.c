#include "include/kalman.h"

#define g ((float) 9.81f)

void KalmanInit(kalman_data_t *kal, float Pinit, float *Q, float *R)
{
    kal->phi_rad = 0.0f;
    kal->theta_rad = 0.0f;
    kal->P[0] = Pinit;
    kal->P[1] = 0.0f;
    kal->P[2] = 0.0f;
    kal->P[3] = Pinit;
    kal->Q[0] = Q[0];
    kal->Q[1] = Q[1];
    kal->R[0] = R[0];
    kal->R[1] = R[1];
    kal->R[2] = R[2];
}

void KalmanPredict(kalman_data_t *kal, float gx, float gy, float gz, float T)
{
    float p = gx;
    float q = gy;
    float r = gz;

    float sp = sin(kal->phi_rad);
    float cp = cos(kal->phi_rad);
    float tt = tan(kal->theta_rad);

    // x = x + T f(x, u)
    kal->phi_rad += T * (p + tt * (q * sp + r * cp));
    kal->theta_rad += T * (q * cp - r * sp);

    sp = sin(kal->phi_rad);
    cp = cos(kal->phi_rad);
    float st = sin(kal->theta_rad);
    float ct = cos(kal->theta_rad);
    tt = st / ct;

    // jacobian f(x, u)
    float A[4] = {
        tt * (q * cp - r * sp),
        (r * cp + q * sp) * (tt * tt + 1.0f),
        -(r * cp + q * sp),
        0.0f};

    // P = P + T * (A*P + P*A' + Q)
    float Ptmp[4] = {
        T * (kal->Q[0] + 2.0f * A[0] * kal->P[0] + A[1] * kal->P[1] + A[1] * kal->P[2]),
        T * (kal->P[1] * A[0] + A[2] * kal->P[0] + A[1] * kal->P[3] + A[3] * kal->P[1]),
        T * (kal->P[2] * A[0] + A[2] * kal->P[0] + A[1] * kal->P[3] + A[3] * kal->P[2]),
        T * (kal->Q[1] + A[2] * kal->P[1] + A[2] * kal->P[2] + 2.0f * A[3] * kal->P[3])
    };

    kal->P[0] += Ptmp[0];
    kal->P[1] += Ptmp[1];
    kal->P[2] += Ptmp[2];
    kal->P[3] += Ptmp[3];
}

void KalmanUpdate(kalman_data_t *kal, float ax, float ay, float az)
{
    float sp = sin(kal->phi_rad);
    float cp = cos(kal->phi_rad);
    float st = sin(kal->theta_rad);
    float ct = cos(kal->theta_rad);

    // h(x, u) output
    float h[3] = {
        g * st,
        -g * ct * sp,
        -g * ct * cp
    };
    // jacobian h -> 2x3
    float C[6] = {
        0.0f,         g * ct,
        -g * cp * ct, g * sp * st,
        g * sp * ct,  g * cp * st
    };
    // kalman gain? K = P*C' * inv(C*P*C' + R)
    float PC_T[6] = {
        kal->P[1] *C[1], (kal->P[1]*C[3] - kal->P[0]*C[4]), (kal->P[0] *C[4] + kal->P[1]*C[5]),
        kal->P[3] *C[1], (kal->P[3]*C[3] - kal->P[2]*C[4]), (kal->P[2] *C[4] + kal->P[3]*C[5])
    };
    float C_P[6] = {
        kal->P[2]*C[1],                        C[1]*kal->P[3],
        (kal->P[2]*C[3] + kal->P[0]*C[2]), (kal->P[3]*C[3] + kal->P[1]*C[2]),
        (kal->P[0]*C[4] + kal->P[2]*C[5]), (kal->P[1]*C[4] + kal->P[3]*C[5])
    };
    float CPC_T[9] = {
        C_P[1] *C[1], (C_P[1] *C[3] + C[2]*C_P[0]), (C_P[0]*C[4] + C_P[1]*C[5]),
        C_P[3] *C[1], (C_P[3] *C[3] + C[2]*C_P[2]), (C_P[2]*C[4] + C_P[3]*C[5]),
        C_P[5] *C[1], (C_P[5] *C[3] + C[2]*C_P[4]), (C_P[4]*C[4] + C_P[5]*C[5])
    };
    float to_inv[9] = {
        CPC_T[0] + kal->R[0], CPC_T[1] + kal->R[1], CPC_T[2] + kal->R[2],
        CPC_T[3] + kal->R[3], CPC_T[4] + kal->R[4], CPC_T[5] + kal->R[5],
        CPC_T[6] + kal->R[6], CPC_T[7] + kal->R[7], CPC_T[8] + kal->R[8] 
    };

    float det = 1.0f/(to_inv[0]*to_inv[4]*to_inv[8] - to_inv[0]*to_inv[5]*to_inv[7] - to_inv[1]*to_inv[3]*to_inv[8] + to_inv[1]*to_inv[5]*to_inv[6] + to_inv[2]*to_inv[3]*to_inv[7] - to_inv[2]*to_inv[4]*to_inv[6]);
    
    float inv [9] = {
         det * (to_inv[4]*to_inv[8] - to_inv[5]*to_inv[7]), -det * (to_inv[1]*to_inv[8] - to_inv[2]*to_inv[7]),  det * (to_inv[1]*to_inv[5] - to_inv[2]*to_inv[4]),
        -det * (to_inv[3]*to_inv[8] - to_inv[5]*to_inv[6]),  det * (to_inv[0]*to_inv[8] - to_inv[2]*to_inv[6]), -det * (to_inv[0]*to_inv[5] - to_inv[2]*to_inv[3]),
         det * (to_inv[3]*to_inv[7] - to_inv[4]*to_inv[6]), -det * (to_inv[0]*to_inv[7] - to_inv[1]*to_inv[6]),  det * (to_inv[0]*to_inv[4] - to_inv[1]*to_inv[3])
    };

    float K[6] = {
        PC_T[0] * inv[0] + PC_T[1] * inv[3] + PC_T[2] * inv[6], PC_T[0] * inv[1] + PC_T[1] * inv[4] + PC_T[2] * inv[7], PC_T[0] * inv[2] + PC_T[1] * inv[5] + PC_T[2] * inv[8], 
        PC_T[3] * inv[0] + PC_T[4] * inv[3] + PC_T[5] * inv[6], PC_T[3] * inv[2] + PC_T[5] * inv[5] + PC_T[5] * inv[7], PC_T[3] * inv[2] + PC_T[4] * inv[5] + PC_T[5] * inv[8]
    };
    

    // P = (I - K * C) * P
    // x = x + K (y - h), y = ax ay az
    kal->phi_rad +=   K[0] * (ax - h[0]) + K[1] * (ay - h[1]) + K[2] * (az - h[2]);
    kal->theta_rad += K[3] * (ax - h[0]) + K[4] * (ay - h[1]) + K[5] * (az - h[2]);
} 
