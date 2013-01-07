import os
import subprocess
import sys
sys.path.insert(1,"../")
from myutils import sClient,decode,printer
import el_agente
import time

def preexec():
    os.setpgrp()

class coolingBox_agente(el_agente.el_agente):
    def __init__(self, timestamp, log, sclient):
        el_agente.el_agente.__init__(self, timestamp, log, sclient)
        self.name = "coolingBoxClient"
        self.log = log
        self.sclient = sclient
        self.active = 0
        self.pending = False
        self.Temperature = 17
        self.counter = 0
        self.currentTest = "none"
        self.nCycles = 1

    def setup_configuration(self, conf):
        self.port = conf.get("jumoClient","port")
        self.logDir = conf.get("Directories","dataDir")+"logfiles"
        self.logFileName = "jumo.log"
        self.subscription = conf.get("subsystem","coolingBoxSubscription")
        self.programDir = conf.get("Directories","jumoDir")
        self.programName = conf.get("jumoClient","programName")
        return True
    def setup_initialization(self, init):
        self.cycleHigh = init.get("Cycle","highTemp")
        self.cycleLow  = init.get("Cycle","lowTemp")
        self.nCycles   = init.get("Cycle","nCycles")
        self.active = init.getboolean("CoolingBox","CoolingBoxUse")
        return True
    def check_logfiles_presence(self):
        # Returns a list of logfiles present in the filesystem
        return []

    def check_client_running(self):
        # Check whether a client process is running
        if not self.active:
            return False
        process = os.system("ps aux | grep -v grep | grep -v vim | grep -v emacs | grep %s"%self.name)
        if type(process) == str and process != "":
            raise Exception("Another %s client is already running. Please close client first." %client.name)
            return True
        return False
    def start_client(self, timestamp):
        # Start a client process
        if not self.active:
            return True
        command  = "xterm +sb -geometry 80x25+1200+0 -fs 10 -fa 'Mono' -e '"
        command += "%s/%s "%(self.programDir,self.programName)
        command += "-d %s "%self.port
        command += "|tee %s/%s'"%(self.logDir,self.logFileName)
        self.log << "Starting %s ..."%self.name
        #self.log << "Command: %s"%command
        self.child = subprocess.Popen(command, shell = True, preexec_fn = preexec)
        return True;

    def subscribe(self):
        if(self.active):
            self.sclient.subscribe(self.subscription)
    def check_subscription(self):
        # Verify the subsystem connection
        if (self.active):
                return self.sclient.checkSubscription(self.subscription)
        return True

    def request_client_exit(self):
        # Request the client to exit with a command
        # through subsystem
        if not self.active:
            return True
        self.sclient.send(self.subscription, ":EXIT\n")
        return False
    def kill_client(self):
    # Kill a client with a SIGTERM signal
        if not self.active:
            return True
        self.send(":PROG:STOP")
        time.sleep(1.0)
        try:
            self.child.kill()
        except:
            self.log<<" Could not kill child"
        return True
    def prepare_test(self, test, environment):
        # Run before a test is executed
        if not self.active:
            return True
        self.log << "%s: Preparing %s @ %s..."%(self.name,test,environment.name)
        if "cycle" not in test:
            pass
        self.Temperature = environment.temperature
        self.log << "Target temperature: %s"%self.Temperature
        self.sclient.clearPackets(self.subscription)
        self.stabalizeTemperature(environment.temperature)
        self.set_pending()
        return True

    def execute_test(self, test, environment):
        # Initiate a test
        self.currentTest = test
        self.log << "%s: execute Test \'%s\'"%(self.name,self.currentTest)
        if not self.active:
            return True
        if "cycle" in test.lower():
            self.sclient.clearPackets(self.subscription)
            time.sleep(1.0)
            self.set_pending()
            self.log << "%s: Starting Cycle"%self.name
            self.log << "\tHighTemp: %s"%self.cycleHigh
            self.log << "\tLowTemp:  %s"%self.cycleLow
            self.log << "\tnCycles:  %s"%self.nCycles
            self.sclient.send(self.subscription,":PROG:CYCLE:HIGHTEMP %s\n"%self.cycleHigh)
            self.sclient.send(self.subscription,":PROG:CYCLE:LOWTEMP %s\n"%self.cycleLow)
            self.sclient.send(self.subscription,":PROG:CYCLE %s\n"%self.nCycles)
        return True

    def cleanup_test(self, test, environment):
        self.currentTest = "none"
        # Run after a test has executed
        if not self.active:
            return True
        #if 'waiting' not in self.status()
        #    self.log << "%s: Cleaning up %s ..."%(self.name,test)
        return False

    def final_test_cleanup(self):
        # Cleanup after all tests have finished to return
        # everything to the state before the test
        #Heat Up again
        self.currentTest = "final_heating"
        self.log << "%s: Heating up Cooling Box..."%self.name
        self.sclient.send(self.subscription,":PROG:HEAT\n")
        self.set_pending()
        return False

    def stabalizeTemperature(self,Temperature):
        self.currentTest = "stabalizeTemp"
        self.sclient.send(self.subscription,":PROG:START 0\n")
        self.sclient.send(self.subscription,":PROG:TEMP %s\n"%Temperature)
        
    def check_finished(self):
        # Check whether the client has finished its task
        # but also check for errors and raise an exception
        # if one occurs.
        if not self.active or not self.pending:
            return True
        if "none" in self.currentTest:
            return True
        elif "cycle" in self.currentTest.lower():
            return self.check_cycleRunning()
        elif "final_heating" in self.currentTest:
            return self.checkFinalHeating()
        else:
            self.checkStabalized()
            return not self.pending
        return False

    def checkFinalHeating(self):
        self.sclient.clearPackets(self.subscription)
        self.sclient.send(self.subscription,":PROG:STAT?\n")
        time.sleep(1.0)
        bGotAnswer = False
        while not bGotAnswer:
            packet = self.sclient.getFirstPacket(self.subscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
                if len(coms)>1:
                    if "prog" in coms[0].lower():
                        if "stat" in coms[1].lower() and typ == 'a':
                            bGotAnswer = True
                            if 'waiting' in msg.lower():
                                self.log << "%s: CoolingBox is heated up."%self.name
                                self.pending = False
                            elif not 'heating' in msg.lower():
                                self.final_test_cleanup()
            else:
                time.sleep(1.0)
        return False

    def checkStabalized(self):
        self.sclient.clearPackets(self.subscription)
        self.sclient.send(self.subscription,":PROG:STAT?\n")
        time.sleep(1.0)
        bGotAnswer = False
        while not bGotAnswer:
            packet = self.sclient.getFirstPacket(self.subscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
                if len(coms)>1:
                    if "prog" in coms[0].lower():
                        if "stat" in coms[1].lower() and typ == 'a':
                            bGotAnswer = True
                            if 'stable' in msg.lower():
                                self.log << "%s: Temperature stable at %s"%(self.name,Time)
                                self.pending = False
                            elif 'waiting' in msg.lower():
                                self.stabalizeTemperature(self.Temperature)
                            else:
                                try:
                                    if counter%10 ==0:
                                        self.log << "CoolingBox is in status %s"%msg.lower()
                                    counter+=1
                                except:
                                    pass
            else:
                time.sleep(1.0)

    def check_cycleRunning(self):
        packet = self.sclient.getFirstPacket(self.subscription)
        if not packet.isEmpty() and not "pong" in packet.data.lower():
            data = packet.data
            Time,coms,typ,msg = decode(data)[:4]
            if len(coms)>1:
                if "prog" in coms[0].lower():
                    if  "cycle" in coms[1].lower() and typ == 'a' and 'finished' in msg.lower():
                        self.pending = False
                        self.log << "%s: Cycle has been finished"%self.name
                    elif "stat" in coms[1].lower() and typ == 'a':
                        if "cycle_restart" in msg.lower():
                            message = msg.split(' ')
                            if len(message)==2:
                                if is_number(message[1]):
                                        self.log <<"%s: %s more Cycles to finish"%(self.name,int(message[1])+1)
                            else:
                                self.log <<"%s: couldn't extract cycles out of msg '%s'"%(self.name,msg)
        return not self.pending

    def set_pending(self):
        #self.sclient.send(self.subscription,":FINISHED\n")
        self.pending = True
