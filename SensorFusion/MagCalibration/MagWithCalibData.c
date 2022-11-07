#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#define MilliGtoMetarsPerSquareSecond ((float)0.000980665f)

typedef struct
{
    double ax, ay, az, gx, gy, gz, mx, my, mz;
} IMU_state;

int keepRunning = 1;
void intHandler(int dummy)
{
    (void)(dummy);
    keepRunning = 0;
}
double toRadians(double x)
{
    return x * M_PI / 180.0f;
}
double toDegrees(double x)
{
    return x * 180.f / M_PI;
}

float B[] = {18.89, -8.25, -34.08};

float Ainv[][3] = {{1.17236, -0.01105, 0.00045},
                   {-0.01105, 1.14734, -0.00666},
                   {0.00045, -0.00666, 1.17480}};

int main()
{

    signal(SIGINT, intHandler);
    IMU_state imu;
    imu.ax = 0.0;
    imu.ay = 0.0;
    imu.az = 0.0;
    imu.gx = 0.0;
    imu.gy = 0.0;
    imu.gz = 0.0;
    imu.mx = 0.0;
    imu.my = 0.0;
    imu.mz = 0.0;

    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open("/dev/ttyUSB0", O_RDWR); // Create new termios struc, we call it 'tty' for convention
    if (serial_port < 0)
    {
        printf("Unable to open device! %d\n", serial_port);
        goto exitSerial;
    }
    struct termios tty; // Read in existing settings, and handle any error
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

        imu.ax = atof(aX_buf) * MilliGtoMetarsPerSquareSecond;
        imu.ay = atof(aY_buf) * MilliGtoMetarsPerSquareSecond;
        imu.az = atof(aZ_buf) * MilliGtoMetarsPerSquareSecond;
        imu.gx = toRadians(atof(gX_buf));
        imu.gy = toRadians(atof(gY_buf));
        imu.gz = toRadians(atof(gZ_buf));
        imu.mx = atof(mX_buf);
        imu.my = atof(mY_buf);
        imu.mz = atof(mZ_buf);

        float calib_mx, calib_my, calib_mz;

        imu.mx = imu.mx - B[0];
        imu.my = imu.my - B[1];
        imu.mz = imu.mz - B[2];

        calib_mx = Ainv[0][0] * imu.mx + Ainv[0][1] * imu.my + Ainv[0][2] * imu.mz;
        calib_my = Ainv[1][0] * imu.mx + Ainv[1][1] * imu.my + Ainv[1][2] * imu.mz;
        calib_mz = Ainv[2][0] * imu.mx + Ainv[2][1] * imu.my + Ainv[2][2] * imu.mz;

        float heading = toDegrees(atan2(calib_my, calib_mx)) - 90.0f;
        if (heading < 0)
            heading = heading + 360;
        printf("%f\n", heading);
    }
exitSerial:
    close(serial_port);
    fprintf(stdout, "Program exited - status OK\n\n");
    return 0;
}