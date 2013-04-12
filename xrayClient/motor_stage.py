## @file
# Implements the class that defines the standard interface for motor stages
# @ingroup xrayClient

## Implements the basic interface to a motor stage
#
# In order to support multiple different motor stages it is necessary to
# define an interface that is common to all of them. This is a base class
# which has all the functions that any motor stage device implementation is
# required to define to be used in elComandante. To implement a specific
# device a class has to be written which inherits from this class and
# replaces all the functions with actual code that communicates with the
# device. If a function is not replaced it will act in a conservative
# telling the calling program, that the function call was unsuccessful.
# @ingroup xrayClient
class motor_stage():
	## Constructor that sets the number of dimensions of the motor stage
	#
	# The number of dimensions for a device that moves in x, y, and z
	# would be 3. This constructor should be called when it is re-defined
	# when inherited.
	# @param dimensions Sets the dimensions of the motor stage
	def __init__(self, dimensions):
		## The dimensions of the motor stage
		self.dimensions = dimensions

	## Tests whether the device file of the motor stage is open or not
	#
	# Hardware devices are ususlly opened through a device file (e.g.
	# /dev/ttyS0) to communicate with them. This function tests has to
	# state, whether such a device file was opened or not.
	# @return Boolean, whether the device file is open or not
	def is_open(self):
		return False

	## Tests whether communication with the device is established
	#
	# An open device is often not enough to guarantee that communication
	# is possible. Serial devices for instance can be opened even with
	# the cable unplugged. This function is supposed to send something
	# to the device to provoque a response. When this response is received
	# and is correct then this function should return True.
	# @return Boolean, whether the device answers or not.
	def test_communication(self):
		return False

	## Moves the motor stage to a specific coordinate
	#
	# This function is supposed to command the motor stage
	# to move to a specific point which is given in the argument.
	# The function should wait until the position is reached
	# until it returns. If the position cannot be reached, False
	# should be returned.
	# @param coordinates Tuple of floating point coordinates that
	# descripe the position where the motor stage is supposed to
	# move. The number of items in the tuple has to match the
	# number of dimensions of the motor stage.
	# @return Boolean, whether or not the position was reached or not
	def move_absolute(self, coordinates):
		return None

	## Moves the motor stage to a specific coordinate relative
	#  to the current one
	#
	# This function is supposed to command the motor stage
	# to move to a specific point relative to the current point
	# which is given in the argument.
	# The function should wait until the position is reached
	# until it returns. If the position cannot be reached, False
	# should be returned.
	# @param coordinates Tuple of floating point coordinates that
	# descripe the position relative to the current position
	# where the motor stage is supposed to move. The number of
	# items in the tuple has to match the number of dimensions
	# of the motor stage.
	# @return Boolean, whether or not the position was reached or not
	def move_relative(self, coordinates):
		return None

	## Sets the acceleration of the motor stage
	#
	# This function should communicate with the device
	# and set the acceleration of the stage. The devices
	# that do not support this should return False.
	# @param acceleration Value that the acceleration should
	# be set to
	# @return Boolean, whether the operation was successful
	# or not. If the stage does not support this, it should
	# return False.
	def set_acceleration(self, acceleration):
		return None

	## Moves the device to the home position
	#
	# This function is supposed to move the motor stage to
	# its home position to reset its coordinates. The function
	# should not return until the movement is complete.
	# @return Boolean, whether the position was reached or not
	def home(self):
		return None

	## Resets the device and reinitialises it
	#
	# This function is supposed to bring the device in a known
	# state from which other operations can be performed.
	# @return Boolean, whether the operation was successful or not
	def reset(self):
		return None
