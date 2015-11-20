import os
import glob
import sys
sys.path.insert(1, "../")
from myutils import BetterConfigParser, sClient, decode, printer, preexec
from time import strftime, gmtime, sleep
import time
from shutil import copytree,rmtree
import argparse
import environment
import signal
import el_agente
import subprocess
import glob
import shutil
import errno

class alerts_agente(el_agente.el_agente):
    def __init__(self, timestamp,log, sclient, name='elComandanteAlertAgente'):
        el_agente.el_agente.__init__(self,timestamp, log, sclient)
        self.agente_name = "alertsAgente"
        self.client_name = "alertsClient"
        self.subscription = "/alerts"
        self.Directories['configDir'] = "../config/"
        self.active = True
        self.Name = name

    def setup_configuration(self, conf):
        self.conf = conf
        self.active = True
        self.logDir = conf.get("Directories", "dataDir") + "/logfiles/"

    def check_client_running(self):
        if not self.active:
            return False
        process = os.system("ps aux | grep -v grep | grep -v vim | grep -v emacs | grep %s" % self.client_name)
        if type(process) == str and process != "":
            raise Exception("Another %s self.sclient is already running. Please close this self.sclient first." % self.sclient.name)
            return True
        return False

    def check_subscription(self):
        # Verify the subsystem connection
        if self.active:
            return self.sclient.checkSubscription(self.subscription)
        else:
            return True

    def subscribe(self):
        if (self.active):
            self.sclient.subscribe(self.subscription)

    def start_client(self, timestamp):
        self.timestamp = timestamp
        if not self.active:
            return True
        command = "xterm -T 'AlertsMaster' +sb -sl 5000 -geometry 30x10+860+32 -fs 12 -fa 'Mono' -e "
        command += "python ../myutils/alertsmaster.py "
        command += "-c %s "%(self.Directories['configDir'])
        command += "-dir %s "%(self.logDir)
        command += "-n '%s' "%(self.Name)
        command += ";"
        self.log << "Starting " + self.client_name + " ..."
        self.child = subprocess.Popen(command, shell = True, preexec_fn = preexec)
        return True

    def request_client_exit(self):
        if not self.active:
            return True
        self.sclient.send(self.subscription, ":EXIT\n")
        return False

    def kill_client(self):
        if not self.active:
            return True
        self.child.kill()
        return True
