#ifndef _ORQA_VEGVISIR_COMMON_H_
#define _ORQA_VEGVISIR_COMMON_H_

#include <vendor/cglm/cglm.h>
#include "orqa_gen_mash.h"
#include "orqa_clock.h"
#include "orqa_opengl.h"
#include "orqa_input.h"
#include "orqa_window.h"
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h>
#include <netinet/in.h>

#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

#ifdef __cplusplus
extern "C" {
#endif
extern int set_zero_point_bool;
#ifdef __cplusplus
}
#endif

float orqa_radians(const float deg);
void *orqa_udp_thread(void *c_ptr);
void orqa_process_input(GLFWwindow *window);
void *orqa_read_from_serial(void * c_ptr);

#endif // ORQA_VEGVISIR_COMMON_H