import os
import sys
sys.path.insert(1, "../")
from myutils import BetterConfigParser, sClient, decode, printer
from myutils import Testboard as Testboarddefinition
from time import strftime, gmtime
import time
from shutil import copytree,rmtree
import argparse
import environment
import signal
import el_agente


class highVoltage_agente(el_agente.el_agente):
    def __init__(self, log, sself.sclient,config):
        super(highVoltage_agente,self).__init__(log, sself.sclient)
        self.config = config
        self.name = "keithleyClient"
        self.currenttest=None

    def setup_configuration(self, conf, Testboards, timestamp):
        self.conf = conf
        self.timestamp = timestamp
        self.numerator = 0
        self.subscription = conf.get("subsystem", "keithleySubscription")
        self.logdir = conf.get("Directories", "dataDir") + "/logfiles/"
        self.active = True

    def check_client_running(self):
        if not self.active:
            return False
        process = os.system("ps aux | grep -v grep | grep -v vim | grep -v emacs | grep %s" % self.name)
        if type(process) == str and process != "":
            raise Exception("Another %s self.sclient is already running. Please close this self.sclient first." % self.sclient.name)
            return True
        return False

    def start_client(self, timestamp):
        self.timestamp = timestamp
        if not self.active:
            return True
        command =  "xterm +sb -geometry 80x25+1200+1300 -fs 10 -fa 'Mono' -e "
        command += "%s/keithleyClient.py "%(Directories['keithleyDir'])
        command += "-d %s "%(config.get("keithleyClient","port"))
        command += "-dir %s "%(Directories['logDir'])
        command += "-ts %s "%(timestamp)
        self.log << "Starting " + self.name + " ..."
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

    def prepare_test(self, whichtest):
        if not self.active:
            return False
        # Run before a test is executed
        self.powercycle()
        self.curenttest = whichtest
        self.log << self.name + ": Preparing " + self.currenttest + " ..."
      
        return True

    def execute_test(self):
        if not self.active:
            return False
        # Runs a test
        self.sclient.clearPackets(keithley)
        if not self.currenttest == 'powercycle':
            self.log << self.name + ": Executing " + test + " ..."
        else:
            self.log << 'Powercycling Testboards'
        for Testboard in self.Testboards:
            self._execute_testboard(Testboard):
        while self.sclient.anzahl_threads > 0 and any([Testboard.busy for Testboard in Testboards]):
            sleep(.5)
            packet = self.sclient.getFirstPacket(self.subscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
        return True    
    
    def cleanup_test(self):
        # Run after a test has executed
        if not self.active:
            return True
        return True

    def final_test_cleanup(self):
        # Run after a test has executed
        if not self.active:
            return True
        return True

    def check_finished(self):
        if not self.active or not self.pending:
            return True
