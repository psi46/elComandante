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
from shutil import copytree, rmtree, copyfile
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
import lowVoltage_agente
import watchDog_agente
import signal
import socket
import tarfile
import glob
import socket
#import scp


## Base class for elComandante
##
## elComandante is implemented in a class to facilitate the use of variables
## and functions which would otherwise cause scope related issues. As a class
## global variables and local functions can be avoided. This makes the code
## more readable and flexible.
## The constructor is used to set up a number of variables which are used
## throughout the program and the elComandante::run() function actually executes
## the program. Other functions are used as helpers, having also access to the
## just mentioned variables.
class el_comandante:
    ## Sets up variables used throughout the program
    ##
    ## Uses member variables to store variables and lists
    ## that are used by most helper functions. This avoids
    ## the use of global variables.
    def __init__(self):
        ## List of el_agente classes that control clients
        self.los_agentes = []
        ## Dictionary of all file directories that are used
        self.directories = {}
        ## List of subsystem subscriptions that are not handled by any el_agente
        self.subscriptionList = []
        ## Subsystem client handle, also used by the el_agente classes
        self.subsystem_client = None

    ## Kills all the sub processes
    ##
    ## Asks the sub processes to exit and if that fails kills them
    ## with the SIGKILL signal.
    def killChildren(self):
        # Ask the client processes to exit
        for agente in self.los_agentes:
            agente.request_client_exit()

        try:
            for subscription in self.subscriptionList:
                self.subsystem_client.send(subscription,':prog:exit\n')
            time.sleep(1)
        except:
            pass

        # Close the subsystem client connection
        try:
            self.subsystem_client.closeConnection()
        except:
            pass

        # Kill the client processes
        for agente in self.los_agentes:
            try:
                agente.kill_client()
            except:
                agente.log.warning("Could not kill %s" % agente.client_name)

    ## Creates a gzip compressed tar archive of the test result data
    def createTarFiles(self, psi_agente, log):
        for Testboard in psi_agente.Testboards:
            tarFileName = Testboard.parentDir
            if tarFileName.endswith('/'):
                tarFileName=tarFileName[:-1]
            tarFileName += '.tar.gz'
            tar = tarfile.open(tarFileName, "w:gz")
            log << 'Creating archive: %s' % tarFileName
            tar.add(Testboard.parentDir, arcname=Testboard.moduleDir);
            tar.close()
            pass

    ## Removes the test result data directory
    def removeDir(self, dir, Logger):
        if  userQueries.query_yes_no("Do you want to store the data %s?" % dir):
            return
        dir = '%s/%s' % (self.directories['dataDir'], dir)
        Logger << "Removing Directory: '%s'" % dir

        try:
            rmtree(dir)
        except:
            pass

    ## Main procedure of elComandante
    ##
    ## Runs the procedural part of elComandante which includes initialization,
    ## testing, and cleanup. Throws exceptions that have to be caught on the
    ## outside where the procedure is called.
    def run(self):
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
                        localStorage = self.directories['storageDir']
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
                                localStorage = self.directories['storageDir']
                                dir = fileName.rstrip('.tar.gz')
                                moveDirToStorage(dir,localStorage,Logger)
                            except Exception as e:
                                Logger.warning("Coulnt move directory: %s"%e)
                                pass
                        else:
                            try:
                                localStorage = self.directories['storageDir']
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
            dir = '%s/%s'%(self.directories['dataDir'],dir)
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

        self.directories['configDir'] = configDir
        self.directories['baseDir'] = config.get('Directories','baseDir')
        self.directories['testdefDir'] = config.get('Directories','testDefinitions')
        self.directories['dataDir'] = config.get('Directories','dataDir')
        self.directories['defaultParameters'] = config.get('Directories','defaultParameters')
        self.directories['subserverDir'] = config.get('Directories','subserverDir')
        self.directories['keithleyDir'] = config.get('Directories','keithleyDir')
        self.directories['jumoDir'] = config.get('Directories','jumoDir')
        self.directories['logDir'] = self.directories['dataDir']+'/logfiles/'
        if config.has_option('Directories','storageDir'):
             self.directories['storageDir'] = config.get('Directories','storageDir')
        else:
            self.directories['storageDir']= self.directories['dataDir']+'/storage/'
        config.Directories = self.directories

        for dir in self.directories:
            #if "$configDir$" in self.directories[dir]:
            self.directories[dir] = os.path.abspath(self.directories[dir].replace("$configDir$",configDir))
        try:
            os.stat(self.directories['dataDir'])
        except:
            os.mkdir(self.directories['dataDir'])
        try:
            os.stat(self.directories['subserverDir'])
        except:
            os.mkdir(self.directories['subserverDir'])

        try:
            logFiles = (os.listdir(self.directories['logDir']))
            nLogFiles = len(logFiles)
            print nLogFiles
        except:
            os.mkdir(self.directories['logDir'])
        else:
            print nLogFiles
            if nLogFiles>0:
                if userQueries.query_yes_no('Do you want to overwrite \'%s\'?'%logFiles,default='no'):
                    rmtree(self.directories['logDir'])
                    os.mkdir(self.directories['logDir'])
                else:
                    raise Exception('LogDir is not empty. Please clean logDir: %s'%self.directories['logDir'])

        #initialise Logger
        Logger = printer()
        Logger.set_name("elComandanteLog")
        Logger.timestamp = timestamp
        Logger.set_logfile(self.directories['logDir'],'elComandante.log')
        Logger.printw()
        Logger<<'Set LogFile to %s'%Logger.f
        check_for_tar(self.directories['dataDir'],Logger)

        #check if subsystem server is running, if not START subserver

        if os.system("ps -ef | grep -v grep | grep subserver > /dev/null"):
            os.system("cd %s && subserver"%(self.directories['subserverDir']))
            if os.system("ps -ef | grep -v grep | grep subserver"):
                raise Exception("Could not start subserver");

        #check if subsystem server is running, if not START subserver
        #if not "subserver.pid" in os.listdir("/var/tmp"):
        #    Logger << "Starting subserver ..."
        #    os.system("cd %s && subserver"%(self.directories['subserverDir']))
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
        self.subsystem_client = sClient(serverZiel,serverPort,"elComandante")

        # Create agentes that are responsible for client processes
        self.los_agentes.append(psi_agente.psi_agente(timestamp, Logger, self.subsystem_client))
        self.los_agentes.append(highVoltage_agente.highVoltage_agente(timestamp,Logger,self.subsystem_client))
        self.los_agentes.append(watchDog_agente.watchDog_agente(timestamp,Logger,self.subsystem_client))
        if init.getboolean("Xray", "XrayUse"):
            self.los_agentes.append(xray_agente.xray_agente(timestamp, Logger, self.subsystem_client))
        self.los_agentes.append(analysis_agente.analysis_agente(timestamp, Logger, self.subsystem_client))
        if init.getboolean("CoolingBox", "CoolingBoxUse"):
            self.los_agentes.append(coolingBox_agente.coolingBox_agente(timestamp, Logger,self.subsystem_client))
        if init.getboolean("LowVoltage", "LowVoltageUse"):
            self.los_agentes.append(lowVoltage_agente.lowVoltage_agente(timestamp, Logger,self.subsystem_client))

        # Make the agentes read their configuration and initialization parameters
        for agente in self.los_agentes:
            Logger << "Setting up agente %s ..." % agente.agente_name
            agente.setup_dir(self.directories)
            agente.setup_configuration(config)
            agente.setup_initialization(init)
        Logger.printn()

        #subscribe subscriptions
        if init.getboolean("CoolingBox", "CoolingBoxUse"):
            self.subscriptionList.append(coolingBoxSubscription)
        for subscription in self.subscriptionList:
            self.subsystem_client.subscribe(subscription)

        for agente in self.los_agentes:
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
                dir = self.directories['defaultParameters'] + '/' + dir
                dir_list.append(dir)
                tb += 1
        test_chain.parameter_dir = dir_list

        #-------------------------------------

        def setupParentDir(timestamp,Testboard,init):
            Testboard.dataDir = self.directories['dataDir']
            if init.has_option('Tests','TestDescription'):
                Testboard.moduleDir = '%s_%s_%s_%s'%(Testboard.module,init.get('Tests','TestDescription'),strftime("%Y-%m-%d_%Hh%Mm",localtime(timestamp)),timestamp)
            else:
                Testboard.moduleDir = '%s_%s_%s'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",localtime(timestamp)),timestamp)
            try:
                os.stat(Testboard.parentDir)
            except:
                os.mkdir(Testboard.parentDir)
            return Testboard.parentDir

        def wait_until_finished(log, los_agentes):
            finished = all([agente.check_finished() for agente in los_agentes])
            while not finished:
                queue = []
                for agente in los_agentes:
                    if not agente.check_finished():
                        queue.append(agente.agente_name)
                if (len(queue) > 0):
                    sys.stdout.write("\r\x1b[2K" + log.get_prefix() + "Waiting for " + ", ".join(queue) + " ... ")
                    sys.stdout.flush()
                time.sleep(0.25)
                finished = all([agente.check_finished() for agente in los_agentes])
                if finished:
                    sys.stdout.write("\r\x1b[2K")

        # Check whether the client is already running before trying to start it
        Logger << "Checking whether clients are runnning ..."
        for agente in self.los_agentes:
            agente.check_client_running()

        for agente in self.los_agentes:
            agente.start_client(timestamp)
        Logger.printn()#        self.parentDir='.'

        # Check the client subscriptions
        Logger << "Checking subscriptions of the clients ..."
        time.sleep(2)
        for subscription in self.subscriptionList:
            if not self.subsystem_client.checkSubscription(subscription):
                raise Exception("Cannot read from %s subscription"%subscription)
            else:
                Logger << "\t%s is answering." % subscription
        for agente in self.los_agentes:
            if not agente.check_subscription():
                raise Exception("Cannot read from agente: %s subscription" % agente.subscription)
            else:
                Logger << "\t%s is answering." % agente.subscription

        #-------------SETUP TESTBOARDS----------------
        Logger.printv()
        Logger << 'The following testboards with modules were found:'
        Logger.printn()
        #ToDo:
        for Testboard in self.los_agentes[0].Testboards:
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
                for Testboard in self.los_agentes[0].Testboards:
                    test_str_list.append('%s_TB%s@%s' % (whichtest, Testboard.slot, env))
                test.multiply(test_str_list)
            test = test.next()

        Logger << 'The following tests will be executed:'
        Logger.printn()
        test = test_chain.next()
        number_of_tests = 0
        while test:
            Logger << "\t- %s" % test.test_str
            test = test.next()
            number_of_tests += 1

        Logger.printv()

        #--------------LOOP over TESTS-----------------

        test = test_chain.next()
        testno = 0
        while test:
            env = environment.environment(test.test_str, init)
            test.environment = env
            test.testname = test.test_str.split("@")[0]

            testno += 1
            Logger << "Test %i/%i: %s" % (testno, number_of_tests, test.test_str)
            Logger.printn()

            for agente in self.los_agentes:
                agente.set_test(test)

            # Prepare for the tests
            Logger << "Preparing test %s ..." % test.test_str
            for agente in self.los_agentes:
                agente.prepare_test(test.test_str, env)
            wait_until_finished(Logger, self.los_agentes)

            Logger.printn()

            # Execute tests
            Logger << "Executing test %s ..." % test.test_str
            for agente in self.los_agentes:
                agente.execute_test()
            wait_until_finished(Logger, self.los_agentes)

            Logger.printn()

            # Cleanup tests
            Logger << "Cleaning up after test %s ..." % test.test_str
            for agente in self.los_agentes:
                agente.cleanup_test()
            wait_until_finished(Logger, self.los_agentes)

            Logger.printv()
            test = test.next()

        # Final cleanup
        Logger << "Final cleanup after all tests ..."
        for agente in self.los_agentes:
            agente.final_test_cleanup()
        # Wait for final cleanup to finish
        wait_until_finished(Logger, self.los_agentes)

        #-------------EXIT----------------

        Logger.printv()
        userQueries.query_any("Finished all tests. Press ENTER to terminate the clients. ", Logger)

        Logger << "Asking clients to quit ..."
        for agente in self.los_agentes:
            agente.request_client_exit()

        self.subsystem_client.closeConnection()
        Logger << 'Subsystem connection closed.'

        time.sleep(1)
        Logger << "Killing remaining children ..."
        self.killChildren()
        time.sleep(1)

        while self.subsystem_client.anzahl_threads > 0:
            pass
        Logger.printv()
        try:
            os.stat(self.directories['logDir'])
        except:
            raise Exception("Couldn't find logDir %s"%self.directories['logDir'])
        self.killChildren();


        #todo
        for Testboard in self.los_agentes[0].Testboards:
                try:
                    dest = Testboard.parentDir+'logfiles'
                    copytree(self.directories['logDir'],dest)
                    dest = Testboard.parentDir+'/configfiles/'
                    if not os.path.exists(dest):
                        os.mkdir(dest)
                    shutil.copy2(iniFile,dest)
                    shutil.copy2(configFile,dest)
                except:
                    raise
        self.createTarFiles(self.los_agentes[0], Logger)
        tarList =['%s.tar.gz'%Testboard.parentDir.rstrip('/') for Tesboard in self.los_agentes[0].Testboards]
        uploadTarFiles(tarList,Logger)

        Logger.printv()
        Logger << 'ciao!'

        del Logger

        try:
            rmtree(self.directories['logDir'])
        except:
            pass
        #cleanup
        for Testboard in self.los_agentes[0].Testboards:
            try: rmtree(Testboard.parentDir+'/tmp/')
            except: pass

    ## Procedure called in the event of an exception
    ##
    ## Cleans up the lose ends when an exception occurs. It mainly
    ## kills all the children.
    ## @FIXME It should ask the clients first to bring everyting into a safe state.
    def clean_up_exception(self):
        if len(self.los_agentes)>0 and self.los_agentes[0]:
            try:
                for Testboard in self.los_agentes[0].Testboards:
                    self.removeDir(Testboard.moduleDir, Logger)
            except:
                pass
        self.killChildren()

### PROCEDURAL CODE ##################################################################

## Main instance of the elComandante class
elComandante = el_comandante()
try:
    elComandante.run()
except:
    # Gather information about the exception
    exc_type = sys.exc_info()[0]
    exc_obj = sys.exc_info()[1]
    exc_tb = sys.exc_info()[2]

    # Print the exception information
    print "Exception (" + exc_type.__name__ + ") in " + exc_tb.tb_frame.f_code.co_filename + ", line " + str(exc_tb.tb_lineno) + ":"
    print exc_obj
    print ""

    # Wait for the user to acknowledge
    userQueries.query_any("An exception occurred. Press ENTER to close the program. ")

    # Allow elComandante to clean up
    elComandante.clean_up_exception()

    sys.exit(0)
