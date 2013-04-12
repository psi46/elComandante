## @file
# Implements the class which defines the basic interface to
# x-ray generator devices
# @ingroup xrayClient

## Implements the basic interface to an x-ray generator device
#
# In order to support multiple different x-ray generator devices
# it is necessary to define an interface that is common to all
# of them. This is a base class which has all the functions
# that any x-ray genrator device implementation is required to
# define to be used in elComandante. To implement a specific device
# a class has to be written which inherits from this class and replaces
# all the functions with actual code that communicates with the
# device. If a function is not replaced it will act in a conservative
# telling the calling program, that the function call was unsuccessful.
# @ingroup xrayClient
class xray_generator():
	## Constructor of the class, setting the number of beams that the device has
	#
	# When inheriting from this class the constructor has to call this function
	# and not just override it. The new constructor can have any number of
	# arguments, such as the device file name etc.
	# @param number_of_beams The number of x-ray beams the device can have
	def __init__(self, number_of_beams):
		## Number of beams that the device can have
		self.number_of_beams = number_of_beams

	## Test whether the device communication has been opened.
	#
	# Most commonly a device is represented as a device file that
	# has to be opened for communication. This function should return
	# whether or not that device file has been opened or not. In case
	# of a serial device for example, this would return for example
	# whether the file /dev/ttyS0 has been opened.
	# @return Boolean, whether the device file has been opened or not
	def is_open(self):
		return False

	## Test whether the communication to the device works or not.
	#
	# An open device file is sometimes not enough to guarantee the
	# communication with the device. Serial devices for instances
	# can be opened and written to even if no physical device is
	# connected. The communication test is supposed to send something
	# to the device, wait for a response and if the response arrives
	# and is correct, then this function should return True.
	# @return Boolean, whether the communication with the device
	# works or not
	def test_communication(self):
		return False

	## Set the acceleration voltage of the x-ray tube
	#
	# This function is supposed to communicate with the x-ray
	# device to set the acceleration voltage to the given value.
	# It is desireable that the function waits for the device
	# to reach that value by querying the device after sending
	# the initial command. If the voltage can not be reached
	# the function should return False.
	# @param kV The voltage to be set in kilovolts.
	# @return Boolean, whether the voltage could be successfully
	# set or not
	def set_voltage(self, kV):
		return None

	## Get the acceleration voltage of the x-ray tube
	#
	# This function is supposed to communicate with the x-ray
	# device to get the current, actual accleration voltage.
	# @return The current, actual accereation voltage in kilovolts,
	# or None
	def get_voltage(self):
		return None

	## Set the current of the x-ray tube
	#
	# This function is supposed to communicate with the x-ray
	# device to set the current in the tube to the given value.
	# It is desireable that the function waits for the device
	# to reach that value by querying the device after sending
	# the initial command. If the current can not be reached
	# the function should return False.
	# @param mA The current to be set in milliamperes.
	# @return Boolean, whether the current could be successfully
	# set or not
	def set_current(self, mA):
		return None

	## Get the current of the x-ray tube
	#
	# This function is supposed to communicate with the x-ray
	# device to get the current, actual tube current.
	# @return The current, actual tube current in kilovolts, or None
	def get_current(self):
		return None

	## Turn on of off the high voltage for the x-ray tube
	#
	# This function is supposed to communicate with the x-ray
	# device to turn on or off the voltage in the tube. It
	# should also, if it can, verify that the high voltage
	# was actually set, and to wait, until the voltage and
	# current are stabilised.
	# @param on Boolean, whether the high voltage should be turned
	# on (True) or off (False)
	# @return Boolean, whether or not the operation was successful
	def set_hv(self, on):
		return None

	## Get the high voltage status from the x-ray tube
	#
	# This function is supposed to communicate with the x-ray
	# device to determine the status of the high voltage.
	# @return Boolean, whether the high voltage is on (True) or
	# off (False)
	def get_hv(self):
		return None

	## Set the shutter of one beam to opened or closed
	#
	# Some x-ray devices have beam shutters. This function is
	# supposed to communicate with the x-ray device to open
	# or close the beam shuter for some beam (if it has multiple).
	# The devices that do not have beam shutters should not
	# re-implement this function. If it does not, it just turns
	# off the high voltage. It must be guaranteed that if this
	# function wants to close the shutter, no radiation is emitted
	# and if it wants to open the shutter, radiation emission is
	# resumed.
	# @param beamno The beam for which the shutter is supposed
	# to be opened or closed
	# @param on Boolean, whether the shutter should be opened (True)
	# or close (False)
	# @return Should return whether the beam is now on
	def set_beam_shutter(self, beamno, on):
		return self.set_hv(on)
