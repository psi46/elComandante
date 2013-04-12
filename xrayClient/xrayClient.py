#!/usr/bin/python2

## @file
# Implements the program code of the xrayClient
# @ingroup xrayClient

## \addtogroup xrayClient
# @brief elComandante client for controlling an x-ray device
#
# @details
# This device receives commands from elComandante to operate and supervise
# and x-ray device. The commands are exchanged over the subsystem. The
# nature of the commands is very high level and they do not care about the
# details of the implementation and the type of x-ray device used. An
# example for a series of commands is: SET VOLTAGE 30, SET CURRENT 10,
# SET HV ON, SET BEAM ON. The xrayClient interprets these high level
# commands and, by knowing the specifics of the x-ray device, sends the
# appropriate commands to it. Additionally it checks whether the specified
# conditions are reached successfully and monitors them when nothing else
# is happening.
#
# To extend the x-ray client with more x-ray device implementations new classes
# have to be defined which inherit from the xray_generator.xray_generator and
# the motor_stage.motor_stage classes. Examples of such implementations are
# the id3003.id3003_xray_generator and the zaber.zaber_motor_stage classes.

import sys
sys.path.insert(1, "../")
import time
import argparse
import zaber
import id3003
import myutils
import signal
from myutils import process

process.create_pid_file()

## Instance of the logger from the myutils package
log = myutils.printer()

## @addtogroup xrayClient
# @details
# Since the xrayClient can use multiple different x-ray setups there is
# quite a number of command line arguments to chose how the client
# operates. Normally, the client is started by elComandante who reads
# the required arguments from its configuration files.
#
# Command line arguments:
# - \c \--xray-device	Xray generator device, e.g. /dev/ttyS0	(default: /dev/ttyF0)
# - \c \--xray-type	Xray generator device type, e.g. id3003	(default: id3003)
# - \c \--stage-device	Fluorescence device, e.g. /dev/ttyS0	(default: /dev/ttyF1)
# - \c \--stage-type	Fluorescence device type, e.g. zaber	(default: zaber)
# - \c \--directory	Directory for log files			(default: .)
# - \c \--timestamp	Timestamp for creation of file		(default: 0)
# - \c \--targets	Fluorescence target description		(default: "")

# Parse command line arguments
## Instance of the ArgumentParser from the argparse package
parser = argparse.ArgumentParser()
parser.add_argument("-xd",	"--xray-device",	dest="xray_device",	help="Xray generator device, e.g. /dev/ttyS0",	default="/dev/ttyF0")
parser.add_argument("-xt",	"--xray-type",		dest="xray_type",	help="Xray generator device type, e.g. id3003",	default="id3003")
parser.add_argument("-sd",	"--stage-device",	dest="stage_device",	help="Fluorescence device, e.g. /dev/ttyS0",	default="/dev/ttyF1")
parser.add_argument("-st",	"--stage-type",		dest="stage_type",	help="Fluorescence device type, e.g. zaber",	default="zaber")
parser.add_argument("-dir",	"--directory",		dest="directory",	help="Directory for log files",			default=".")
parser.add_argument("-ts",	"--timestamp",		dest="timestamp",	help="Timestamp for creation of file",		default=0)
parser.add_argument("-tg",	"--targets",		dest="targets",		help="Fluorescence target description",		default="")
## Parsed command line arguments
args = parser.parse_args()

# Setup logging handle
log.timestamp = float(args.timestamp)
log.set_logfile(args.directory,"xrayClient.log")
log.set_prefix = ""

# Print welcome
log << "Xray Client"
log.printv()

# Setup Subsystem
## Subsystem channel on which the xrayClient is listening
abo = "/xray"
## Subsystem client instance
client = myutils.sClient("127.0.0.1", 12334, "xrayClient")
client.subscribe(abo)
client.send(abo, 'Connecting xrayClient with Subsystem\n')

## @addtogroup xrayClient
# @details The target description is a comma separated list of targets where the
# targets are given by a label and a set of coordinates, e.g.
# Ag:35:59,Sn:12:69 whereas the coordinates will be used as the motor stage
# positions where the target is located. The label is such that the high
# level command SET TARGET Ag (or else) can be used by elComandante.

## Target dictionary with label and coordinates
targets = {}
# Decode target argument
for target in args.targets.split(","):
	## Coordinates of the target
	positions = target.split(":")
	## Label of the target
	name = positions[0]
	positions = positions[1:]
	if len(positions) == 0:
		error = "Invalid target description: " + target
		log.warning(error)
		continue
	str = name + " target at position "
	if len(positions) > 1:
		str += "["
	for p in positions[:-1]:
		str += p + ":"
	str += positions[-1]
	if len(positions) > 1:
		str += "]"
	log << str
	for i in range(len(positions)):
		positions[i] = int(positions[i])
	targets[name] = positions

log.printv()

## @addtogroup xrayClient
# @details
# Depending on the command line options a specific x-ray generator and motor stage
# is instantiated. These instances are then used to perform the actions required
# by elComandante. This is the reason why every device specific class has to implement
# the standard set of functions (which are inherited from xray_generator.xray_generator
# and motor_stage.motor_stage). If this is guaranteed the specific nature of the device
# is entirely transparent to xrayClient (and elComandante).

# Open the xray generator device ###################################################
log << "Opening " + args.xray_type + " xray device at " + args.xray_device + " ..."
if args.xray_type == "id3003":
	## X-ray generator instance
	xray_generator = id3003.id3003_xray_generator(args.xray_device)
else:
	error = "Unknown device " + args.xray_type + "."
	log.warning(error)
	sys.exit(1)

if not xray_generator.is_open():
	error = "Unable to open " + args.xray_type + " device."
	log.warning(error)
	sys.exit(1)
else:
	log << "Xray device is open."

if not xray_generator.test_communication():
	error = "Unable to communicate with " + args.xray_type + " device."
	log.warning(error)
	sys.exit(1)
else:
	log << "Communication with xray device is OK."

# Sleep a little to allow the device to reset properly
#sleep_seconds = 0.5
#log << "Sleeping " + `sleep_seconds` + " seconds ..."
#time.sleep(sleep_seconds)

log.printv()

# Open the motor stage device ################################################################
log << "Opening " + args.stage_type + " fluorescence device at " + args.stage_device + " ..."
if args.stage_type == "zaber":
	## Motor stage instance
	motor_stage = zaber.zaber_motor_stage(args.stage_device)
else:
	error = "Unknown device " + args.stage_type + "."
	log.warning(error)
	sys.exit(1)

if not motor_stage.is_open():
	error = "Unable to open " + args.stage_type + " device."
	log.warning(error)
	sys.exit(1)
else:
	log << "Fluorescence device is open."

if not motor_stage.test_communication():
	error = "Unable to communicate with " + args.stage_type + " device."
	log.warning(error)
	sys.exit(1)
else:
	log << "Communication with fluorescence device is OK."

# Reset the device
#log << "Resetting device ..."
#success = motor_stage.reset()
#if not success:
#	error = "Unable to reset the device."
#	log.warning(error)
#	sys.exit(1)
#else:
#	log << "Device reset."

# Sleep a little to allow the device to reset properly
sleep_seconds = 0.5
log << "Sleeping " + `sleep_seconds` + " seconds ..."
time.sleep(sleep_seconds)

# Move the device to its home position
log << "Moving device to home position ..."
success = motor_stage.home()
if not success:
	error = "Unable to move to home position."
	log.warning(error)
	sys.exit(1)
else:
	log << "Home position reached."

log << "Initialization finished."
log.printv()

## @addtogroup xrayClient
# @details
# During operation the xrayClient may be terminated unexpectedly by a UNIX signal
# such as SIGINT. This can happen at any time either due to a user interaction
# (Ctrl-C) or by a parent process (elComandante) that whishes to terminate the
# client. If this happens, the client is given a chance to clean up. This is
# especially important for hazardous devices such as the x-ray generator such
# that no possible danger can occur due to unexpected events.

## Signal handler that handles the SIGINT (Ctrl-C) or KILL signal and exits gracefully
def handler(signal, frame):
	log.printv()
	log << "Received signal " + `signal` + "."
	log << "Closing connection ..."
	client.closeConnection()
	if client.isClosed == True:
		log << "Client connection closed."
	process.remove_pid_file()

signal.signal(signal.SIGINT, handler)

## @addtogroup xrayClient
# @details
# The state of the x-ray device which is set through this client is saved
# in internally. This is used to monitor the device. The reason for this is
# that at any time, due to physical interaction for example, the conditions
# of the device can change. In some devices there is a user interface that
# can be manipulated directly without computer (for changing voltages and
# currents for example). In all devices there is an interlock that turns
# off the high voltage or closes the shutter if the device is opened. When
# such an event happens, the xrayClient needs to know about it and this is
# the reason why the variables are monitored.

## Saved voltage that the x-ray generation is supposed to be at
state_kV = xray_generator.get_voltage()
## Saved current that the x-ray generation is supposed to be at
state_mA = xray_generator.get_current()
## Saved high voltage state that the x-ray generation is supposed to be at
state_HV = xray_generator.get_hv()


## Checks whether the state of the x-ray device has changed or not
def check_state(xray, kV, mA, HV):
	if xray.get_hv() != HV:
		return False
	if xray.get_voltage() != kV:
		return False
	if xray.get_current() != mA:
		return False
	return True

# Wait for new commands from elComandante

shutter = 3 # FIXME: Read from config

## @addtogroup xrayClient
# @details
# The core of the xrayClient is the message query loop where it
# waits for new commands. When no commands are received the state
# of the devices is checked. Commands that arrive are executed
# sequentially. Whenever a problem occurs, an ERROR message is sent
# back. Otherwise, no messages are sent, except when the FINISHED
# query arrives, at which FINISHED is sent back.

log << "Waiting for commands ..."
while client.anzahl_threads > 0 and client.isClosed == False:
	packet = client.getFirstPacket(abo)
	if not packet.isEmpty():
		log << "Received packet from " + abo + ": " + packet.data
		timeStamp, commands, type, message, command = myutils.decode(packet.data)
		if len(commands) == 2 and commands[0].upper() == "SET":
			if commands[1].upper() == "TARGET":
				target = message
				if target in targets:
					log << "Moving to target " + target + " ..."
					motor_stage.move_absolute(targets[target])
					log << "Current target is now " + target + "."
					shutter = 1 # FIXME: Use config value
				elif target.lower() == "none" or target == "":
					shutter = 3 # FIXME: Use config value
				else:
					error = "Invalid target selected."
					log.warning(error)
			elif commands[1].upper() == "VOLTAGE":
				kV = int(message)
				log << "Setting voltage to " + `kV` + " kV ..."
				success = xray_generator.set_voltage(kV)
				if not success:
					error = "Unable to set the voltage."
					log.warning(error)
					client.send(abo, ":ERROR %s\n" % error)
				else:
					log << "Voltage set."
					state_kV = kV
			elif commands[1].upper() == "CURRENT":
				mA = int(message)
				log << "Setting current to " + `mA` + " mA ..."
				success = xray_generator.set_current(mA)
				if not success:
					error = "Unable to set the current."
					log.warning(error)
					client.send(abo, ":ERROR %s\n" % error)
				else:
					log << "Current set."
					state_mA = mA
			elif commands[1].upper() == "HV":
				on = 0
				if message.upper() == "ON":
					on = 1
				elif message.upper() == "OFF":
					on = 0
				else:
					error = "Invalid command: " + packet.data
					log.warning(error)
					client.send(abo, ":ERROR %s\n" % error)

				if on:
					log << "Turning high voltage on ..."
				else:
					log << "Turning high voltage off ..."
				success = xray_generator.set_hv(on)
				if not success:
					if on:
						error = "Unable to turn on the high voltage."
					else:
						error = "Unable to turn off the high voltage."
					log.warning(error)
					client.send(abo, ":ERROR %s\n" % error)
				else:
					if on:
						log << "High voltage on and stable."
					else:
						log << "High voltage off."
					state_HV = on
			elif commands[1].upper() == "BEAM":
				on = 0
				if message.upper() == "ON":
					on = 1
				elif message.upper() == "OFF":
					on = 0
				else:
					error = "Invalid command: " + packet.data
					log.warning(error)
					client.send(abo, ":ERROR %s\n" % error)

				if on:
					log << "Turning beam on ..."
				else:
					log << "Turning beam off ..."
				success = xray_generator.set_beam_shutter(shutter, on)
				if not success:
					if on:
						error = "Unable to turn on the beam."
					else:
						error = "Unable to turn off the beam."
					log.warning(error)
					client.send(abo, ":ERROR %s\n" % error)
				else:
					if on:
						log << "Beam on."
					else:
						log << "Beam off."
		elif len(commands) == 1 and commands[0].upper() == "FINISHED":
			client.send(abo, ":FINISHED\n")
		elif len(commands) == 1 and commands[0].upper() == "EXIT":
			break
	else:
		if not check_state(xray_generator, state_kV, state_mA, state_HV):
			log.warning("Error: Xray generator state changed unexpectedly!")
			client.send(abo, ":ERROR Xray generator state changed unexpectedly!\n")

			# Get the new state to avoid sending more errors
			state_kV = xray_generator.get_voltage()
			state_mA = xray_generator.get_current()
			state_HV = xray_generator.get_hv()
		time.sleep(0.5)


log.printv()
log << "Closing connection ..."
client.closeConnection()
if client.isClosed == True:
	log << "Client connection closed."
log << "Exit."
process.remove_pid_file()
