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
from myutils import BetterConfigParser, sClient, printer, testchain, scp, userQueries
from collections import OrderedDict
from time import strftime, localtime
import time
from shutil import copytree, rmtree
import paramiko
import os
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
import socket
import tarfile
import glob
import traceback


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
        ## Handle to the configuration file
        self.config = None
        ## Handle to the initialization file
        self.init = None
        ## Logging handle
        self.log = None

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
            time.sleep(3)
        except:
            pass

        # Close the subsystem client connection
        try:
            self.subsystem_client.closeConnection()
        except:
            pass

        time.sleep(1)
        # Kill the client processes
        for agente in self.los_agentes:
            try:
                agente.kill_client()
            except:
                agente.log.warning("Could not kill %s" % agente.client_name)

    ## Creates a gzip compressed tar archive of the test result data
    def createTarFiles(self, psi_agente):
        for Testboard in psi_agente.Testboards:
            tarFileName = Testboard.parentDir
            if tarFileName.endswith('/'):
                tarFileName=tarFileName[:-1]
            tarFileName += '.tar'
            tar = tarfile.open(tarFileName, "w:gz")
            self.log << 'Creating archive: %s' % tarFileName
            tar.add(Testboard.parentDir, arcname=Testboard.moduleDir);
            tar.close()
            pass

    ## Removes the test result data directory
    def removeDir(self, dir):
        if  userQueries.query_yes_no("Do you want to store the data %s?" % dir):
            return
        dir = '%s/%s' % (self.directories['dataDir'], dir)
        self.log << "Removing Directory: '%s'" % dir

        try:
            rmtree(dir)
        except:
            self.log.warning("Unable to remove '%s'!" % dir)
            pass

    def uploadTarFiles(self, tarList):
        #check if all needed options are defined
        if len(tarList) ==0:
            return
        checkConfig = self.config.has_option('Transfer','host')
        checkConfig = checkConfig and self.config.has_option('Transfer','port')
        checkConfig = checkConfig and self.config.has_option('Transfer','user')
        checkConfig = checkConfig and self.config.has_option('Transfer','destination')
        if not checkConfig:
            self.log.warning("cannot upload data since no all needed options are defined: section 'Transfer', options: 'host,'port','user','destination'")
            return
        try:
            dest = self.config.get('Transfer','destination')
            ssh = paramiko.SSHClient()
            ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            self.log <<'Creating ssh connection: %s:%s'%(self.config.get('Transfer','host'),self.config.getint('Transfer','port'))
            ssh.connect(self.config.get('Transfer','host'),self.config.getint('Transfer','port'),username=self.config.get('Transfer','user'),timeout=5.0)
            transport = ssh.get_transport()
            ssh_client = scp.SCPClient(transport)
            if not dest.endswith('/'):
                dest+='/'
            self.log << 'copy files:'
            for item in tarList:
                fileName = item.split('/')[-1]
                localStorage = self.directories['storageDir']
                dir = fileName.rstrip('.tar')
                if userQueries.query_yes_no("Do you want to upload the data of %s?"%(fileName),"yes",self.log):
                    self.log << 'uploading: \t%s --> %s:%s' % (fileName, self.config.get('Transfer','host'),dest)
                    ssh_client.put(item, dest, preserve_times=False)
                    self.log.printn()
                    #TODO checksum
                    checkTransfer = True
                    if checkTransfer:
                        self.log << 'Transfer of %s was successful, delte tar file'%fileName
                        os.remove(item)
                    #remove TAR
                    #move Dir to localStorage
                    try:
                        localStorage = self.directories['storageDir']
                        dir = fileName.rstrip('.tar')
                        self.moveDirToStorage(dir,localStorage)
                    except Exception as e:
                        self.log.warning("Coulnt move directory: %s"%e)
                        pass
                else:
                    try:
                        localStorage = self.directories['storageDir']
                        dir = fileName.rstrip('.tar')
                        if userQueries.query_yes_no("Do you want to move directory '%s' to storage anyway?"%(dir,localStorage),self.log):
                            self.moveDirToStorage(dir,localStorage)
                    except Exception as e:
                        self.log.warning("Coulnt move directory: %s"%e)
                        pass
            ssh.close()
        except paramiko.PasswordRequiredException, e:
            self.log.warning("Couldn't upload need password: %s"%e)
        except socket.gaierror,e:
            if e.errno == 8:
                self.log.warning("couldn't upload: no connection to server established. Errormessage: %s"%(e))
            else:
                self.log.warning("%s"%e)
                raise
        except socket.error:
            errno, errstr = sys.exc_info()[:2]
            if errno == socket.timeout:
                self.log << "Connection time out: Couldn't transfer data"
                pass
            else:
                raise

        except:
            raise

    def moveDirToStorage(self, dir, storage):
        self.log << " move %s ---> %s"%(dir,storage)
        try:
            os.stat(storage)
        except:
            if not userQueries.query_yes_no("Do you want to create the storage dir '%s/?"%storage,"yes",self.log):
                raise
            else:
                self.log<<"Make directory: %s"%storage
                os.mkdir(storage)
        dir = '%s/%s'%(self.directories['dataDir'],dir)
        dir.rstrip('/')
        self.log<< "Move %s --> %s"%(dir,storage)
        shutil.move(dir,storage)

    def check_for_tar(self, dataDir):
        if self.config.has_option('Transfer','checkForTars'):
           if not self.config.getboolean('Transfer','checkForTars'):
               return
        tarList = glob.glob('%s/*.tar'%dataDir)
        self.uploadTarFiles(tarList)

    def parse_command_line_arguments(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("-c", "--config", dest="configDir",
                               help="specify directory containing config files e.g. ../config/",
                               default="../config/")
        args = parser.parse_args()
        return args

    def check_config_directory(self, configDir):
        try:
            os.access(configDir,os.R_OK)
        except:
            raise Exception('configDir \'%s\' is not accessible'%configDir)



    def read_configuration(self, configDir):
        configFile = configDir+'/elComandante.conf'
        self.config = BetterConfigParser()
        self.config.read(configFile)

        self.directories['configDir'] = configDir
        self.directories['baseDir'] = self.config.get('Directories','baseDir')
        self.directories['testdefDir'] = self.config.get('Directories','testDefinitions')
        self.directories['dataDir'] = self.config.get('Directories','dataDir')
        self.directories['defaultParameters'] = self.config.get('Directories','defaultParameters')
        self.directories['subserverDir'] = self.config.get('Directories','subserverDir')
        self.directories['keithleyDir'] = self.config.get('Directories','keithleyDir')
        self.directories['jumoDir'] = self.config.get('Directories','jumoDir')
        self.directories['logDir'] = self.directories['dataDir']+'/logfiles/'
        if self.config.has_option('Directories','storageDir'):
             self.directories['storageDir'] = self.config.get('Directories','storageDir')
        else:
            self.directories['storageDir']= self.directories['dataDir']+'/storage/'
        self.config.Directories = self.directories

        for dir in self.directories:
            self.directories[dir] = os.path.abspath(self.directories[dir].replace("$configDir$",configDir))

    def set_operator(self):
        operator = raw_input('Please enter the name of the operator:\t')
        self.init.set('OperationDetails','Operator',operator)

    def set_testCenter(self):
        testCenter = raw_input('Please enter the name of your TestCenter:\t')
        self.init.set('OperationDetails','TestCenter',testCenter)


    def write_initialization(self, configDir):
        hostname= socket.gethostname()
        if not self.init.has_section('OperationDetails'):
            self.init.add_section('OperationDetails')
        self.init.set('OperationDetails','Hostname',hostname)
     
        if not self.init.has_option('OperationDetails','TestCenter'):
            self.set_testCenter()

        if not self.init.has_option('OperationDetails','Operator'):
            self.set_operator()

        print 'Hostname: ',self.init.get('OperationDetails','Hostname')
        correct = False
        while not correct:
            print '\nOperator: ',self.init.get('OperationDetails','Operator')
            print 'TestCenter: ',self.init.get('OperationDetails','TestCenter')
            correct = userQueries.query_yes_no('Is this correct?')
            if not correct:
                self.set_testCenter()
                self.set_operator()
        with open(configDir+'elComandante.ini', 'wb') as configfile:
            self.init.write(configfile)

        print 'ini file has been updated'

    def read_initialization(self, configDir):
        iniFile = configDir+'/elComandante.ini'
        self.init = BetterConfigParser(dict_type=OrderedDict)
        self.init.read(iniFile)
        self.write_initialization(configDir)

    def setup_directories(self):
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
        except:
            os.mkdir(self.directories['logDir'])
        else:
            if nLogFiles>0:
                if userQueries.query_yes_no('Do you want to overwrite \'%s\'?'%logFiles,default='no'):
                    try:
                        rmtree(self.directories['logDir'])
                        os.mkdir(self.directories['logDir'])
                    except:
                        pass
                else:
                    raise Exception('LogDir is not empty. Please clean logDir: %s'%self.directories['logDir'])

    def initialize_logger(self, timestamp):
        self.log = printer()
        self.log.set_name("elComandanteLog")
        self.log.timestamp = timestamp
        self.log.set_logfile(self.directories['logDir'],'elComandante.log')
        self.log.printw()
        #self.log<<'Set LogFile to %s'%self.log.f

    def ensure_subserver_running(self):
        if os.system("ps -ef | grep -v grep | grep subserver > /dev/null"):
            os.system("cd %s && subserver"%(self.directories['subserverDir']))
            if os.system("ps -ef | grep -v grep | grep subserver"):
                raise Exception("Could not start subserver");

        #check if subsystem server is running, if not START subserver
        #if not "subserver.pid" in os.listdir("/var/tmp"):
        #    self.log << "Starting subserver ..."
        #    os.system("cd %s && subserver"%(self.directories['subserverDir']))
        #    time.sleep(0.2)
        #    #check again whether it is running
        #    if not "subserver.pid" in os.listdir("/var/tmp"):
        #        raise Exception("Could not start subserver")
        self.log << "Subserver is running."
        self.log.printn()

    def start_subsystem_client(self):
        serverZiel=self.config.get('subsystem','Ziel')
        serverPort = int(self.config.get('subsystem','Port'))
        self.subsystem_client = sClient(serverZiel,serverPort,"elComandante")

    def create_los_agentes(self, timestamp):
        # Create agentes that are responsible for client processes
        self.los_agentes.append(psi_agente.psi_agente(timestamp, self.log, self.subsystem_client))
        if self.init.getboolean("Keithley", "KeithleyUse"):
            self.los_agentes.append(highVoltage_agente.highVoltage_agente(timestamp,self.log,self.subsystem_client))
        self.los_agentes.append(watchDog_agente.watchDog_agente(timestamp,self.log,self.subsystem_client))
        if self.init.getboolean("Xray", "XrayUse"):
            self.los_agentes.append(xray_agente.xray_agente(timestamp, self.log, self.subsystem_client))
        self.los_agentes.append(analysis_agente.analysis_agente(timestamp, self.log, self.subsystem_client))
        if self.init.getboolean("CoolingBox", "CoolingBoxUse"):
            self.los_agentes.append(coolingBox_agente.coolingBox_agente(timestamp, self.log,self.subsystem_client))
        if self.init.getboolean("LowVoltage", "LowVoltageUse"):
            self.los_agentes.append(lowVoltage_agente.lowVoltage_agente(timestamp, self.log,self.subsystem_client))

        # Make the agentes read their configuration and initialization parameters
        for agente in self.los_agentes:
            self.log << "Setting up agente %s ..." % agente.agente_name
            agente.setup_dir(self.directories)
            agente.setup_configuration(self.config)
            agente.setup_initialization(self.init)

    def setupParentDir(self, timestamp, Testboard):
        Testboard.dataDir = self.directories['dataDir']
        if self.init.has_option('Tests','TestDescription'):
            Testboard.moduleDir = '%s_%s_%s_%s'%(Testboard.module,self.init.get('Tests','TestDescription'),strftime("%Y-%m-%d_%Hh%Mm",localtime(timestamp)),timestamp)
        else:
            Testboard.moduleDir = '%s_%s_%s'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",localtime(timestamp)),timestamp)
        try:
            os.stat(Testboard.parentDir)
        except:
            os.mkdir(Testboard.parentDir)
        return Testboard.parentDir

    def wait_until_finished(self):
        finished = all([agente.check_finished() for agente in self.los_agentes])
        time0 = time.time()
        while not finished:
            queue = []
            for agente in self.los_agentes:
                if not agente.check_finished():
                    queue.append(agente.agente_name)
            if (len(queue) > 0):
                time1 = time.time()
                timeDuration = divmod(time1-time0, 60)
                sys.stdout.write("\r\x1b[2K" + self.log.get_prefix() + "Waiting for " + ", ".join(queue) + " ... %d min %d sec"%(timeDuration[0], timeDuration[1]))
                sys.stdout.flush()
            time.sleep(0.25)
            finished = all([agente.check_finished() for agente in self.los_agentes])
            if finished:
                sys.stdout.write("\r\x1b[2K")

    ## Main procedure of elComandante
    ##
    ## Runs the procedural part of elComandante which includes initialization,
    ## testing, and cleanup. Throws exceptions that have to be caught on the
    ## outside where the procedure is called.
    def run(self):
        timestamp = int(time.time())
        args = self.parse_command_line_arguments()
        self.check_config_directory(args.configDir)
        self.read_configuration(args.configDir)
        self.read_initialization(args.configDir)
        self.setup_directories()
        self.initialize_logger(timestamp)

        self.check_for_tar(self.directories['dataDir'])

        self.ensure_subserver_running()
        self.start_subsystem_client()

        self.create_los_agentes(timestamp)

        self.log.printn()

        #subscribe subscriptions
        coolingBoxSubscription = self.config.get('subsystem','coolingBoxSubscription')
        if self.init.getboolean("CoolingBox", "CoolingBoxUse"):
            self.subscriptionList.append(coolingBoxSubscription)
        for subscription in self.subscriptionList:
            self.subsystem_client.subscribe(subscription)

        for agente in self.los_agentes:
            agente.subscribe()

        #directory config
        #get list of tests to do:
        testlist=self.init.get('Tests','Test')
        test_chain = testchain.parse_test_list(testlist)
        testlist= testlist.split(',')
        while '' in testlist:
                testlist.remove('')

        # Get default parameter directories
        dir_list = []
        tb = 0
        while True:
            try:
                module_type = self.init.get("ModuleType", "TB" + `tb`)
                dir = self.config.get("defaultParameters", module_type)
            except:
                break
            else:
                dir = self.directories['defaultParameters'] + '/' + dir
                self.log << "append " + dir + " to dir_list"
                dir_list.append(dir)
                tb += 1
        test_chain.parameter_dir = dir_list

        #-------------------------------------

        # Check whether the client is already running before trying to start it
        self.log << "Checking whether clients are runnning ..."
        for agente in self.los_agentes:
            agente.check_client_running()

        for agente in self.los_agentes:
            agente.start_client(timestamp)
        self.log.printn()#        self.parentDir='.'

        # Check the client subscriptions
        self.log << "Checking subscriptions of the clients ..."
        time.sleep(2)
        for subscription in self.subscriptionList:
            if not self.subsystem_client.checkSubscription(subscription):
                raise Exception("Cannot read from %s subscription"%subscription)
            else:
                self.log << "\t%s is answering." % subscription
        for agente in self.los_agentes:
            if not agente.check_subscription():
                raise Exception("Cannot read from agente: %s subscription" % agente.subscription)
            else:
                self.log << "\t%s is answering." % agente.subscription

        #-------------SETUP TESTBOARDS----------------
        self.log.printv()
        self.log << 'The following testboards with modules were found:'
        self.log.printn()
        #ToDo:
        for Testboard in self.los_agentes[0].Testboards:
                self.setupParentDir(timestamp,Testboard)
                self.log << '\t- Testboard %s at address %s with module %s'%(Testboard.slot,Testboard.address,Testboard.module)

        self.log.printn()

        test = test_chain.next()
        while test:
            if test.test_str.find('@')>=0:
                whichtest, env = test.test_str.split('@')
            else:
                whichtest = test.test_str
                env = 17.0
            if whichtest== "IV" or whichtest=='leakageCurrentPON' or whichtest=='leakageCurrentPOFF':
                test_str_list = []
                for Testboard in self.los_agentes[0].Testboards:
                    test_str_list.append('%s_TB%s@%s' % (whichtest, Testboard.slot, env))
                test.multiply(test_str_list)
            test = test.next()

        self.log << 'The following tests will be executed:'
        self.log.printn()
        test = test_chain.next()
        number_of_tests = 0
        while test:
            self.log << "\t- %s" % test.test_str
            test = test.next()
            number_of_tests += 1

        self.log.printv()

        #--------------LOOP over TESTS-----------------

        test = test_chain.next()
        testno = 0
        test_chain_durations = []
        while test:
            startTimeCompleteTest = time.time()
            env = environment.environment(test.test_str, self.init)
            test.environment = env
            test.testname = test.test_str.split("@")[0]

            testno += 1
            self.log << "Test %i/%i: %s" % (testno, number_of_tests, test.test_str)
            self.log.printn()

            for agente in self.los_agentes:
                agente.set_test(test)

            # Prepare for the tests
            self.log << "Preparing test %s ..." % test.test_str
            startTime = time.time()
            for agente in self.los_agentes:
                agente.prepare_test(test.test_str, env)
            self.wait_until_finished()
            endTime = time.time()

            testDuration = divmod(endTime-startTime, 60)
            self.log << "Preparation took %i seconds (%i min %i sec)" % (endTime-startTime, testDuration[0], testDuration[1])

            self.log.printn()

            # Execute tests
            startTime = time.time()
            self.log << "Executing test %s ..." % test.test_str
            for agente in self.los_agentes:
                agente.execute_test()
            self.wait_until_finished()

            self.log.printn()
            endTime = time.time()
            testDuration = divmod(endTime-startTime, 60)
            self.log << "Test took %i seconds (%i min %i sec)" % (endTime-startTime, testDuration[0], testDuration[1])
            # Cleanup tests
            self.log << "Cleaning up after test %s ..." % test.test_str
            for agente in self.los_agentes:
                agente.cleanup_test()
            self.wait_until_finished()

            self.log.printv()
            endTimeCompleteTest = time.time()
            test_chain_durations.append([test.test_str, endTimeCompleteTest-startTimeCompleteTest])
            test = test.next()

        # Final cleanup
        self.log << "Final cleanup after all tests ..."
        for agente in self.los_agentes:
            agente.final_test_cleanup()
        # Wait for final cleanup to finish
        self.wait_until_finished()

        #-------------EXIT----------------

        self.log.printv()
        self.log << "Finished all tests. Summary of test durations:"
        totalDuration = 0
        for testDuration in test_chain_durations:
            totalDuration += testDuration[1]
            testDurationMinSec = divmod(testDuration[1], 60)
            self.log << "    {0:<30} {1:>4d} min {2:>2d} sec".format(testDuration[0], int(testDurationMinSec[0]), int(testDurationMinSec[1]))
        self.log << "  --------------------------------------------------"
        testDurationMinSec = divmod(totalDuration, 60)
        self.log << "    {0:<30} {1:>4d} min {2:>2d} sec".format("total", int(testDurationMinSec[0]), int(testDurationMinSec[1]))
       
        userQueries.query_any("Press ENTER to terminate the clients. ", self.log)

        self.log << "Asking clients to quit ..."
        for agente in self.los_agentes:
            agente.request_client_exit()

        self.subsystem_client.closeConnection()
        self.log << 'Subsystem connection closed.'

        time.sleep(1)
        self.log << "Killing remaining children ..."
        self.killChildren()
        time.sleep(1)

        while self.subsystem_client.anzahl_threads > 0:
            pass
        self.log.printv()
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
                    shutil.copy2(args.configDir+'/elComandante.ini', dest)
                    shutil.copy2(args.configDir+'/elComandante.conf', dest)
                except:
                    raise
        self.createTarFiles(self.los_agentes[0])
        self.check_for_tar(self.directories['dataDir'])

        self.log.printv()
        self.log << 'ciao!'

        del self.log

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
                    self.removeDir(Testboard.moduleDir)
            except:
                pass
        self.killChildren()

### PROCEDURAL CODE ##################################################################

## Main instance of the elComandante class
elComandante = el_comandante()
try:
    elComandante.run()
except:
    # Print information about the exception
    traceback.print_exc()

    # Print an empty line
    print

    # Wait for the user to acknowledge
    userQueries.query_any("An exception occurred. Press ENTER to close the program. ")

    # Allow elComandante to clean up
    elComandante.clean_up_exception()

    sys.exit(0)
