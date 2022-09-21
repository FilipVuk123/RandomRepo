#define STB_IMAGE_IMPLEMENTATION


extern "C"
{
    #include <pthread.h>
// C library headers
#include <stdio.h>   // printf
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <stdbool.h> // bool
#include "bus.h"
#include "util.h"
#include "config.h"
#include "parser.h"
#include "state.h"
#include "sensor_fusion.h"
#include "orqa_vegvisir_common.h"
#include <signal.h>
#include <unistd.h>
#include <math.h>
}

#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

static volatile int keepRunning = 1;

void intHandler(int dummy)
{
    (void)(dummy);
    keepRunning = 0;
}

const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

typedef struct
{
    euler_angles_t resultEuler;
    quaternion_t __resultQuat;
    char* serial_port_name;
} opengl_cam_t;

void *readDMSfromOpenLogAtremis(void *c_ptr)
{
    opengl_cam_t *cam = (opengl_cam_t *)c_ptr;

    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    int serial_port = open(cam->serial_port_name, O_RDWR); // Create new termios struc, we call it 'tty' for convention
    struct termios tty;                             // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return NULL;
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
        close(serial_port);
        printf("Serial port closed!\n\n");
        return NULL;
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

    int ccc = 0;

    quaternion_t q_zero = createQuat();
    quaternion_t quat = createQuat();
    printf("Before while!\n");

    while (keepRunning)
    {
        char q1Buf[12] = "\0";
        char q2Buf[12] = "\0";
        char q3Buf[12] = "\0";
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
                q1Buf[b++] = ch;
            else if (count == 3)
                q2Buf[b++] = ch;
            else if (count == 4)
                q3Buf[b++] = ch;
        }
        float q1, q2, q3;
        q1 = atof(q1Buf);
        q2 = atof(q2Buf);
        q3 = atof(q3Buf);

        quat = getQuat(q1, q2, q3);
        cam->__resultQuat = hamilton_quaternions(q_zero, quat);

        if (ccc++ % 300 == 1)
        {
            printf("ZERO POINT!!!\n");
            set_zero_point(&q_zero, &quat);
        }

        cam->resultEuler = quatToEuler(cam->__resultQuat);
        // printEuler(cam->resultEuler);
    }
    close(serial_port);
    printf("Serial port closed!\n\n");
    return NULL; // success
}

int main()
{
    signal(SIGINT, intHandler);

    /**********************************************************************************************************************/
    orqa_set_error_cb(orqa_error_cb);

    if (orqa_init_glfw(3, 3))
        return -1;
    orqa_GLFW_make_window_full_screen();                                                                 // Full screen
    GLFWwindow *window = orqa_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "Vegvisir Project", NULL, NULL); // glfw window object creation
    orqa_set_input_mode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);                                      // use cursor but do not display it
    if (window == NULL)
        return -1;
    orqa_make_window_current(window);

    orqa_set_frame_buffer_cb(window, orqa_framebuffer_size_callback); // manipulate view port
    orqa_set_cursor_position_cb(window, orqa_mouse_callback);         // move camera_t with cursor
    orqa_set_scroll_cb(window, orqa_scroll_callback);                 // zoom in/out using mouse wheel

    if (!orqa_load_glad((GLADloadproc)orqa_get_proc_address))
    { // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return -1;
    }

    // mash generation
    orqa_window_t lr = orqa_create_window(1.0, 40, 20, -0.7, -0.5, 0.55);
    orqa_window_t rr = orqa_create_window(1.0, 40, 20, 0.7, -0.5, 0.55);
    orqa_window_t DSS1 = orqa_create_window(1.0, 50, 25, -0.7, 0, 0.64);
    orqa_window_t DSS2 = orqa_create_window(1.0, 50, 25, 0, 0, 1);
    orqa_window_t DSS3 = orqa_create_window(1.0, 50, 25, 0.7, 0, 0.64);
    orqa_window_t mr = orqa_create_window(1.0, 35, 20, 0, -0.32, 0.5);
    orqa_window_t BW = orqa_create_window(1.0, 35, 20, 0, 0.6, 0.65);
    orqa_window_t MRSS = orqa_create_window(1.0, 130, 60, 0, 0, 1);
    orqa_sphere_t sph = orqa_create_sphere(1.0, 150, 150);

    // shader init, compilation and linking
    GLuint shaders[2];

    shaders[0] = orqa_load_shader_from_file("./shaders/vertexShader.vert", GL_VERTEX_SHADER);
    shaders[1] = orqa_load_shader_from_file("./shaders/fragmentShader.frag", GL_FRAGMENT_SHADER);
    GLuint shaderProgram = orqa_create_program(shaders, 2);
    orqa_use_program(shaderProgram);

    // get indexes for shader variables
    GLuint posLoc = orqa_get_attrib_location(shaderProgram, "aPos");
    GLuint texLoc = orqa_get_attrib_location(shaderProgram, "aTexCoord");

    // init & binding array & buffer objects
    GLuint *VAOs = orqa_generate_VAOs(10);
    GLuint *VBOs = orqa_generate_VBOs(10);
    GLuint *EBOs = orqa_generate_EBOs(10);

    orqa_bind_VAOs(VAOs[0]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[0], rr.numVertices * sizeof(float), rr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[0], rr.numTriangles * sizeof(int), rr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[1]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[1], lr.numVertices * sizeof(float), lr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[1], lr.numTriangles * sizeof(int), lr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[2]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[2], DSS1.numVertices * sizeof(float), DSS1.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[2], DSS1.numTriangles * sizeof(int), DSS1.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[3]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[3], DSS2.numVertices * sizeof(float), DSS2.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[3], DSS2.numTriangles * sizeof(int), DSS2.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[4]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[4], DSS3.numVertices * sizeof(float), DSS3.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[4], DSS3.numTriangles * sizeof(int), DSS3.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[5]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[5], BW.numVertices * sizeof(float), BW.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[5], BW.numTriangles * sizeof(int), BW.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[6]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[6], mr.numVertices * sizeof(float), mr.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[6], mr.numTriangles * sizeof(int), mr.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[7]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[7], MRSS.numVertices * sizeof(float), MRSS.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[7], MRSS.numTriangles * sizeof(int), MRSS.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    orqa_bind_VAOs(VAOs[8]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[8], sph.numVertices * sizeof(float), sph.Vs, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[8], sph.numTriangles * sizeof(int), sph.Is, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    // texture init
    GLuint *textures = orqa_create_textures(5);
    // loading image!

    orqa_bind_texture(textures[0]);
    orqa_load_texture_from_file("./data/MRSS.png");
    orqa_bind_texture(textures[1]);
    orqa_load_texture_from_file("./data/DSS.png");
    orqa_bind_texture(textures[2]);
    orqa_load_texture_from_file("./data/pic.bmp");
    orqa_bind_texture(textures[3]);
    orqa_load_texture_from_file("./data/panorama1.bmp");

    // camera init
    orqa_camera_t cam;
    cam.cameraPos[0] = 0.0f;
    cam.cameraPos[1] = 0.0f;
    cam.cameraPos[2] = 0.0f;
    cam.resultQuat[0] = 0.0f;
    cam.resultQuat[1] = 0.0f;
    cam.resultQuat[2] = 0.0f;
    cam.resultQuat[3] = 1.0f;
    cam.yaw = 0.0;
    cam.pitch = 0.0;
    cam.roll = 0.0;
    cam.fov = 5.4f;
    cam.serial_port_name = "/dev/ttyUSB0";
    orqa_set_window_user_pointer(window, &cam); // sent camera object to callback functions

    orqa_camera_t cam1;
    cam1.cameraPos[0] = 0.0f;
    cam1.cameraPos[1] = 0.0f;
    cam1.cameraPos[2] = 0.0f;
    cam1.resultQuat[0] = 0.0f;
    cam1.resultQuat[1] = 0.0f;
    cam1.resultQuat[2] = 0.0f;
    cam1.resultQuat[3] = 1.0f;
    cam1.fov = 5.4f;
    cam1.yaw = 0.0;
    cam1.pitch = 0.0;
    cam1.roll = 0.0;
    cam1.serial_port_name = "/dev/ttyUSB1";

    // MVP matrices init
    mat4 model, proj, view;
    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(proj);

    // get MVP shader indexes
    GLuint modelLoc = orqa_get_uniform_location(shaderProgram, "model");
    GLuint viewLoc = orqa_get_uniform_location(shaderProgram, "view");
    GLuint projLoc = orqa_get_uniform_location(shaderProgram, "proj");

    opengl_cam_t opengl_cam;
    opengl_cam.serial_port_name = "/dev/ttyUSB0";

    // pthread_t goggles_ht;
    // pthread_create(&goggles_ht, NULL, orqa_read_from_serial, &cam);

    pthread_t imu_ht;
    pthread_create(&imu_ht, NULL, readDMSfromOpenLogAtremis, &opengl_cam);

    // pthread_t imu_ht;
    // pthread_create(&imu_ht, NULL, orqa_read_from_serial, &cam1);

    versor pitchQuat, rollQuat, yawQuat, tmpQuat;

    vec3 vec3_pitch = {1.0f, 0.0f, 0.0f};
    vec3 vec3_yaw = {0.0f, 1.0f, 0.0f};
    vec3 vec3_roll = {0.0f, 0.0f, 1.0f};

    cv::VideoCapture cap(0);

    if (!cap.isOpened())
    {
        printf("Could not open camera!\n");
        return 1;
    }

    while (keepRunning)
    {
        // input
        orqa_process_input(window);
        cv::Mat frame;
        cap >> frame;

        // render
        orqa_clear_color_buffer(0.2f, 0.2f, 0.2f, 1.0f);
        orqa_clear_buffer(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);

        // generate projection matrix
        glm_perspective(cam.fov, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.01f, 100.0f, proj); // zoom

        glm_quatv(pitchQuat, orqa_radians(- cam.pitch + opengl_cam.resultEuler.pitch), vec3_pitch);
        glm_quatv(yawQuat, orqa_radians(- cam.yaw + opengl_cam.resultEuler.yaw + 180.0), vec3_yaw);
        glm_quatv(rollQuat, orqa_radians(cam.roll - opengl_cam.resultEuler.roll), vec3_roll);

        // glm_quatv(pitchQuat, orqa_radians(cam.pitch - cam1.pitch), vec3_pitch);
        // glm_quatv(yawQuat, orqa_radians(cam.yaw - cam1.yaw + 180.0), vec3_yaw);
        // glm_quatv(rollQuat, orqa_radians(cam.roll - cam1.roll), vec3_roll);

        // printf("Goggles: %f, %f, %f\n", cam.yaw, cam.pitch, cam.roll);
        printf("IMU: %f, %f, %f\n", opengl_cam.resultEuler.yaw, opengl_cam.resultEuler.pitch, opengl_cam.resultEuler.roll);

        glm_quat_mul(yawQuat, pitchQuat, tmpQuat);
        glm_quat_mul(tmpQuat, rollQuat, cam.resultQuat);

        glm_quat_look(cam.cameraPos, cam.resultQuat, view);
        // printf("%f, %f, %f, %f\n", cam.resultQuat[0], cam.resultQuat[1], cam.resultQuat[2], cam.resultQuat[3]);

        // send MVP matrices to vertex shader
        orqa_send_shander_4x4_matrix(modelLoc, 1, &model[0][0]);
        orqa_send_shander_4x4_matrix(viewLoc, 1, &view[0][0]);
        orqa_send_shander_4x4_matrix(projLoc, 1, &proj[0][0]);

        // DSS
        orqa_bind_texture(textures[0]);
        orqa_bind_vertex_object_and_draw_it(VAOs[0], GL_TRIANGLES, rr.numTriangles);
        orqa_bind_vertex_object_and_draw_it(VAOs[1], GL_TRIANGLES, lr.numTriangles);

        orqa_bind_texture(textures[2]);
        orqa_bind_vertex_object_and_draw_it(VAOs[5], GL_TRIANGLES, BW.numTriangles);
        orqa_bind_vertex_object_and_draw_it(VAOs[6], GL_TRIANGLES, mr.numTriangles);

        orqa_bind_texture(textures[1]);
        orqa_bind_vertex_object_and_draw_it(VAOs[2], GL_TRIANGLES, DSS1.numTriangles);
        orqa_bind_vertex_object_and_draw_it(VAOs[4], GL_TRIANGLES, DSS3.numTriangles);

        orqa_bind_texture(textures[3]);
        orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_RGB, frame.size().width, frame.size().height, GL_BGR, GL_UNSIGNED_BYTE, frame.ptr());
        orqa_bind_vertex_object_and_draw_it(VAOs[3], GL_TRIANGLES, DSS2.numTriangles);
        // printf("\r Render FPS: %f", 1000/orqa_get_time_diff_msec(clock, orqa_time_now()));

        // glfw: swap buffers and poll IO events
        orqa_swap_buffers(window);
        orqa_pool_events();
    }
    orqa_window_free(&lr);
    orqa_window_free(&rr);
    orqa_window_free(&DSS1);
    orqa_window_free(&DSS2);
    orqa_window_free(&DSS3);
    orqa_window_free(&mr);
    orqa_window_free(&BW);
    orqa_window_free(&MRSS);
    orqa_sphere_free(&sph);
    orqa_delete_VAOs(10, VAOs);
    orqa_delete_buffers(10, VBOs);
    orqa_delete_buffers(10, EBOs);
    orqa_delete_textures(5, textures);
    orqa_delete_program(shaderProgram);

    glfwTerminate();

    // fclose(csv_file);

    printf("Exit OK!\n");

    return 0;
}
