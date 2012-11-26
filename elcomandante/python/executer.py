import subprocess
from threading import Thread
from printcolor import printc
from Queue import Queue

color=['green','red','blue','magenta']

def readout(proc, stopping,number):
    busy = True
    isalive = True
    while busy:
        line = proc.stdout.readline()
        printc(color[number],'',"PSI%s stdout: %s"%(number,line.rstrip()))
        if 'error' in line.rstrip() or 'Error' in line.rstrip():
            busy=False
            isalive=False
            print "--> PSI%s KILLED"%number
            proc.kill()
        if stopping in line.rstrip() or line.rstrip()=='':
            busy=False
    return isalive

def writein(proc,cmd):
	proc.stdin.write(cmd+'\n')
	proc.stdin.flush()



def doPretestRoc(executestr,number,tests,rocs):
    alive=True
    p = subprocess.Popen([executestr,''], shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    print '--> PSI%s STARTED'%number
    for i in range(0,rocs):
        alive=readout(p,'[Roc]',number)
    print '--> PSI%s CONFIGURED'%number
    
    
    for test in tests:
    
        if alive:
            print '--> PSI%s READY'%number
            writein(p,test)
            alive=readout(p,'[TestModule] %s: Start.'%test,number)
        else:
            print '--> PSI%s FAILED'%number
            tq.put(alive)
            return    
        if alive:
            print '--> PSI%s %s STARTED'%(number,test.upper())
            alive=readout(p,'[TestModule] %s: End.'%test,number)
        else:
            print '--> PSI%s FAILED'%number
            tq.put(alive)
            return
        if alive:            
            print '--> PSI%s %s END'%(number, test.upper())
        
        
        
    if alive:
        writein(p,'exit')
        p.wait()
        print '--> PSI%s TERMINATED'%number
    else:
        print '--> PSI%s FAILED'%number
        psi_alive[number]=alive
        return
    psi_alive[number]=alive


global psi_alive
psi_alive = [True]*4

psi1 = Thread(target=doPretestRoc, args=('psi46expert -dir singleRocTest_TB1/',1,['Pretest'],1,))
psi1.start()

psi2 = Thread(target=doPretestRoc, args=('psi46expert -dir singleRocTest_TB2/',2,['Pretest'],1,))
psi2.start()



#success = reduce(lambda x,y: x and y, psi_alive)
#print 'Job successfull: %s\n'%success
