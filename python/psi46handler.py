from sclient import *
from decode import *
from colorprinter import printer
#from printcolor import printc
from threading import Thread
import subprocess
from BetterConfigParser import BetterConfigParser
import sys
import argparse
#setup client /psi46
Logger = printer()
Logger.set_prefix('')
#------------some configuration--------------
parser = argparse.ArgumentParser()

parser.add_argument("-c", "--config", dest="configDir",
                       help="specify directory containing config files e.g. ../config/",
                       default="../config/")
parser.add_argument("-dir","--directory", dest="loggingDir",
                        help="specify directory containing all logging files e.g. ../DATA/logfiles/",
                        default="../DATA/logfiles")

args = parser.parse_args()
Logger.set_logfile('%s/psi46Handler.log'%(args.loggingDir))
Logger <<'ConfigDir: "%s"'%args.configDir
configDir= args.configDir

#load config
config = BetterConfigParser()
config.read(configDir+'/elComandante.conf')
#config
serverZiel=config.get('subsystem','Ziel')
Port = int(config.get('subsystem','Port'))
serverPort = int(config.get('subsystem','serverPort'))
psiSubscription = config.get('subsystem','psiSubscription')
#construct
client = sClient(serverZiel,serverPort,"psi46")
#subscribe
client.subscribe(psiSubscription)
#handler
def handler(signum, frame):
    Logger << 'Close Connection'
    client.closeConnection()
    Logger << 'Signal handler called with signal', signum
signal.signal(signal.SIGINT, handler)

color=['green','red','blue','magenta']






#def readout(proc,TB,client):
#
#
#    #---non blocking RO---
#    try:
#        from Queue import Queue, Empty
#    except ImportError:
#        from queue import Queue, Empty  # python 3.x
#    
#    
#    def enqueue_output(out, queue):
#        for line in iter(out.readline, b''):
#            queue.put(line)
#        out.close()
#    
#    q = Queue()
#    t = Thread(target=enqueue_output, args=(proc.stdout, q))
#    t.daemon = True # thread dies with the program
#    t.start()
#    #------------------------
#
#
#    failed = False
#    Logger << 'HERE i am\n'
#    while proc.poll() is None and not ClosePSI[TB]:
#        if Abort[TB]:
#            Logger << 'ABORT!'
#            proc.kill()
#            Logger << "--> PSI%s KILLED"%TB
#            failed=True
#            Abort[TB]=False
#        #if ClosePSI[TB]:
#	#    Logger << 'communicate!'
#        #    proc.communicate(input='exit\n')[0]
#        #    TestEnd[TB]=True
#
#	try:  line = q.get_nowait() # or q.get(timeout=.1)
#	except Empty:
#	    pass
#	    #print('no output yet')
#	else: # got line
#    # ... do something with line
#
#
#            line = proc.stdout.readline()
#            hesays=line.rstrip()
#            client.send('/TB%s'%TB,hesays+'\n')        
#            printc(color[TB],'',"PSI%s stdout: %s"%(TB,hesays))
#            if 'error' in line.rstrip() or 'Error' in line.rstrip():
#                failed=True
#                Logger << "--> PSI%s KILLED"%TB
#                proc.kill()
#            if 'command not found' in line.strip():
#                Logger << "--> psi46expert for TB%s not found"%TB
#            if Abort[TB]:
#                Logger << 'ABORT!'
#                proc.kill()
#                Logger << "--> PSI%s KILLED"%TB
#                failed=True            
#                Abort[TB]=False
#    return failed
#trying with select:

def readAllSoFar(proc, retVal=''): 
  while (select.select([proc.stdout],[],[],0)[0]!=[]) and proc.poll() is None:   
    retVal+=proc.stdout.read(1)
  return retVal

def readout(proc,TB,client):
    failed = False
    Logger << 'HERE i am\n'
    while proc.poll() is None and ClosePSI[TB]==False:
        if Abort[TB]:
            Logger.warning('ABORT!')
            try:
                proc.kill()
            except:
                pass
            Logger.warning("--> PSI%s KILLED"%TB)
            failed=True
            Abort[TB]=False
        #if ClosePSI[TB]:
	#    Logger << 'communicate!'
        #    proc.communicate(input='exit\n')[0]
        #    TestEnd[TB]=True
	lines = ['']
	lines = readAllSoFar(proc, lines[-1]).split('\n')
	#Logger << lines
	for a in range(len(lines)-1):
	    line=lines[a]
	    #if not line == '':
	    #    Logger << line
            #line = proc.stdout.readline()
            hesays=line.rstrip()
            client.send('/TB%s'%TB,hesays+'\n')        
            Logger.printcolor("PSI%s stdout: %s"%(TB,hesays),color[TB])
            if 'error' in line.rstrip() or 'Error' in line.rstrip():
            	failed=True
            	Logger.warning("--> PSI%s KILLED"%TB)
                try:
            	    proc.kill()
                except:
                    pass
            if 'command not found' in line.strip():
            	Logger.warning("--> psi46expert for TB%s not found"%TB)
            if Abort[TB]:
            	Logger.warning('ABORT!')
                try:
            	    proc.kill()
                except:
                    pass
            	Logger.warning("--> PSI%s KILLED"%TB)
            	failed=True            
            	Abort[TB]=False
    Logger << 'I am done'
    #proc.kill()
    return failed

#def readout(proc,TB,client):
#    failed = False
#    Logger << 'HERE i am\n'
#    while proc.poll() is None and not ClosePSI[TB]:
#        if Abort[TB]:
#            Logger << 'ABORT!'
#            proc.kill()
#            Logger << "--> PSI%s KILLED"%TB
#            failed=True
#            Abort[TB]=False
#        #if ClosePSI[TB]:
#	#    Logger << 'communicate!'
#        #    proc.communicate(input='exit\n')[0]
#        #    TestEnd[TB]=True
#
#        line = proc.stdout.readline()
#        hesays=line.rstrip()
#        client.send('/TB%s'%TB,hesays+'\n')        
#        printc(color[TB],'',"PSI%s stdout: %s"%(TB,hesays))
#        if 'error' in line.rstrip() or 'Error' in line.rstrip():
#            failed=True
#            Logger << "--> PSI%s KILLED"%TB
#            proc.kill()
#        if 'command not found' in line.strip():
#            Logger << "--> psi46expert for TB%s not found"%TB
#        if Abort[TB]:
#            Logger << 'ABORT!'
#            proc.kill()
#            Logger << "--> PSI%s KILLED"%TB
#            failed=True            
#            Abort[TB]=False
#    return failed


global busy
global failed
global TestEnd
global DoTest
global ClosePSI
global Abort
End=False
#MAINLOOP
numTB = 4
def initVariables(): 
    global busy
    global failed
    global TestEnd
    global DoTest
    global ClosePSI
    global Abort
    busy = [False]*numTB
    failed = [False]*numTB
    TestEnd = [False]*numTB
    DoTest=[False]*numTB
    ClosePSI=[False]*numTB
    #Abort.lock = threading.Lock()
    #Abort.lock.aquire()
    Abort=[False]*numTB
    #Abort.lock.release()



def executeTest(whichTest,dir,fname,TB,client):
    initVariables()
    Logger << 'psi46 %s in TB%s'%(whichTest,TB)
    failed[TB] = False
    TestEnd[TB] = False
    executestr='psi46expert -dir %s -f %s -r %s.root -log %s.log'%(dir,whichTest,fname,fname)
    proc = subprocess.Popen([executestr,''], shell=True, stdout=subprocess.PIPE)
    client.subscribe('/TB%s'%TB)
    failed[TB]=readout(proc,TB,client)
    Logger << 'done'
    TestEnd[TB] = True
    busy[TB] = False
    if failed[TB]:
        client.send(psiSubscription,':STAT:TB%s! test:failed\n'%TB)
    else:
        client.send(psiSubscription,':STAT:TB%s! test:finished\n'%TB)

def openTB(dir,fname,TB,client):
    initVariables()
    #ON_POSIX = 'posix' in sys.builtin_module_names
    Logger << 'open TB%s'%(TB)
    failed[TB] = False
    TestEnd[TB] = False
    executestr='psi46expert -dir %s -r %s.root -log %s.log'%(dir,fname,fname)
    proc = subprocess.Popen([executestr,''], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    #p = subprocess.Popen(['myprogram.exe'], stdout=subprocess.PIPE, bufsize=1, close_fds=ON_POSIX)
    client.subscribe('/TB%s'%TB)
    failed[TB]=readout(proc,TB,client)
    Logger << 'done! oO'
    #TestEnd[TB] = True
    #busy[TB] = False
    while not ClosePSI[TB]:
        pass
    Logger << 'CLOSE TB %s HERE'%(TB)
    proc.communicate(input='exit\n')[0] 
    proc.poll()
    if (None == proc.returncode):
        try:
            proc.send_signal(signal.SIGINT)
        except:
            Logger<< 'Process already killed'

    if failed[TB]:
        client.send(psiSubscription,':STAT:TB%s! test:failed\n'%TB)
        Logger << ':STAT:TB%s! test:failed'%TB
    else:
        client.send(psiSubscription,':STAT:TB%s! test:finished\n'%TB)
        Logger << ':STAT:TB%s! test:finished'%TB




Logger << 'Hello\n'
initVariables()
#RECEIVE COMMANDS:
while client.anzahl_threads > 0 and not End:
    sleep(.5)
    packet = client.getFirstPacket(psiSubscription)
    if not packet.isEmpty() and not "pong" in packet.data.lower():
        time,coms,typ,msg,cmd = decode(packet.data)
 
        Logger << time,coms,typ,msg
        Logger << cmd
        if coms[0].find('PROG')==0 and coms[1].find('TB')==0 and coms[2].find('OPEN')==0 and typ == 'c':
	    Logger << msg
            dir,fname=msg.split(',')
            TB=int(coms[1][2])
            if not busy[TB]:
                #Logger << whichTest
                #if whichTest == 'IV':
                #Logger << 'IV test\n'
                DoTest[TB] = Thread(target=openTB, args=(dir,fname,TB,client,))
                DoTest[TB].start()

        elif coms[0].find('PROG')==0 and coms[1].find('TB')==0 and coms[2][0:5] == 'CLOSE' and typ == 'c':
            if len(coms[1])>=3:
		TB=int(coms[1][2])
	    	Logger << 'trying to close TB...'
            	ClosePSI[TB]=True



        elif coms[0].find('PROG')==0 and coms[1][0:2] == 'TB' and coms[2][0:5] == 'START' and typ == 'c':
            whichTest,dir,fname=msg.split(',')
            TB=int(coms[1][2])
            if not busy[TB]:
		Logger << whichTest
                #if whichTest == 'IV':
                #    Logger << 'IV test\n'
                #    DoTest[TB] = Thread(target=openTB, args=(dir,fname,TB,client,))
		#    DoTest[TB].start()

                
                Logger << '\t--> psi46 execute %s in TB%s'%(whichTest,TB)
                DoTest[TB] = Thread(target=executeTest, args=(whichTest,dir,fname,TB,client,))
                DoTest[TB].start()
                client.send(psiSubscription,':STAT:TB%s! %s:started\n'%(TB,whichTest))
                busy[TB]=True
            else:
                client.send(psiSubscription,':STAT:TB%s! busy\n'%TB)

        elif coms[0][0:4] == 'PROG' and coms[1][0:2] == 'TB' and coms[2][0:4] == 'KILL' and typ == 'c':
            TB=int(coms[1][2])
            if not DoTest[TB]:
                Logger << 'nothing to be killed!'
            else:
                failed[TB]=True
                busy[TB]=False
                Abort[TB]=True
                Logger << 'killing TB%s...'%TB

                
        elif coms[0].find('PROG')==0 and coms[1].find('EXIT')==0 and typ == 'c':
            Logger << 'exit'
            if not reduce(lambda x,y: x or y, busy):
                End = True
            else:
                for i in range(0,numTB):
                    if busy[i]: client.send(psiSubscription,':STAT:TB%s! busy\n'%i)
                

        elif coms[0][0:4] == 'STAT' and coms[1][0:2] == 'TB' and typ == 'q':
            TB=int(coms[1][2])
            if busy[TB]:
                client.send(psiSubscription,':STAT:TB%s! busy\n'%TB)
            elif failed[TB]:
                client.send(psiSubscription,':STAT:TB%s! test:failed\n'%TB)
            elif TestEnd[TB]:
                client.send(psiSubscription,':STAT:TB%s! test:finished\n'%TB)
            else:
                client.send(psiSubscription,':STAT:TB%s! status:unknown\n'%TB)
            

        else:
            Logger << 'unknown command: ', coms, msg
    else:
        pass
        #Logger << 'waiting for answer...\n'



for i in range(0,numTB):
    if failed[i]: client.send(psiSubscription,':STAT:TB%s! test:failed\n'%i)
    elif TestEnd[i]: client.send(psiSubscription,':STAT:TB%s! test:finished\n'%i)


client.send(psiSubscription,':prog:stat! exit\n')    
Logger << 'exiting...'
client.closeConnection()

#END
while client.anzahl_threads > 0: 
    Logger << 'waiting for client to be closed...'
    client.closeConnection()
    sleep(0.5)
    pass    		
Logger << 'ciao!'
