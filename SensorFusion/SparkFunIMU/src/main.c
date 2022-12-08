#include <stdio.h>
#include "sensor_fusion.h"
#include "MahonyMadgwick.h"
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <string.h>
#include <stdlib.h>
#include <signal.h>


#define MilliG2MetarsPerSquareSecond ((float)0.00980665f)
#define g ((float)9.80665f)

#define WriteToFile 1

int EXIT = 0;

void myHandler()
{
    EXIT = 1;
}

int main()
{
    signal(SIGINT, myHandler);
    float sample_freq = 60.0;
    const char *serial_port_name = "/dev/ttyUSB0";
    const char *raw_data_file_name = "./imu_data.txt";

    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open(serial_port_name, O_RDWR); 
    if (serial_port < 0){
        printf("Error while opening serial port on %s\nExiting program...", serial_port_name);
        return 1;
    }
    
    // Create new termios struc, we call it 'tty' for convention
    // Read in existing settings, and handle any error
    struct termios tty;                               
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("%s: Error %i from tcgetattr: %s\n", serial_port_name, errno, strerror(errno));
        close(serial_port);
        return 1;
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
        printf("%s Error %i from tcsetattr: %s\n", serial_port_name, errno, strerror(errno));
        close(serial_port);
        printf("Serial port closed!\n\n");
        return 1;
    }
    printf("SERIAL OK!\n");

#if WriteToFile
    FILE *imu_data_fptr;
    imu_data_fptr = fopen(raw_data_file_name, "w+");
    fprintf(imu_data_fptr, "Ax, Ay, Az, Gx, Gy, Gz, Mx, My, Mz, MahonyYaw, MahonyPitch, MahonyRoll, MadgwickYaw, MadgwickPitch, MadgwickRoll\n");
#endif

    // prvih 50 linija ne ucitavam jer se na pocetku ispisuje neki default text i podatci nisu najtocniji
    char ch;
    for (int i = 0; i < 50; i++)
    {
        do
        {
            read(serial_port, &ch, sizeof(ch));
        } while (ch != '\n');
    }

    quaternion_t q_zero = initQuat();
    quaternion_t madgwick_quat = initQuat();
    quaternion_t mahony_quat = initQuat();

    quaternion_t final_quat_madgwick;
    quaternion_t final_quat_mahony;

    while (!EXIT)
    {

        char Buf1[12] = "\0";
        char Buf2[12] = "\0";
        char Buf3[12] = "\0";
        char Buf4[12] = "\0";
        char Buf5[12] = "\0";
        char Buf6[12] = "\0";
        char Buf7[12] = "\0";
        char Buf8[12] = "\0";
        char Buf9[12] = "\0";
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
                Buf1[b++] = ch;
            else if (count == 3)
                Buf2[b++] = ch;
            else if (count == 4)
                Buf3[b++] = ch;
            else if (count == 5)
                Buf4[b++] = ch;
            else if (count == 6)
                Buf5[b++] = ch;
            else if (count == 7)
                Buf6[b++] = ch;
            else if (count == 8)
                Buf7[b++] = ch;
            else if (count == 9)
                Buf8[b++] = ch;
            else if (count == 10)
                Buf9[b++] = ch;
        }

        float ax, ay, az, gx, gy, gz, mx, my, mz;

        // convert u potrebne mjerne jedinice
        ax = atof(Buf1) * MilliG2MetarsPerSquareSecond;
        ay = atof(Buf2) * MilliG2MetarsPerSquareSecond;
        az = atof(Buf3) * MilliG2MetarsPerSquareSecond;
        gx = toRadians(atof(Buf4));
        gy = toRadians(atof(Buf5));
        gz = toRadians(atof(Buf6));
        mx = atof(Buf7);
        my = atof(Buf8);
        mz = atof(Buf9);

#if WriteToFile

        fprintf(imu_data_fptr, "%f, %f, %f, %f, %f, %f, %f, %f, %f", ax, ay, az, gx, gy, gz, mx, my, mz);
#endif

        MahonyUpdate(&mahony_quat, gx, gy, gz, ax, ay, az, mx, my, mz, sample_freq);
        final_quat_mahony = hamilton_quaternions(q_zero, mahony_quat); // vraca novi quaternion ovisno o zeropointu

        MadgwickUpdate(&madgwick_quat, gx, gy, gz, ax, ay, az, mx, my, mz, sample_freq);
        final_quat_madgwick = hamilton_quaternions(q_zero, madgwick_quat); // vraca novi quaternion ovisno o zeropointu

        // Inace se koristi pri zero poitnu
        // if (set_zero_point_bool == 1)
        // {
        //     set_zero_point(&q_zero, &madgwick_quat);
        //     set_zero_point(&q_zero, &mahony_quat);
        // }

        euler_angles_t eulerMahony = quatToEuler(final_quat_mahony);
#if WriteToFile
        fprintf(imu_data_fptr, "%f, %f, %f, ", eulerMahony.yaw, eulerMahony.pitch, eulerMahony.roll);
#endif
        euler_angles_t eulerMadgwick = quatToEuler(final_quat_madgwick);
#if WriteToFile
        fprintf(imu_data_fptr, "%f, %f, %f, ", eulerMadgwick.yaw, eulerMadgwick.pitch, eulerMadgwick.roll);
#endif


    printf("Mahony: %f, %f, %f, Madgwick: %f, %f, %f\n", eulerMahony.yaw, eulerMahony.pitch, eulerMahony.roll, eulerMadgwick.yaw, eulerMadgwick.pitch, eulerMadgwick.roll);

    }
    fclose(imu_data_fptr);
    close(serial_port);
    printf("Program exit OK!\n");
    return 0;
}