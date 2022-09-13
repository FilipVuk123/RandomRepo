#ifndef BUS_H
#define BUS_H

#include <termios.h> // Contains POSIX terminal control definitions

/* Maximum amount of bytes that can be read at once from serial port. */
#define READ_MAX_LEN 32 

typedef struct {
  /* Port used for connecting with serial bus. */
	int serial_port;

  /* This buffer is used for reading from serial bus. */
	char *read_buf;

  /* This is the length of the read message from serial bus. */
	int read_buf_len;

  /* Terminal interface for controlling communication ports. */
	struct termios tty;
} serial_bus_t;

/*
 * Open serial bus connection to device and setup default communication settings. 
 */
serial_bus_t *create_serial_bus(char const *device);

/*
 * Configures settings for termios.
 */
void configure_tty_settings(struct termios *tty);

/*
 * Write message <msg> to serial bus.
 */
void write_to_bus(serial_bus_t *bus, char const *const msg);

/*
 * Reads message from the serial bus and returns number of bytes read.
 * If no bytes are read it will return 0.
 */
int read_from_bus(serial_bus_t *bus);

/*
 * Closes bus port and frees up memory.
 */
void delete_bus(serial_bus_t *bus);

#endif // BUS_H
