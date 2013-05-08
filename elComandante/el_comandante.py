#!/usr/bin/env python

## @file
## Program code for elComandante
## @ingroup elComandante

## @addtogroup elComandante
## @brief [short description]
##
## @details
## [detailed description]

import sys
sys.path.insert(1, "../")
from myutils import BetterConfigParser, sClient, decode, printer, preexec, testchain,scp,userQueries
from myutils import Testboard as Testboarddefinition
from time import strftime, localtime
import time
from shutil import copytree,rmtree, copyfile
import paramiko
import os
import subprocess
import argparse
import environment
import xray_agente
import psi_agente
import shutil
import coolingBox_agente
import analysis_agente
import highVoltage_agente
import watchDog_agente
import signal
import socket
import tarfile
import glob
import socket
#import scp

los_agentes = []
Directories={}

def killChildren():
#    print "Killing clients ..."

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
            agente.log.warning("Could not kill %s" % agente.client_name)

def handler(signum, frame):
    print 'Signal handler called with signal %s' % signum
    killChildren();
    sys.exit(0)

def createTarFiles(psi_agente):
     #create tar.gz files
    for Testboard in psi_agente.Testboards:
        tarFileName = Testboard.parentDir
        if tarFileName.endswith('/'):
            tarFileName=tarFileName[:-1]
        tarFileName += '.tar.gz'
        tar = tarfile.open(tarFileName, "w:gz")
        print 'creating Tarfile: %s'%tarFileName 
        tar.add(Testboard.parentDir, arcname=Testboard.moduleDir);
#        for name in ["file1", "file2", "file3"]:
#    tar.add(name)
        tar.close()
        pass
#signal.signal(signal.SIGINT, handler)


def removeDir(dir,Logger):
    if  userQueries.query_yes_no("Do you want to store the data %s?"%dir):
        return
    dir = '%s/%s'%(config.Directories['dataDir'],dir)
    Logger << "Removing Directory: '%s'"%dir
    
    try:
        rmtree(dir)
    except:
        pass

def uploadTarFiles(tarList,Logger):
    #check if all needed options are defined
    if len(tarList) ==0:
        return
    checkConfig = config.has_option('Transfer','host')
    checkConfig = checkConfig and config.has_option('Transfer','port')
    checkConfig = checkConfig and config.has_option('Transfer','user')
    checkConfig = checkConfig and config.has_option('Transfer','destination')
    if  checkConfig:
        if len(tarList)==0:
            return
        try:
            dest = config.get('Transfer','destination')
            ssh = paramiko.SSHClient()
            ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            Logger <<'Creating ssh connection: %s:%s'%(config.get('Transfer','host'),config.getint('Transfer','port'))
            ssh.connect(config.get('Transfer','host'),config.getint('Transfer','port'),username=config.get('Transfer','user'),timeout=5.0)
            transport = ssh.get_transport()
            ssh_client = scp.SCPClient(transport)
            if not dest.endswith('/'):
                dest+='/'
            Logger << 'copy files:'
            for item in tarList:
                fileName = item.split('/')[-1]
                localStorage = Directories['storageDir']
                dir = fileName.rstrip('.tar.gz')
                if userQueries.query_yes_no("Do you want to upload the data of %s?"%(fileName),"yes",Logger):
                    Logger << 'uploading: \t%s --> %s:%s' % (fileName, config.get('Transfer','host'),dest)
                    ssh_client.put(item, dest, preserve_times=False)
                    Logger.printn()
                    #TODO checksum
                    checkTransfer = True
                    if checkTransfer:
                        Logger << 'Transfer of %s was successful, delte tar file'%fileName
                        os.remove(item)
                    #remove TAR
                    #move Dir to localStorage
                    try:
                        localStorage = Directories['storageDir']
                        dir = fileName.rstrip('.tar.gz')
                        moveDirToStorage(dir,localStorage,Logger)
                    except Exception as e:
                        Logger.warning("Coulnt move directory: %s"%e)
                        pass
                else:
                    try:
                        localStorage = Directories['storageDir']
                        dir = fileName.rstrip('.tar.gz')
                        if userQueries.query_yes_no("Do you want to move directory '%s' to storage anyway?"%(dir,localStorage),Logger):
                            moveDirToStorage(dir,localStorage,Logger)
                    except Exception as e:
                        Logger.warning("Coulnt move directory: %s"%e)
                        pass
            ssh.close()                
        except paramiko.PasswordRequiredException, e:
            Logger.warning("Couldn't upload need password: %s"%e)
        except socket.gaierror,e:
            if e.errno == 8:
                Logger.warning("couldn't upload: no connection to server established. Errormessage: %s"%(e))
            else:
                Logger.warning("%s"%e)
                raise
        except socket.error:
            errno, errstr = sys.exc_info()[:2]
            if errno == socket.timeout:
                Logger << "Connection time out: Couldn't transfer data"
                pass
            else:
                raise

        except:
            raise
    else:
        Logger.warning("cannot upload data since no alll needed options are defined: section 'Transfer', options: 'host,'port','user','destination'")

def moveDirToStorage(dir,storage,Logger):
    Logger << " move %s ---> %s"%(dir,storage)
    try:
        os.stat(storage)
    except:
        if not userQueries.query_yes_no("Do you want to create the storage dir '%s/?"%storage,"yes",Logger):
            raise
        else:
            Logger<<"Make directory: %s"%storage
            os.mkdir(storage)
    dir = '%s/%s'%(Directories['dataDir'],dir)
    dir.rstrip('/')
    Logger<< "Move %s --> %s"%(dir,storage)
    shutil.move(dir,storage)
    pass

def check_for_tar(dataDir, Logger):
    if config.has_option('Transfer','checkForTars'):
       if not config.getboolean('Transfer','checkForTars'):
           return 
    tarList = glob.glob('%s/*.tar.gz'%dataDir)
#    tarList = ['%s/%s'%(dataDir,item) for item in tarList]
    uploadTarFiles(tarList,Logger)
    
try:
    #get timestamp
    timestamp = int(time.time())
    #------------some configuration--------------

    parser = argparse.ArgumentParser()

    parser.add_argument("-c", "--config", dest="configDir",
                           help="specify directory containing config files e.g. ../config/",
                           default="../config/")

    args = parser.parse_args()
    configDir= args.configDir
    try:
        os.access(configDir,os.R_OK)
    except:
        raise Exception('configDir \'%s\' is not accessible'%configDir)
        #sys.exit()
        #raise SystemExittimes

    #load config
    configFile = configDir+'/elComandante.conf'
    config = BetterConfigParser()
    config.read(configFile)
    #load init
    iniFile = configDir+'/elComandante.ini'
    init = BetterConfigParser()
    init.read(iniFile)

    Directories['configDir'] = configDir
    Directories['baseDir'] = config.get('Directories','baseDir')
    Directories['testdefDir'] = config.get('Directories','testDefinitions')
    Directories['dataDir'] = config.get('Directories','dataDir')
    Directories['defaultParameters'] = config.get('Directories','defaultParameters')
    Directories['subserverDir'] = config.get('Directories','subserverDir')
    Directories['keithleyDir'] = config.get('Directories','keithleyDir')
    Directories['jumoDir'] = config.get('Directories','jumoDir')
    Directories['logDir'] = Directories['dataDir']+'/logfiles/'
    if config.has_option('Directories','storageDir'):
         Directories['storageDir'] = config.get('Directories','storageDir')
    else:
        Directories['storageDir']= Directories['dataDir']+'/storage/'
    config.Directories = Directories

    for dir in Directories:
        #if "$configDir$" in Directories[dir]:
        Directories[dir] = os.path.abspath(Directories[dir].replace("$configDir$",configDir))
    try:
        os.stat(Directories['dataDir'])
    except:
        os.mkdir(Directories['dataDir'])
    try:
        os.stat(Directories['subserverDir'])
    except:
        os.mkdir(Directories['subserverDir'])

    try:
        logFiles = (os.listdir(Directories['logDir']))
        nLogFiles = len(logFiles)
        print nLogFiles
    except:
        os.mkdir(Directories['logDir'])
    else:
        print nLogFiles
        if nLogFiles>0:
            if userQueries.query_yes_no('Do you want to overwrite \'%s\'?'%logFiles,default='no'):
                rmtree(Directories['logDir'])
                os.mkdir(Directories['logDir'])
            else:
                raise Exception('LogDir is not empty. Please clean logDir: %s'%Directories['logDir'])

    #initialise Logger
    Logger = printer()
    Logger.set_name("elComandanteLog")
    Logger.timestamp = timestamp
    Logger.set_logfile(Directories['logDir'],'elComandante.log')
    Logger.printw()
    Logger<<'Set LogFile to %s'%Logger.f
    check_for_tar(Directories['dataDir'],Logger)

    #check if subsystem server is running, if not START subserver

    if os.system("ps -ef | grep -v grep | grep subserver > /dev/null"):
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
    Logger.printn()

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
    los_agentes.append(watchDog_agente.watchDog_agente(timestamp,Logger,client))
    if init.getboolean("Xray", "XrayUse"):
        los_agentes.append(xray_agente.xray_agente(timestamp, Logger, client))
    los_agentes.append(analysis_agente.analysis_agente(timestamp, Logger, client))
    if init.getboolean("CoolingBox", "CoolingBoxUse"):
        los_agentes.append(coolingBox_agente.coolingBox_agente(timestamp, Logger,client))

    # Make the agentes read their configuration and initialization parameters
    for agente in los_agentes:
        Logger << "Setting up agente %s ..." % agente.agente_name
        agente.setup_dir(Directories)
        agente.setup_configuration(config)
        agente.setup_initialization(init)
    Logger.printn()

    #subscribe subscriptions
    subscriptionList = []
    if init.getboolean("CoolingBox", "CoolingBoxUse"):
        subscriptionList.append(coolingBoxSubscription)
    for subscription in subscriptionList:
        client.subscribe(subscription)

    for agente in los_agentes:
        agente.subscribe()

    #directory config
    #get list of tests to do:
    testlist=init.get('Tests','Test')
    test_chain = testchain.parse_test_list(testlist)
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

    def setupParentDir(timestamp,Testboard,init):
        Testboard.dataDir = Directories['dataDir']
        if init.has_option('Tests','TestDescription'):
            Testboard.moduleDir = '%s_%s_%s_%s'%(Testboard.module,init.get('Tests','TestDescription'),strftime("%Y-%m-%d_%Hh%Mm",localtime(timestamp)),timestamp)
        else:
            Testboard.moduleDir = '%s_%s_%s'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",localtime(timestamp)),timestamp)
        try:
            os.stat(Testboard.parentDir)
        except:
            os.mkdir(Testboard.parentDir)
        return Testboard.parentDir

    def wait_until_finished(los_agentes):
        time.sleep(1)
        finished = False
        while not finished:
            time.sleep(1.0)
            finished = all([agente.check_finished() for agente in los_agentes])
            output = ' \t'
            for agente in los_agentes:
                output += '%s: %s\t'%(agente.agente_name,int(agente.check_finished()))
            if not finished:
                sys.stdout.write('%s\r' %output)
            sys.stdout.flush()
        Logger << "finished"
        time.sleep(2)

    # Check whether the client is already running before trying to start it
    Logger << "Checking whether clients are runnning ..."
    for agente in los_agentes:
        agente.check_client_running()

    for agente in los_agentes:
        agente.start_client(timestamp)
    Logger.printn()#        self.parentDir='.'

    # Check the client subscriptions
    Logger << "Checking subscriptions of the clients ..."
    time.sleep(2)
    for subscription in subscriptionList:
        if not client.checkSubscription(subscription):
            raise Exception("Cannot read from %s subscription"%subscription)
        else:
            Logger << "\t%s is answering." % subscription
    for agente in los_agentes:
        if not agente.check_subscription():
            raise Exception("Cannot read from agente: %s subscription" % agente.subscription)
        else:
            Logger << "\t%s is answering." % agente.subscription

    #-------------SETUP TESTBOARDS----------------
    Logger.printv()
    Logger << 'The following testboards with modules were found:'
    Logger.printn()
    #ToDo:
    for Testboard in los_agentes[0].Testboards:
            parentDir=setupParentDir(timestamp,Testboard,init)
            Logger << '\t- Testboard %s at address %s with module %s'%(Testboard.slot,Testboard.address,Testboard.module)

    Logger.printn()

    test = test_chain.next()
    while test:
        if test.test_str.find('@')>=0:
            whichtest, env = test.test_str.split('@')
        else:
            whichtest = test.test_str
            env = 17.0
        if whichtest== "IV":
            test_str_list = []
            for Testboard in los_agentes[0].Testboards:
                test_str_list.append('%s_TB%s@%s' % (whichtest, Testboard.slot, env))
            test.multiply(test_str_list)
        test = test.next()

    Logger << 'The following tests will be executed:'
    Logger.printn()
    test = test_chain.next()
    while test:
        Logger << "\t- %s" % test.test_str
        test = test.next()

    Logger.printv()

    #--------------LOOP over TESTS-----------------

    test = test_chain.next()
    while test:
        env = environment.environment(test.test_str, init)
        test.environment = env
        test.testname = test.test_str.split("@")[0]

        for agente in los_agentes:
            agente.set_test(test)

        # Prepare for the tests
        Logger << "Preparing test %s ..." % test.test_str
        for agente in los_agentes:
            agente.prepare_test(test.test_str, env)
        wait_until_finished(los_agentes)

        Logger.printn()

        # Execute tests
        Logger << "Executing test %s ..." % test.test_str
        for agente in los_agentes:
            agente.execute_test()
        wait_until_finished(los_agentes)

        Logger.printn()

        # Cleanup tests
        Logger << "Cleaning up after test %s ..." % test.test_str
        for agente in los_agentes:
            agente.cleanup_test()
        wait_until_finished(los_agentes)

        Logger.printv()
        test = test.next()

    # Final cleanup
    Logger << "Final cleanup after all tests ..."
    for agente in los_agentes:
        agente.final_test_cleanup()
    # Wait for final cleanup to finish
    wait_until_finished(los_agentes)

    #-------------EXIT----------------

    Logger.printv()

    for agente in los_agentes:
        agente.request_client_exit()

    client.closeConnection()
    Logger << 'Subsystem connection closed.'

    time.sleep(1)
    killChildren()
    time.sleep(1)

    while client.anzahl_threads > 0:
        pass
    Logger.printv()
    Logger << 'ciao!'
    try:
        os.stat(Directories['logDir'])
    except:
        raise Exception("Couldn't find logDir %s"%Directories['logDir'])
    killChildren();

   
    #todo
    for Testboard in los_agentes[0].Testboards:
            try:
                dest = Testboard.parentDir+'logfiles'
                copytree(Directories['logDir'],dest)
                dest = Testboard.parentDir+'/configfiles/'
                if not os.path.exists(dest):
                    os.mkdir(dest)
                shutil.copy2(iniFile,dest)
                shutil.copy2(configFile,dest)
            except:
                raise
    createTarFiles(los_agentes[0])
    tarList =['%s.tar.gz'%Testboard.parentDir.rstrip('/') for Tesboard in los_agentes[0].Testboards]
    uploadTarFiles(tarList,Logger)
                #raise Exception('Could not copy Logfiles into testDirectory of Module %s\n%s ---> %s'%(Testboard.module,Directories['logDir'],Testboard.parentdir))
    del Logger
    
    try:
        rmtree(Directories['logDir'])
    except:
        pass
    #cleanup
    for Testboard in los_agentes[0].Testboards:
        try: rmtree(Testboard.parentDir+'/tmp/')
        except: pass
   
except:
    if len(los_agentes)>0 and los_agentes[0]:
        try:
            for Testboard in los_agentes[0].Testboards:
                removeDir(Testboard.moduleDir, Logger)
        except:
            pass 
    killChildren()
    raise
    sys.exit(0)

# DOCUMENTATION ##############################################################################

## @mainpage elComandante documentation
## @tableofcontents
## @section introduction Introduction
## elComandante is a program that is used for the automatic testing of CMS pixel modules ...
##
## @section user User documentation
##
## @subsection installation Installation
##
## @subsection running Running elComandante
##
## @subsection conf Configuration
## @verbinclude elComandante.conf.default
##
## @subsection ini Initialization
## @verbinclude elComandante.ini.default
##
## @subsection modconf Module and testboard configuration
## @verbatim
## [Modules]
## TB0: M0696
## TB1: M1148
## TB2: M1148
## TB3: M3476
##
## [ModuleType]
## TB0: Roc
## TB1: Full
## TB2: Full
## TB3: Full
##
## [TestboardUse]
## TB0: False
## TB1: True
## TB2: False
## TB3: False
## @endverbatim
##
## @subsection env Environments
## An environment describes the (controllable) physical conditions at which a test with
## elComandante is performed. At this time there are only the temperature and x-ray radiation
## that can be supervised by elComandante and for this reason the definition of the enviromnents
## is completely given in the following example
## @verbatim
## [Environment Mo]
## Temperature: 17
## XrayVoltage: 60
## XrayCurrent: 30
## XrayTarget: Mo
## @endverbatim
## Here an environment with the label \c Mo is defined. It specifies a temperature of 17 degrees
## celsius, x-ray radiation with a tube setting of 60 kilovolts and 30 milliamperes. It also
## specifies an x-ray target for fluorescence, which is a label that should be defined among
## others in the \c elComandante.conf. The default settings are 17 degrees, 30 kV and 10 mA, no
## target (direct beam). Note that when no x-ray setting is specified in the environment, the
## x-ray device will not turn on (despite the default settings)! The defaults are just there
## such that you can leave away something, for instance the target.
##
## Environments can be added to tests in the test list:
## @verbatim
## Test: VcalCalibrationStep@Mo,VcalCalibrationStep@Ag,VcalCalibrationStep@Ba
## @endverbatim
## Every test is appended with an environment with an \c \@ character in between. In this example
## other environments that describe \c Ag and \c Ba are required (but omitted here for brevity).
##
## There is an alternative (easier, but more restrictive) way to specify environments. Instead
## of defining an environment like above one can simply write a floating point number after the
## \c \@ character which defines the temperature
## @verbatim
## Test: Pretest@17,Fulltest@17
## @endverbatim
## The environment and the \c \@ can also be omitted so it will default to 17 degrees Celsius.
##
## elComandante will try to establish the environment specified \a before the test is actually
## started, ensuring stable conditions for the test.
##
## @subsection testlists Test lists
## The tests that elComandante is supposed to run (in sequence) can be specified in a list.
## This list is to be put in \c elComandante.ini and in its most simple form would look like
## this:
## @verbatim
## [Tests]
## Test: Pretest,Fulltest,Trim,PHCalibration
## @endverbatim
## These tests (if defined) will then run in sequence once elComandante is started. The pixel
## modules/ROCs will get their parameters from a default parameters repository that can be
## specified in elComandante.con (see @ref conf).
##
## Tests can also be run in specific physical environments, where each test in the list gets
## a suffix in the form like \c Pretest@17. See @ref env for details.
##
## Sometimes it is desireable to use the module/ROC parameters from a previous test in the
## current test. This is especially the case if the previous test was the \c Pretest or
## \c Trim. To achieve this one can put an angular bracket (\c >) instead of a comman between
## tests:
## @verbatim
## Test: Pretest>Fulltest,Trim,PHCalibration
## @endverbatim
## In this example the \c Fulltest will get its parameters from the previously run \c Pretest.
## The \c Trim and \c PHCalibration tests will get the parameters from the default parameters
## repository (which is probably not desireable but mentioned here only as a (bad) example).
## If multiple tests require the parameters from the same previous test this can be specified
## as follows:
## @verbatim
## Test: Pretest>{Fulltest,Trim,PHCalibration}
## @endverbatim
## In this example all of the three \c Fulltest, \c Trim, and \c PHCalibration would get their
## parameters from the \c Pretest.
##
## It is also worth noting that these expressions with angular brackets (\c >) and curly brackets
## can be nested:
## @verbatim
## Test: Pretest>{Fulltest,Trim>{TrimVerification,PHCalibration}}
## @endverbatim
## Here the \c TrimVerification and the \c PHCalibration would get their parameters from \c Trim
## which in turn got its parameters from \c Pretest.
##
## @note It is not possible to have an angular bracket (\c >) after a closing curling bracket.
## Commas are still possible.
## @note Some of the tests mentioned in this section may not (yet) exist.
##
## @subsection tests Tests
##
## @subsubsection pretest Pretest
##
## @subsubsection fulltest Fulltest
##
## @subsubsection trimming Trimming
##
## @subsubsection phcal Pulse height calibration
##
## @subsubsection vcalcal Vcal calibration
## The Vcal calibration is somewhat special among the tests/calibrations that elComandante
## can perform. This is because some measurements require different environments (see @ref env)
## and therefore the calibration has to be split into multiple tests. There are two categories
## of tests in the Vcal calibration:
##
##   - X-ray threshold scan
##   - Vcal threshold scan
##
## The former must run at different x-ray conditions, namely monochromatic x-rays of different
## wavelengths/energies. This is achieved by using different targes in the x-ray beam. The
## nature and number of targets is configureable.
##
## Additionally there is the fact that all these tests must run with the same configuration
## of the module/ROC which (probably) was created or modified through another test such as
## @ref pretest or @ref trimming. To achieve this special considerations have to be made when
## setting the testlist in the initialization file:
## @verbatim
## Test: Pretest@17>Trim@17>{VcalCalibrationStep@Mo,VcalCalibrationStep@Ag,VcalCalibrationStep@Ba,VcalVsThreshold@17}
## @endverbatim
## In this example there are three X-ray threshold scans (the test name is \c VcalCalibrationStep)
## that are run in three different environments (\c Mo, \c Ag, \c Ba which stand for molybdenum,
## silver, and barium fluorescence targets). Before these scans are performed a @ref pretest and
## a @ref trimming step are executed. In the list they are not comma separated, but connected with an
## arrow (\c >) character. This means, that the test parameter files (such as DAC parameters or trim
## parameters) are taken from the previous test. The curly brackets mean that all tests listed within
## use test parameters from the test before the bracket. (Brackets can be nested.) After the
## \c VcalCalibrationSteps there is the VcalVsThreshold step which is a test that determines the
## \c Vcal DAC valuess that correspond to different \c VcThr (threshold) values.
##
## The different @ref env (\c Mo, \c Ag, \c Ba) can be defined in the initialization file as follows:
## @verbatim
## [Environment Mo]
## Temperature: 17
## XrayVoltage: 60
## XrayCurrent: 30
## XrayTarget: Mo
## @endverbatim
## In this example an environment label \c Mo is created that stands for a temperature of 17 degrees
## Celsius, a x-ray tube voltage and current of 60 kV and 30 mA respectively, as well as a fluorescence
## target with label \c Mo. The possible targets are defined in the \c elComandante.conf file. Here it
## is a coincidence (and convenience) that the environment label and the target label are the same.
##
## @subsubsection highrate High rate tests
## There are three high rate tests:
##
##   - \c HighRatePixelMapTest
##   - \c HighRateEfficiencyTest
##   - \c HighRateSCurveTest
##
## They are separate (and not in one big test) because one may want to run them at different conditions
## (@ref env). For instance, one may want the \c HighRatePixelMapTest and the \c HighRateEfficiencyTest
## to be run at different x-ray intensities, but \c HighRateSCurveTest only at one (because it takes
## a much longer time). This could be listed in the test list like
## @verbatim
## Test: HighRatePixelMapTest@HR100,HighRateEfficiencyTest@HR100,HighRatePixelMapTest@HR250,HighRateEfficiencyTest@HR250,HighRateSCurveTest@HR250
## @endverbatim
## with @ref env like
## @verbatim
## [Environment HR100]
## Temperature: 17
## XrayVoltage: 30
## XrayCurrent: 10
## @endverbatim
## for testing at intensities of 100 MHz / cm2 and 250 MHz / cm2.
##
## @subsection testparams Changing test parameters
## @note This is not yet possible.
##
## @subsection ana Analyses
## Analyses are nothing but tests but instead of the @ref psiClient they address the @ref analysisClient.
## Because of this they can be added to the test list (see @ref testlists) like any other test:
## @verbatim
## Test: Pretest,Fulltest,FulltestAnalysis
## @endverbatim
## Analyses have to be declared in the \c elComandante.ini file:
## @verbatim
## [Analysis FulltestAnalysis]
## command: FulltestAnalysis --output analysis_Fulltest.root *Fulltest*/commander_Fulltest.root
## @endverbatim
## Analyses are nothing but information about how to run scripts on test results. The scripts can
## be anything from shell scripts to ROOT macros to compiled programs.
## @note At the moment it is not possible to tell the script the paths and filenames of the results
## from previously run tests. For this reason tricks have to be used such as the one shown above
## where the script will find some folder that has the name \c Fulltest in it.
##
## @note Another limitation is that the analysis does not get its own output directory (yet).
##
## The @ref analysisClient can be configured in \c elComandante.conf. It searches for scripts in the
## folder indicated with \c scriptDir in
## @verbatim
## [Directories]
## dataDir:   <!Directories|baseDir!>/DATA/
## scriptDir: <!Directories|baseDir!>/analysisClient/scripts/
## @endverbatim
## The scripts are executed in the directory \c dataDir which is the place where the test results
## are stored.
##
## @section devel Developer documentation
##
## @subsection newclient Creating a new client for elComandante
##
## @subsection newdevice Adding support for a new device
## Devices are handled by the clients, not by elComandante directly. Ideally the clients are written
## in such a way that the device dependent routines are easily exchangeable.
##
## @subsubsection newxraydevice Adding an x-ray device
## The @ref xrayClient is easily extendable with new devices. There are two different device types
## handlded by the @ref xrayClient:
##   - x-ray generators
##   - motor stages for moving fluorescence targets
##
## It has the abstract classes xray_generator.xray_generator and motor_stage.motor_stage which
## define the minimum features that the device has to support. The only thing to do is to create
## a new file with the class which will communicate with the new device. The new class has to
## inherit from one of the abstract classes mentioned before. The routines from the abstract
## classes then have to be redefined to do whatever is necessary to accomplish the functions.
## Please look at the abstract class documentation for xray_generator.xray_generator and
## motor_stage.motor_stage, as well as the specific implementation id3003.id3003_xray_generator
## and zaber.zaber_motor_stage for details about how to implement the new device.
##
## Secondly the new file has to be imported in the \c xrayClient.py file and another condition
## added to the code:
## @code
## if args.xray_type == "id3003":
##     ## X-ray generator instance
##     xray_generator = id3003.id3003_xray_generator(args.xray_device)
## elif args.xray_type == "newdevice":
##     xray_generator = newdevice.newdevice(args.xray_device)
## else:
##     error = "Unknown device " + args.xray_type + "."
##     log.warning(error)
##     sys.exit(1)
## @endcode
##
## Thirdly you have to adapt your \c elComandante.conf file to set the parameters of the xrayClient
## to your new values:
## @verbatim
## [xrayClient]
## xrayDevice: /dev/ttyF0
## xrayType: newdevice
## xrfDevice: /dev/ttyF1
## xrfType: zaber
## xrfTargets: Fe:0,Cu:25320,Mo:50640,Ag:75960,Sn:101280,Ba:126600
## @endverbatim
## In this example a new x-ray generator of type \c newdevice was added.
