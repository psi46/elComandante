#!/usr/bin/python2

## @file
## Implements the program code of the lowVoltageClient
## @ingroup lowVoltageClient

## \addtogroup lowVoltageClient
## @brief elComandante client for controlling low voltage device
##
## @details
## This client receives commands from elComandante to operate and supervise
## and low voltage devices. The commands are exchanged over the subsystem. The
## nature of the commands is very high level and they do not care about the
## details of the implementation and the type of low voltage device used. An
## example for a series of commands is: SET OUTPUT ON, SET OUTPUT OFF,
## EXEC POWERCYCLE. The lowVoltageClient interprets these high level
## commands and, by knowing the specifics of the low voltage device, sends the
## appropriate commands to it. Additionally it checks whether the specified
## conditions are reached successfully and monitors them when nothing else
## is happening.
##
## To extend the lowVoltageClient with more low voltage device implementations new classes
## have to be defined which inherit from the lowVoltageDevice.lowVoltageDevice.
## An examples of such an implementations is yoctorelay.yoctorelay.

import sys
sys.path.insert(1, "../")
import time
import argparse
import myutils
import signal
from myutils import process

# Devices
import yoctorelay

process.create_pid_file()

## Instance of the logger from the myutils package
log = myutils.printer()
log.printv()
log.set_name("lowVoltageClient")
# Print welcome
log << "LowVoltage Client"
log.printv()

## @addtogroup lowVoltageClient
## @details
## Since the lowVoltageClient can use multiple different setups there are
## a few of command line arguments to chose how the client operates. Normally,
## the client is started by elComandante who reads the required arguments
## from its configuration files.
##
## Command line arguments:
## - \c \--device	Low voltage device, e.g. /dev/ttyS0		(default: any)
## - \c \--device-type	Low voltage device type, e.g. yoctorelay	(default: yoctorelay)
## - \c \--directory	Directory for log files				(default: .)
## - \c \--timestamp	Timestamp for creation of file			(default: 0)

# Parse command line arguments
## Instance of the ArgumentParser from the argparse package
parser = argparse.ArgumentParser()
parser.add_argument("-d",	"--device",		dest="device",		help="Low voltage device, e.g. /dev/ttyS0",		default="any")
parser.add_argument("-t",	"--device-type",	dest="device_type",	help="Low voltage device type, e.g. yoctorelay",	default="yoctorelay")
parser.add_argument("-dir",	"--directory",		dest="directory",	help="Directory for log files",				default=".")
parser.add_argument("-ts",	"--timestamp",		dest="timestamp",	help="Timestamp for creation of file",			default=0)
log << "Parsing command line arguments ..."
## Parsed command line arguments
args = parser.parse_args()
log << "Device:\t\t" + args.device
log << "Device type:\t" + args.device_type
log << "Log directory:\t" + args.directory
log << "Timestamp:\t" + str(args.timestamp)

# Setup logging handle
log.timestamp = float(args.timestamp)
log.set_logfile(args.directory, "lowVoltageClient.log")
log.set_prefix = ""

log.printv()

# Setup Subsystem
## Subsystem channel on which the lowVoltageClient is listening
abo = "/lowVoltage"
log << "Connecting to subsystem via " + abo + " ..."
## Subsystem client instance
client = myutils.sClient("127.0.0.1", 12334, "lowVoltageClient")
client.subscribe(abo)
client.send(abo, 'Connecting lowVoltageClient with Subsystem\n')

log.printv()

## @addtogroup lowVoltageClient
## @details
## Depending on the command line options a specific low voltage device
## is instantiated. This instance is then used to perform the actions required
## by elComandante. This is the reason why every device specific class has to implement
## the standard set of functions (which are inherited from lowVoltageDevice.lowVoltageDevice).
## If this is guaranteed the specific nature of the device
## is entirely transparent to lowVoltageClient (and elComandante).

# Open the low voltage device ###################################################
log << "Opening " + args.device_type + " low voltage device " + args.device + " ..."
if args.device_type == "yoctorelay":
	## Yoctopuce relais
	device = yoctorelay.yoctorelay(args.device)
else:
	error = "Unknown device " + args.device_type + "."
	log.warning(error)
	client.closeConnection()
	sys.exit(1)

# Connect to the device
connected = device.connect()
if not connected:
	error = "Unable to open " + args.device_type + " device."
	log.warning(error)
	client.closeConnection()
	sys.exit(1)
else:
	log << "Low voltage device is open."

if not device.test_communication():
	error = "Unable to communicate with " + args.device_type + " device."
	log.warning(error)
	client.closeConnection()
	sys.exit(1)
else:
	log << "Communication with low voltage device is OK."

log << "Initialization finished."

log.printv()

## @addtogroup lowVoltageClient
## @details
## During operation the lowVoltageClient may be terminated unexpectedly by a UNIX signal
## such as SIGINT. This can happen at any time either due to a user interaction
## (Ctrl-C) or by a parent process (elComandante) that whishes to terminate the
## client. If this happens, the client is given a chance to clean up.

## Signal handler that handles the SIGINT (Ctrl-C) or KILL signal and exits gracefully
def handler(signal, frame):
	log.printv()
	log << "Received signal " + `signal` + "."
	log << "Closing connection ..."
	client.closeConnection()
	if client.isClosed == True:
		log << "Client connection closed."
	process.remove_pid_file()
	log << "Exit."
	sys.exit(0)

signal.signal(signal.SIGINT, handler)

## @addtogroup lowVoltageClient
## @details
## The state of the low voltage device which is set through this client is saved
## in internally. This is used to monitor the device. The reason for this is
## that at any time, due to physical interaction for example, the conditions
## of the device can change. In some devices there is a user interface that
## can be manipulated directly without computer (turning the output on and off
## for example). When such an event happens, the lowVoltageClient needs to know
## about it and this is the reason why the variables are monitored.

## Saved output state the device is supposed to be at
state_output = device.get_output()

## Checks whether the state of the low voltage device has changed or not
def check_state(device, output):
	if device.get_output() != output:
		return False
	return True

# Wait for new commands from elComandante

## @addtogroup lowVoltageClient
## @details
## The core of the lowVoltageClient is the message query loop where it
## waits for new commands. When no commands are received the state
## of the devices is checked. Commands that arrive are executed
## sequentially. Whenever a problem occurs, an ERROR message is sent
## back. Otherwise, no messages are sent, except when the FINISHED
## query arrives, at which FINISHED is sent back.

log << "Waiting for commands ..."
while client.anzahl_threads > 0 and client.isClosed == False:
	packet = client.getFirstPacket(abo)
	if not packet.isEmpty():
		log << "Received packet from " + abo + ": " + packet.data
		timeStamp, commands, type, message, command = myutils.decode(packet.data)
		if len(commands) == 2 and commands[0].upper() == "SET":
			if commands[1].upper() == "OUTPUT":
				state = message
				log << "Setting output to " + state.upper() + "."
				device.set_output(state.upper())
				state_output = state.upper()
		if len(commands) == 2 and commands[0].upper() == "EXEC":
			if commands[1].upper() == "POWERCYCLE":
				log << "Power cycling ..."
				device.power_cycle()
				# Sleep afterwards, to let the electronics start up
				time.sleep(2)
				state_output = device.get_output()
			if commands[1].upper() == "TOGGLE":
				log << "Toggling output ..."
				device.toggle_output()
				state_output = device.get_output()
		elif len(commands) == 1 and commands[0].upper() == "FINISHED":
			log << "Acknowledged."
			client.send(abo, ":FINISHED\n")
		elif len(commands) == 1 and commands[0].upper() == "EXIT":
			break
	else:
		if not check_state(device, state_output):
			log.warning("Error: low voltage device state changed unexpectedly!")
			client.send(abo, ":ERROR low voltage device state changed unexpectedly!\n")

			# Get the new state to avoid sending more errors
			state_output = device.get_state()
		time.sleep(0.5)

log.printv()
log << "Closing connection ..."
client.closeConnection()
if client.isClosed == True:
	log << "Client connection closed."
log << "Exit."
process.remove_pid_file()
