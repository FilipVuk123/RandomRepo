#include <stdlib.h> // calloc
#include <string.h> // strlen

#include <fcntl.h> // Contains file controls like O_RDWR
#include <unistd.h> // write(), read(), close()

#include "bus.h"
#include "util.h"

serial_bus_t* create_serial_bus(char const *device) {
	serial_bus_t *bus = malloc(sizeof(serial_bus_t)); 

  // Open up the port on device with read/write 
	bus->serial_port = open(device, O_RDWR);
	if (bus->serial_port < 0) 
		log_error("open");
	

	if (tcgetattr(bus->serial_port, &bus->tty) != 0) 
		log_error("tcgetattr");

	// read buffer
	bus->read_buf_len = READ_MAX_LEN;
	bus->read_buf = malloc(bus->read_buf_len * sizeof(char));
	
  // Configure tty settings
  configure_tty_settings(&bus->tty);

  // Save tty settings, also checking for error
  if (tcsetattr(bus->serial_port, TCSANOW, &bus->tty) != 0) 
		log_error("tcsetattr");

	return bus;
}

void configure_tty_settings(struct termios *tty) {
	tty->c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty->c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty->c_cflag &= ~CSIZE; // Clear all bits that set the data size 
  tty->c_cflag |= CS8; // 8 bits per byte (most common)
  tty->c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty->c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty->c_lflag &= ~ICANON;
  tty->c_lflag &= ~ECHO; // Disable echo
  tty->c_lflag &= ~ECHOE; // Disable erasure
  tty->c_lflag &= ~ECHONL; // Disable new-line echo
  tty->c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty->c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  tty->c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty->c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty->c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty->c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty->c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty->c_cc[VMIN] = 0;

  // Set in/out baud rate 
  cfsetispeed(tty, B9600);
  cfsetospeed(tty, B115200);
}

void write_to_bus(serial_bus_t *bus, char const *const msg) {
	size_t n = write(bus->serial_port, msg, strlen(msg));
  (void)(n);
}

int read_from_bus(serial_bus_t *bus) {
	int n = read(bus->serial_port, bus->read_buf, bus->read_buf_len);

	if (n < 0) 
		log_error("read");

	return n;
}

void delete_bus(serial_bus_t *bus) {
	close(bus->serial_port);
	free(bus->read_buf);
	free(bus);
}
