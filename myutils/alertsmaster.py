#!/usr/bin/env python
import sys
sys.path.insert(1, "../")
from myutils import sClient, printer, decode, BetterConfigParser, TBmaster
from myutils import colorgenerator as colorGenerator
from threading import Thread
import subprocess
import argparse
import signal
import select
from time import sleep

#------------some configuration--------------
parser = argparse.ArgumentParser()

parser.add_argument("-c", "--config", dest="configDir",
                       help="specify directory containing config files e.g. ../config/",
                       default="../config/")
parser.add_argument("-dir","--directory", dest="loggingDir",
                        help="specify directory containing all logging files e.g. ../DATA/logfiles/",
                        default="../DATA/logfiles")
#parse args and setup logdir
args = parser.parse_args()
Logger = printer()
Logger.set_logfile('%s'%(args.loggingDir), 'alerts.log')
configDir= args.configDir

#load config
alerts = BetterConfigParser()
alerts.read(configDir+'/alerts.conf')
config = BetterConfigParser()
config.read(configDir+'/elComandante.conf')

#config
clientName = "alerts"
serverZiel=config.get('subsystem','Ziel')
Port = int(config.get('subsystem','Port'))
serverPort = int(config.get('subsystem','Port'))
alertsSubscription = "/alerts"
errorSubscription = "/error"

#construct
client = sClient(serverZiel,serverPort,clientName)

#subscribe
client.subscribe(alertsSubscription)

End = False

Logger << "alert master!"

#----------------------------------------------------

#handler
def handler(signum, frame):
    Logger << 'Close Connection'
    client.closeConnection()
    #Logger << 'Signal handler called with signal', signum

def RaiseException(exception, msg):
    Logger << "\x1b[31m%s\x1b[0m:\x1b[32m%s\x1b[0m"%(exception, msg)

    NotificationTargets = []

    exceptionParts = exception.split('.')
    for i in range(0, len(exceptionParts)):
        exceptionName = '.'.join(exceptionParts[0:i+1])
        print "test %s"%exceptionName

        Notify = []
        try:
            Notify = [x.strip() for x in alerts.get(exceptionName, 'Notify').split(',')]
        except:
            pass

        NotificationTargets.extend(Notify)

    # delete duplicates
    NotificationTargets = list(set(NotificationTargets))

    print "notify: %s"%repr(NotificationTargets)



def analysePacket(packet):
    time,coms,typ,msg,cmd = decode(packet.data)
    coms = [x.lower() for x in coms]

    if len(coms) >= 2:
        if coms[0].startswith('raise'):
            AlertName = '.'.join([x.upper() for x in coms[1:]])
            RaiseException(AlertName, msg)
            return
    if len(coms) > 0 and coms[0].startswith('exit'):
        print "exiting..."
        End = True
        return
            
    Logger << 'unknown command: %s, %s'%(coms, msg) 

#RECEIVE COMMANDS (mainloop)

signal.signal(signal.SIGINT, handler)
counter = 0 
while client.anzahl_threads > 0 and not End:
    packet = client.getFirstPacket(alertsSubscription)
    if packet.isEmpty():
        sleep(1)
        continue
    if not "pong" in packet.data.lower():
        analysePacket(packet)
        counter += 1
    else:
        sleep(1)

client.send(alertsSubscription,':prog:stat! exit\n')    
print 'exiting...'
client.closeConnection()
