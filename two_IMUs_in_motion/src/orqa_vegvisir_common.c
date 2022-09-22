#include "orqa_vegvisir_common.h"

#define BUFSIZE 1024
#define HEADTRACKING_BUFFER_SIZE 64
#define PORT 8000

volatile int toEXIT = 0;

// The User-defined method
static int str_length(char str[]) {
    // initializing count variable (stores the length of the string)
    int count; 
    
    // incrementing the count till the end of the string
    for (count = 0; str[count] != '\0'; ++count);
    
    // returning the character count of the string
    return count; 
}

/// This function keeps track all the input code.
/// Moves between DSS, MRSS and 360 modules using 'D', 'M' or '3' keys
void orqa_process_input(GLFWwindow *window)
{
    // keeps all the input code
    if (orqa_get_key(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        toEXIT = 1;
        orqa_sleep(ORQA_SLEEP_SEC, 1);
        glfwSetWindowShouldClose(window, true);
    }
}

/// This function converts radians from degrees.
/// Returns radians in float.
float orqa_radians(const float deg)
{
    return (deg * M_PI / 180.0f); // calculate radians
}
/// This function connects to ORQA FPV.One goggles via UDP socket and performs motorless gimbal while goggles are in use.
void *orqa_udp_thread(void *c_ptr)
{
    // inits
    fprintf(stderr, "In thread\n");
    orqa_camera_t *c = c_ptr;
    char buf[BUFSIZE];
    mat4 rollMat;
    glm_mat4_identity(rollMat);
    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat);
    glm_quat_identity(yawQuat);
    glm_quat_identity(pitchQuat);
    struct sockaddr_in serveraddr;
    int s, recv_len, optval = 1;

    
    // FILE *fptr;
    // fptr = fopen("UDP_headtracking_latency.txt", "w");
    // if (fptr == NULL) {
    //     printf("Error!");
    //     return NULL;
    // }

    // create a UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("socket init failed\n");
        return NULL;
    }
    else
        printf("Socket created!\n");

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    memset((char *)&serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket to port
    if (bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        printf("Binding error!\n");
        goto exit;
    }
    printf("Bind done!\n");
    while (1)
    {
        bzero(buf, BUFSIZE);
        char yawBuf[12] = "\0";
        char pitchBuf[12] = "\0";
        char rollBuf[12] = "\0";
        int b = 0, count = 0;
        if ((recv_len = recv(s, buf, BUFSIZE, 0)) < 0)
        {
            printf("Recieving error!\n");
            break;
        }
        printf("%s\n", buf);
        if (toEXIT) goto exit;
        for(int i = 0; i < BUFSIZE; i++){
            const char ch = buf[i];

            if (ch == ';')
                break;

            if (ch == ',')
            {
                b = 0;
                count++;
                continue;
            }
            if (count == 0)
                yawBuf[b++] = ch;
            else if (count == 1)
                pitchBuf[b++] = ch;
            else
                rollBuf[b++] = ch;
            if (toEXIT)
                return NULL;
        }
        float pitch, yaw, roll;
        pitch = -atof(pitchBuf);
        yaw = atof(yawBuf);
        roll = -atof(rollBuf);
        // Using quaternions to calculate camera rotations
        glm_quatv(pitchQuat, orqa_radians(pitch), (vec3){1.0f, 0.0f, 0.0f});
        glm_quatv(yawQuat, orqa_radians(yaw), (vec3){0.0f, 1.0f, 0.0f});
        glm_quatv(rollQuat, orqa_radians(roll), (vec3){0.0f, 0.0f, 1.0f});

        glm_quat_mul(yawQuat, pitchQuat, c->resultQuat);
        glm_quat_mul(c->resultQuat, rollQuat, c->resultQuat);
    }
exit:
    // fclose(fptr);
    close(s);
    return NULL;
}

void *orqa_read_from_serial(void *c_ptr)
{
    orqa_camera_t *c = c_ptr;

    versor rollQuat, pitchQuat, yawQuat;
    glm_quat_identity(rollQuat);
    glm_quat_identity(yawQuat);
    glm_quat_identity(pitchQuat);

    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open(c->serial_port_name, O_RDWR); // Create new termios struc, we call it 'tty' for convention
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
    do
    {
        read(serial_port, &ch, sizeof(ch));
    } while (ch != ';');
    char yawBuf[12] = "\0";
    char pitchBuf[12] = "\0";
    char rollBuf[12] = "\0";
    while (1)
    {
        if (toEXIT)
            goto exitSerial;
        char headTrackingBuffer[32] = "\0";
        orqa_clock_t clock1 = orqa_time_now();
        orqa_sleep(ORQA_SLEEP_MSEC, 5);
        read(serial_port, &headTrackingBuffer, sizeof(headTrackingBuffer));
        // printf("Buffer: %s\n", headTrackingBuffer);
        // printf("Time: %f\n", orqa_get_time_diff_msec(clock1, orqa_time_now()));

        int b = 0, count = 0;

        for (int i = 0; i < HEADTRACKING_BUFFER_SIZE - 1; i++)
        {
            const char ch = headTrackingBuffer[i];

            if (ch == ';')
                break;

            if (ch == ',')
            {
                b = 0;
                count++;
                continue;
            }
            if (count == 0)
                yawBuf[b++] = ch;
            else if (count == 1)
                pitchBuf[b++] = ch;
            else if(count == 2)
                rollBuf[b++] = ch;
            if (toEXIT)
                return NULL;
        }
        if( str_length(pitchBuf) == 0 || str_length(yawBuf) == 0 || str_length(rollBuf) == 0){
            continue;
        }
        float yaw, pitch, roll;
        yaw = atof(yawBuf);
        pitch = -atof(pitchBuf);
        roll = atof(rollBuf);
        if(fabs(yaw) < 1.00 && fabs(pitch) < 1.0 && fabs(roll) < 1.0){
            set_zero_point_bool = 1;
        }
        c->yaw = yaw;
        c->pitch = pitch;
        c->roll = roll;

        glm_quatv(pitchQuat, orqa_radians(pitch), (vec3){1.0f, 0.0f, 0.0f});
        glm_quatv(yawQuat, orqa_radians(yaw), (vec3){0.0f, 1.0f, 0.0f});
        glm_quatv(rollQuat, orqa_radians(roll), (vec3){0.0f, 0.0f, 1.0f});

        glm_quat_mul(yawQuat, pitchQuat, c->resultQuat);
        glm_quat_mul(c->resultQuat, rollQuat, c->resultQuat);
    }
exitSerial:
    close(serial_port);
    printf("Serial port closed!\n\n");
    return NULL; // success
}