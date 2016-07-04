
from myutils import sClient, printer, decode, BetterConfigParser, colorgenerator
from threading import Thread
import subprocess
import sys
import select
from time import sleep
import os
class TBmaster(object):
    def __init__(self, TB, client, psiSubscription, Logger, color='black', psiVersion='psi46expert', trimVcal=-1):
        self.TB = TB
        self.client = client
        self.psiSubscription = psiSubscription
        self.color = color
        self.Logger = Logger
        self.TBSubscription = '/TB%s'%self.TB
        self.alertSubscription = '/alerts'
        self.client.subscribe(self.TBSubscription)
        self.dir = ''
        self.psiVersion = psiVersion
        if self.psiVersion.lower().endswith("psi46expert"):
            self.version ='psi46expert'
        elif self.psiVersion.lower().endswith("pyxar"):
            self.version ='pyxar'
        elif self.psiVersion.lower().endswith("pxar"):
            self.version ='pxar'
        else:
            self.version = 'unknown'
        self.failed = False
        self.busy = False
        self.testName ='unkown'
        self.testNo = -1
        self.TestEnd = False
        self.DoTest= False
        self.ClosePSI= False
        self.Abort = False
        self.LogFile = ""
        self.RootFile = ""
        # default value in Vcal units, -1 means use untrimmed parameters
        try:
            self.trimVcal = int(trimVcal)
        except:
            self.trimVcal = -1

        if self.trimVcal > -1:
            self.Logger << "TB%s: using option '-T %s' when calling pxar"%(self.TB, self.trimVcal)
        else:
            self.Logger << "TB%s: no [Test Trim] section found in ini file, using untrimmed parameters"%self.TB

    def _spawn(self,executestr):
        my_env = os.environ
        if my_env.has_key("LD_PRELOAD"):
            my_env["LD_PRELOAD"] = "/opt/glibc-2.14/lib/libc.so.6:" + my_env["LD_PRELOAD"]
        else:
            my_env["LD_PRELOAD"] = "/opt/glibc-2.14/lib/libc.so.6:"
        self.proc = subprocess.Popen([executestr,''], shell = True, stdout = subprocess.PIPE, stdin = subprocess.PIPE, env = my_env)
        self.busy = True

    def _kill(self):
        try:
            self.proc.kill()
            self.Logger.warning("PSI%s KILLED"%self.TB)
        except:
            self.Logger.warning("nothing to be killed")

    def _abort(self):
        self.Logger.warning('ABORT!')
        self._kill()
        self.Abort = False
        return True

    def _resetVariables(self):
        self.busy = False
        self.failed = False
        self.TestEnd = False
        self.DoTest = False
        self.ClosePSI = False
        self.Abort = False

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
        while self.proc.poll() is None and self.ClosePSI==False:
            if self.Abort:
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
                    self.failed = True
                    self._kill()
                if 'command not found' in line.strip():
                    self.Logger.warning("psi46expert for TB%s not found"%self.TB)
                if self.Abort:
                    internalFailed = self._abort()
                    self.failed = internalFailed or self.failed
        self.Logger << '>>> Release Testboard %s <<<'%self.TB
        self.TestEnd = True
        self.busy = False

        # check if log and root files have been written
        ErrorMessage = ""
        try:
            with open(self.LogFile) as f:
                lines = f.readlines()
            if not "welcome to pxar" in lines[0].lower() or not ("this is the end, my friend" in lines[-1].lower() or "pixsetup free fpxarmemory" in lines[-1].lower()):
                ErrorMessage = "WARNING: incomplete logfile: '%s'!"%self.LogFile
                print "\x1b[46m\x1b[97m",ErrorMessage,"\x1b[0m"
                internalFailed = True
                self.failed = True

        except:
            if len(self.LogFile) > 0:
                ErrorMessage = "WARNING: can't open logfile: '%s'!"%self.LogFile
                print "\x1b[46m\x1b[97m",ErrorMessage,"\x1b[0m"
                internalFailed = True
                self.failed = True

        if len(self.RootFile) > 0 and not os.path.isfile(self.RootFile):
            ErrorMessage = "CRITICAL: .root file does not exist: %s!"%self.RootFile
            print "\x1b[46m\x1b[97m",ErrorMessage,"\x1b[0m"
            internalFailed = True
            self.failed = True

        # send alert messages
        if not internalFailed:
            self.client.send(self.alertSubscription, ":RAISE:TB:TEST:FINISHED TB%s:%s\n"%(self.TB, self.get_directory_name()))
        else:
            self.client.send(self.alertSubscription, ":RAISE:TB:TEST:FAILED TB%s:%s %s\n"%(self.TB, self.get_directory_name(), ErrorMessage))

        return internalFailed

    def _answer(self):
        name = self.get_directory_name()
        if self.failed:
            self.client.send(self.psiSubscription,':STAT:TB%s! %s:failed\n'%(self.TB,name))
            self.Logger.warning(':Test %s failed in TB%s'%(name,self.TB))
            self.client.send(self.psiSubscription,':STAT:TB%s! %s:failed\n'%(self.TB,name))
        elif self.busy:
            self.client.send(self.psiSubscription,':STAT:TB%s! %s:busy\n'%(self.TB,name))
            #self.Logger << ':Test %s busy in TB%s'%(name,self.TB)
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
        if self.version == 'pyxar':
            executestr='%s --dir %s --nogui < %s'%(self.psiVersion,dir,whichTest)
        elif self.version == 'pxar':
            # cat test | ../bin/pXar -d whereever
            #executestr = 'cat {testfile} | {psiVersion} -dir {dir}  -r {rootfilename}.root -log {logfilename}.log'.format(testfile = whichTest, psiVersion = self.psiVersion, dir = dir, rootfilename = fname, logfilename = fname)
            logIDString = 'TB%s'%self.TB
            trimParameters = ''
            if self.trimVcal >= 0:
                trimParameters = '-T %i'%self.trimVcal
            executestr = 'cat %(testfile)s | %(psiVersion)s -d %(dir)s %(trim)s -r %(rootfilename)s.root -L %(logIDString)s'%{'testfile' : whichTest, 'psiVersion' : self.psiVersion, 'dir' : dir, 'rootfilename' : fname, 'trim' : trimParameters, 'logIDString' : logIDString} 
        else:
            executestr='%s -dir %s -f %s -r %s.root -log %s.log'%(self.psiVersion,dir,whichTest,fname,fname)
        self.LogFile = "%s/%s.log"%(dir, fname)
        self.RootFile = "%s/%s.root"%(dir, fname)
        self._spawn(executestr)
        self.failed=self._readout()
        self._answer()

    def openTB(self,dir,fname,poff=False):
        self._resetVariables()
        self.dir = dir
        self.Logger << 'open TB%s'%(self.TB)
        if self.version == 'pyxar':
            executestr='%s --dir %s --nogui'%(self.psiVersion,dir)
        elif self.version == 'pxar':
            # cat test | ../bin/pXar -d whereever
            trimParameters = ''
            if self.trimVcal >= 0:
                trimParameters = '-T %i'%self.trimVcal
            executestr = '%(psiVersion)s -d %(dir)s %(trim)s -r %(rootfilename)s.root'%{'psiVersion' : self.psiVersion, 'dir' : dir, 'rootfilename' : fname, 'trim' : trimParameters}
        else:
            executestr='%s -dir %s -r %s.root -log %s.log'%(self.psiVersion,dir,fname,fname)
            if poff:
                executestr+=' -t poff -i'
        self.Logger << 'exec string  = %s'%executestr
        self._spawn(executestr)
        self.failed=self._readout()
        self.Logger << 'done, failed = %s'%self.failed
        self._answer()
        while not self.ClosePSI:
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

    def sendTBStatus(self):
        self._answer()

