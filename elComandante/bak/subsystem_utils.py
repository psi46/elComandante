#!/usr/bin/python2

from sclient import *
import utils
import subprocess

serverZiel = None
Port = None
serverPort = None
coolingBoxSubscription = None
keithleySubscription = None
psiSubscription = None

client = None

def start_subserver(directory):
    # Check if the subsystem server is running. if not, START the subserver
    if os.system("ps -ef | grep -v grep | grep subserver"):
        os.system("cd %s && ./subserver" % directory)

    # Check again if the subsystem server is running
    if os.system("ps -ef | grep -v grep | grep subserver"):
        raise Exception("Could not start subserver");

def get_subsystem_settings(config):
    global serverZiel, Port, serverPort, coolingBoxSubscription, keithleySubscription, psiSubscription
    serverZiel = config.get("subsystem", "Ziel")
    Port = int(config.get("subsystem", "Port"))
    serverPort = int(config.get("subsystem", "serverPort"))
    coolingBoxSubscription = config.get("subsystem", "coolingBoxSubscription")
    keithleySubscription = config.get("subsystem", "keithleySubscription")
    psiSubscription = config.get("subsystem", "psiSubscription")

def create_subsystem_client():
    global client
    client = sClient(serverZiel, serverPort, "kuehlingboxcommander")

def make_subscriptions():
    subscriptionList = [keithleySubscription, coolingBoxSubscription, psiSubscription]
    for subscription in subscriptionList:
        client.subscribe(subscription)
    return subscriptionList

def check_clients_running(client_list):
    for clientName in client_list:
        if not os.system("ps aux |grep -v grep| grep -v vim|grep -v emacs|grep %s"%clientName):
            raise Exception("another %s is already running. Please Close client first"%clientName)

def start_clients(timestamp, config, Directories):
    psiChild = subprocess.Popen("xterm +sb -geometry 120x20+0+900 -fs 10 -fa 'Mono' -e python ../python/psi46handler.py ", shell=True,preexec_fn = preexec)
    jumoChild = subprocess.Popen("xterm +sb -geometry 80x25+1200+0 -fs 10 -fa 'Mono' -e '%s/jumoClient -d %s |tee %s/jumo.log'"%(Directories['jumoDir'],config.get("jumoClient","port"),Directories['logDir']), shell=True,preexec_fn = preexec)
    keithleyChild = subprocess.Popen("xterm +sb -geometry 80x25+1200+1300 -fs 10 -fa 'Mono' -e %s/keithleyClient.py -d %s -dir %s -ts %s"%(Directories['keithleyDir'],config.get("keithleyClient","port"),Directories['logDir'],timestamp), shell=True,preexec_fn = preexec)

def check_subscriptions(subscriptionList):
    for subscription in subscriptionList:
        if not client.checkSubscription(subscription):
            raise Exception("Cannot read from %s subscription"%subscription)
        else:
            utils.Logger << "%s is answering" % subscription

def preexec():
    # Don't forward Signals.
    os.setpgrp()

def exit_clients():
    client.send(psiSubscription,':prog:exit\n')
    utils.Logger << 'heating up coolingbox...'
    client.send(coolingBoxSubscription,':prog:heat\n')
    sleep(3.0)
    client.clearPackets(coolingBoxSubscription)
    client.send(coolingBoxSubscription,':prog:stat?\n')
    i = 0
    isWarm = False
    while client.anzahl_threads > 0 and not isWarm:
        sleep(.5)
        packet = client.getFirstPacket(coolingBoxSubscription)
        if not packet.isEmpty() and not "pong" in packet.data.lower():
            data = packet.data
            Time,coms,typ,msg = decode(data)[:4]
            if len(coms) > 1:
                if coms[0].find('PROG')>=0 and coms[1].find('STAT')>=0 and typ == 'a' and (msg.lower() == 'waiting'):
                    Logger << '\t--> Got information to be done at %s from packet @ %s'%(int(time.time()),Time)
                    Logger << '\t--> Cooling Box is heated up now.'
                    isWarm = True
                elif coms[0][0:4] == 'PROG' and coms[1][0:4] == 'STAT' and typ == 'a':
                    if not i%10:
                        Logger << '\t--> Jumo is in status %s'%(msg)
                    if not 'heating' in msg.lower() and not 'waiting' in msg.lower():
                        client.send(coolingBoxSubscription,':prog:heat\n')
                    i+=1
            else:
                pass
        else:
            client.send(coolingBoxSubscription,':prog:stat?\n')
            pass

    client.closeConnection()
    Logger << 'I am done for now!'

def wait_for_clients():
    while client.anzahl_threads > 0:
        pass
"""

#--------------LOOP over TESTS-----------------
#print testlist
    for item in testlist:
     #   print item
        sleep(1.0)
        if item == 'Cycle':
            doCycle()
#    elif item == 'IV':
#        if(item.find('@'))
#        doIVCurve()
        else:
            client.send(keithleySubscription,':OUTP ON\n')
            if item.find('@')>=0:
                whichtest, temp = item.split('@')
            else:
                whichtest = item
                temp =17.0
            Logger.printv()
            Logger << 'I do now the following Test:'
            Logger << '\t%s at %s degrees'%(whichtest, temp)

            stablizeTemperature(temp)
            if whichtest == 'IV':
                doIVCurve(temp)
            else:
                doPSI46Test(whichtest,temp)
        client.send(keithleySubscription,':OUTP OFF\n')

    sleep(1)
    utils.killChildren()
    sleep(1)
#-------------EXIT----------------
    Logger.printv()
    Logger << 'ciao!'
    del Logger
    try:
        os.stat(Directories['logDir'])
    except:
        raise Exception("Couldn't find logDir %s"%Directories['logDir'])
    utils.killChildren();

    for Testboard in Testboards:
            try:
                copytree(Directories['logDir'],Testboard.parentDir+'logfiles')
            except:
                raise
                #raise Exception('Could not copy Logfiles into testDirectory of Module %s\n%s ---> %s'%(Testboard.module,Directories['logDir'],Testboard.parentdir))

    rmtree(Directories['logDir'])

    #cleanup
    for Testboard in Testboards:
        try: rmtree(Testboard.parentDir+'/tmp/')
        except: pass
except:
    print 'kill Children'
    utils.killChildren()
    print 'DONE'
    raise
    sys.exit(0)"""
