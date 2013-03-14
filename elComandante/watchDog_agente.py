
import os
import subprocess
import sys
sys.path.insert(1,"../")
from myutils import sClient,decode,printer,preexec
from shutil import copyfile
import time
import el_agente
import operator
from sets import Set


class watchDog_agente(el_agente.el_agente):
    def __init__(self,timestamp,log,sclient):
        el_agente.el_agente.__init__(self, timestamp, log, sclient) 
        self.agente_name = "watchDogAgente"
        self.client_name = "watchDog"
        self.log = printer()
        self.sclient = None
        self.active = True
        self.pending = False
        self.currentTestTempLogger = None
        self.testOverview ={}
        self.FAILED  = -1
        self.SUCCESS = +1
        self.UNKOWN  = -2
        self.BUSY    = 0
        
        
    def setup_configuration(self, conf):
        #self.port = conf.get("jumoClient","port")
        self.logDir = conf.Directories['logDir']
        self.configFile = conf.Directories['configDir']+'/elComandante.conf'
        self.initFile = conf.Directories['configDir']+'/elComandante.ini'
#                                       get("Directories","dataDir")+"logfiles"
        self.logFileName = "temperature.log"
        self.tempLog = printer()
        self.tempLog.set_name('Temperature')
        self.tempLog.set_logfile(self.logDir,self.logFileName)
        self.tempLog.disable_print()

        #todo find a better way to define list...
        self.subscriptions = {}
        self.subscriptions['temp'] = "/temperature/jumo"
        self.subscriptions['psi'] = "/psi"
        serverZiel=conf.get('subsystem','Ziel')
        serverPort = conf.getint('subsystem','Port')
        self.sclient = sClient(serverZiel,serverPort,"watchDog")
        return True
    
    def setup_initialization(self, init):
        return True;
    def check_logfiles_presence(self):
        return []
    def check_subscription(self):
        return True

    def check_client_running(self):
        return False

    def start_client(self, timestamp):
        try:
            self.subscribe()
            return True
        except:
            return False

    
    def subscribe(self):
        if(self.active):
            for subscription in self.subscriptions:
                self.sclient.subscribe(self.subscriptions[subscription])
#            self.sclient.subscribe(self.subscription)

    def request_client_exit(self):
        # Request the client to exit with a command
        # through subsystem
        if not self.active:
            return True
        if self.sclient:
            for subscription in self.subscriptions:
                self.sclient.unsubscribe(subscription)
            self.sclient.closeConnection()
        return True
    
    def kill_client(self):
    # Kill a client with a SIGTERM signal
        if self.sclient:
            self.sclient.closeConnection()
        if not self.active:
            return True
        return True


    def prepare_test(self, test, environment):
        # Run before a test is executed
        if not self.active:
            return True
        self.currentTest = test
        self.readTemperatures()
        self.check_dew_point()
        self.check_testboards()
        return True

    def execute_test(self):
        # Initiate a test
        self.readTemperatures()
        self.check_dew_point()
        self.check_testboards()
#        self.currentTestTempLogger = logging.getLogger('%sTemplog'%self.currentTest)
#        self.currentTestTempLogger.handler = logging.FileHandler('%s/temperature.log')
        return True


    def cleanup_test(self):
        self.currentTest = "none"
        # Run after a test has executed
        self.readTemperatures()
        self.check_dew_point()
        self.check_testboards()
        if not self.active:
            return True
        sortedOverview = sorted(self.testOverview.items())
        self.log << "%s: Status of Test"%self.agente_name
        for item in sortedOverview:
            TB = item [0]
            testDict = item[1]
            sortedTests = sorted(testDict.items())
            testNo = max(testDict.keys())
            self.log << "%s  %s-%s\t%s-->%s" %(' '*len(self.agente_name),TB,testNo,self.testOverview[TB][testNo][0], self.testOverview[TB][testNo][1])
#        self.log << msg
        #if 'waiting' not in self.status()
        #    self.log << "%s: Cleaning up %s ..."%(self.agente_name,test)
        return False

    
    def final_test_cleanup(self):
        # Cleanup after all tests have finished to return
        # everything to the state before the test

        # create Config directory
        sortedOverview = sorted(self.testOverview.items())
        self.log << "%s: Status of Test"%self.agente_name
        agenteFiller = ' '*len(self.agente_name)
        self.log << "%s   testNo - testName   \tStatus"%(agenteFiller)
        # create a set from test numbers
        testNumbers = Set()
        for item in sortedOverview:
            testDict = item[1]
            sortedTests = sorted(testDict.items())
            for test in sortedTests:
                testNumbers.add(test[0])
        msg = '%s testNo\t'%agenteFiller
        for item in sortedOverview:
            msg += 'TB%02d\t'%int(item[0])
        self.log << msg
        for testNo in sorted(testNumbers):
            num = int(testNo)
            msg = "%s   %03d:\t"%(agenteFiller,num)
            testStats = ''
            testName = ''
            for item in sortedOverview:
                TB = item [0]
                testDict = item[1]
                if testNo in testDict:
                    status = testDict[testNo]
                    if status[0] in testName:
                        testName +='/'
                    else:
                        testName += '%s/'%status[0]
                    testStat = status[1]
                    testStats += "%2s\t"%testStat
                else:
                    msg += " \t"
                    testName += '?/'
            self.log << '%s%s\t%s'%(msg,testStats,testName)
#                sortedTests = sorted(testDict.items())
#                if 'powercycle' in sortedTests[-1][1][0].lower():
#                    sortedTest=sortedTests[:-1]
#                for test in sortedTests:
#                    testNo = test[0]
#                    test[1][0]
#                    testStat = test[1][1]
#                    msg =  "%s  %4s: %6s - %10s\t%2s"%(' '*len(self.agente_name),TB,testNo,testName,testStat)
#                    if testStat == self.FAILED:
#                        self.log.warning(msg)
#                    else:
#                        self.log << msg
        return False
    
    def check_finished(self):
        # Check whether the client has finished its task
        # but also check for errors and raise an exception
        # if one occurs.
        self.readTemperatures()
        self.check_dew_point()
        self.check_testboards()
        return True

    def readTemperatures(self):
        while True:
            packet = self.sclient.getFirstPacket(self.subscriptions['temp'])
            if packet.isEmpty():
                break
            if "pong" in packet.data.lower():
                continue
            data = packet.data
            Time,coms,typ,msg = decode(data)[:4]
            if len(msg)>=1:
                self.tempLog << "%s\t %s"%(Time,msg[0])
        return True

    def check_testboard(self,data):
        if "pong" in data.lower():
            return False
        Time,coms,typ,msg = decode(data)[:4]
        if not typ == 'a':
            return False
        if not len(coms) == 2:
            return False
        if not coms[0].lower().startswith('stat'):
            return False
        if not coms[1].lower().startswith('tb'):
            return False
        TB_str = coms[1][2:];
        tb_no = int(TB_str)
        msg = msg.split(':')
        if len(msg) == 2:
            testName = msg[0].lower()
            testTemp = 17
            test_no = -1
            status = msg[1].lower()
            if  testName == 'test':
                self.log << "%s: got standard stats for TB %d: %s"%(self.agente_name,tb_no,msg)
                return True
            else:
                testName = testName.split("_")
                if len(testName) == 3:
                    test_no = testName[0]
                    testTemp = testName[2]
                    testName = testName[1]
                else:
                    self.log.warning("%s: Couln't convert testName: %s, '%s'"%(self.agente_name,testName,data))
            if test_no ==-1:
                self.log.warning("Couln't read valid test_no for TB_No %d: '%s'"%(tb_no,msg))
                return
            self.update_test_overview(tb_no, test_no, status,testName)
        pass
    
    def check_testboards(self):
        while True:
            packet = self.sclient.getFirstPacket(self.subscriptions['psi'])
            if packet.isEmpty():
                return False
            else:
                self.check_testboard(packet.data)
        
    
    def get_test_status(self,tb_no,test_no):
        status = self.UNKOWN
        if tb_no in self.testOverview:
            if test_no in self.testOverview[tb_no]:
                status = self.testOverview[tb_no][test_no]
                if len(status)>1:
                    status = status[1]
        return status

    def set_test_status(self,tb_no,test_no,test_name,status):
        oldStatus = self.get_test_status(tb_no, test_no)
        if oldStatus != self.UNKOWN:
#            self.log << "%s: Updating test %s for tb %s: %s --> %s"%(self.agente_name,test_no,tb_no,oldStatus,status)
            self.testOverview[tb_no][test_no] = [test_name,status]
        else:
            if not tb_no in self.testOverview:
#                self.log << "%s: adding new tb %s to testOverview"%(self.agente_name,tb_no)
                self.testOverview[tb_no] = {}
#            self.log << "%s: adding new test %s for tb %s to testOverview: %s"%(self.agente_name,test_no,tb_no,test_name)
            self.testOverview[tb_no][test_no] =[test_name,status]
                
        pass
    def update_test_overview(self,tb_no,test_no,status,test_name):
        oldStatus = self.get_test_status(tb_no, test_no)
        if status.startswith('finished'):
#            self.log << "%s: Test no %s '%s' on TB %d finished"%(self.agente_name,test_no,test_name,tb_no)
            self.set_test_status(tb_no, test_no, test_name, self.SUCCESS)
            pass
        elif status.startswith('failed'):
#            self.log <<"%s: Test '%s' on TB %d failed"%(self.agente_name,test_name,tb_no)
            self.set_test_status(tb_no, test_no, test_name, self.FAILED)
        elif status.startswith('busy'):
#            self.log <<"%s: Test '%s' on TB %d busy"%(self.agente_name,test_name,tb_no)
            self.set_test_status(tb_no, test_no, test_name, self.BUSY)
            pass
        pass
    
    def check_dew_point(self):
        pass
    def set_pending(self):
        #self.sclient.send(self.subscription,":FINISHED\n")
        self.pending = True
        

