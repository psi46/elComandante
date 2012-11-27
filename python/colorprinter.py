#!/usr/bin/env python
from time import strftime,time, gmtime
class printer:
    def __init__(self):
        self.set_prefix('    |  ')
        self.set_color('')
        self.verbosity=1
        self.loglevel=1
        self.f = None
        self.timestamp = time()
    def __lshift__(self,x):
        print self.prefix + self.A + x + self.B
        if self.f and self.loglevel > 0: self.f.write(x+'\n')
    def printcolor(self,x,color=''):
        A='\033[1;3%sm'%(self.identifyer(color))
        B='\033[1;m'
        print self.prefix + A + x + B
        if self.f and self.loglevel > 0: self.f.write(x+'\n')
    def warning(self,x):
        A='\033[1;31m'
        B='\033[1;m'
        print self.prefix + A + x + B
        if self.f and self.loglevel > 0: self.f.write('!!! '+x+' !!!\n')
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
        self.f = open(path,'append')
        self.f.write('#--------LOG from %s ---------\n'%strftime("%a %d %b %Y at %Hh:%Mm:%Ss",gmtime(self.timestamp)))
    def __del__(self):
        print '    |'
        print '----+-----------------------------------------------------------------------'
        print '    |'
        if self.f:
            self.f.write('#---------------------------------------------------------\n\n')
            self.f.close()
    def printv(self):
        print '    |'
        print '----+-----------------------------------------------------------------------'
        print '    |'
        if self.f and self.loglevel > 0: self.f.write('----------------\n')
    def printn(self):
        print '    |'
        if self.f and self.loglevel > 0: self.f.write('\n')
    def printw(self):
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
        print '    |  Developped at ETHZ in 2012'
        print '    |  Felix Bachmair & Philipp Eller'
        print '    |'
        print '----+-----------------------------------------------------------------------'
        print '    |'



# ---example---

Logger = printer()
Logger.printw()
Logger.set_logfile('test.txt')
Logger << 'hello'
Logger.printcolor('I am green','green')
Logger.printcolor('I am blue','blue')
Logger << 'we make a horizontal line now'
Logger.printv()
Logger.warning('ouch! this is how a warning message looks like')
Logger << 'plain text'
Logger << 'plain text'
Logger << 'plain text'
Logger << 'plain text'
