from time import strftime,time, localtime
import os
import logging
import logging.handlers
class printer:
    def __init__(self):

        #logging.basicConfig(level=logging.INFO)
        self.set_prefix('    |  ')
        self.set_color('')
        self.verbosity=1
        self.loglevel=1
        self.timestamp = time()
        self.showOutput = True
        #self.logger1 = None
        self.outputfile = None
        self.name = 'none'
        
    def __del__(self):
        #print "Closing Logger with name %s"%self.name
        if (self.showOutput):
            print self.prefix
        if (self.showOutput):
            print '----+-----------------------------------------------------------------------'
        if (self.showOutput):
            print '    |'
        self.close_logfiles()
            
            
    def set_name(self,name):
        self.name = name
    def get_name(self):
        return self.name
    def SetShowOutput(self):
        self.showOutput = True
    def UnsetShowOutput(self):
        self.showOutput = False
    def disable_print(self):
        self.UnsetShowOutput()
        
    def __lshift__(self,*arg):
        x = ' '.join(str(i) for i in arg)
        self._print(x)
        self.logToFile(x)
    
    def _print(self,output):
        if (self.showOutput):
            print self.prefix + self.A + output + self.B
        

    def printcolor(self,x,color=''):
        A='\033[1;3%sm'%(self.identifyer(color))
        B='\033[1;m'
        self._print(A + x + B)
        self.logToFile(x)

    def warning(self,x):
        A='\033[1;31m'
        B='\033[1;m'
        self._print(A + x + B)
        self.warningToFile(x)

    def warningToFile(self,log):
        #if self.logger1 and self.loglevel >0:
        #    self.logger1.warning(log)
        if self.outputfile and self.loglevel > 0 and not self.outputfile.closed:
            self.outputfile.write('WARNING: %s\n'%log)
            self.outputfile.flush()
        #self.flushHandlers()

    def logToFile(self,log):
        if self.outputfile and self.loglevel > 0 and not self.outputfile.closed:
            self.outputfile.write('%s\n'%log)
            self.outputfile.flush()
        #if self.logger1 and self.loglevel > 0 and len(self.logger1.handlers) > 0:
        #    self.logger1.info(log)
        #self.flushHandlers()

    def flushHandlers(self):
        return
        if self.logger1:
            for handler in self.logger1.handlers:
                handler.flush()

    @staticmethod
    def identifyer(color):
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

    def get_prefix(self):
        return self.prefix

    def set_verbosity(self,verbosity):
        self.verbosity=verbosity

    def set_loglevel(self,loglevel):
        self.loglevel=loglevel

    def set_logfile(self,path,fileName):
        path=path.rstrip('/')
        self._print('%s: Set Logfile to "%s/%s"'%(self.name,path,fileName))
        name = '%s/%s'%(path,fileName)
        self.check_path(path)
        if self.outputfile:
            if self.outputfile.name != name:
                self.outputfile.close()

        if not self.outputfile or self.outputfile.closed:
            self.outputfile = open(name,'w')
            x = '#\n#--------LOG from %s ---------\n#'%strftime("%a %d %b %Y at %Hh:%Mm:%Ss",localtime(self.timestamp))
            self.logToFile(x)

        #if not self.logger1:
        #    loggerName = 'log_%s'%self.name
        #    print 'getLogger: %s'%loggerName
        #    self.logger1 = logging.getLogger('log_%s'%self.name)
        
        #print 'get FileHander'
        #logFileHandler = logging.FileHandler('%s/%s'%(path,fileName))
        #logFileHandler.setLevel(logging.INFO)
        #print 'add File Handler'
        #self.logger1.addHandler(logFileHandler)
        #self.logger1.setLevel(logging.INFO) 

    def close_logfiles(self):
        if self.outputfile:
            self.outputfile.close()

        #if self.logger1:
        #    #print 'closing %s logfiles of loggger %s'%(len(self.logger1.handlers),self.logger1.name)
        #    for handler in self.logger1.handlers:
        #        self.logger1.removeHandler(handler)

        #        #print 'closing %s'%handler.baseFilename
        #        handler.flush()
        #        #handler.stream.close()
        #        handler.close()
        #        del handler
        #    #print 'closed logfiles of loggger %s'%self.logger1.name

    def check_path(self,path):
        path = os.path.abspath(path)
        try:
            os.stat(path)
        except:
            os.mkdir(path)
            
    def printv(self):
        if not self.showOutput:
            return
        self._print('')
        print '----+-----------------------------------------------------------------------'
        self._print('')
        if self.loglevel > 0:
            self.logToFile('----------------\n') 
   
    def printn(self):
        if (self.showOutput):
            print '    |'
        if self.loglevel > 0: 
            self.logToFile('')
    
    def printw(self):
        if not self.showOutput:
            return
        print '    |'
        print '----+-----------------------------------------------------------------------'
        print '    |'
        print '    | \033[1;34mdBBBBBBBBBBBBBBBBBBP  dBP\033[1;m'
        print '    |\033[1;34mdBP       dBP    dBP  dBP\033[1;m'
        print '    \033[1;34mdBBBBP    dBP    dBBBBBBP\033[1;m'
        print '   \033[1;34mdBP       dBP    dBP  dBP\033[1;m'
        print '  \033[1;34mdBBBBBP   dBP    dBP  dBP\033[1;m  \033[1;30mSwiss Federal Institute of Technology Zurich\033[1;m'
        print '    |'
        print '----+-----------------------------------------------------------------------'
        print '    |'
        print '    |  \033[1;30mEL COMANDANTE\033[1;m - CMS Pixel Detector Module Testing Software'
        print '    |  \033[1;30mElComandante:\033[1;m An \033[1;30mEL\033[1;maborate \033[1;30mC\033[1;momputer \033[1;30mO\033[1;mperated \033[1;30mM\033[1;modular \033[1;30mA\033[1;mccessible' 
        print '    |                   \033[1;30mN\033[1;mested \033[1;30mD\033[1;mata \033[1;30mA\033[1;mggregation \033[1;30mN\033[1;metwork \033[1;30mT\033[1;mesting \033[1;30mE\033[1;mnvironment'
        print '    |  Developped at ETHZ in 2012'
        print '    |  Felix Bachmair & Philipp Eller'
        print '    |'
        print '----+-----------------------------------------------------------------------'
        print '    |'



if __name__ == '__main__':
# ---example---
#
    Logger = printer()
    Logger.printw()

    Logger.set_logfile('.','test0.log')
    Logger << 'hallo0'
    Logger.close_logfiles()
    #print len(Logger.logger1.handlers)
    Logger<< 'Hallo1'
    Logger.set_logfile('..','test0.log')

    Logger<< 'Hallo2'
    Logger<< 'Hallo2.1'
    Logger.set_logfile('.','test2.log')

    Logger<< 'Hallo2.2'
    Logger<< 'Hallo2.3'
    Logger<< 'Hallo3'
    Logger.set_logfile('../..','test0.log')
    Logger<< 'Hallo2.4'
    Logger<< 'Hallo2.5'

    Logger<< 'Hallo4'
#Logger.printcolor('I am green','green')
#Logger.printcolor('I am blue','blue')
#Logger << 'we make a horizontal line now'
#Logger.close_logfiles()
#Logger.set_logfile('.','test3.txt')
#Logger.printv()
#Logger.warning('ouch! this is how a warning message looks like')
#Logger << 'plain text'
#Logger << 'plain text'
#Logger << 'plain text'
#Logger << 'plain text'
