#!/usr/bin/env python
import sys
sys.path.insert(1, "../")
from myutils import BetterConfigParser, sClient, decode, printer, preexec, testchain
from myutils import Testboard as Testboarddefinition
from time import strftime, gmtime
import time
from shutil import copytree,rmtree
import os
import subprocess
import argparse
import environment
import xray_agente
import psi_agente
import coolingBox_agente
import analysis_agente
import highVoltage_agente
import signal

los_agentes = []

def killChildren():
    print "Killing clients ..."

    # Ask the client processes to exit
    for agente in los_agentes:
        agente.request_client_exit()

    try:
        for subscription in subscriptionList:
            client.send(subscription,':prog:exit\n')
        time.sleep(1)
    except:
        pass

    # Close the subsystem client connection
    try:
        client.closeConnection()
    except:
        pass

    # Kill the client processes
    for agente in los_agentes:
        try:
            agente.kill_client()
        except:
            agente.log.warning("Could not kill %s" % agente.name)

def handler(signum, frame):
    print 'Signal handler called with signal %s' % signum
    killChildren();
    sys.exit(0)

#signal.signal(signal.SIGINT, handler)

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
    Directories['defaultParameters']=config.get('Directories','defaultParameters')
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
#i
    #check if subsystem server is running, if not START subserver
     
    if os.system("ps -ef | grep -v grep | grep subserver"):
        os.system("cd %s && subserver"%(Directories['subserverDir']))
        if os.system("ps -ef | grep -v grep | grep subserver"):
            raise Exception("Could not start subserver");

    #check if subsystem server is running, if not START subserver
    #if not "subserver.pid" in os.listdir("/var/tmp"):
    #    Logger << "Starting subserver ..."
    #    os.system("cd %s && subserver"%(Directories['subserverDir']))
    #    time.sleep(0.2)
    #    #check again whether it is running
    #    if not "subserver.pid" in os.listdir("/var/tmp"):
    #        raise Exception("Could not start subserver")
    Logger << "Subserver is running."

#read subserver settings
    serverZiel=config.get('subsystem','Ziel')
    Port = int(config.get('subsystem','Port'))
    serverPort = int(config.get('subsystem','serverPort'))
    coolingBoxSubscription = config.get('subsystem','coolingBoxSubscription')
    keithleySubscription = config.get('subsystem','keithleySubscription')
    psiSubscription = config.get('subsystem','psiSubscription')
    xraySubscription = config.get('subsystem','xraySubscription')
    analysisSubscription = config.get('subsystem','analysisSubscription')

#create subserver client
    client = sClient(serverZiel,serverPort,"elComandante")

    # Create agentes that are responsible for client processes
    los_agentes.append(psi_agente.psi_agente(timestamp, Logger, client))
    los_agentes.append(highVoltage_agente.highVoltage_agente(timestamp,Logger,client))
    if init.getboolean("Xray", "XrayUse"):
        los_agentes.append(xray_agente.xray_agente(timestamp, Logger, client))
    los_agentes.append(analysis_agente.analysis_agente(timestamp, Logger, client))
    if init.getboolean("CoolingBox", "CoolingBoxUse"):
        los_agentes.append(coolingBox_agente.coolingBox_agente(timestamp, Logger,client))
    
    print 'agentes: '
    print [agente.name for agente in los_agentes]

    # Make the agentes read their configuration and initialization parameters
    for agente in los_agentes:
        Logger << "setup of Agente: %s"% agente.name
        agente.setup_dir(Directories)
        agente.setup_configuration(config)
        agente.setup_initialization(init)
        
#subscribe subscriptions
    subscriptionList = []
    if init.getboolean("CoolingBox", "CoolingBoxUse"):
        subscriptionList.append(coolingBoxSubscription)
    for subscription in subscriptionList:
        client.subscribe(subscription)

    for agente in los_agentes:
        agente.subscribe()

 #directory config
    Logger.printw() #welcome message
#get list of tests to do:
    testlist=init.get('Tests','Test')
    test_chain = testchain.parse_test_list(testlist)
    print test_chain
    testlist= testlist.split(',')
    while '' in testlist:
            testlist.remove('')

    # Get default parameter directories
    dir_list = []
    tb = 0
    while True:
        try:
            module_type = init.get("ModuleType", "TB" + `tb`)
            dir = config.get("defaultParameters", module_type)
        except:
            break
        else:
            dir = Directories['defaultParameters'] + '/' + dir
            dir_list.append(dir)
            tb += 1
    test_chain.parameter_dir = dir_list

#-------------------------------------
    def setupParentDir(timestamp,Testboard):
            Testboard.parentDir=Directories['dataDir']+'/%s_%s_%s/'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",gmtime(timestamp)),timestamp)
            try:
                os.stat(Testboard.parentDir)
            except:
                os.mkdir(Testboard.parentDir)
            return Testboard.parentDir
        
    def waitForFinished(los_agentes):
        os.system('setterm -cursor off')
        finished = False
        while not finished:
            time.sleep(1.0)
            finished = all([agente.check_finished() for agente in los_agentes])
            output = ' \t'
            for agente in los_agentes:
                output += '%s: %s\t'%(agente.name,int(agente.check_finished()))
            if not finished:
                sys.stdout.write('%s\r' %output)
            sys.stdout.flush()
        print ''


    # Check whether the client is already running before trying to start it
    Logger << "Check whether clients are runnning ..."
    for agente in los_agentes:
        Logger << "%s: "%agente.name
        agente.check_client_running()

    for agente in los_agentes:
        agente.start_client(timestamp)


#check subscriptions?

    # Check the client subscriptions
    Logger<<"Check Subscription of the Clients:"
    time.sleep(2)
    for subscription in subscriptionList:
        if not client.checkSubscription(subscription):
            raise Exception("Cannot read from %s subscription"%subscription)
        else:
            Logger << "\t%s is answering"%subscription
    for agente in los_agentes:
        if not agente.check_subscription():
            raise Exception("Cannot read from %s subscription" % agente.subscription)
        else:
            Logger << "\t%s is answering" % agente.subscription
    Logger << "Subscriptions checked"

#-------------SETUP TESTBOARDS----------------
    Logger << 'I found the following Testboards with Modules:'
    Logger.printn()
    #ToDo:
    for Testboard in los_agentes[0].Testboards:
            parentDir=setupParentDir(timestamp,Testboard)
            Logger << '\t- Testboard %s at address %s with Module %s'%(Testboard.slot,Testboard.address,Testboard.module)


    Logger.printv()
    Logger << 'I found the following Tests to be executed:'
    Logger.printn()
    testlist2 = []
    #for item in testlist:
    #    if item.find('@')>=0:
    #        whichtest, env = item.split('@')
    #    else:
    #        whichtest = item
    #        env = 17.0
    #    if 'IV' in item:
    #        for Testboard in los_agentes[0].Testboards:
    #            testlist2.append('%s_TB%s@%s'%(whichtest,Testboard.slot,env))
    #            Logger << '\t- %s_TB%s at %s degrees'%(whichtest,Testboard.slot, env)
    #    else:
    #        testlist2.append(item)
    #        Logger << '\t- %s at %s degrees'%(whichtest, env)
    #testlist = testlist2
    test = test_chain.next()
    while test:
        Logger << "\t- %s" % test.test_str
        test = test.next()
#------------------------------------------


    Logger.printv()

#--------------LOOP over TESTS-----------------

    test = test_chain.next()
    while test:
        Logger << test.test_str
        env = environment.environment(test.test_str, init)
        test.environment = env
        test.testname = test.test_str.split("@")[0]
        temp = env.temperature

        for agente in los_agentes:
            agente.set_test(test)

        # Prepare for the tests
        Logger << "Prepare Test: %s" % test.test_str
        for agente in los_agentes:
            agente.prepare_test(test.test_str, env)
        # Wait for preparation to finish
        Logger << "Wait for preparation to finish"
        finished = False
        waitForFinished(los_agentes)

                
        Logger << "Prepared for test %s" % test.test_str

        # Execute tests
        Logger.printv()
        Logger << "Execute Test: %s" % test.test_str
        for agente in los_agentes:
            agente.execute_test()
            time.sleep(1.0)            

        # Wait for test execution to finish
        Logger << "Wait for test execution to finish"
        waitForFinished(los_agentes)
        Logger << "Test %s has been finished." % test.test_str

        # Cleanup tests
        Logger << "start with Clean Up tests."
        for agente in los_agentes:
            agente.cleanup_test()

        # Wait for cleanup to finish
        waitForFinished(los_agentes)
        Logger << " Clean Up tests Done"
        Logger.printv()

        test = test.next()

    # Final cleanup
    Logger << "Do final Clean Up after all tests"
    for agente in los_agentes:
        agente.final_test_cleanup()

    # Wait for final cleanup to finish
    Logger << "Wait for final clean up to finish"
    waitForFinished(los_agentes)
    Logger << "Final Clean up has been done"

#-------------Heat up---------------
    #client.send(psiSubscription,':prog:exit\n')

    for agente in los_agentes:
        agente.request_client_exit()

    client.closeConnection()
    Logger << 'I am done for now!'

    time.sleep(1)
    killChildren()
    time.sleep(1)
#-------------EXIT----------------
    while client.anzahl_threads > 0:
        pass
    Logger.printv()
    Logger << 'ciao!'
    try:
        os.stat(Directories['logDir'])
    except:
        raise Exception("Couldn't find logDir %s"%Directories['logDir'])
    killChildren();

    del Logger
    #todo
    for Testboard in los_agentes[0].Testboards:
            try:
                copytree(Directories['logDir'],Testboard.parentDir+'logfiles')
            except:
                raise
                #raise Exception('Could not copy Logfiles into testDirectory of Module %s\n%s ---> %s'%(Testboard.module,Directories['logDir'],Testboard.parentdir))
    try:
        rmtree(Directories['logDir'])
    except:
        pass
    #cleanup
    for Testboard in los_agentes[0].Testboards:
        try: rmtree(Testboard.parentDir+'/tmp/')
        except: pass
except:
    killChildren()
    raise
    sys.exit(0)
