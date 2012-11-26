/*
 *	serial.h	08-Oct-2007
 *	Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

/// Device file to use for communications
#define TERM_DEVICE "/dev/ttyS0" /* = COM2 */

/// Baud rate, the parameters 8n1 are hardcoded in serial.cpp
#define TERM_SPEED B38400 /* Bit/Sek */

#define OK		0
#define NO_INIT		1

/// \brief Serial Port Abstraction

/**
 * This class provides simplified access to the serial port. It uses non-blocking i/o
 * to provide the possibility of i/o multiplexing. 
 */
class serial {
private:
	int fd;
	int stat;

	/// terminal settings
	struct termios term_attr;

	/// old terminal settings (restored in destructor)
	struct termios old_flags;

public:
	serial();
	~serial();

	///
	/// \brief return overall status
	/// \return Value of the stat variable
	int status();

	///
	/// \brief get file desciptor
	/// The fd returned by this function can be used for a fd_set (see "man 2 select").
	/// \return file descriptor of serial port if initialized, else -1
	int getfd();

	/// \brief init serial port
	/// Open serial port device file (defined by TERM_DEVICE ) and set all
	/// neccessary communication characteristics like baud, parity, non-blocking i/o, etc.
	int init();

	/// \brief transmit data
	int sendout(char* msg, int len);

	/// \brief check for received data
	///
	/// \return -1, if no data received
	int readin(char* msg, int maxlen);

};


#endif
