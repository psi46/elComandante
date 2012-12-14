#!/usr/bin/env python

import os
import argparse
from BetterConfigParser import *
import utils

def check_config_dir_access(config_dir):
    configDir= config_dir
    try:
        os.access(configDir,os.R_OK)
    except:
        raise Exception('configDir \'%s\' is not accessible'%configDir)

def load_config(filename):
    config = BetterConfigParser()
    config.read(filename)
    return config

def load_init(filename):
    init = BetterConfigParser()
    init.read(filename)
    return init

def get_config_directories(config, configDir):
    Directories={}
    Directories['configDir'] = configDir
    Directories['baseDir'] = config.get('Directories', 'baseDir')
    Directories['testdefDir'] = config.get('Directories', 'testDefinitions')
    Directories['dataDir'] = config.get('Directories', 'dataDir')
    Directories['defaultDir'] = config.get('Directories', 'defaultParameters')
    Directories['subserverDir'] = config.get('Directories', 'subserverDir')
    Directories['keithleyDir'] = config.get('Directories', 'keithleyDir')
    Directories['jumoDir'] = config.get('Directories', 'jumoDir')
    Directories['logDir'] = Directories['dataDir'] + '/logfiles/'
    for dir in Directories:
        Directories[dir] = os.path.abspath(Directories[dir].replace("$configDir$", configDir))
    return Directories

def get_list_of_tests(init):
    testlist = init.get('Tests','Test')
    testlist = testlist.split(',')
    while '' in testlist:
            testlist.remove('')
    return testlist

def print_tests(testlist):
    utils.Logger.printv()
    utils.Logger << 'I found the following Tests to be executed:'
    utils.Logger.printn()
    for item in testlist:
        if item.find('@')>=0:
            whichtest, temp = item.split('@')
        else:
            whichtest = item
            temp = 17.0
        utils.Logger << '\t- %s at %s degrees'%(whichtest, temp)

#####################################################

"""
try:
    print 'check logdirectory'
    try:
        logFiles = (os.listdir(Directories['logDir']))
        nLogFiles = len(logFiles)
        print nLogFiles
    except:
        os.mkdir(Directories['logDir'])
        print 'mkdir'
    else:
        print nLogFiles
        if nLogFiles>0:
            answer = raw_input('Do you want to overwrite \'%s\'? [y]es or [n]o\n\t'%logFiles)
            if 'y' in answer.lower():
                rmtree(Directories['logDir'])
                os.mkdir(Directories['logDir'])
            else:
                raise Exception('LogDir is not empty. Please clean logDir: %s'%Directories['logDir'])

#------------------------------------------


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
            utils.Logger.printv()
            utils.Logger << 'I do now the following Test:'
            utils.Logger << '\t%s at %s degrees'%(whichtest, temp)

            stablizeTemperature(temp)
            if whichtest == 'IV':
                doIVCurve(temp)
            else:
                doPSI46Test(whichtest,temp)
        client.send(keithleySubscription,':OUTP OFF\n')

#-------------Heat up---------------
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
                    utils.Logger << '\t--> Got information to be done at %s from packet @ %s'%(int(time.time()),Time)
                    utils.Logger << '\t--> Cooling Box is heated up now.'
                    isWarm = True
                elif coms[0][0:4] == 'PROG' and coms[1][0:4] == 'STAT' and typ == 'a':
                    if not i%10:
                        utils.Logger << '\t--> Jumo is in status %s'%(msg)
                    if not 'heating' in msg.lower() and not 'waiting' in msg.lower():
                        client.send(coolingBoxSubscription,':prog:heat\n')
                    i+=1
            else:
                pass
        else:
            client.send(coolingBoxSubscription,':prog:stat?\n')
            pass

    client.closeConnection()
    utils.Logger << 'I am done for now!'

    sleep(1)
    killChildren()
    sleep(1)
#-------------EXIT----------------
    while client.anzahl_threads > 0:
        pass
    utils.Logger.printv()
    utils.Logger << 'ciao!'
    del utils.Logger
    try:
        os.stat(Directories['logDir'])
    except:
        raise Exception("Couldn't find logDir %s"%Directories['logDir'])
    killChildren();

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
    killChildren()
    print 'DONE'
    raise
    sys.exit(0)"""
