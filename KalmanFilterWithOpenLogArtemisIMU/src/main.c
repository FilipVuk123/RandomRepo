#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include "complementary_filter.h"
#include "orqa_clock.h"
#include "kalman.h"

float KALMAN_PREDICT_MS =  16.0;
float KALMAN_UPDATE_MS  = 100.0;


typedef struct
{
    double yaw, pitch, roll;
} euler_t;

typedef struct
{
    double x, y, z, w;
} quaternion_t;

typedef struct
{
    double ax, ay, az, gx, gy, gz, mx, my, mz;
} IMU_state;

typedef struct
{
    euler_t euler;
    quaternion_t quaternion;
    IMU_state imu_state;
} opengl_cam_t;

int keepRunning = 1;
pthread_mutex_t artemis_mutex = PTHREAD_MUTEX_INITIALIZER;
void intHandler(int dummy)
{
    (void)(dummy);
    keepRunning = 0;
}

void *readDMSfromOpenLogAtremis(void *c_ptr);

int main()
{
    signal(SIGINT, intHandler);

    opengl_cam_t camera;
    camera.imu_state.ax = 0.0;
    camera.imu_state.ay = 0.0;
    camera.imu_state.az = 0.0;
    camera.imu_state.gx = 0.0;
    camera.imu_state.gy = 0.0;
    camera.imu_state.gz = 0.0;
    camera.imu_state.mx = 0.0;
    camera.imu_state.my = 0.0;
    camera.imu_state.mz = 0.0;

    pthread_t artemis_thread;
    pthread_create(&artemis_thread, NULL, readDMSfromOpenLogAtremis, &camera);

    sleep(5);

    comp_filter_t comp_filt;
    comp_filt.phiHat_rad = 0.0f;
    comp_filt.thetaHat_rad = 0.0f;

    orqa_clock_t predict_clock = orqa_time_now();
    orqa_clock_t update_clock = orqa_time_now();

    kalman_data_t kalman;
    float Q[2] = {0.000001, 0.000001}; 
    float R[3] = {0.000011, 0.000011, 0.000011};
    KalmanInit(&kalman, 0.000001f, Q, R);

    printf("Kalman after init: %f, %f, %f, %f, %f, %f, %f, %f, %f\n", kalman.P[0], kalman.P[1], kalman.P[2], kalman.P[3], kalman.Q[0], kalman.Q[1], kalman.R[0], kalman.R[1], kalman.R[2]);

    while (keepRunning)
    {
        // printf("A: %f, %f, %f, G: %f, %f, %f, M: %f, %f, %f\n", 
        //     camera.imu_state.ax, camera.imu_state.ay, camera.imu_state.az, 
        //     camera.imu_state.gx, camera.imu_state.gy, camera.imu_state.gz, 
        //     camera.imu_state.mx, camera.imu_state.my, camera.imu_state.mz);

        // re-map of the IMU axis for EKF  
        double kalman_ax, kalman_ay, kalman_az, kalman_gy, kalman_gx, kalman_gz;
        kalman_ax = -camera.imu_state.ay;
        kalman_ay = -camera.imu_state.ax;
        kalman_az = -camera.imu_state.az;
        kalman_gx = camera.imu_state.gy;
        kalman_gy = camera.imu_state.gx;
        kalman_gz = camera.imu_state.gz;



        // ComplementaryFilterPitchRoll(&comp_filt, 
        //     camera.imu_state.ax, camera.imu_state.ay, camera.imu_state.az,
        //     camera.imu_state.gx, camera.imu_state.gy, camera.imu_state.gz, 0.01666f);
        // printf("Complementary filter: %f, %f\n", comp_filt.phiHat_rad * 180/3.14, comp_filt.thetaHat_rad * 180/3.14);

        if (orqa_get_time_diff_msec(predict_clock, orqa_time_now()) >= KALMAN_PREDICT_MS)
        {
            KalmanPredict(&kalman, kalman_gx, kalman_gy, kalman_gz, KALMAN_PREDICT_MS / 1000.0f);
            predict_clock = orqa_time_now();
        }

        if (orqa_get_time_diff_msec(update_clock, orqa_time_now()) >= KALMAN_UPDATE_MS)
        {
            KalmanUpdate(&kalman, kalman_ax, kalman_ay, kalman_az);
            printf("%f, %f\n", kalman.phi_rad*180/3.14, kalman.theta_rad*180/3.14);
            update_clock = orqa_time_now();
        }
        
        
    }
printf("EXIT OK!\n");
    return 0;
}

void *readDMSfromOpenLogAtremis(void *c_ptr)
{
    opengl_cam_t *cam = (opengl_cam_t *)c_ptr;

    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open("/dev/ttyUSB0", O_RDWR); // Create new termios struc, we call it 'tty' for convention
    if(serial_port < 0){
        printf("Unable to open device! %d\n", serial_port);
    }
    struct termios tty;                             // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        goto exitSerial;
    }
    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST;                                                       // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR;                                                       // Prevent conversion of newline to carriage return/line feed

    // This is a blocking read of any number of chars with a maximum timeout (given by VTIME)
    tty.c_cc[VTIME] = 1; // Wait for up to 1 deciseconds, returning as soon as any data is received
    tty.c_cc[VMIN] = 0;  // if > 0 -> will make read() always wait for bytes (exactly how many is determined by VMIN)

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // Save tty settings, also checking for errors
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        goto exitSerial;
    }
    printf("SERIAL OK!\n");
    // Allocate memory for read buffer, set size according to your needs
    char ch;
    for (int i = 0; i < 50; i++)
    {
        do
        {
            read(serial_port, &ch, sizeof(ch));
        } while (ch != '\n');
    }
    while (keepRunning)
    {

        char aX_buf[10] = "\0";
        char aY_buf[10] = "\0";
        char aZ_buf[10] = "\0";
        char gX_buf[10] = "\0";
        char gY_buf[10] = "\0";
        char gZ_buf[10] = "\0";
        char mX_buf[10] = "\0";
        char mY_buf[10] = "\0";
        char mZ_buf[10] = "\0";

        char ch;
        int b = 0, count = 0;
        while (1)
        {
            read(serial_port, &ch, sizeof(ch));
            if (ch == '\n')
                break;

            if (ch == ',')
            {
                b = 0;
                count++;
                continue;
            }
            if (count == 2)
                aX_buf[b++] = ch;
            else if (count == 3)
                aY_buf[b++] = ch;
            else if (count == 4)
                aZ_buf[b++] = ch;
            else if (count == 5)
                gX_buf[b++] = ch;
            else if (count == 6)
                gY_buf[b++] = ch;
            else if (count == 7)
                gZ_buf[b++] = ch;
            else if (count == 8)
                mX_buf[b++] = ch;
            else if (count == 9)
                mY_buf[b++] = ch;
            else if (count == 10)
                mZ_buf[b++] = ch;
        }
        pthread_mutex_lock(&artemis_mutex);

        cam->imu_state.ax = atof(aX_buf);
        cam->imu_state.ay = atof(aY_buf);
        cam->imu_state.az = atof(aZ_buf);
        cam->imu_state.gx = atof(gX_buf);
        cam->imu_state.gy = atof(gY_buf);
        cam->imu_state.gz = atof(gZ_buf);
        cam->imu_state.mx = atof(mX_buf);
        cam->imu_state.my = atof(mY_buf);
        cam->imu_state.mz = atof(mZ_buf);

        pthread_mutex_unlock(&artemis_mutex);
    }
exitSerial:
    keepRunning = 0;
    close(serial_port);
    printf("Serial port closed!\n\n");
    return NULL; // success
}
