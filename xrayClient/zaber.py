## @file
## Implements specific motor devices from Zaber
## @ingroup xrayClient

import struct
import serial

from motor_stage import motor_stage

## Motor stage implementation for the Zaber company devices
#
# This motor stage implementation is specifically made for the
# device X made by the company Zaber. It uses a serial interface
# for communication. The serial commands are 6 byte packets with
# packet ID (2 bytes) and data (4 bytes).
# @ingroup xrayClient
class zaber_motor_stage(motor_stage):
	## Constructor that takes the serial device file as argument
	##
	## The constructor takes the serial device file as argument and
	## stores it internally. It does not open the device.
	## @param device The serial device file name of the zaber motor stage
	def __init__(self, device):
		motor_stage.__init__(self, 1)
		## Serial device file name
		self.serialdevice = device
		## Serial device handle
		self.serial = serial.Serial(device)

	## Destructor for cleaning up
	##
	## The destructor closes the serial device, leaving the
	## position of the motor stage where it currently is.
	def __del__(self):
		self.serial.close()

	def is_open(self):
		return self.serial.isOpen()
	def test_communication(self):
		answer = self.serial_command_with_response(55, 12345)
		return answer == self.make_packet(55, 12345)
	def move_absolute(self, coordinates):
		if type(coordinates) != list or len(coordinates) != self.dimensions:
			return 0
		answer = self.serial_command_with_response(20, coordinates[0])
		return answer != ""
	def move_relative(self, coordinates):
		if type(coordinates) != tuple or len(coordinates) != self.dimension:
			return 0
		answer = self.serial_command_with_response(21, coordinates[0])
		return answer != ""
	def set_acceleration(self, acceleration):
		answer = self.serial_command_with_response(43, acceleration)
		return answer != ""
	def home(self):
		answer = self.serial_command_with_response(21, 0)
		if answer == "":
			return 0
		answer = self.serial_command_with_response(1, 0)
		return answer != ""
	def reset(self):
		self.serial_command_no_response(0, 0)
		return True

	## Creates a data packet from command number and data value
	##
	## Makes a binary packet of 6 bytes that represents a valid
	## command for zaber motor stage devices
	## @param command Command number for the zaber device
	## @param data Integer data value that is passed to the device
	## @return Returns a python string with the binary data of the
	## command
	def make_packet(self, command, data):
		return struct.pack("<b", 1) + struct.pack("<b", command) + struct.pack("<i", data)

	## Sends a serial command and does listen for a response
	##
	## Creates a command packet and sends it without listening
	## or waiting for an answer.
	## @param command The command number to be sent
	## @param data The data accompanying the command
	## @return No value is returned
	def serial_command_no_response(self, command, data):
		packet = self.make_packet(command, data)
		self.serial.write(packet)

	## Sends a serial command and listens for a response
	##
	## Creates a command packet and sends it. After that it
	## listens for a response and returns it as a python
	## string that represents the 6 byte binary command.
	## @param command The command number to be sent
	## @param data The data accompanying the command
	## @return The 6 byte data packet received from the
	## device is returned as a python string
	def serial_command_with_response(self, command, data):
		packet = self.make_packet(command, data)
		self.serial.write(packet)
		answer = self.serial.read(6)
		return answer
