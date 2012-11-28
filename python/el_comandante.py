#!/usr/bin/env python
from BetterConfigParser import BetterConfigParser
from sclient import *
from decode import *
from time import strftime, gmtime
import time
from shutil import copytree,rmtree
from testboardclass import Testboard as Testboarddefinition
import os,sys
import subprocess
import argparse
from colorprinter import printer

def killChildren():
    print 'killChildren'
    try:
        for subscription in subscriptionList:
            client.send(subscription,':prog:exit\n')
        sleep(1)
    except:
        pass
    try:
        client.closeConnection()
    except:
        pass
    try:
        psiChild.kill()
    except:
        pass
    try:
        keithleyChild.kill()
    except:
        pass
    try:
        jumoChild.kill()
    except:
        pass

def handler(signum, frame):
    try:
        for subscription in subscriptionList:
            client.send(subscription,':prog:exit\n')
    except:
        pass
    killChildren();
    try:
        Logger << 'Close Connection'
    except:
        pass
    sleep(1)
    try:
        client.closeConnection()
        Logger << 'Signal handler called with signal %s'%signum
    except:
        pass
    sys.exit(0)
signal.signal(signal.SIGINT, handler)
try:
#get timestamp
    timestamp = int(time.time())
#------------some configuration--------------

    parser = argparse.ArgumentParser()

    parser.add_argument("-c", "--config", dest="configDir",
                           help="specify directory containing config files e.g. ../config/",
                           default="../config/")

    args = parser.parse_args()
#print args.configDir
    configDir= args.configDir
    try: 
        os.access(configDir,os.R_OK)
    except:    
        raise Exception('configDir \'%s\' is not accessible'%configDir)
        #sys.exit()
        #raise SystemExit

#load config
    config = BetterConfigParser()
    config.read(configDir+'/elComandante.conf')
#load init
    init = BetterConfigParser()
    init.read(configDir+'/elComandante.ini')

    Directories={}

    Directories['configDir']=configDir
    Directories['baseDir']=config.get('Directories','baseDir')
    Directories['testdefDir']=config.get('Directories','testDefinitions')
    Directories['dataDir']=config.get('Directories','dataDir')
    Directories['defaultDir']=config.get('Directories','defaultParameters')
    Directories['subserverDir']=config.get('Directories','subserverDir')
    Directories['keithleyDir']=config.get('Directories','keithleyDir')
    Directories['jumoDir']=config.get('Directories','jumoDir')
    Directories['logDir']=Directories['dataDir']+'/logfiles/'

    for dir in Directories:
        #if "$configDir$" in Directories[dir]:
        Directories[dir] = os.path.abspath(Directories[dir].replace("$configDir$",configDir))
#print Directories
    try:
        os.stat(Directories['dataDir'])
    except:
        os.mkdir(Directories['dataDir'])
    try:
        os.stat(Directories['subserverDir'])
    except:
        os.mkdir(Directories['subserverDir'])

    print 'check logdirectory'
    try:
        logFiles = (os.listdir(Directories['logDir']))
        nLogFiles = len(logFiles)
        print nLogFiles
    except:
        os.mkdir(Directories['logDir'])
        print 'mkdir'
    else:
        print nLogFiles
        if nLogFiles>0:
            answer = raw_input('Do you want to overwrite \'%s\'? [y]es or [n]o\n\t'%logFiles)
            if 'y' in answer.lower():
                rmtree(Directories['logDir'])
                os.mkdir(Directories['logDir'])
            else:
                raise Exception('LogDir is not empty. Please clean logDir: %s'%Directories['logDir'])


#initialise Logger
    Logger = printer()
    Logger.timestamp = timestamp
    Logger.set_logfile('%s/elComandante.log'%(Directories['logDir']))
    Logger<<'Set LogFile to %s'%Logger.f
#check if subsystem server is running, if not START subserver

    if os.system("ps -ef | grep -v grep | grep subserver"):
        os.system("cd %s && subserver"%(Directories['subserverDir']))

    if os.system("ps -ef | grep -v grep | grep subserver"):
        raise Exception("Could not start subserver");

#read subserver settings
    serverZiel=config.get('subsystem','Ziel')
    Port = int(config.get('subsystem','Port'))
    serverPort = int(config.get('subsystem','serverPort'))
    coolingBoxSubscription = config.get('subsystem','coolingBoxSubscription')
    keithleySubscription = config.get('subsystem','keithleySubscription')
    psiSubscription = config.get('subsystem','psiSubscription')
#create subserver client
    client = sClient(serverZiel,serverPort,"kuehlingboxcommander")
#subscribe subscriptions
    subscriptionList = [keithleySubscription,coolingBoxSubscription,psiSubscription]
    for subscription in subscriptionList:
        client.subscribe(subscription)

#handler
 #directory config
    Logger.printw() #welcome message
#get list of tests to do:
    testlist=init.get('Tests','Test')
    testlist= testlist.split(',')
    while '' in testlist:
            testlist.remove('')

#-------------------------------------


#-----------setup Test directory function-------
    def setupdir(Testboard):
        Logger.printn()
        Logger << 'I setup the directories:'
        Logger << '\t- %s'%Testboard.testdir
        Logger << '\t  with default Parameters from %s'%Testboard.defparamdir
        #copy directory
        try:
            copytree(Testboard.defparamdir, Testboard.testdir)
            f = open( '%s/configParameters.dat'%Testboard.testdir, 'r' )
            lines = f.readlines()
            f.close()
            lines[0]='testboardName %s'%Testboard.address
            f = open( '%s/configParameters.dat'%Testboard.testdir, 'w' )
            f.write(''.join(lines))
            f.close()
        except IOError as e:
            Logger.warning("I/O error({0}): {1}".format(e.errno, e.strerror))
        except OSError as e: 
            Logger.warning("OS error({0}): {1}".format(e.errno, e.strerror))
        #change address
#------------------------------------------------


#
    def stablizeTemperature(temp):
#-------------set temp----------------
        stable = False
        Logger << '\t Stablize CoolingBox Temperature @ %s degrees'%temp
        client.clearPackets(coolingBoxSubscription)
        client.send(coolingBoxSubscription,':prog:start\n')
        client.send(coolingBoxSubscription,':PROG:TEMP %s\n'%temp)
        #client.receiveThread() 
        sleep(1.0)
        client.clearPackets(coolingBoxSubscription)
        client.send(coolingBoxSubscription,':prog:stat?\n')
        i = 0
        while client.anzahl_threads > 0 and not stable:
            sleep(.5)
            packet = client.getFirstPacket(coolingBoxSubscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
                if len(coms) > 1:
                    if coms[0].find('PROG')>=0 and coms[1].find('STAT')>=0 and typ == 'a' and (msg == 'stable' or msg =='STABLE'):
                        Logger << '\t--> Got information to be stable at %s from packet @ %s'%(int(time.time()),Time)
                        Logger << '\t--> Temp is stable now. I begin with the %s'%(whichtest)
                        stable = True
                    elif coms[0][0:4] == 'PROG' and coms[1][0:4] == 'STAT' and typ == 'a':
                        if not i%10:
                            Logger << '\t--> Jumo is in status %s'%(msg)
                        if 'waiting' in msg.lower():
                            client.send(coolingBoxSubscription,':prog:start\n')
                            client.send(coolingBoxSubscription,':PROG:TEMP %s\n'%temp)
                        i+=1
                else:
                    pass
            else:
                client.send(coolingBoxSubscription,':prog:stat?\n')
                pass
        #-------------temp stable----------------
#
#-----------cycle function-----------------------
    def doCycle():
            highCycleTemp = float(init.get('Cycle','highTemp'))
            lowCycleTemp = float(init.get('Cycle','lowTemp'))
            nCycles = int(init.get('Cycle','nCycles'))
            client.send(coolingBoxSubscription,':prog:cycle:highTemp %s\n'%highCycleTemp)
            client.send(coolingBoxSubscription,':prog:cycle:lowTemp %s\n'%lowCycleTemp)
            client.send(coolingBoxSubscription,':prog:cycle %s\n'%nCycles)
            Logger << 'Temperature cycling with %s cycles between %s and %s'%(nCycles,lowCycleTemp,highCycleTemp)
            cycleDone = False
            while client.anzahl_threads >0 and not cycleDone:
                sleep(.5)
                packet = client.getFirstPacket(coolingBoxSubscription)
                if not packet.isEmpty():
                    #DONE
                    data = packet.data
                    Time,coms,typ,msg = decode(data)[:4]
                    if len(coms) > 1:
                        if coms[0].find('PROG')>=0 and coms[1].find('CYCLE')>=0 and typ == 'a' and (msg == 'FINISHED'):
                            Logger << '\t--> Cycle FINISHED'
                            cycleDone = True
                        else:
                            pass
                    else:
                        pass
                    pass
                else:
                    pass
#------------------------------------------------


    def setupParentDir(timestamp,Testboard):
            Testboard.parentdir=Directories['dataDir']+'/%s_%s_%s/'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",gmtime(timestamp)),timestamp)
            try:
                os.stat(Testboard.parentdir)
            except:
                os.mkdir(Testboard.parentdir)
            return Testboard.parentdir
#
    def doPSI46Test(whichtest):
#-------------start test-----------------
        for Testboard in Testboards:
            #Setup Test Directory
            Testboard.timestamp=timestamp
            Testboard.currenttest=item
            Testboard.testdir=Testboard.parentdir+'/%s_%s/'%(int(time.time()),Testboard.currenttest)
            setupdir(Testboard)
            #Start PSI
            Testboard.busy=True
            #client.send(psiSubscription,':prog:TB1:start Pretest,~/supervisor/singleRocTest_TB1,commanderPretest')
            client.send(psiSubscription,':prog:TB%s:start %s,%s,commander_%s\n'%(Testboard.slot,Directories['testdefDir']+'/'+ whichtest,Testboard.testdir,whichtest))
            Logger.printn()
            Logger << 'psi46 at Testboard %s is now started'%Testboard.slot

        #wait for finishing
        busy = True
        while client.anzahl_threads > 0 and busy:
            sleep(.5)
            packet = client.getFirstPacket(psiSubscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
                if coms[0].find('STAT')==0 and coms[1].find('TB')==0 and typ == 'a' and msg=='test:finished':
                    index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                    #print Testboards[index].tests
                    #print Testboards[index].currenttest
                    Testboards[index].finished()
                    Testboards[index].busy=False
                if coms[0][0:4] == 'STAT' and coms[1][0:2] == 'TB' and typ == 'a' and msg=='test:failed':
                    index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                    Testboards[index].failed()
                    Testboards[index].busy=False
                    
            packet = client.getFirstPacket(coolingBoxSubscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
                #nnprint "MESSAGE: %s %s %s %s "%(Time,typ,coms,msg.upper()) 
                if coms[0].find('STAT')==0 and typ == 'a' and 'ERROR' in msg[0].upper():
                    Logger.warning('jumo has error!')
                    Logger.warning('\t--> I will abort the tests...')
                    Logger.printn()
                    for Testboard in Testboards:
                        client.send(psiSubscription,':prog:TB%s:kill\n'%Testboard.slot)
                        Logger.warning('\t Killing psi46 at Testboard %s'%Testboard.slot)
                        index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                        Testboard.failed()
                        Testboard.busy=False
            busy=reduce(lambda x,y: x or y, [Testboard.busy for Testboard in Testboards])
        #-------------test finished----------------
        
        
        #---------------Test summary--------------
        Logger.printv()
        for Testboard in Testboards:
                client.send(psiSubscription,':stat:TB%s?\n'%Testboard.slot)
                received=False
                while client.anzahl_threads > 0 and not received:
                    sleep(.1)
                    packet = client.getFirstPacket(psiSubscription)
                    if not packet.isEmpty() and not "pong" in packet.data.lower():
                        data = packet.data
                        Time,coms,typ,msg = decode(data)[:4]
                        if coms[0][0:4] == 'STAT' and coms[1][0:3] == 'TB%s'%Testboard.slot and typ == 'a':
                            received=True
                            if msg == 'test:failed':
                                Logger.warning('\tTest in Testboard %s failed! :('%Testboard.slot)
                            elif msg == 'test:finished':
                                Logger << '\tTest in Testboard %s successful! :)'%Testboard.slot
                            else:
                                Logger << '%s %s %s %s @ %s'%(Time,coms,typ,msg,int(time.time()))
                                Logger.printn()
                                Logger.warning('\tStatus of Testboard %s unknown...! :/'%Testboard.slot)
        Logger.printv()
        #---------------iterate in Testloop--------------
        
#
#-----------IV function-----------------------
    def doIVCurve():
        for Testboard in Testboards:
            Testboard.timestamp=timestamp
            Testboard.currenttest=item
            Testboard.testdir=Testboard.parentDir+'/%s_IV_%s'%(timestamp, Testboard.module)
            setupdir(Testboard)
            Logger << 'DO IV CURVE for Testboard slot no %s'%Testboard.slot
            #%(Testboard.address,Testboard.module,Testboard.slot),Testboard
            ivStart = float(init.get('IV','Start'))
            ivStop  = float(init.get('IV','Stop'))
            ivStep  = float(init.get('IV','Step'))
            ivDone = False
            client.send(keithleySubscription,':PROG:IV:START %s'%ivStart)
            client.send(keithleySubscription,':PROG:IV:STOP %s'%ivStop)
            client.send(keithleySubscription,':PROG:IV:STEP %s'%ivStep) 
            client.send(psiSubscription,':prog:TB%s:open %s,commander_%s\n'%(Testboard.slot,Testboard.testdir,whichtest))
            sleep(2.0)	
            client.send(keithleySubscription,':PROG:IV MEAS\n')
            while client.anzahl_threads >0 and not ivDone:
                    sleep(.5)
                    packet = client.getFirstPacket(keithleySubscription)
                    if not packet.isEmpty() and not "pong" in packet.data.lower():
                        #DONE
                        data = packet.data
                        Time,coms,typ,msg,fullComand = decode(data)
                        if len(coms) > 1:
                            if coms[0].find('PROG')>=0 and coms[1].find('IV')>=0 and typ == 'a' and (msg == 'FINISHED'):
                                Logger << '\t--> IV-Curve FINISHED'
                                ivDone = True
                            elif coms[0].find('IV')==0 and typ == 'q':
                                #print fullComand                            
                                pass
                        else:
                            pass
                        pass
                    else:
                        pass

        Logger << 'try to close TB'
        client.send(psiSubscription,':prog:TB%s:close %s,commander_%s\n'%(Testboard.slot,Testboard.testdir,whichtest))
    def preexec():#Don't forward Signals.
        os.setpgrp()
                
    for clientName in ["jumoClient","psi46handler","keithleyClient"]:
        if not os.system("ps aux |grep -v grep| grep -v vim|grep -v emacs|grep %s"%clientName):
            raise Exception("another %s is already running. Please Close client first"%clientName)
#open psi46handler in annother terminal
    psiChild = subprocess.Popen("xterm +sb -geometry 120x20+0+900 -fs 10 -fa 'Mono' -e python psi46handler.py ", shell=True,preexec_fn = preexec)
#psiChild = subprocess.Popen("xterm +sb -geometry 160x20+0+00 -fs 10 -fa 'Mono' -e python psi46handler.py ", shell=True)


#open jumo handler
    jumoChild = subprocess.Popen("xterm +sb -geometry 80x25+1200+0 -fs 10 -fa 'Mono' -e %s/jumoClient -d %s"%(Directories['jumoDir'],config.get("jumoClient","port")), shell=True,preexec_fn = preexec)
#open Keithley handler
    keithleyChild = subprocess.Popen("xterm +sb -geometry 80x25+1200+1300 -fs 10 -fa 'Mono' -e %s/keithleyClient.py -d %s -dir %s -ts %s"%(Directories['keithleyDir'],config.get("keithleyClient","port"),Directories['logDir'],timestamp), shell=True,preexec_fn = preexec)
#check subscriptions?
    Logger<<"Check Subscription of the Clients:"
    time.sleep(2)
    for subscription in subscriptionList:
        if not client.checkSubscription(subscription):
            raise Exception("Cannot read from %s subscription"%subscription)
        else:
            Logger << "%s is answering"%subscription
        
#-------------SETUP TESTBOARDS----------------
    Logger << 'I found the following Testboards with Modules:'
    Logger.printn()
    Testboards=[]
    for tb, module in init.items('Modules'):
        if init.getboolean('TestboardUse',tb):
            Testboards.append(Testboarddefinition(int(tb[2]),module,config.get('TestboardAddress',tb),init.get('ModuleType',tb)))
            Testboards[-1].tests=testlist
            Testboards[-1].defparamdir=Directories['defaultDir']+'/'+config.get('defaultParameters',Testboards[-1].type)
            #print Testboards[-1].defparamdir
            Logger << '\t- Testboard %s at address %s with Module %s'%(Testboards[-1].slot,Testboards[-1].address,Testboards[-1].module)
            parentDir=setupParentDir(timestamp,Testboards[-1])
    Logger.printv()
    Logger << 'I found the following Tests to be executed:'
    Logger.printn()
    for item in testlist:
        if item.find('@')>=0:
            whichtest, temp = item.split('@')
        else:
            whichtest = item
            temp = 17.0
        Logger << '\t- %s at %s degrees'%(whichtest, temp)
#------------------------------------------


#--------------LOOP over TESTS-----------------
#print testlist
    for item in testlist:
     #   print item
        sleep(1.0)
        if item == 'Cycle':
            doCycle()
#    elif item == 'IV':
#        if(item.find('@'))
#        doIVCurve()        
        else:
            client.send(keithleySubscription,':OUTP ON\n')
            if item.find('@')>=0:
                whichtest, temp = item.split('@')
            else:
                whichtest = item
                temp =17.0
            Logger.printv()
            Logger << 'I do now the following Test:'
            Logger << '\t%s at %s degrees'%(whichtest, temp)
            
            stablizeTemperature(temp)
            if whichtest == 'IV':
                doIVCurve()
            else:
                doPSI46Test(whichtest)
        client.send(keithleySubscription,':OUTP OFF\n')        

#-------------Heat up---------------
    client.send(psiSubscription,':prog:exit\n')    
    Logger << 'heating up coolingbox...'
    client.send(coolingBoxSubscription,':prog:next\n')
    client.closeConnection()
    Logger << 'I am done for now!'

    sleep(1)
    killChildren()
    sleep(1)
#-------------EXIT----------------
    while client.anzahl_threads > 0: 
        pass
    Logger.printv()
    Logger << 'ciao!'
    del Logger
    try:
        os.stat(Directories['logDir'])
    except:
        raise Exception("Couldn't find logDir %s"%Directories['logDir'])

    for Testboard in Testboards:
            try:
                copytree(Directories['logDir'],Testboard.parentdir+'logfiles')
            except:
                raise
                #raise Exception('Could not copy Logfiles into testDirectory of Module %s\n%s ---> %s'%(Testboard.module,Directories['logDir'],Testboard.parentdir))

    rmtree(Directories['logDir'])
except:
    print 'kill Children'
    killChildren()
    print 'DONE'
    raise 
    sys.exit(0)
