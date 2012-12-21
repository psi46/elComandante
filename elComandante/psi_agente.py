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

def preexec():
    os.setpgrp()

class psi_agente(el_agente.el_agente):
	def __init__(self, log, sself.sclient,config):
	    super(psi_agente,self).__init__(log, sself.sclient)
        self.config = config
		self.name = "psiClient"
        self.currenttest=None

	def setup_configuration(self, conf, Testboards, timestamp):
        self.conf = conf
        self.timestamp = timestamp
        self.numerator = 0
        self.subscription = conf.get("subsystem", "xraySubscription")
        self.logdir = conf.get("Directories", "dataDir") + "/logfiles/"
        self.Testboards=Testboards
        self.numTestboards = len(self.Testboards)
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
        command = "xterm +sb -geometry 120x20+0+300 -fs 10 -fa 'Mono' -e "
        command += "python ../psiClient/psi46handler.py "
        command += "-dir %s"%(Directories['logDir'])
        command += "-num %s"%slef.numTestboards
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
        for Testboard in self.Testboards:
            self._prepare_testboard(Testboard):
        return True

    def _prepare_testboard(self,Testboard):
        Testboard.testdir=Testboard.parentDir+'/%s_%s/'%(str(self.numerator).zfill(3),self.currenttest)
        self._setupdir_testboard(Testboard)

    def powercycle(self):
        self.currenttest='powercycle'
        for Testboard in self.Testboards:
            self._prepare_testboard(Testboard):
        self.execute_test()

	def execute_test(self):
		if not self.active:
			return False
		# Runs a test
        self.sclient.clearPackets(psiSubscription)
        if not self.currenttest == 'powercycle':
            self.log << self.name + ": Executing " + test + " ..."
        else:
            self.log << 'Powercycling Testboards'
        for Testboard in self.Testboards:
            self._execute_testboard(Testboard):
        while self.sclient.anzahl_threads > 0 and any([Testboard.busy for Testboard in Testboards]):
            sleep(.5)
            packet = self.sclient.getFirstPacket(psiSubscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
                if coms[0].find('STAT')==0 and coms[1].find('TB')==0 and typ == 'a' and msg=='test:finished':
                    index=[Testboard.slot==int(coms[1][2]) for Testboard in self.Testboards].index(True)
                    self.Testboards[index].finished()
                    self.Testboards[index].busy=False
                if coms[0][0:4] == 'STAT' and coms[1][0:2] == 'TB' and typ == 'a' and msg=='test:failed':
                    index=[Testboard.slot==int(coms[1][2]) for Testboard in self.Testboards].index(True)
                    if not self.currenttest == 'powercycle':
                        self.Testboards[index].failed()
                    else:
                        sleep(1)
                        raise Exception('Could not open Testboard at %s.'%Testboard.slot)
                    self.Testboards[index].busy=False
        return True    

            #Here we need the Errorstream of the watchdog:

            #packet = self.sclient.getFirstPacket(coolingBoxSubscription)
            #if not packet.isEmpty() and not "pong" in packet.data.lower():
            #    data = packet.data
            #    Time,coms,typ,msg = decode(data)[:4]
            #    if coms[0].find('STAT')==0 and typ == 'a' and 'ERROR' in msg[0].upper():
            #        self.log.warning('jumo has error!')
            #        self.log.warning('\t--> I will abort the tests...')
            #        self.log.printn()
            #        for Testboard in self.Testboards:
            #            self.sclient.send(psiSubscription,':prog:TB%s:kill\n'%Testboard.slot)
            #            self.log.warning('\t Killing psi46 at Testboard %s'%Testboard.slot)
            #            index=[Testboard.slot==int(coms[1][2]) for Testboard in self.Testboards].index(True)
            #            Testboard.failed()
            #            Testboard.busy=False


    def _execute_testboard(self,Testboard): 
        Testboard.busy=True
        self.sclient.send(psiSubscription,':prog:TB%s:start %s,%s,commander_%s\n'%(Testboard.slot,Directories['testdefDir']+'/'+ self.currenttest,Testboard.testdir,self.currenttest))
        if not self.currenttest == 'powercycle':
            self.log.printn()
            self.log << 'psi46 at Testboard %s is now started'%Testboard.slot

	def cleanup_test(self):
		# Run after a test has executed
		if not self.active:
			return True
        if not self.currenttest == 'powercycle':
            self.numerator += 1
        else:
            for Testboard in Testboards:
                self._deldir(Testboard)
		return True

	def final_test_cleanup(self):
		# Run after a test has executed
		if not self.active:
			return True
		return True

	def check_finished(self):
		if not self.active or not self.pending:
			return True

    def _setupdir_testboard(self,Testboard):
        self.log.printn()
        self.log << 'I setup the directories:'
        self.log << '\t- %s'%Testboard.testdir
        self.log << '\t  with default Parameters from %s'%Testboard.defparamdir
        #copy directory
        try:
            copytree(Testboard.defparamdir, Testboard.testdir)
            #change TB address
            f = open( '%s/configParameters.dat'%Testboard.testdir, 'r' )
            lines = f.readlines()
            f.close()
            lines[0]='testboardName %s'%Testboard.address
            f = open( '%s/configParameters.dat'%Testboard.testdir, 'w' )
            f.write(''.join(lines))
            f.close()
        except IOError as e:
            self.log.warning("I/O error({0}): {1}".format(e.errno, e.strerror))
        except OSError as e:
            self.log.warning("OS error({0}): {1}".format(e.errno, e.strerror))

    def _deldir_testboard(self,Testboard):
        try:
            rmtree(Testboard.testdir)
        except:
            self.log.warning("Couldn't remove directory")
            pass

    def open_testboard(self,Testboard):
        self.sclient.clearPackets(psiSubscription)
        self.log << self.name + ": Opening Testboard " + Testboard.slot  + " ..."
        self.client.send(psiSubscription,':prog:TB%s:open %s\n'%(Testboard.slot,Testboard.testdir)) 
    
    def close_testboard(self,Testboard):
        self.log << self.name + ": Closing Testboard " + Testboard.slot  + " ..."
        self.client.send(psiSubscription,':prog:TB%s:close %s\n'%(Testboard.slot,Testboard.testdir))
