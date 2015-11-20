#!/usr/bin/env python
import sys
import os
import re
sys.path.insert(1, "../")
from myutils import sClient, printer, decode, BetterConfigParser, TBmaster
from myutils import colorgenerator as colorGenerator
from threading import Thread
import subprocess
import argparse
import signal
import select
from time import sleep
from smtplib import SMTP
from email.MIMEText import MIMEText
import datetime
import shutil

#------------some configuration--------------
parser = argparse.ArgumentParser()

parser.add_argument("-c", "--config", dest="configDir",
                       help="specify directory containing config files e.g. ../config/",
                       default="../config/")
parser.add_argument("-dir","--directory", dest="loggingDir",
                        help="specify directory containing all logging files e.g. ../DATA/logfiles/",
                        default="../DATA/logfiles")
parser.add_argument("-n", "--name", dest="name",
                       help="qualification/setup/etc. name that is shown with each message",
                       default="../config/")
#parse args and setup logdir
args = parser.parse_args()
Logger = printer()
Logger.set_logfile('%s'%(args.loggingDir), 'alerts.log')
configDir= args.configDir

#load config
alerts = BetterConfigParser()
if not os.path.isfile(configDir+'/alerts.conf'):
    if os.path.isfile(configDir+'/alerts.conf.default'):
        shutil.copy(configDir+'/alerts.conf.default', configDir+'/alerts.conf')
    else:
        print "warning: no default alerts configuration file found!"
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

def RaiseException(exception, msg, msg_time=None):
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
    notify(NotificationTargets, exception, msg, msg_time=msg_time)

def sendMail(target, exception, msg, msg_time = None):

    # mail configuration
    SMTPserver = 'mail.phys.ethz.ch'
    sender =     'pixelproduction@phys.ethz.ch'
    destination = [target]
    USERNAME = "pixelproduction"
    PASSWORD = ""
    text_subtype = 'plain'

    # contents
    subject="elComandante notification %s"%exception
    content = 'Dear user,\n\nthis is an automatedly generated message. Do not reply!\n\n'

    try:
        content += "type: %s\n"%exception
    except:
        content += "type: unknown"

    try:
        content += "source: %s\n"%args.name
    except:
        pass

    try:
        content += "message: %s\n"%msg
    except:
        pass

    try:
        if msg_time:
            content += "time: %s\n"%datetime.datetime.fromtimestamp(int(msg_time)).strftime('%Y-%m-%d %H:%M:%S')
    except:
        pass

    content +="\n\nYours sincerely,\n-elComandante"

    try:
        msg = MIMEText(content, text_subtype)
        msg['Subject']= subject
        msg['From']   = sender # some SMTP servers will do this automatically, not all

        conn = SMTP(SMTPserver, 587)
        conn.set_debuglevel(False)
        try:
            conn.sendmail(sender, destination, msg.as_string())
        finally:
            conn.close()

    except Exception, exc:
        Logger << "mail failed; %s" % str(exc)

def notify(targets, exception, msg, msg_time=None):
    for target in targets:
        if '@' in target:
            sendMail(target=target, exception=exception, msg=msg, msg_time=msg_time)
        elif len(target) > 0 and target[0] == '+':
            print "not implemented"
            #sendSMS(target, exception, msg)

def analysePacket(packet):
    print "PD: %s"%packet.data
    time,coms,typ,msg,cmd = decode(packet.data)
    coms = [x.lower() for x in coms]

    if len(coms) >= 2:
        if coms[0].startswith('raise'):
            AlertName = '.'.join([x.upper() for x in coms[1:]])
            RaiseException(AlertName, msg, msg_time=time)
            return
    if len(coms) > 0 and coms[0].lower().startswith('exit'):
        client.send(alertsSubscription,':prog:stat! exit\n')    
        print "exiting..."
        End = True
        exit()
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
print 'exiting......'
exit()
