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
import alerts_agente
import socket
import tarfile
import glob
import traceback
import operator

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
        self.trimVcal = -1

        self.alertSubscription = "/alerts"
        self.agenteTimeoutMins = 120
        self.agenteTimeoutAlertSent = False

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
    def removeDir(self, DirectoryName):
        AbsolutePath = '%s/%s' % (self.directories['dataDir'], DirectoryName)
        if not os.path.isdir(AbsolutePath) or userQueries.query_yes_no("Do you want to store the data %s?" % DirectoryName):
            return
        self.log << "Removing Directory: '%s'" % AbsolutePath

        try:
            rmtree(AbsolutePath)
        except:
            self.log.warning("Unable to remove '%s'!" % AbsolutePath)
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
        parser.add_argument("-I","--initfile",dest='initfiles',
                           help='additional InitFiles which can overwrite the defaults',
                           default=[],
                           action='append')
        parser.add_argument("-C","--configfile",dest='configfiles',
                           help='additional ConfigFiles which can overwrite the defaults',
                           default=[],
                           action='append')
        parser.add_argument("-T", "--trim", dest="trim",
                               help="trim to vcal",
                               default="")
        args = parser.parse_args()
        return args

    def check_config_directory(self, configDir):
        try:
            os.access(configDir,os.R_OK)
        except:
            raise Exception('configDir \'%s\' is not accessible'%configDir)



    def read_configuration(self, configDir,configFileNames = []):

        # read main config file
        configFile = configDir+'/elComandante.conf'
        self.config = BetterConfigParser()
        self.config.read(configFile)

        # read additional config files which can overwrite configuration
        for fname in configFileNames:
            confFileFullPath = configDir + '/' + fname
            if os.path.isfile(confFileFullPath):
                self.config.read(confFileFullPath)
                print "read additional config file: \x1b[32m%s\x1b[0m"%fname
            elif os.path.isfile(confFileFullPath + '.conf'):
                self.config.read(confFileFullPath + '.conf')
                print "read additional config file: \x1b[32m%s.conf\x1b[0m"%fname
            else:
                print "\x1b[31madditional config file not found: %s\x1b[0m"%fname

        # set directories
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

    def read_initialization(self, configDir, iniFileNames = []):
        iniFile = configDir+'/elComandante.ini'
        self.init = BetterConfigParser(dict_type=OrderedDict)
        self.init.read(iniFile)
        for fname in iniFileNames:
            iniFileFullPath = configDir + '/' + fname
            if os.path.isfile(iniFileFullPath):
                self.init.read(iniFileFullPath)
                print "read additional ini file: \x1b[32m%s\x1b[0m"%fname
            elif os.path.isfile(configDir+ '/' + fname + '.ini'):
                self.init.read(iniFileFullPath + '.ini')
                print "read additional ini file: \x1b[32m%s.ini\x1b[0m"%fname
            else:
                print "\x1b[31madditional ini file not found: %s\x1b[0m"%fname


    def check_for_barcode_reader(self, configDir):
        BarcodeReaderUse = False
        try:
            if self.init.has_option('BarcodeReader','BarcodeReaderUse') and self.init.get('BarcodeReader','BarcodeReaderUse').strip().lower() == 'true':
                BarcodeReaderUse = True
        except:
            print "no BarcodeReader option defined in .ini file, skipping"

        if BarcodeReaderUse:
            print 'BarcodeReader is activated'

            # workaround for wrong barcode labels
            CorrectModuleNames = False
            try:
                CorrectModuleNames = self.init.get('BarcodeReader','CorrectModuleNames').strip().lower() == 'true'
            except:
                pass

            # get number of modules
            Modules = []
            TBMax = 99 #maximum number of testboards to check
            for TB in range(0,TBMax):
                if self.init.has_option('Modules','TB%d'%TB):
                    Modules.append(self.init.get('Modules','TB%d'%TB))
                else:
                    break
            NModules = len(Modules)
            print "scan modules from TB0 to TB%d, ENTER to leave entry unchanged:"%NModules

            # scan all modules
            ModulesNew = []
            for TB in range(0, NModules):
                ModuleNew = raw_input(" scan module TB%d (current parameters: %s):"%(TB, Modules[TB])).upper().strip()
                if CorrectModuleNames and len(ModuleNew) > 0 and ModuleNew[0] == 'D':
                    ModuleNew = 'M' + ModuleNew[1::]
                    print " => module name corrected to: %s"%ModuleNew
                ModulesNew.append(ModuleNew)

            # fill module names
            if self.init.get('BarcodeReader','Fill').lower() in ['name', 'both']:
                for TB in range(0, NModules):
                    if len(ModulesNew[TB]) > 0: 
                        self.init.set('Modules','TB%d'%TB, ModulesNew[TB])

            # fill module types
            if self.init.get('BarcodeReader','Fill').lower() in ['type', 'both']:
                for TB in range(0, NModules):
                    if len(ModulesNew[TB]) > 0: 
                        self.init.set('ModuleType','TB%d'%TB, ModulesNew[TB])

            self.write_initialization(configDir)


    def display_configuration(self):
        TBMax = 99
        if self.trimVcal > 0:
            print 'Using parameters trimmed to: %d Vcal'%self.trimVcal
        else:
            print 'Using untrimmed parameters.'
        print 'Module configuration:'

        # read module, module type and testboard identifier from ini and conf file
        Testboards = []
        for TB in range(0,TBMax):
            if self.config.has_option('TestboardAddress','TB%d'%TB):
                Testboards.append(self.config.get('TestboardAddress','TB%d'%TB))
            else:
                break

        Modules = []
        for TB in range(0,TBMax):
            if self.init.has_option('Modules','TB%d'%TB):
                Modules.append(self.init.get('Modules','TB%d'%TB))
            else:
                break

        ModuleTypes = []
        for TB in range(0,TBMax):
            if self.init.has_option('ModuleType','TB%d'%TB):
                ModuleTypes.append(self.init.get('ModuleType','TB%d'%TB))
            else:
                break

        TBUse = []
        for TB in range(0,TBMax):
            if self.init.has_option('TestboardUse','TB%d'%TB):
                TBUse.append(self.init.get('TestboardUse','TB%d'%TB))
            else:
                break

        if not all(i==len(Testboards) for i in [len(Modules), len(ModuleTypes), len(TBUse)]):
            print '\x1b[31mERROR: number of TestboardAddress/TestboardUse/Modules/ModuleType rows not equal!\x1b[0m'

        # display modules table
        for TBIndex in range(0, len(Testboards)):
            BoxWidth = 54
            print '     +%s+'%('-'*BoxWidth)
            InfoStr = '%s %s (%s) %r'%(Testboards[TBIndex], Modules[TBIndex], ModuleTypes[TBIndex], TBUse[TBIndex])
            InfoStrLen = len(InfoStr)
            if TBUse[TBIndex].strip().lower() != 'true':
                InfoStr = '\x1b[31m%s\x1b[0m'%InfoStr
            print '     |     %s%s|'%(InfoStr, ' '*max(0, BoxWidth - 5 - InfoStrLen))
            print '     +%s+'%('-'*BoxWidth)

        Problems = 0

        # verify all tests in testlist exist
        print 'Verify test list:'
        testlist=self.init.get('Tests','Test')
        print "\x1b[32m%s\x1b[0m"%testlist
        test_chain = testchain.parse_test_list(testlist)
        testlist= testlist.replace('>',',').replace('{','').replace('}','').split(',')
        while '' in testlist:
                testlist.remove('')
        testlist = [testname.split('@')[0] if '@' in testname else testname for testname in testlist]
        SpecialTests = ['powercycle', 'leakagecurrent', 'pause', 'cycle']
        for testname in testlist:
            TestFound = False
            for SpecialTest in SpecialTests:
                if testname.lower().strip().startswith(SpecialTest) or testname.strip() == 'IV':
                    TestFound = True
                    break

            if not TestFound and not os.path.isfile("%s/%s"%(self.directories['testdefDir'], testname)):
                    print "\x1b[31mWARNING: test '%s' does not exist! \x1b[0m"%testname
                    Problems += 1

        # verify DACs
        if self.init.has_option('VerifyDACs','dacs'):
            DacList=self.init.get('VerifyDACs','dacs')
            DacConditions=[dac for dac in DacList.split(',') if len(dac.strip())>0]
            print "check DACs: %r"%DacConditions
            Operators = [
                    {'repr' : '==', 'op': operator.eq, 'op_swapped': operator.eq},
                    {'repr' : '!=', 'op': operator.ne, 'op_swapped': operator.ne},
                    {'repr' : '<=', 'op': operator.le, 'op_swapped': operator.ge},
                    {'repr' : '>=', 'op': operator.ge, 'op_swapped': operator.le},
                    {'repr' : '<', 'op': operator.lt, 'op_swapped': operator.gt},
                    {'repr' : '>', 'op': operator.gt, 'op_swapped': operator.lt},
                ]
            for DacCondition in DacConditions:

                Operator = None
                for CheckOperator in Operators:
                    OperatorPosition = DacCondition.find(CheckOperator['repr'])
                    if OperatorPosition > -1:
                        First = DacCondition[0:OperatorPosition].strip()
                        Second = DacCondition[OperatorPosition + len(CheckOperator['repr']):].strip()
                        if First.isdigit() and not Second.isdigit():
                            Dac = Second.lower()
                            CompareTo = int(First)
                            Operator = CheckOperator['op_swapped']
                        elif not First.isdigit() and Second.isdigit():
                            Dac = First.lower()
                            CompareTo = int(Second)
                            Operator = CheckOperator['op']
                        else:
                            print "\x1b[31mWARNING: illegal DAC condition %s! \x1b[0m"%DacCondition
                            Operator = None
                        break

                if Operator is not None:
                    for TBIndex in range(0, len(Testboards)):
                        if TBUse[TBIndex].lower().strip() == 'true':
                            module_type = self.init.get("ModuleType", "TB%d"%TBIndex)
                            try:
                                ModuleParametersDirectory = self.config.get("defaultParameters", module_type)
                            except:
                                ModuleParametersDirectory = module_type
                            ParametersDir = self.directories['defaultParameters'] + '/' + ModuleParametersDirectory
                            DacParameterFileNames = glob.glob(os.path.join(ParametersDir, 'dacParameters*.dat'))
                            for DacParameterFileName in DacParameterFileNames:
                                with open(DacParameterFileName, "r") as DacParameterFile:
                                    for line in DacParameterFile:
                                        data = [value for value in line.split(' ') if len(value.strip())>0]
                                        if data[1].lower() == Dac:
                                            DacValue = int(data[2])
                                            if not Operator(DacValue, CompareTo):
                                                print "\x1b[31mWARNING: DAC condition not fulfilled: '%s' \x1b[0m"%DacCondition
                                                print " module: %s"%self.init.get("Modules", "TB%d"%TBIndex)
                                                print " file: %s"%DacParameterFileName
                                                print " value found: %d"%DacValue
                                                Problems += 1

        if Problems < 1:
            print "no problems found."
        else:
            print "\x1b[31m%s problems found!\x1b[0m"%Problems

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
        self.los_agentes.append(psi_agente.psi_agente(timestamp=timestamp, log=self.log, sclient=self.subsystem_client, trimVcal=self.trimVcal))
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
        try:
            if self.init.getboolean("Alerts", "AlertsUse"):
                try:
                    AlertsAgenteName = "%s@%s"%(self.init.get('Tests','TestDescription'), socket.gethostname())
                except:
                    AlertsAgenteName = 'elComandante'
                self.los_agentes.append(alerts_agente.alerts_agente(timestamp, self.log, self.subsystem_client, name=AlertsAgenteName))
        except:
            pass

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
                try:
                    if timeDuration[0] >= self.agenteTimeoutMins and not self.agenteTimeoutAlertSent:
                        self.subsystem_client.send(self.alertSubscription, ":RAISE:WARNING:AGENTE:TIMEOUT waiting for %s for %d min %d sec\n"%(", ".join(queue), timeDuration[0], timeDuration[1]))
                        self.agenteTimeoutAlertSent = True
                except:
                    pass
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

        # read only main ini file first
        self.read_configuration(args.configDir, configFileNames=args.configfiles)
        self.read_initialization(args.configDir)

        # if barcode reader is enabled, user is asked to read in barcodes now
        self.check_for_barcode_reader(args.configDir)

        # ask if operator/host information is correct, write changes made to main config file (operator etc. and module id's from barcode scanner)
        self.write_initialization(args.configDir)

        # now read again including also additional config files specified in command line
        self.read_initialization(args.configDir, iniFileNames=args.initfiles)

        # try to read trimming parameter from command line first
        if len(args.trim.strip()) > 0:
            try:
                self.trimVcal = int(args.trim)
            except:
                print "bad trim parameter: '%s'"%args.trim

        # otherwise from config file
        if self.trimVcal < 0:
            try:
                testParameters = self.init.get('Test Trim','testParameters')
                pos1 = testParameters.find("=")
                if pos1 > 0:
                    testParametersName = testParameters[0:pos1]
                    testParametersValue = testParameters[pos1+1:]
                    if testParametersName.lower() == "vcal":
                        self.trimVcal = int(testParametersValue)
            except:
                pass

        # display full configuration
        self.display_configuration()

        correct = userQueries.query_yes_no('Start Qualification?')
        if not correct:
            exit()

        try:
            with open(args.configDir+'/LastQualification.ini', 'wb') as inifile:
                self.init.write(inifile)
            with open(args.configDir+'/LastQualification.conf', 'wb') as conffile:
                self.config.write(conffile)
        except:
            print "could not write LastQualification.ini/conf"

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

        try:
            QualificationType = self.init.get('Tests','TestDescription')
        except:
            QualificationType = 'unknown'

        self.subsystem_client.send(self.alertSubscription, ":RAISE:RUN %s\n"%QualificationType)

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
                try:
                    dir = self.config.get("defaultParameters", module_type)
                except:
                    dir = module_type
            except:
                break
            else:
                dir = self.directories['defaultParameters'] + '/' + dir
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
            if whichtest== "IV" or whichtest.lower()=='leakagecurrentpon' or whichtest.lower()=='leakagecurrentpoff':
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

        # alerts
        try:
            AlertsMessageModules = []
            for TB in range(0, 99):
                if self.init.has_option('Modules','TB%d'%TB):
                    if self.init.get('TestboardUse','TB%d'%TB).strip().lower() == 'true':
                        AlertsMessageModules.append(self.init.get('Modules','TB%d'%TB))
                else:
                    break
            AlertsMessage = "Modules: " + ", ".join(AlertsMessageModules)
        except:
            AlertsMessage = "Unable to read module configuration!"
        self.subsystem_client.send(self.alertSubscription, ":RAISE:QUALIFICATION:START %s\n"%AlertsMessage)

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

            # reset timeout alerts for each new test
            self.agenteTimeoutAlertSent = False

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
        
        Duration = "{0:>4d} min {1:>2d} sec".format(int(testDurationMinSec[0]), int(testDurationMinSec[1]))

        # alerts
        try:
            AlertsMessageModules = []
            for TB in range(0, 99):
                if self.init.has_option('Modules','TB%d'%TB):
                    if self.init.get('TestboardUse','TB%d'%TB).strip().lower() == 'true':
                        AlertsMessageModules.append(self.init.get('Modules','TB%d'%TB))
                else:
                    break
            AlertsMessage = "Modules: " + ", ".join(AlertsMessageModules)
        except:
            AlertsMessage = "Unable to read module configuration!"
        self.subsystem_client.send(self.alertSubscription, ":RAISE:QUALIFICATION:FINISHED %s; %s\n"%(AlertsMessage, Duration))

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
                    with open(dest+'/elComandante.ini', 'wb') as inifile:
                        self.init.write(inifile)
                    with open(dest+'/elComandante.conf', 'wb') as conffile:
                        self.config.write(conffile)
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
except Exception as e:
    # Print information about the exception
    traceback.print_exc()

    # Print an empty line
    print ""

    try:
        elComandante.subsystem_client.send(elComandante.alertSubscription, ":RAISE:EXCEPTION %s\n"%str(e))
    except:
        pass

    # Wait for the user to acknowledge
    userQueries.query_any("An exception occurred. Press ENTER to close the program. ")

    # Allow elComandante to clean up
    elComandante.clean_up_exception()

    sys.exit(0)

except KeyboardInterrupt:
    # Print information about the exception
    traceback.print_exc()

    # Print an empty line
    print ""

    try:
        elComandante.subsystem_client.send(elComandante.alertSubscription, ":RAISE:EXCEPTION:KEYBBOARDINTERRUPT \n")
    except:
        pass

    # Wait for the user to acknowledge
    userQueries.query_any("An exception occurred. Press ENTER to close the program. ")

    # Allow elComandante to clean up
    elComandante.clean_up_exception()

    sys.exit(0)