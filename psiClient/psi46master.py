#!/usr/bin/env python
#
# This program controls the PSI46-Testboards using the standard psi46expert software and executes Tests or opens and closes the TB
# it should work for N Testboards
# 
import sys
sys.path.insert(1, "../")
from myutils import sClient, printer, decode, BetterConfigParser
from threading import Thread
import subprocess
import argparse
import signal
import select
from time import sleep
print 'hallo'
#------------some configuration--------------
parser = argparse.ArgumentParser()

parser.add_argument("-c", "--config", dest="configDir",
                       help="specify directory containing config files e.g. ../config/",
                       default="../config/")
parser.add_argument("-dir","--directory", dest="loggingDir",
                        help="specify directory containing all logging files e.g. ../DATA/logfiles/",
                        default="../DATA/logfiles")
parser.add_argument("-num","--numTB", dest="numTB",
                        help="specify the number of Testboards in use",
                        default="1")
#parse args and setup logdir
args = parser.parse_args()
Logger = printer()
Logger.set_name("Psi46Log")
Logger.set_prefix('')
Logger.set_logfile('%s/psi46Handler.log'%(args.loggingDir))
Logger <<'ConfigDir: "%s"'%args.configDir
configDir= args.configDir
numTB = int(args.numTB)
#load config
config = BetterConfigParser()
config.read(configDir+'/elComandante.conf')
#config
clientName = "psi46"
serverZiel=config.get('subsystem','Ziel')
Port = int(config.get('subsystem','Port'))
serverPort = int(config.get('subsystem','serverPort'))
psiSubscription = config.get('subsystem','psiSubscription')
errorSubscription = "/error"
#construct
client = sClient(serverZiel,serverPort,clientName)
#subscribe
client.subscribe(psiSubscription)
#----------------------------------------------------

#handler
def handler(signum, frame):
    Logger << 'Close Connection'
    client.closeConnection()
    #Logger << 'Signal handler called with signal', signum
signal.signal(signal.SIGINT, handler)

#color gadget
def colorGenerator():
    list=['green','blue','magenta','cyan']
    i=0
    while True:
        yield list[i]
        i = (i+1)%len(list)

class TBmaster(object):
    def __init__(self, TB, client, psiSubscription, Logger, color='black', psiVersion='psi46expert'):
        self.TB = TB
        self.client = client
        self.psiSubscription = psiSubscription
        self.color = color
        self.Logger = Logger
        self.TBSubscription = '/TB%s'%self.TB
        self.client.subscribe(self.TBSubscription)
        self.dir = ''
        self.psiVersion = psiVersion

    def _spawn(self,executestr):
        self.proc = subprocess.Popen([executestr,''], shell = True, stdout = subprocess.PIPE, stdin = subprocess.PIPE)
        busy[self.TB] = True

    def _kill(self):
        try:
            self.proc.kill()
            self.Logger.warning("PSI%s KILLED"%self.TB)
        except:
            self.Logger.warning("nothing to be killed")

    def _abort(self):
        self.Logger.warning('ABORT!')
        self._kill()
        Abort[self.TB] = False
        return True

    def _resetVariables(self):
        busy[self.TB] = False
        failed[self.TB] = False
        TestEnd[self.TB] = False
        DoTest[self.TB] = False
        ClosePSI[self.TB] = False
        Abort[self.TB] = False

    def _readAllSoFar(self, retVal = ''): 
        while (select.select([self.proc.stdout],[],[],0)[0]!=[]) and self.proc.poll() is None:   
            retVal += self.proc.stdout.read(1)
        return retVal

    @staticmethod
    def findError(stat):
        return any([Error in stat for Error in ['error','Error','anyOtherString','command not found']])

    def _readout(self):
        internalFailed = False
        self.Logger << '>>> Aquire Testboard %s <<<'%self.TB
#        self._answer(self)
        while self.proc.poll() is None and ClosePSI[self.TB]==False:
            if Abort[self.TB]:
                internalFailed = self._abort()
            lines = ['']
            lines = self._readAllSoFar(lines[-1]).split('\n')
            for a in range(len(lines)-1):
                line=lines[a]
                hesays=line.rstrip()
                self.client.send(self.TBSubscription,'%s\n'%hesays)
                self.Logger.printcolor("psi46@TB%s >> %s"%(self.TB,hesays),self.color)
                if self.findError(line.rstrip()):
                    self.Logger << 'The following error triggered the exception:'
                    self.Logger.warning(line.rstrip())
                    self.client.send(self.psiSubscription, 'psi46@TB%s - Error >> %s\n'%(self.TB,line.rstrip()))
                    self.client.send(self.TBSubscription, 'Error >> %s\n'%(line.rstrip()))
                    internalFailed = True
                    failed[self.TB] = True
                    self._kill()
                if 'command not found' in line.strip():
                    self.Logger.warning("psi46expert for TB%s not found"%self.TB)
                if Abort[self.TB]:
                    internalFailed = self._abort()
                    failed[self.TB] = internalFailed or failed[self.TB]
        self.Logger << '>>> Release Testboard %s <<<'%self.TB
        TestEnd[self.TB] = True
        busy[self.TB] = False
        return internalFailed

    def _answer(self):
        name = self.get_directory_name()
        
        if failed[self.TB]:
            self.client.send(self.psiSubscription,':STAT:TB%s! %s:failed\n'%(self.TB,name))
            self.Logger.warning(':Test %s failed in TB%s'%(name,self.TB))
            self.client.send(self.psiSubscription,':STAT:TB%s! %s:failed\n'%(self.TB,name))
        elif busy[self.TB]:
            self.client.send(self.psiSubscription,':STAT:TB%s! %s:busy\n'%(self.TB,name))
            self.Logger << ':Test %s busy in TB%s'%(name,self.TB)
        else:
            self.client.send(self.psiSubscription,':STAT:TB%s! %s:finished\n'%(self.TB,name))
            self.Logger << ':Test %s finished in TB%s'%(name,self.TB)
            
            
    def get_directory_name(self):
        dir = self.dir.rstrip('/')
        name = dir.split('/')[-1]
        return name

    def executeTest(self,whichTest,dir,fname):
        self._resetVariables()
        self.dir = dir
        self.Logger << 'executing psi46 %s in TB%s'%(whichTest,self.TB)
        executestr='%s -dir %s -f %s -r %s.root -log %s.log'%(psiVersion,dir,whichTest,fname,fname)
        self._spawn(executestr)
        failed[self.TB]=self._readout()
        self._answer()

    def openTB(self,dir,fname):
        self._resetVariables()
        self.dir = dir
        Logger << 'open TB%s'%(self.TB)
        executestr='%s -dir %s -r %s.root -log %s.log'%(psiVersion,dir,fname,fname)
        self._spawn(executestr)
        failed[self.TB]=self._readout()
        self._answer()
        while not ClosePSI[self.TB]:
            pass
        self.Logger << 'CLOSE TB %s HERE'%(self.TB)
        self.proc.communicate(input='exit\n')[0] 
        self.proc.poll()
        if (None == self.proc.returncode):
            try:
                self.proc.send_signal(signal.SIGINT)
            except:
                self.Logger << 'Process already killed'
        self._answer()



#Globals
global busy
global testName
global failed
global TestEnd
global DoTest
global ClosePSI
global Abort

#numTB = 4

busy = [False]*numTB
testName =['unkown']*numTB
testNo = [-1]*numTB
failed = [False]*numTB
TestEnd = [False]*numTB
DoTest=[False]*numTB
ClosePSI=[False]*numTB
Abort=[False]*numTB

End=False
#MAINLOOP
color = colorGenerator()
print 'PSI Master'

psiVersion = config.get('psiClient','psiVersion')
#ToDo:
#initGlobals(numTB)
#init TBmasters:
TBmasters=[]
for i in range(numTB):
    TBmasters.append(TBmaster(i, client, psiSubscription, Logger, next(color), psiVersion))

def openTB(TB,msg):
    if TB>=0:
        splittedMsg =msg.split(',')
        if len(splittedMsg) !=2:
            Logger.warning("openTB: couldnt convert Msg: %s --> %s"%(msg,splittedMsg))
            client.send(psiSubscription,'Cannot convert msg: %s -->%s'%(msg,splittedMsg))
            return
            #raise Exception
        dir, fname = splittedMsg
        if not busy[TB]:
            DoTest[TB] = Thread(target=TBmasters[TB].openTB, args=(dir,fname,))
            DoTest[TB].start()
            testNo[TB] = int(dir.rstrip('/').split('/')[-1].split('_')[0])
            testName[TB] = 'open'
def closeTB(TB):
    if TB >=0:
        Logger << 'trying to close TB...'
        ClosePSI[TB]=True
        testName[TB] = 'close'

def startTestTB(TB,msg):
    if TB>=0:
        splittedMsg=msg.split(',')
        if len(splittedMsg) == 3:
            whichTest,dir,fname = splittedMsg
        else:
            Logger.warning("startTestTB: couldnt convert Msg: %s --> %s"%(msg,splittedMsg))
            client.send(psiSubscription,'Cannot convert msg: %s -->%s'%(msg,splittedMsg))
            return
        if not busy[TB]:
            #Logger << whichTest
            testName[TB] = whichTest.split('/')[-1]
            testNo[TB] = int(dir.rstrip('/').split('/')[-1].split('_')[0])
            
            Logger << 'got command to execute %s in TB%s  -- %s:%s'%(whichTest,TB,testNo[TB],testName[TB])
            DoTest[TB] = Thread(target=TBmasters[TB].executeTest, args=(whichTest,dir,fname,))
            DoTest[TB].start()
            name = TBmasters[TB].get_directory_name()
            if name =='':
                Logger << "Directory name not valid.....'%s'-whichTest '%s'"%(name,whichTest)
                client.send(psiSubscription,':STAT:TB%s! %s:started\n'%(TB,name))
                busy[TB]=True
            else:
                Logger <<"Testboard %d is still busy cannot execute %s" %(TB, whichTest)
                name = TBmasters[TB].get_directory_name()
                client.send(psiSubscription,':STAT:TB%s! %s:busy\n'%(name,TB))
                client.send(errorSubscription, '%s: Cannot start test %s - TB %s is busy'%(clientName,whichTest,TB))

def killTestTB(TB):
    if TB>=0:
        if not DoTest[TB]:
            Logger << 'nothing to be killed!'
        else:
            failed[TB]=True
            busy[TB]=False
            Abort[TB]=True
            Logger.warning('killing TB%s...'%TB)


def analyseProg_TB(coms,msg,typ,TB):
    if len(coms)==3:
        if typ == 'c':  
            if coms[2].startswith('open'):
                openTB(TB,msg)
            elif coms[2].startswith('close'):
                closeTB(TB) and TB >=0 
            elif coms[2].startswith('start'):
                startTestTB(TB,msg)
            elif coms[2].startswith('kill'):
                killTestTB(TB)
            else:
                Logger << 'unknown command: %s, %s,%s'%(coms, msg,typ)
             
def exitProg():
   Logger << 'exit'
   if not reduce(lambda x,y: x or y, busy):
       End = True
   else:
       for TB in range(0,numTB):
           name = TBmasters[TB].get_directory_name()
       if busy[TB]: 
           client.send(psiSubscription,':STAT:TB%s! busy %s \n'%(TB,name))
           Logger << " TB %s still busy with %s "%(TB,name)

def sendStatsTB(TB):
    if len(TBmasters) > TB: 
        name = TBmasters[TB].get_directory_name()
        if len(busy)>TB and busy[TB]:
            client.send(psiSubscription,':STAT:TB%s! %s:busy\n'%(TB,name))
            return
        elif len(failed)>TB and failed[TB]:
            client.send(psiSubscription,':STAT:TB%s! %s:failed\n'%(TB,name))
            return
        elif len(TestEnd) >TB and TestEnd[TB]:
            client.send(psiSubscription,':STAT:TB%s! %s:finished\n'%(TB,name))
            return
    client.send(psiSubscription,':STAT:TB%s! status:unknown\n'%TB)


def analysePacket(packet):
    time,coms,typ,msg,cmd = decode(packet.data)
    coms = [x.lower() for x in coms]
    try:
        TB=int(coms[1][2:])
    except:
        TB = -1
    if(len(coms)>=2):
        if coms[0].startswith('prog') and coms[1].startswith('tb'):
            analyseProg_TB(coms,msg,typ,TB)
            return
        elif coms[0].startswith('prog') and coms[1].startswith('exit'):
            exitProg()
            return
        elif coms[0].startswith('stat') and coms[1].startswith('tb') and typ == 'q':
            sendStatsTB(TB)
            return
    Logger << 'unknown command: %s, %s'%(coms, msg) 

#RECEIVE COMMANDS (mainloop)
while client.anzahl_threads > 0 and not End:
    sleep(.5)
    packet = client.getFirstPacket(psiSubscription)
    counter = 0 
    if not packet.isEmpty() and not "pong" in packet.data.lower():
        analysePacket(packet)
        counter += 1 
        if counter %100 == 0:
            Logger << time,coms,typ,msg
        #Logger << cmd
        
    else:
        pass
        #Logger << 'waiting for answer...\n'


#final stats....
for i in range(0,numTB):
    if failed[i]: client.send(psiSubscription,':STAT:TB%s! test:failed\n'%i)
    elif TestEnd[i]: client.send(psiSubscription,':STAT:TB%s! test:finished\n'%i)

client.send(psiSubscription,':prog:stat! exit\n')    
print 'exiting...'
client.closeConnection()

#END
while client.anzahl_threads > 0: 
    Logger << 'waiting for client to be closed...'
    client.closeConnection()
    sleep(0.5)
    pass
Logger << 'done'
