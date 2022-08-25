extern "C"
{
#include "orqa_clock.h"
#include "orqa_opengl.h"
#include "orqa_window.h"
}
#include <stdio.h>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <signal.h>

using namespace std;
using namespace cv;

// screen resolution
const GLuint SCR_WIDTH = 1920;
const GLuint SCR_HEIGHT = 1080;

const GLuint width = 960;
const GLuint height = 540;

const GLfloat vertices[] = {
    // pisitions         // texture coords
    1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right vertex
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right vertex
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left vertex
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left vertex
};
const GLuint indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

static volatile int keepRunning = 1;

void intHandler(int dummy)
{
    keepRunning = 0;
}

int main(int argc, const char **argv)
{
    signal(SIGINT, intHandler);

    double scale = 4.0;
    VideoCapture cap(0);

    if (!cap.isOpened())
    {
        printf("Could not open camera!\n");
        return 1;
    }

    CascadeClassifier cascade;
    cascade.load("home/filip/opencv-4.x/data/haarcascades/haarcascade_fullbody.xml");

    orqa_set_error_cb(orqa_error_cb);

    if (orqa_init_glfw(3, 3))
        return 1;
    orqa_GLFW_make_window_full_screen();                                                 // Full screen
    GLFWwindow *window = orqa_create_GLFW_window(SCR_WIDTH, SCR_HEIGHT, "", NULL, NULL); // glfw window object creation
    if (window == NULL)
        return 1;

    orqa_make_window_current(window);

    if (!orqa_load_glad((GLADloadproc)orqa_get_proc_address))
    { // glad: load all OpenGL function pointers. GLFW gives us glfwGetProcAddress that defines the correct function based on which OS we're compiling for
        fprintf(stderr, "In file: %s, line: %d Failed to create initialize GLAD\n", __FILE__, __LINE__);
        glfwTerminate();
        return 1;
    }

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
    GLuint *VAOs = orqa_generate_VAOs(2);
    GLuint *VBOs = orqa_generate_VBOs(2);
    GLuint *EBOs = orqa_generate_EBOs(2);

    orqa_bind_VAOs(VAOs[0]);
    orqa_bind_buffer_set_data(GL_ARRAY_BUFFER, VBOs[0], sizeof(vertices), vertices, GL_STATIC_DRAW);
    orqa_bind_buffer_set_data(GL_ELEMENT_ARRAY_BUFFER, EBOs[0], sizeof(indices), indices, GL_STATIC_DRAW);
    orqa_enable_vertex_attrib_array(posLoc, 3, GL_FLOAT, 5 * sizeof(float), (float *)0);
    orqa_enable_vertex_attrib_array(texLoc, 2, GL_FLOAT, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    // texture init
    GLuint *textures = orqa_create_textures(2);
    // loading image!
    orqa_bind_texture(textures[0]);
    orqa_generate_texture_from_buffer(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, NULL);


    while (keepRunning)
    {
        // render
        orqa_clear_color_buffer(0.2f, 0.2f, 0.2f, 1.0f);
        orqa_clear_buffer(GL_COLOR_BUFFER_BIT);

        Mat frame;
        cap >> frame;

        Mat grayscale;
        cvtColor(frame, grayscale, COLOR_BGR2GRAY);
        resize(grayscale, grayscale, Size(grayscale.size().width / scale, grayscale.size().height / scale));
        vector < Rect > people;
        cascade.detectMultiScale(grayscale, people, 1.1, 3, 0, Size(50,50)); 

        for (Rect area : people)
        {
            Scalar drawColor = Scalar(0, 0, 0);
            rectangle(frame, Point(cvRound(area.x * scale), cvRound(area.y * scale)), Point(cvRound((area.x + area.width -1) * scale), cvRound((area.y + area.height -1) * scale)), drawColor);
        }

        // imshow("WebCam feed", frame);
        orqa_bind_texture(textures[0]);

        orqa_update_texture_from_buffer(GL_TEXTURE_2D, 0,0, width, height, GL_RGB, GL_UNSIGNED_INT, frame.ptr());

        orqa_bind_vertex_object_and_draw_it(VAOs[0], GL_TRIANGLES, 6);

        // glfw: swap buffers and poll IO events
        orqa_swap_buffers(window);
        orqa_pool_events();
    }

    // deallocating stuff
    orqa_delete_VAOs(2, VAOs);
    orqa_delete_buffers(2, VBOs);
    orqa_delete_buffers(2, EBOs);
    orqa_delete_textures(2, textures);
    orqa_delete_program(shaderProgram);

    glfwTerminate(); // glfw: terminate, clearing all previously allocated GLFW resources.
    printf("\n\nProgram executed!\n");
    return 0;
}
