#!/usr/bin/env python
from time import strftime,time, gmtime
import logging
class printer:
    def __init__(self):

        self.set_prefix('    |  ')
        self.set_color('')
        self.verbosity=1
        self.loglevel=1
        self.f = None
        self.timestamp = time()
        self.showOutput = True
        self.logger1 = None
        self.logFileHandler = None
        self.name = 'none'
    def setName(self,name):
        self.name = name
    def SetShowOutput(self):
        self.showOutput = True
    def UnsetShowOutput(self):
        self.showOutput = False
    def DisablePrint(self):
        self.UnsetShowOutput()
        
    def __lshift__(self,*arg):
        x = ' '.join(str(i) for i in arg)
        if (self.showOutput):
            print self.prefix + self.A + x + self.B
        self.logToFile(x)

    def printcolor(self,x,color=''):
        A='\033[1;3%sm'%(self.identifyer(color))
        B='\033[1;m'
        if (self.showOutput):
            print self.prefix + A + x + B
        self.logToFile(x)

    def warning(self,x):
        A='\033[1;31m'
        B='\033[1;m'
        if (self.showOutput):
            print self.prefix + A + x + B
        self.warningToFile(x)

    def warningToFile(self,log):
        if self.logger1 and self.loglevel >0:
            self.logger1.warning(log)
            self.logFileHandler.flush()

    def logToFile(self,log):
        if self.logger1 and self.loglevel > 0:
            self.logger1.info(log)
            self.logFileHandler.flush()

    def identifyer(self,color):
        if color == 'black': return 0
        elif color == 'red': return 1
        elif color == 'green': return 2
        elif color == 'yellow': return 3
        elif color == 'blue': return 4
        elif color == 'magenta': return 5
        elif color == 'cyan': return 6
        elif color == 'white': return 7

    def set_color(self,color):
        if not color == '':
            self.A='\033[1;3%sm'%(self.identifyer(color))
            self.B='\033[1;m'
        else:
            self.A=''
            self.B=''

    def set_prefix(self,prefix):
        self.prefix=str(prefix)

    def set_verbosity(self,verbosity):
        self.verbosity=verbosity

    def set_loglevel(self,loglevel):
        self.loglevel=loglevel

    def set_logfile(self,path):
        print 'Set Logfile to "%s"'%path
        self.logger1 = logging.getLogger('log%s'%self.name)
        self.logFileHandler = logging.FileHandler(path)
        self.logger1.addHandler(self.logFileHandler)
        self.logger1.setLevel(logging.INFO) 
        #self.f = open(path,'append')
        x = '#--------LOG from %s ---------\n'%strftime("%a %d %b %Y at %Hh:%Mm:%Ss",gmtime(self.timestamp))
        self.logToFile(x)
        #if self.f and self.loglevel > 0: self.f.write(x+'\n')
        #self.f.write()

    def __del__(self):
        if (self.showOutput):
            print '    |'
        if (self.showOutput):
            print '----+-----------------------------------------------------------------------'
        if (self.showOutput):
            print '    |'
        if self.f:
            self.f.write('#---------------------------------------------------------\n\n')
            self.f.close()
    def printv(self):
        if (self.showOutput):
            print '    |'
        if (self.showOutput):
            print '----+-----------------------------------------------------------------------'
        if (self.showOutput):
            print '    |'
        if self.f and self.loglevel > 0: self.f.write('----------------\n')
    def printn(self):
        if (self.showOutput):
            print '    |'
        if self.f and self.loglevel > 0: self.f.write('\n')
    def printw(self):
        if (self.showOutput):
            print '    |'
        if (self.showOutput):
            print '----+-----------------------------------------------------------------------'
        if (self.showOutput):
            print '    |'
        if (self.showOutput):
            print '    | \033[1;34mdBBBBBBBBBBBBBBBBBBP  dBP\033[1;m'
        if (self.showOutput):
            print '    |\033[1;34mdBP       dBP    dBP  dBP\033[1;m'
        if (self.showOutput):
            print '    \033[1;34mdBBBBP    dBP    dBBBBBBP\033[1;m'
        if (self.showOutput):
            print '   \033[1;34mdBP       dBP    dBP  dBP\033[1;m'
        if (self.showOutput):
            print '  \033[1;34mdBBBBBP   dBP    dBP  dBP\033[1;m  \033[1;30mSwiss Federal Institute of Technology Zurich\033[1;m'
        if (self.showOutput):
            print '    |'
        if (self.showOutput):
            print '----+-----------------------------------------------------------------------'
        if (self.showOutput):
            print '    |'
        if (self.showOutput):
            print '    |  \033[1;30mEL COMANDANTE\033[1;m - CMS Pixel Detector Module Testing Software'
        if (self.showOutput):
            print '    |  \033[1;30mElComandante:\033[1;m An \033[1;30mEL\033[1;maborate \033[1;30mC\033[1;momputer \033[1;30mO\033[1;mperated \033[1;30mM\033[1;modular \033[1;30mA\033[1;mccessible' 
        if (self.showOutput):
            print '    |                   \033[1;30mN\033[1;mested \033[1;30mD\033[1;mata \033[1;30mA\033[1;mggregation \033[1;30mN\033[1;metwork \033[1;30mT\033[1;mesting \033[1;30mE\033[1;mnvironment'
        if (self.showOutput):
            print '    |  Developped at ETHZ in 2012'
        if (self.showOutput):
            print '    |  Felix Bachmair & Philipp Eller'
        if (self.showOutput):
            print '    |'
        if (self.showOutput):
            print '----+-----------------------------------------------------------------------'
        if (self.showOutput):
            print '    |'



# ---example---
#
#Logger = printer()
#Logger.printw()
#Logger.set_logfile('test.txt')
#Logger << 'hello'+' Philipp '+ 'Eller'
#Logger.printcolor('I am green','green')
#Logger.printcolor('I am blue','blue')
#Logger << 'we make a horizontal line now'
#Logger.printv()
#Logger.warning('ouch! this is how a warning message looks like')
#Logger << 'plain text'
#Logger << 'plain text'
#Logger << 'plain text'
#Logger << 'plain text'
