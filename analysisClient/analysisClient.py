## @file
## Program code for the analysisClient

## @addtogroup analysisClient
## @brief elComandante client to run analysis scripts
##
## @details
## The analysis client is a program that listens to commands (from elComandante)
## and upon reception of such a command executes a script.

#!/usr/bin/python2

import sys
sys.path.insert(1, "../")
import time
import os
import argparse
import myutils
import signal
import subprocess
import shlex
from myutils import process

process.create_pid_file()

log = myutils.printer()

## @addtogroup analysisClient
## @details
## The program takes a number of command line arguments which determines
## the behaviour of the client. Because the program is normally started by
## elComandante these command line arguments are normally given automatically
## (through specification in elComandante's configuration files)
##
## - \c \--log-dir	Directory where the client stores the log file			(default: .)
## - \c \--exec-dir	Directory where scripts are executed (working directory)	(default: .)
## - \c \--script-dir	Directory where scripts are searched for			(default: .)
## - \c \--timestamp	Timestamp for creation of files					(default: 0)

# Parse command line arguments
parser = argparse.ArgumentParser()
parser.add_argument("-ld",	"--log-dir",		dest="log_dir",		help="Directory where the client stores the log file",	default=".")
parser.add_argument("-ed",	"--exec-dir",		dest="exec_dir",	help="Directory where scripts are executed",		default=".")
parser.add_argument("-sd",	"--script-dir",		dest="script_dir",	help="Directory where scripts are searched for",	default=".")
parser.add_argument("-ts",	"--timestamp",		dest="timestamp",	help="Timestamp for creation of files",			default=0)
args = parser.parse_args()

# Setup logging handle
log.timestamp = float(args.timestamp)
log.set_logfile(args.log_dir,"/analysisClient.log")
log.set_prefix = ""

# Setup Subsystem
abo = "/analysis"
client = myutils.sClient("127.0.0.1", 12334, "analysisClient")
client.subscribe(abo)
client.send(abo, 'Connecting analysisClient with Subsystem\n')

## Function that handles unexpected events caused by the SIGKILL signal
##
## These function that either comes from user interaction (Ctrl-C) or a
## parent program (elComandante) that wishes to terminate the program.
## If such an event occurs the program cleans up the subsytem client connection.
def handler(signal, frame):
	log.printv()
	log << "Received signal " + `signal` + "."
	log << "Closing connection ..."
	client.closeConnection()
	if client.isClosed == True:
		log << "Client connection closed."
	process.remove_pid_file()

signal.signal(signal.SIGINT, handler)

# Wait for new commands from elComandante

exec_dir = args.exec_dir

## @addtogroup analysisClient
## @details
## The main function of the analysisClient is to listen to commands
## from elComandante and it processes them sequentially.

log << "Waiting for commands ..."
while client.anzahl_threads > 0 and client.isClosed == False:
	packet = client.getFirstPacket(abo)
	if not packet.isEmpty():
		log << "Received packet from " + abo + ": " + packet.data
		timeStamp, commands, type, message, command = myutils.decode(packet.data)
		if len(commands) == 2 and commands[0].upper() == "ANALYZE":
			if commands[1].upper() == "EXECDIR":
				exec_dir = message.strip()
			if commands[1].upper() == "EXECUTE":
				## @addtogroup analysisClient
				## @details
				## When the command is received to execute a scipt the
				## script is searched in the search path. After that te
				## program switches to the execution directory and
				## executes the script in a child process. After execution
				## the program switches back to the original directory.
				## The output of the program is read through a pipe and
				## logged in the analysisClient logfile. The child process
				## status code is then examined and if it is 0 (success)
				## the status is returned to elComandante.
				cargs = message.split(",")
				cargs[0] = os.path.abspath(args.script_dir + "/" + cargs[0])
				log << "Executing: " + " ".join(cargs)
				log << "in directory " + exec_dir
				try:
					cwd_save = os.getcwd()
					log << "Switching to directory " + exec_dir
					os.chdir(exec_dir)
					p = subprocess.Popen(" ".join(cargs), stdout = subprocess.PIPE, stderr = subprocess.STDOUT, shell = True)
					log << "Switching back to " + cwd_save
					os.chdir(cwd_save)
				except:
					log << "Execution failed!"
					client.send(abo, ":ERROR\n")
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
	else:
		time.sleep(0.5)

log.printv()
log << "Closing connection ..."
client.closeConnection()
if client.isClosed == True:
	log << "Client connection closed."
log << "Exit."
process.remove_pid_file()
