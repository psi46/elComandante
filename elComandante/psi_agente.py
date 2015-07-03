import os
import glob
import sys
sys.path.insert(1, "../")
from myutils import BetterConfigParser, sClient, decode, printer, preexec
from myutils import Testboard as Testboarddefinition
from time import strftime, gmtime, sleep
import time
from shutil import copytree,rmtree
import argparse
import environment
import signal
import el_agente
import subprocess
import glob
import shutil


class psi_agente(el_agente.el_agente):
    def __init__(self, timestamp,log, sclient):
        el_agente.el_agente.__init__(self,timestamp, log, sclient)
        self.agente_name = "psiAgente"
        self.client_name = "psiClient"
        self.currenttest=None
        self.init = None
        self.active = True
        self.Testboards = []
        
    def setup_configuration(self, conf):
        self.conf = conf
        self.subscription = conf.get("subsystem", "psiSubscription")
        self.highVoltageSubscription = conf.get("subsystem","keithleySubscription");
        #self.logdir = conf.get("Directories", "dataDir") + "/logfiles/"
        self.active = True
    def setup_initialization(self, init):
        self.init = init
        self.Testboards=[]
        for tb, module in init.items('Modules'):
            if init.getboolean('TestboardUse',tb):
                self.Testboards.append(Testboarddefinition(int(tb[2]),module,self.conf.get('TestboardAddress',tb),init.get('ModuleType',tb)))
                defaultParameterDirectory = self.conf.get('defaultParameters',self.Testboards[-1].type)
                self.Testboards[-1].defparamdir=self.Directories['defaultParameters']+'/'+ defaultParameterDirectory
                self.log << '\t- Testboard %s at address %s with Module %s reading configuration from %s '%(self.Testboards[-1].slot,self.Testboards[-1].address,self.Testboards[-1].module, defaultParameterDirectory)
        self.numTestboards = len(init.items('Modules'))
    def check_client_running(self):
        if not self.active:
            return False
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
        command = "xterm -T 'PSI46master' +sb -sl 5000 -geometry 120x50+660+32 -fs 10 -fa 'Mono' -e "
        command += "python ../psiClient/psi46master.py "
        command += "-dir %s "%(self.Directories['logDir'])
        command += "-num %s"%self.numTestboards
        self.log << "Starting " + self.client_name + " ..."
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
    def set_test(self, test):
        el_agente.el_agente.set_test(self, test)
        self.test.parameter_dir = list(test.parent.parameter_dir)

    def prepare_test(self, whichtest, env):
        if not self.active:
            return False
        # Run before a test is executed
        self.activeTestboard = -1
        self.currenttest = whichtest.split('@')[0]
        if 'IV' in self.currenttest:
            activeTestboard = self.currenttest.split('_')
            if len(activeTestboard) == 2:
                self.currenttest = 'IV'
                self.activeTestboard = int(activeTestboard[1].strip('TB'))
                #print 'active testboard for IV is: %s'%self.activeTestboard
        elif 'leakageCurrent' in self.currenttest:
            activeTestboard = self.currenttest.split('_')
            if len(activeTestboard) == 2:
                if 'PON' in self.currenttest:
                    self.currenttest = 'leakageCurrentPON'
                elif 'POFF' in self.currenttest:
                    self.currenttest = 'leakageCurrentPOFF'
                self.activeTestboard = int(activeTestboard[1].strip('TB'))
                #print 'active testboard for IV is: %s'%self.activeTestboard
        for Testboard in self.Testboards:
            self._prepare_testboard(Testboard)
        self.pending = False
        return True

    def _prepare_testboard(self,Testboard):
        if 'IV' in self.currenttest or 'leakageCurrent' in self.currenttest:
            if Testboard.slot != self.activeTestboard:
            #check if it's this TB's turn
                return
        if self.test.environment.temperature >=0:
            tempString = "p%s"%int(self.test.environment.temperature)
        else:
            tempString = "m%s"%(-1*int(self.test.environment.temperature))

        Testboard.testdir = Testboard.parentDir + '/%s_%s_%s/' % (str(Testboard.numerator).zfill(3), self.currenttest, tempString)
        self._setupdir_testboard(Testboard)
        if 'IV' in self.currenttest:
            self.sclient.send(self.highVoltageSubscription,":PROG:IV:TESTDIR %s\n"%Testboard.testdir)
            self.open_testboard(Testboard)
        elif 'leakageCurrent' in self.currenttest:
            poff = False
            if 'POFF' in self.currenttest:
                poff = True
            self.sclient.send(self.highVoltageSubscription, ":PROG:LEAKAGECURRENT:TESTDIR %s\n"%Testboard.testdir)
            self.open_testboard(Testboard,poff)

    def powercycle(self):
        self.currenttest='powercycle'
        for Testboard in self.Testboards:
            self._prepare_testboard(Testboard)
        time.sleep(4)
        self.execute_test()
        time.sleep(4)
        self.log <<" %s: do internal power_cycle" %(self.agente_name)
        start_time = time.time()
        while True:
            if self.check_finished():
                break
            current_time = start_time
            if current_time - start_time > 120:
                self.log <<" %s: internal power_cycle didn't finished in 120sec, restarting powercycle" % self.agente_name
                self.execute_test()
                start_time = time.time()
            sleep(4)
        time.sleep(4)
        self.cleanup_test()
        self.sclient.clearPackets(self.subscription)

    def execute_test(self):
        if not self.active:
            return False
        self.pending = True
        # Runs a test
        self.sclient.clearPackets(self.subscription)
        if self.currenttest.lower().startswith('iv') or self.currenttest.lower().startswith('cycle') or self.currenttest.lower().startswith('leakagecurrent'):
            self.pending = False
            return True
        elif not self.currenttest == 'powercycle':
            pass
        else:
            self.log << 'Powercycling Testboards'
        for Testboard in self.Testboards:
            self._execute_testboard(Testboard)
            sleep(10)
        sleep(1)
        self.sclient.clearPackets(self.subscription)
        return True    

            #Here we need the Errorstream of the watchdog:

            #packet = self.sclient.getFirstPacket(coolingBoxSubscription)
            #if not packet.isEmpty() and not "pong" in packet.data.lower():
            #    data = packet.data
            #    Time,coms,typ,msg = decode(data)[:4]
            #    if coms[0].find('STAT')==0 and typ == 'a' and 'ERROR' in msg[0].upper():
            #        self.log.warning('jumo has error!')
            #        self.log.warning('\t--> I will abort the tests...')
            #        self.log.printn()
            #        for Testboard in self.Testboards:
            #            self.sclient.send(self.subscription,':prog:TB%s:kill\n'%Testboard.slot)
            #            self.log.warning('\t Killing psi46 at Testboard %s'%Testboard.slot)
            #            index=[Testboard.slot==int(coms[1][2]) for Testboard in self.Testboards].index(True)
            #            Testboard.failed()
            #            Testboard.busy=False


    def _execute_testboard(self,Testboard): 
        Testboard.busy=True

        # Determine the batch file name. If not specified in the .ini file this file name
        # is equal to the test name.
        batchFile = self.currenttest
        try:
            batchFile = self.init.get("Test " + self.currenttest, "batchFile")
        except:
            pass

        self.sclient.send(self.subscription,':prog:TB%s:start %s,%s,commander_%s\n'%(Testboard.slot,self.Directories['testdefDir']+'/'+ batchFile,Testboard.testdir,self.currenttest))
        self.sclient.clearPackets(self.subscription)


    def cleanup_test(self):
        # Run after a test has executed
        if not self.active:
            return True
        if 'IV' in self.currenttest or 'leakageCurrent' in self.currenttest:
            for Testboard in self.Testboards:
                if Testboard.slot == self.activeTestboard:
                    self.close_testboard(Testboard)
                    Testboard.numerator += 1 
        elif not self.currenttest == 'powercycle':  
            for Testboard in self.Testboards:
                Testboard.numerator += 1
        else:
            for Testboard in self.Testboards:
                self._deldir(Testboard)
        self.sclient.clearPackets(self.subscription) 
        return True

    def final_test_cleanup(self):
        # Run after a test has executed
        #self.powercycle()
        if not self.active:
            return True
        return True

    def check_finished(self):
        if not self.active or not self.pending:
            return True
        sleep(1)
        for Testboard in self.Testboards:
            if Testboard.busy:
                self.sclient.send(self.subscription,":STAT:TB%d?\n"%Testboard.slot)
                sleep(1)
        sleep(1)
        while True:
            packet = self.sclient.getFirstPacket(self.subscription) 
            if packet.isEmpty() or not self.pending:
                break
            if "pong" in packet.data.lower():
            	continue
            data = packet.data
            Time,coms,typ,msg = decode(data)[:4]
            if type(msg)== str:
                msg = msg.lower()
            if len(coms) >= 2:
                if 'busy' in msg:
                    continue
                msg = msg.split(':')
                if coms[0].lower().find('stat')==0 and coms[1].lower().find('tb')==0 and typ == 'a':
                    com = coms[1][2:] 
                    try:
                        TBslot = int(com)
                    except:
                        self.log << "%s: Couldn't convert command to TBslot %s"%(self.agente_name,com)
                        continue
                    if len(msg)>1:
                        #Finished
                        if msg[1].startswith('finished'):
                            try:
                                index=[Testboard.slot==TBslot for Testboard in self.Testboards].index(True)
                                if self.Testboards[index].busy!=False:
                                    self.Testboards[index].busy=False
                                    self.Testboards[index].failedPowercycles = 0
                                    TBsbusy = [TB.busy for TB in self.Testboards]
                                    TBsindex= [TB.slot for TB in self.Testboards]
                            except:
                                self.log.warning("%s: Couldn't find TB with slot %s, %s"%(self.agente_name,TBslot,[TB.slot for TB in self.Testboards]))
                                #raise
                        #FAILED
                        elif msg[1].startswith('failed'):
                            try:
                                index=[Testboard.slot==TBslot for Testboard in self.Testboards].index(True)
                            except:
                                self.log.warning("%s: Couldn't find TB with slot %s, %s"%(self.agente_name,TBslot,[TB.slot for TB in self.Testboards]))
                                index =-1
                                #raise
                            if self.currenttest == 'powercycle':
                                if index !=-1:
                                    sleep(3)
                                    TBsbusy = [TB.busy for TB in self.Testboards]
                                    TBsindex= [TB.slot for TB in self.Testboards]
                                    self.log<<"%s:  self.Testboards[%s] could not be opened: %s: %s-%s" % (self.agente_name,index, self.pending, TBsindex,TBsbusy)
                                    self.Testboards[index].failedPowercycles += 1
                                    
                                    if self.Testboards[index].failedPowercycles < 3:
                                        self.log<<"%s:  self.Testboards[%s] restart Powecycle: %s: %s-%s" % (self.agente_name,index, self.pending, TBsindex,TBsbusy)
                                        self._execute_testboard(self.Testboards[index])
                                    else:
                                        self.log.warning('Could not open Testboard at %s.'%Testboard.slot)
                                else:
                                    self.log.warning('index is out of range: %s'%index)

                            elif self.Testboards[index].busy==True:
                                self.log<<""
                                self.Testboards[index].busy=False
                                TBsbusy = [TB.busy for TB in self.Testboards]
                                TBsindex= [TB.slot for TB in self.Testboards]
                                self.log<<"%s:  self.Testboards[%s] failed- the following boards are busy: %s: %s-%s" % (self.agente_name,index, self.pending, TBsindex,TBsbusy)
                        elif msg[1].startswith('unknown'):
                            TBsbusy = [TB.busy for TB in self.Testboards]
                            TBsindex= [TB.slot for TB in self.Testboards]
                            try:
                                index=[Testboard.slot==TBslot for Testboard in self.Testboards].index(True)
                            except:
                                self.log<<"%s: Couldn't find TB with slot %s, %s"%(self.agente_name,TBslot,[TB.slot for TB in self.Testboards])
                                index =-1
                            if self.Testboards[index].busy==True:
                                self.Testboards[index].busy=False
                                self.log<<""
                                self.log <<"%s: Status of TB in slot %s is unknown"%(self.agente_name,TBslot)

            self.pending = any([Testboard.busy for Testboard in self.Testboards])
        self.pending = any([Testboard.busy for Testboard in self.Testboards])
        #self.log<<[Testboard.busy for Testboard in self.Testboards]
        return not self.pending

    def _setupdir_testboard(self,Testboard):
        #if not self.currenttest == 'powercycle':
        self.log << 'Setting up the directory: %s'%Testboard.testdir
        self.log << '... with Parameters from: %s' % self.test.parent.parameter_dir[Testboard.slot]
        #copy directory
        try:
            self.test.parameter_dir[Testboard.slot] = Testboard.testdir
            self.sclient.send('/watchDog',':TB%s:TESTDIR! %s\n'%(Testboard.slot,Testboard.testdir))
            copytree(self.test.parent.parameter_dir[Testboard.slot], Testboard.testdir)
            if Testboard.DTB:    
                #self.log.warning("if Testboard.DTB is fulfilled") 
                print self.test.parent.parameter_dir[Testboard.slot]
                directories = [self.test.parent.parameter_dir[Testboard.slot]+d for d in os.listdir(self.test.parent.parameter_dir[Testboard.slot]) if os.path.isdir(self.test.parent.parameter_dir[Testboard.slot]+d)] 
                if len(directories) > 0:
                    self.log.warning("len(directories)>0 is fulfilled") 
                    latest_subdir = max(directories, key=os.path.getmtime)
                    print latest_subdir
                    for filename in glob.glob(os.path.join(latest_subdir, '*.*')):
                        shutil.copy(filename, Testboard.testdir)
                        self.log.warning("copying %s to folder %s" %(filename, Testboard.testdir)) 
            self._setup_configfiles(Testboard)
        except IOError as e:
            self.log.warning("I/O error({0}): {1}".format(e.errno, e.strerror))
            raise
        except OSError as e:
            self.log.warning("OS error({0}): {1}".format(e.errno, e.strerror))
            raise

    def _setup_configfiles(self, Testboard):
        """ Changes config files in the already copied test directory according to test definitions
            from elComandante's init file. """

        # Delete all root files which are already in the directory
        root_files = glob.glob(Testboard.testdir+'/*.root')
        for f in root_files:
            os.remove(f)
        # Change testboard name
	if Testboard.DTB and os.path.isfile(Testboard.testdir + "/tb"):
            self._config_file_content_substitute(Testboard.testdir + "/tb", {"id":Testboard.address})
        else:
            self._config_file_content_substitute(Testboard.testdir + "/configParameters.dat", {"testboardName":Testboard.address})

        # Get test specific config parameters (if available)
        params = ()
        try:
            params = self.init.items("Test " + self.test.testname)
        except:
            return
        for par in params:
            file = par[0]
            if '.cfg' in file:
                section,pair = par[1].split(':')
                key,value = pair.split('=')
                config_file = BetterConfigParser()
                config_file.read(Testboard.testdir + "/" + file)
                config_file.set(section,key,value)
                write_file = open(Testboard.testdir + "/" + file, 'write')
                config_file.write(write_file)
                write_file.close()
                continue
            # Check for valid keys that represent config files
            elif "testParameters" in file or "dacParameters" in file or "configParameters" in file:
                pass
            elif "tbmParameters" in file or "tbParameters" in file:
                pass
            else:
                continue

            encoded_keys = par[1].split(",")
            keys = {}
            for key in encoded_keys:
                key = key.split("=", 2)
                if len(key) != 2:
                    continue
                keys[key[0]] = key[1]
            if len(file) < 4 or file[-4:] != ".dat":
                file += ".dat"
            self._config_file_content_substitute(Testboard.testdir + "/" + file, keys)

    def _config_file_content_substitute(self, filename, keys):
        """ Substitutes configuration lines in a file according to the dictionary "keys" """
        # Open the file for substitution
        try:
            f = open(filename, "r")
            lines = f.readlines()
            f.close()
        except:
            self.log.warning("Error reading from parameter file " + filename + ".")
            raise

        try:
            # Backup the original file
            f = open(filename + ".original", "w")
            f.write("".join(lines))
            f.close()
        except:
            self.log.warning("Error making a backup file of " + filename + ". Skipped.")

        # Define the fields within the file
        fields = 2
        keyfield = 0
        datafield = 1
        if "dacParameters" in filename or "tbmParameters" in filename or "tbParameters" in filename:
            fields = 3
            keyfield = 1
            datafield = 2

        # iterate over all lines
        for i in range(len(lines)):
            line = lines[i].strip()
            if len(line) == 0 or line[0] == '-' or line[0] == '#':
                continue
            line = line.split(None, fields - 1)
            if len(line) != fields:
                continue
            # check whether this line matches a key
            if not line[keyfield] in keys:
                continue
            line[datafield] = keys[line[keyfield]]
            if line[datafield].startswith('DTB') and line[keyfield] == 'id':
                lines[i] = " : ".join(line)
                lines[i] += '\n'
            else:
                lines[i] = " ".join(line)
                lines[i] += '\n'

        try:
            # Write the new file
            f = open(filename, "w")
            f.write("".join(lines))
            f.close()
        except:
            self.log.warning("Error saving parameters in " + filename + ".")
            raise

    def _deldir(self,Testboard):
        try:
            rmtree(Testboard.testdir)
        except:
            self.log.warning("Couldn't remove directory")
            pass

    def open_testboard(self,Testboard,poff=False):
        self.sclient.clearPackets(self.subscription)
        self.log << "%s: Opening Testboard %s ..." % (self.agente_name, Testboard.slot)
        if poff:
            self.sclient.send(self.subscription,':prog:TB%s:openpoff %s, %s\n'%(Testboard.slot,Testboard.testdir,self.currenttest)) 
        else:
            self.sclient.send(self.subscription,':prog:TB%s:open %s, %s\n'%(Testboard.slot,Testboard.testdir,self.currenttest)) 
    
    def close_testboard(self,Testboard):
        self.log << "%s: Closing Testboard %s ..." % (self.agente_name, Testboard.slot)
        self.sclient.send(self.subscription,':prog:TB%s:close %s\n'%(Testboard.slot,Testboard.testdir))
