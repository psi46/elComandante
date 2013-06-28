## @file
## Implements the agente class lowVoltage_agente
## @ingroup elComandante
## @ingroup elAgente

import os
import subprocess

from myutils import process
import el_agente

def preexec():
	os.setpgrp()

## Agente class that communicates with the lowVoltageClient
##
## This is the agente whose job is to communicte with the lowVoltageClient. It
## has a very simple task: To turn on and off the low voltage for the test
## setup.
##
## The low voltage device normally has to operate only before the test
## (lowVoltage_agente.prepare_test) and nothing has to be done during the
## actual testing, except for monitoring the device state.
##
## The action performed is normally only a power cycle which serves as a hard
## reset for the test hardware.
##
## The lowVoltag agente sends very high level commands to the lowVoltageClient such
## as SET OUTPUT ON, SET OUTPUT OFF, or EXEC POWERCYCLE and it does not have to know about the
## details of these operations. It expects that the client handles these
## things and that when it if finished, it will answer the FINISHED command
## with giving back FINISHED. Therefore, the agente waits for the operations
## of the client to finish. Since the client is a separate process, elComandante
## (of which this agente is a part) may continue to start or monitor other
## processes through other agentes.
##
## The configuration of the lowVoltag agente is made in the elComandante.conf
## and the elComandante.ini files. The elComandante.conf file contains information
## about the setup such as low voltage device type and device file name:
## @code
## lowVoltageSubscription: /lowVoltage
##
## [lowVoltageClient]
## lowVoltageType: yoctorelay
## @endcode
##
## The initialization only holds the parameter
## @code
## LowVoltageUse: True
## @endcode
## which enables or disables the lowVoltageAgente.
## @ingroup elComandante
## @ingroup elAgente
class lowVoltage_agente(el_agente.el_agente):
	## Initializes the agente
	## @param timestamp Timestamp from elComandante
	## @param log Log handler
	## @param sclient Subsystem client handle
	def __init__(self, timestamp, log, sclient):
		el_agente.el_agente.__init__(self, timestamp, log, sclient)
		self.agente_name = "lowVoltageAgente"
		self.client_name = "lowVoltageClient"

	## Sets up the permanent configuration of the agente
	##
	## Determines settings such as low voltage device type
	## from elComandante's permanent configuration.
	## @param conf Configuration handle
	## @return Boolean for success
	def setup_configuration(self, conf):
		## Type of the low voltage device, to be passed to the client
		self.device_type = conf.get("lowVoltageClient", "lowVoltageType")
		self.subscription = conf.get("subsystem", "lowVoltageSubscription")
		## Directory for the log files
		self.logdir = conf.get("Directories", "dataDir") + "/logfiles/"
		return True

	## Sets up the initialization of the agente
	##
	## Determines settings such as whether the low voltage device is used
	## for this run from elComandante's run time configuration
	## (initialization)
	## @param init Initialization handle
	## @return Boolean for success
	def setup_initialization(self, init):
		self.active = init.getboolean("LowVoltage", "LowVoltageUse")
		return True

	## Checks whether the lowVoltageClient is running
	##
	## Checks whether the lowVoltageClient is running by finding
	## the PID file and checking the process.
	## @return Boolean, whether the client is running or not
	def check_client_running(self):
		if not self.active:
			return False
		if process.check_process_running(self.client_name + ".py"):
			raise Exception("Another %s is already running. Please close this client first." % self.client_name)
			return True
		return False

	## Starts the lowVoltageClient
	##
	## If enabled, starts the lowVoltageClient with the parameters read from the
	## configuration.
	## @param Timestamp
	## @return Boolean for success
	def start_client(self, timestamp):
		if not self.active:
			return True
		command = "xterm +sb -geometry 120x20-0+300 -fs 10 -fa 'Mono' -e '"
		command += "cd ../lowVoltageClient && python ../lowVoltageClient/lowVoltageClient.py "
		command += "--timestamp {0:d} ".format(timestamp)
		command += "--directory {0:s} ".format(self.logdir)
		command += "--device-type {0:s}'".format(self.device_type)
		self.log << "Starting " + self.client_name + " ..."
		## Child process handle for the lowVoltageClient
		self.child = subprocess.Popen(command, shell = True, preexec_fn = preexec)
		return True

	## Subscribes to the subsystem channel where the lowVoltageClient listening
	##
	## Enables listening to the subsystem channel that the lowVoltageClient is
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

	## Asks the lowVoltageClient to exit by sending it a command through the subsystem
	## @return Boolean for success
	def request_client_exit(self):
		if not self.active:
			return True
		self.sclient.send(self.subscription, ":EXIT\n")
		return False

	## Tries to kill the lowVoltageClient by sending the SIGTERM signal
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
	## Powercycles the low voltage of the test setup to hard reset
	## all devices
	## @param test The current test
	## @param environment The environment the test should run in
	## @return Boolean for success
	def prepare_test(self, test, environment):
		# Run before a test is executed
		if not self.active:
			return True
		self.sclient.send(self.subscription, ":EXEC:POWERCYCLE\n")
		self.set_pending()
		return True

	## Function to execute the test which is disregarded by this agente
	## @return Always returns True
	def execute_test(self):
		# Runs a test
		if not self.active:
			return True
		return True

	## Function to clean up the test which is disregarded by this agente
	##
	## Turns of the beam. This may change in the future.
	## @return Boolean for success
	def cleanup_test(self):
		# Run after a test has executed
		if not self.active:
			return True
		return True

	## Final test cleanup
	## @return Boolean for success
	def final_test_cleanup(self):
		# Run after a test has executed
		if not self.active:
			return True
		self.sclient.send(self.subscription, ":EXEC:POWERCYCLE\n")
		self.set_pending()
		return True

	## Checks whether the client is finished or has an error
	##
	## Checks whether the client is finished or has an error. Even if
	## no action is pending from the client it may happen that the state
	## of the low voltage device changes. An error is received in this case
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
