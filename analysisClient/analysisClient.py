#!/usr/bin/python2

import sys
sys.path.insert(1, "../python/")
import time
import os
import argparse
import colorprinter
import sclient
import decode
import signal
import subprocess
import shlex

log = colorprinter.printer()

# Parse command line arguments
parser = argparse.ArgumentParser()
parser.add_argument("-ld",	"--log-dir",		dest="log_dir",		help="Directory where the client stores the log file",	default=".")
parser.add_argument("-ed",	"--exec-dir",		dest="exec_dir",	help="Directory where scripts are executed",		default=".")
parser.add_argument("-sd",	"--script-dir",		dest="script_dir",	help="Directory where scripts are searched for",	default=".")
parser.add_argument("-ts",	"--timestamp",		dest="timestamp",	help="Timestamp for creation of files",			default=0)
args = parser.parse_args()

# Setup logging handle
log.timestamp = float(args.timestamp)
log.set_logfile(args.log_dir + "/analysisClient.log")
log.set_prefix = ""

# Print welcome
log.printw()

# Setup Subsystem
abo = "/analysis"
client = sclient.sClient("127.0.0.1", 12334, "analysisClient")
client.subscribe(abo)
client.send(abo, 'Connecting analysisClient with Subsystem\n')

# Setup KILL handler
def handler(signal, frame):
	log.printv()
	log << "Received signal " + `signal` + "."
	log << "Closing connection ..."
	client.closeConnection()
	if client.isClosed == True:
		log << "Client connection closed."

signal.signal(signal.SIGINT, handler)

# Wait for new commands from elComandante

shutter = 3 # FIXME: Read from config

log << "Waiting for commands ..."
while client.anzahl_threads > 0 and client.isClosed == False:
	time.sleep(0.5)
	packet = client.getFirstPacket(abo)
	if not packet.isEmpty():
		log << "Received packet from " + abo + ": " + packet.data
		timeStamp, commands, type, message, command = decode.decode(packet.data)
		if len(commands) == 2 and commands[0].upper() == "ANALYZE":
			if commands[1].upper() == "EXECUTE":
				log << command
				cargs = message.split(",")
				log << "Executing: " + " ".join(cargs)
				cargs[0] = os.path.abspath(args.script_dir + "/" + cargs[0])
				try:
					cwd_save = os.getcwd()
					os.chdir(args.exec_dir)
					p = subprocess.Popen(cargs, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
					os.chdir(cwd_save)
				except:
					log << "Execution failed!"
					client.send(abo, ":FAILED\n")
				else:
					out, err = p.communicate()
					for line in out.split("\n"):
						if len(line) > 0:
							log << "  > " + line
							client.send(abo, line + "\n")
					p.poll()
					status = ""
					if p.returncode == 0:
						status = "(success)"
						client.send(abo, ":STATUS 0\n")
					else:
						status = "(failed)"
						client.send(abo, ":STATUS " + `p.returncode` + "\n")
					log << "Finished execution with return code " + `p.returncode` + " " + status + "."
		elif len(commands) == 1 and commands[0].upper() == "FINISHED":
			client.send(abo, ":FINISHED\n")
		elif len(commands) == 1 and commands[0].upper() == "EXIT":
			break

log.printv()
log << "Closing connection ..."
client.closeConnection()
if client.isClosed == True:
	log << "Client connection closed."
log << "Exit."
