## @file
## Implements the agente class xray_agente
## @ingroup elComandante
## @ingroup elAgente

import os
import subprocess

from myutils import process
import el_agente

def preexec():
	os.setpgrp()

## Agente class that communicates with the xrayClient
##
## This is the agente whose job is to communicte with the xrayClient. It
## knows all the procedures of how to set up the x-ray devices (without
## knowing about the details of the devices) to the state as specified
## in the environment definition that is passed with each test.
##
## The x-ray device normally has to be set up just before the test
## (xray_agente.prepare_test) and nothing has to be done during the
## actual testing, except for monitoring the device state. Between tests
## the beam is turned off, either by means of a shutter (if it exists) or
## by turning off the high voltage. There is otherwise no cleanup of the
## test because the beam might be needed in the next test. In the final
## cleanup the high voltage is turned off.
##
## The x-ray agente sends very high level commands to the xrayClient such
## as SET BEAM ON, SET VOLTAGE 30 and it does not have to know about the
## details of these operations. It expects that the client handles these
## things and that when it if finished, it will answer the FINISHED command
## with giving back FINISHED. Therefore, the agente waits for the operations
## of the client to finish. Since the client is a separate process, elComandante
## (of which this agente is a part) may continue to start or monitor other
## processes through other agentes.
##
## The configuration of the x-ray agente is made in the elComandante.conf
## and the elComandante.ini files. The elComandante.conf file contains information
## about the setup such as x-ray generator type and device file name:
## @code
## xraySubscription: /xray
##
## [xrayClient]
## xrayDevice: /dev/ttyF0
## xrayType: id3003
## xrfDevice: /dev/ttyF1
## xrfType: zaber
## xrfTargets: Fe:0,Cu:25320,Mo:50640,Ag:75960,Sn:101280,Ba:126600
## @endcode
##
## The xrfTarget definition is a comma separated list of targets, where each
## target is a Label, a colon character and a coordinate (multiple dimension
## coordinates can be given like Label:X:Y:Z).
##
## The initialization only holds the parameter
## @code
## XrayUse: True
## @endcode
## which enables or disables the xrayAgente. Appart from that there are the
## environment.environment definitions which specify the x-ray conditions for each test.
## @ingroup elComandante
## @ingroup elAgente
class xray_agente(el_agente.el_agente):
	## Initializes the agente
	## @param timestamp Timestamp from elComandante
	## @param log Log handler
	## @param sclient Subsystem client handle
	def __init__(self, timestamp, log, sclient):
		el_agente.el_agente.__init__(self, timestamp, log, sclient)
		self.agente_name = "xrayAgente"
		self.client_name = "xrayClient"
		## State of the high voltage
		self.hvon = False
		## State of the beam shutter
		self.beamon = None
		## State of the x-ray voltage
		self.voltage = None
		## State of the x-ray current
		self.current = None
		## Current x-ray fluorescence target
		self.target = None

	## Sets up the permanent configuration of the agente
	##
	## Determines settings such as x-ray device and motor stage
	## device types and device file names from elComandante's
	## permanent configuration.
	## @param conf Configuration handle
	## @return Boolean for success
	def setup_configuration(self, conf):
		## Type of the x-ray device, to be passed to the client
		self.xray_type = conf.get("xrayClient", "xrayType")
		## File name of the x-ray device, to be passed to the client
		self.xray_device = conf.get("xrayClient", "xrayDevice")
		## Type of the motor stage device, to be passed to the client
		self.xrf_type = conf.get("xrayClient", "xrfType")
		## File name of the motor stage device, to be passed to the client
		self.xrf_device = conf.get("xrayClient", "xrfDevice")
		## Target description to be passed to the client
		self.targets = conf.get("xrayClient", "xrfTargets")
		## Flag to allow elComandante to turn off the high voltage
		self.turn_off_hv = conf.getboolean("xrayClient", "turnOffHV")
		## Flag to make elComandante to turn off the beam between tests
		self.beam_off_between_tests = conf.getboolean("xrayClient", "beamOffBetweenTests")
		self.subscription = conf.get("subsystem", "xraySubscription")
		## Directory for the log files
		self.logdir = conf.get("Directories", "dataDir") + "/logfiles/"
		return True

	## Sets up the initialization of the agente
	##
	## Determines settings such as whether the x-ray device is used
	## for this run from elComandante's run time configuration
	## (initialization)
	## @param init Initialization handle
	## @return Boolean for success
	def setup_initialization(self, init):
		self.active = init.getboolean("Xray", "XrayUse")
		return True

	## Checks whether the xrayClient is running
	##
	## Checks whether the xrayClient is running by finding
	## the PID file and checking the process.
	## @return Boolean, whether the client is running or not
	def check_client_running(self):
		if not self.active:
			return False
		if process.check_process_running(self.client_name + ".py"):
			raise Exception("Another %s is already running. Please close this client first." % self.client_name)
			return True
		return False

	## Starts the xrayClient
	##
	## If enabled, starts the xrayClient with the parameters read from the
	## configuration.
	## @param Timestamp
	## @return Boolean for success
	def start_client(self, timestamp):
		if not self.active:
			return True
		command = "xterm +sb -geometry 80x20+0+600 -fs 10 -fa 'Mono' -e "
		command += "python ../xrayClient/xrayClient.py "
		command += "--timestamp {0:d} ".format(timestamp)
		command += "--directory {0:s} ".format(self.logdir)
		command += "--xray-device {0:s} ".format(self.xray_device)
		command += "--xray-type {0:s} ".format(self.xray_type)
		command += "--stage-device {0:s} ".format(self.xrf_device)
		command += "--stage-type {0:s} ".format(self.xrf_type)
		command += "--targets {0:s}".format(self.targets)
		self.log << "Starting " + self.client_name + " ..."
		## Child process handle for the xrayClient
		self.child = subprocess.Popen(command, shell = True, preexec_fn = preexec)
		return True

	## Subscribes to the subsystem channel where the xrayClient listening
	##
	## Enables listening to the subsystem channel that the xrayClient is
	## receiving commands on
	## @return None
	def subscribe(self):
		if (self.active):
			self.sclient.subscribe(self.subscription)

	## Checks whether the subsystem channel is open and the server is responding
	## @return Boolean, whether it is responding or not
	def check_subscription(self):
		if (self.active):
			return self.sclient.checkSubscription(self.subscription)
		return True

	## Asks the xrayClient to exit by sending it a command through the subsystem
	## @return Boolean for success
	def request_client_exit(self):
		if not self.active:
			return True
		self.sclient.send(self.subscription, ":EXIT\n")
		return False

	## Tries to kill the xrayClient by sending the SIGTERM signal
	## @return Boolean for success
	def kill_client(self):
		if not self.active:
			return True
		try:
			self.child.kill()
		except:
			pass
		return True

	## Prepares a test with a given environment
	##
	## Looks at the environment to determine the x-ray conditions
	## used for the test. The test itself is irrelevant for this
	## agente. It sends the commands necessary to change the state
	## of the x-ray device to the required ones.
	## @param test The current test
	## @param environment The environment the test should run in
	## @return Boolean for success
	def prepare_test(self, test, environment):
		# Run before a test is executed
		if not self.active:
			return True
		if environment.xray:
			if environment.xray_voltage != self.voltage:
				self.sclient.send(self.subscription, ":SET:VOLTAGE %d\n" % environment.xray_voltage)
				self.voltage = environment.xray_voltage
			if environment.xray_current != self.current:
				self.sclient.send(self.subscription, ":SET:CURRENT %d\n" % environment.xray_current)
				self.current = environment.xray_current
			if environment.xray_target != self.target:
				self.sclient.send(self.subscription, ":SET:TARGET %s\n" % environment.xray_target)
				self.target = environment.xray_target
			if not self.hvon:
				self.sclient.send(self.subscription, ":SET:HV ON\n")
				self.hvon = True
			if not self.beamon:
				self.sclient.send(self.subscription, ":SET:BEAM ON\n")
				self.beamon = True
			self.set_pending()
		else:
			if self.beamon or self.beamon == None:
				self.sclient.send(self.subscription, ":SET:BEAM OFF\n")
				self.beamon = False
			if self.hvon and self.turn_off_hv:
				self.sclient.send(self.subscription, ":SET:HV OFF\n")
				self.hvon = False
			self.set_pending()
		return True

	## Function to execute the test which is disregarded by this client
	## @return Always returns True
	def execute_test(self):
		# Runs a test
		if not self.active:
			return True
		return True

	## Function to clean up the test
	##
	## Turns of the beam. This may change in the future.
	## @return Boolean for success
	def cleanup_test(self):
		# Run after a test has executed
		if not self.active:
			return True
		if self.beamon and self.beam_off_between_tests:
			self.sclient.send(self.subscription, ":SET:BEAM OFF\n")
			self.beamon = False
			self.set_pending()
		return True

	## Final test cleanup
	##
	## Turns off the beam and the high voltage
	## @return Boolean for success
	def final_test_cleanup(self):
		# Run after a test has executed
		if not self.active:
			return True
		self.sclient.send(self.subscription, ":SET:BEAM OFF\n")
		if (self.turn_off_hv):
			self.sclient.send(self.subscription, ":SET:HV OFF\n")
		self.set_pending()
		return True

	## Checks whether the client is finished or has an error
	##
	## Checks whether the client is finished or has an error. Even if
	## no action is pending from the client it may happen that the state
	## of the x-ray device changes. An error is received in this case
	## and an exception is thrown.
	## @return Boolean, whether the client has finished or not
	def check_finished(self):
		if not self.active:
			return True
		while True:
			packet = self.sclient.getFirstPacket(self.subscription)
			if packet.isEmpty():
				break
			if self.pending and "FINISHED" in packet.data.upper():
				self.pending = False
			elif "ERROR" in packet.data.upper():
				self.pending = False
				raise Exception("Error from %s!" % self.client_name)

		return not self.pending

	## Asks whether the client is finished and sets the agente state
	## to pending
	## @return None
	def set_pending(self):
		self.sclient.send(self.subscription, ":FINISHED\n")
		self.pending = True
