#!/usr/bin/env python
#
# This program controls the PSI46-Testboards using the standard psi46expert software and executes Tests or opens and closes the TB
# it should work for N Testboards
# 
import sys
sys.path.insert(1, "../")
from myutils import sClient, printer, decode, BetterConfigParser, TBmaster
from myutils import colorgenerator as colorGenerator
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
Logger.set_logfile('%s'%(args.loggingDir),'psi46Handler.log')
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
serverPort = int(config.get('subsystem','Port'))
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

#Globals
numTB = 4
#
#busy = [False]*numTB
#testName =['unkown']*numTB
#testNo = [-1]*numTB
#failed = [False]*numTB
#TestEnd = [False]*numTB
#DoTest=[False]*numTB
#ClosePSI=[False]*numTB
#Abort=[False]*numTB

End=False
#MAINLOOP
color = colorGenerator.colorGenerator()
print 'PSI Master'
psiVersion = config.get('psiClient','psiVersion')
#ToDo:
#initGlobals(numTB)
#init TBmasters:
TBmasters=[]
for i in range(numTB):
    TBmasters.append(TBmaster(i, client, psiSubscription, Logger, next(color), psiVersion))

def openTB(TBno,msg,poff=False):
    if TBno>=0:
        TB = TBmasters[TBno]
        splittedMsg =msg.split(',')
        if len(splittedMsg) !=2:
            Logger.warning("openTB: couldnt convert Msg: %s --> %s"%(msg,splittedMsg))
            client.send(psiSubscription,'Cannot convert msg: %s -->%s'%(msg,splittedMsg))
            return
            #raise Exception
        dir, fname = splittedMsg
        if not TB.busy:
            TB.DoTest = Thread(target=TB.openTB, args=(dir,fname,poff,))
            TB.DoTest.start()
            TB.testNo = int(dir.rstrip('/').split('/')[-1].split('_')[0])
            TB.testName = 'open'
            
def closeTB(TBno):
    if TBno >=0:
        TB = TBmasters[TBno]
        Logger << 'trying to close TB...'
        TB.ClosePSI = True
        TB.testName = 'close'

def startTestTB(TBno,msg):
    if TBno>=0:
        TB = TBmasters[TBno]
        splittedMsg=msg.split(',')
        if len(splittedMsg) == 3:
            whichTest,dir,fname = splittedMsg
        else:
            Logger.warning("startTestTB: couldnt convert Msg: %s --> %s"%(msg,splittedMsg))
            client.send(psiSubscription,'Cannot convert msg: %s -->%s'%(msg,splittedMsg))
            return
        if not TB.busy:
            #Logger << whichTest
            TB.testName = whichTest.split('/')[-1]
            TB.testNo = int(dir.rstrip('/').split('/')[-1].split('_')[0])
            
            TB.busy = True
            Logger << 'got command to execute %s in TB%s  -- %s:%s'%(whichTest,TBno,TB.testNo,TB.testName)
            TB.DoTest = Thread(target=TB.executeTest, args=(whichTest,dir,fname,))
            TB.DoTest.start()
            name = TB.get_directory_name()
            if name =='':
                Logger << "Directory name not valid.....'%s'-whichTest '%s'"%(name,whichTest)
                client.send(psiSubscription,':STAT:TB%s! %s:started\n'%(TBno,name))
                TB.busy = True
                TB.sendTBStatus()
        else:
            Logger <<"Testboard %d is still busy cannot execute %s" %(TBno, whichTest)
            name = TB.get_directory_name()
            TB.sendTBStatus()
            client.send(errorSubscription, '%s: Cannot start test %s - TB %s is busy'%(clientName,whichTest,TBno))

def killTestTB(TBno):
    if TBno>=0:
        TB = TBmasters[TBno]
        if not TB.DoTest:
            Logger << 'nothing to be killed!'
        else:
            TB.failed = True
            TB.busy = False
            TB.Abort = True
            Logger.warning('killing TB%s...'%TBno)


def analyseProg_TB(coms,msg,typ,TBno):
    if len(coms)==3:
        if typ == 'c':  
            if coms[2].startswith('open'):
                poff = False
                if 'poff' in coms[2]:
                    poff = True
                openTB(TBno,msg,poff)
            elif coms[2].startswith('close'):
                closeTB(TBno) and TBno >=0 
            elif coms[2].startswith('start'):
                startTestTB(TBno,msg)
            elif coms[2].startswith('kill'):
                killTestTB(TBno)
            else:
                Logger << 'unknown command: %s, %s,%s'%(coms, msg,typ)
             
def exitProg():
   Logger << 'exit'
   #
   if not reduce(lambda x,y: x or y, [TB.busy for TB in TBmasters]):
       global End
       End = True
   else:
       for TB in TBmasters:
           name = TB.get_directory_name()
           if TB.busy: 
               TB.sendTBStatus()
               client.send(psiSubscription,':STAT:TB%s! busy %s \n'%(TB,name))
               Logger << " TB %s still busy with %s "%(TB,name)

def sendStatsTB(TBno):
    if len(TBmasters) > TBno: 
        TB = TBmasters[TBno]
        name = TB.get_directory_name()
        TB.sendTBStatus()
        if TB.busy:
            client.send(psiSubscription,':STAT:TB%s! %s:busy\n'%(TBno,name))
            return
        elif TB.failed:
            client.send(psiSubscription,':STAT:TB%s! %s:failed\n'%(TBno,name))
            return
        elif TB.TestEnd:
            client.send(psiSubscription,':STAT:TB%s! %s:finished\n'%(TBno,name))
            return
        else:
            pass
    client.send(psiSubscription,':STAT:TB%s! status:unknown\n'%TBno)


def analysePacket(packet):
    time,coms,typ,msg,cmd = decode(packet.data)
    coms = [x.lower() for x in coms]
    try:
        TBno=int(coms[1][2:])
    except:
        TBno = -1
        pass
    if(len(coms)>=2):
        if coms[0].startswith('prog') and coms[1].startswith('tb'):
            analyseProg_TB(coms,msg,typ,TBno)
            return
        elif coms[0].startswith('exit'):
            exitProg()
            return
        elif coms[0].startswith('stat') and coms[1].startswith('tb') and typ == 'q':
            sendStatsTB(TBno)
            return
    elif len(coms)>0:
        if coms[0].startswith('exit'):
            exitProg()
            return
    Logger << 'unknown command: %s, %s'%(coms, msg) 


#RECEIVE COMMANDS (mainloop)

signal.signal(signal.SIGINT, handler)
counter = 0 
while client.anzahl_threads > 0 and not End:
    packet = client.getFirstPacket(psiSubscription)
    if packet.isEmpty():
        sleep(.1)
        continue
    if not "pong" in packet.data.lower():
        analysePacket(packet)
        counter += 1 
        #if counter %100 == 0:
            #Logger << time,coms,typ,msg
        #Logger << cmd
    else:
        sleep(.5)
        #Logger << 'waiting for answer...\n'


#final stats....
for TB in TBmasters:
    #if TB.failed: 
    #    client.send(psiSubscription,':STAT:TB%s! test:failed\n'%i)
    #elif TB.TestEnd: client.send(psiSubscription,':STAT:TB%s! test:finished\n'%i)
    TB.sendTBStatus()
    if TB.failed: 
        pass
        #client.send(psiSubscription,':STAT:TB%s! test:failed\n'%i)
    elif TB.TestEnd:
        pass
        #client.send(psiSubscription,':STAT:TB%s! test:finished\n'%i)

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
