import os
import sys
sys.path.insert(1, "../")
from myutils import BetterConfigParser, sClient, decode, printer, preexec
from myutils import Testboard as Testboarddefinition
from time import strftime, gmtime
import time
from shutil import copytree,rmtree
import argparse
import environment
import signal
import el_agente
import subprocess


class highVoltage_agente(el_agente.el_agente):
    def __init__(self, timestamp,log, sclient, clientName = 'keithleyClient', configFileNames = []):
        el_agente.el_agente.__init__(self,timestamp, log, sclient)

        self.clientNames = {
            'KEITHLEY': 'keithleyClient', 
            'ISEG': 'isegClient'
        }

        if clientName not in self.clientNames.values():
            raise Exception("Unknown client: " % clientName)
            return True

        self.agente_name = "highVoltageAgente"
        self.client_name = clientName
        self.currenttest = None
        self.ivDone = True
        self.leakageCurrentTestDone = True
        self.log = log
        self.active = True
        self.configFileNames = configFileNames

    def is_type(self, hvClientType):
        if hvClientType in self.clientNames:
            if self.clientNames[hvClientType] == self.client_name:
                return True
            else:
                return False
        else:
            raise Exception("Unknown client type: " % hvClientType)
            return False

    def supports_multichannel_hv(self):
        if self.is_type('KEITHLEY'):
            return False
        elif self.is_type('ISEG'):
            return True
        else:
            return False

    def setup_configuration(self, conf):
       # self.conf = conf
        self.numerator = 0
        if self.is_type('KEITHLEY'):
            self.subscription = conf.get("subsystem", "keithleySubscription")
            self.keithleyDir = conf.get('Directories','keithleyDir')
            self.keithleyPort = conf.get("keithleyClient","port")
        elif self.is_type('ISEG'):
            self.subscription = conf.get("subsystem", "isegSubscription")
            self.isegDir = conf.get('Directories','isegDir')
            self.isegPort = conf.get("isegClient","port")

        #do i need logdir?
        self.logDir = conf.get("Directories", "dataDir") + "/logfiles/"

    def setup_initialization(self, init):
        self.ivStart = float(init.get('IV','Start'))
        self.ivStop  = float(init.get('IV','Stop'))
        self.ivStep  = float(init.get('IV','Step'))
        self.ivDelay = float(init.get('IV','Delay'))

        if self.is_type('KEITHLEY'):
            self.leakageCurrentMeasurementTime = float(init.get('LeakageCurrent','Duration'))
            self.biasVoltage = -abs(float(init.get('Keithley','BiasVoltage')))
            self.active = init.getboolean("Keithley","KeithleyUse")
        elif self.is_type('ISEG'):
            self.leakageCurrentMeasurementTime = float(init.get('LeakageCurrent','Duration'))
            self.biasVoltage = -abs(float(init.get('Iseg','BiasVoltage')))
            self.active = init.getboolean("Iseg","IsegUse")
        else:
            raise Exception("Unknown client: " % self.client_name)
            return False


    def check_client_running(self):
        if not self.active:
            return False
#        return True
        process = os.system("ps aux | grep -v grep | grep -v vim | grep -v emacs | grep %s" % self.client_name)
        if type(process) == str and process != "":
            raise Exception("Another %s self.sclient is already running. Please close this self.sclient first." % self.sclient.name)
            return True
        return False

    def subscribe(self):
        if (self.active):
            self.sclient.subscribe(self.subscription)

    def start_client(self, timestamp):
        self.timestamp = timestamp
        if not self.active:
            return True
 #       return True

        if self.is_type('KEITHLEY'):
            command  = "xterm  -T 'HighVoltage' +sb -geometry 80x25+1200+1300 -fs 10 -fa 'Mono' -e "
            command += "%s/keithleyClient.py "%(self.keithleyDir)
            command += "-d %s "%(self.keithleyPort)
            command += "-dir %s "%(self.logDir)
            command += "-ts %s "%(self.timestamp)
            command += "-iV %s"%self.biasVoltage
        elif self.is_type('ISEG'):
            command  = "xterm  -T 'HighVoltage' +sb -geometry 80x25+1200+1300 -fs 10 -fa 'Mono' -e "
            command += "%s/isegClient.py "%(self.isegDir)
            command += "-d %s "%(self.isegPort)
            if len(self.configFileNames) > 0:
                command += "-c %s "%(','.join(self.configFileNames))
            command += "-dir %s "%(self.logDir)
            command += "-ts %s "%(self.timestamp)
            command += "-iV %s"%self.biasVoltage
        else:
            print "nothing to do..."

        self.log << '%s: Starting %s..."%s" '% (self.agente_name, self.client_name,command)

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

    def prepare_test(self, whichtest, env):
        if not self.active:
            return False
        # Run before a test is executed
        self.currenttest = whichtest.split('@')[0]
        #todo
        if 'IV' in self.currenttest:
            self.prepareIVCurve()
        elif 'leakagecurrent' in self.currenttest.lower():
            if self.is_type('ISEG'):
                self.sclient.send(self.subscription,':OUTP CLEAR\n')
                time.sleep(2)
                self.sclient.send(self.subscription,':OUTP ON\n')
            self.prepareLeakageCurrent()

        #todo: is the output on or off while cycling???/
        elif not whichtest == 'Cycle':
            if self.is_type('KEITHLEY'):
                self.sclient.send(self.subscription,':OUTP ON\n')
                time.sleep(1)
                self.sclient.send(self.subscription,':OUTP OFF\n')
                time.sleep(1)
                self.sclient.send(self.subscription,':OUTP ON\n')
            else:
                self.sclient.send(self.subscription,':OUTP CLEAR\n')
                time.sleep(2)
                self.sclient.send(self.subscription,':OUTP ON\n')
                time.sleep(1)

        return True


    def execute_test(self):
        if not self.active:
            return False
        # Runs a test
        if 'IV' in self.currenttest:
            self.doIVCurve()
        elif 'leakagecurrent' in self.currenttest.lower():
            if self.is_type('KEITHLEY'):
                time.sleep(3)
                self.sclient.send(self.subscription,':OUTP OFF\n')
                time.sleep(1)
                self.sclient.send(self.subscription,':OUTP ON\n')

            self.doLeakageCurrent()
        else:
            if self.is_type('KEITHLEY'):
                time.sleep(1)
                self.sclient.send(self.subscription,':OUTP ON\n')
            else:
                self.sclient.send(self.subscription,':OUTP ON\n')
                time.sleep(2)

        self.pending = True
        self.sclient.clearPackets(self.subscription)
        return True

    def cleanup_test(self):
        # Run after a test has executed
        if not self.active:
            return True
        else:
            self.sclient.send(self.subscription,':OUTP OFF\n')
            self.pending = False
        return True

    def final_test_cleanup(self):
        # Run after a test has executed
        if not self.active:
            return True
        return True

    def check_finished(self):
        if not self.active or not self.pending:
            return True
        while True:
            packet = self.sclient.getFirstPacket(self.subscription)
            if packet.isEmpty():
                break
            if not "pong" in packet.data.lower():
                data = packet.data
                if 'IV' in self.currenttest:
                    return self.checkIVCurveFinished(data)
                elif 'leakagecurrent' in self.currenttest.lower():
                    return self.checkLeakageCurrentFinished(data)
                else:
                    pass
            else:
                #self.log << "packet is Empty %s,\t pong in data.lower:%s"%(packet.isEmpty(),packet.data.lower())
                pass
        return self.ivDone and self.leakageCurrentTestDone

    def prepareIVCurve(self):
        time.sleep(1)
        self.sclient.clearPackets(self.subscription)
        self.ivDone = False
        self.sclient.send(self.subscription,':PROG:IV:START %s\n'%self.ivStart)
        self.sclient.send(self.subscription,':PROG:IV:STOP %s\n'%self.ivStop)
        self.sclient.send(self.subscription,':PROG:IV:STEP %s\n'%self.ivStep)
        self.sclient.send(self.subscription,':PROG:IV:DELAY %s\n'%self.ivDelay)
        #todo: wait for PROG:IV:TESTDIR

    def checkLeakageCurrentFinished(self,data):
        Time,coms,typ,msg = decode(data)[:4]
        if len(coms) > 1:
            if 'PROG' in coms[0].upper() and 'LEAKAGECURRENT' in coms[1].upper() and typ == 'a' and ('FINISHED' in msg.upper()):
                self.log << '\t--> leakageCurrent measurement FINISHED'
                self.leakageCurrentTestDone = True
            else:
                pass
        return self.leakageCurrentTestDone

    def prepareLeakageCurrent(self):
        time.sleep(1)
        self.sclient.clearPackets(self.subscription)
        self.leakageCurrentTestDone = False
        self.sclient.send(self.subscription,':PROG:LEAKAGECURRENT:TIME %s\n'%self.leakageCurrentMeasurementTime)

    def doIVCurve(self):
#        self.sclient.send(self.subscription,':PROG:IV:TESTDIR %s\n'%testdir)
        self.sclient.send(self.subscription,':PROG:IV MEAS\n')

    def doLeakageCurrent(self):
#        self.sclient.send(self.subscription,':PROG:IV:TESTDIR %s\n'%testdir)
        self.log << "run: doLeakageCurrent()"
        self.sclient.send(self.subscription,':PROG:LEAKAGECURRENT:START\n')

    def checkIVCurveFinished(self,data):
        Time,coms,typ,msg = decode(data)[:4]
        if len(coms) > 1:
            if 'PROG' in coms[0].upper() and 'IV' in coms[1].upper() and typ == 'a' and ('FINISHED' in msg.upper()):
                self.log << '\t--> IV-Curve FINISHED'
                self.ivDone = True
            else:
                pass
        return self.ivDone

    def checkDir(self,testDir):
        #todo
        return True
