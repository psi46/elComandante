import subprocess
from threading import Thread
from printcolor import printc
from Queue import Queue

color=['green','red','blue','magenta']

def readout(proc,number):
    isalive = True
    while proc.poll() is None:
        line = proc.stdout.readline()
        printc(color[number],'',"PSI%s stdout: %s"%(number,line.rstrip()))
        if 'error' in line.rstrip() or 'Error' in line.rstrip():
            isalive=False
            print "--> PSI%s KILLED"%number
            proc.kill()
    return isalive
    

def Test(executestr,number):
    alive=True
    p = subprocess.Popen([executestr,''], shell=True, stdout=subprocess.PIPE)
    print '--> PSI%s STARTED'%number
    alive=readout(p,number)
        #p.wait()
    if alive:
        print '--> PSI%s TERMINATED'%number
    else:
        print '--> PSI%s FAILED'%number
        #psi_alive[number]=alive
    return
    #psi_alive[number]=alive


#global psi_alive
#psi_alive = [True]*4

psi1 = Thread(target=Test, args=('psi46expert -dir singleRocTest_TB1/ -f adc',1,))
psi1.start()

psi2 = Thread(target=Test, args=('psi46expert -dir singleRocTest_TB2/ -f adc',2,))
psi2.start()



#success = reduce(lambda x,y: x and y, psi_alive)
#print 'Job successfull: %s\n'%success
