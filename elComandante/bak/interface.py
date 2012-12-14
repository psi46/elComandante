#!/usr/bin/python2

import os
import utils
import subsystem_utils
import decode
from testboardclass import Testboard as Testboarddefinition
from time import strftime, gmtime, sleep
from shutil import copytree,rmtree

def setupdir(Testboard):
    utils.Logger.printn()
    utils.Logger << 'I setup the directories:'
    utils.Logger << '\t- %s'%Testboard.testdir
    utils.Logger << '\t  with default Parameters from %s'%Testboard.defparamdir
    #copy directory
    try:
        copytree(Testboard.defparamdir, Testboard.testdir)
        f = open( '%s/configParameters.dat'%Testboard.testdir, 'r' )
        lines = f.readlines()
        f.close()
        lines[0]='testboardName %s'%Testboard.address
        f = open( '%s/configParameters.dat'%Testboard.testdir, 'w' )
        f.write(''.join(lines))
        f.close()
    except IOError as e:
        utils.Logger.warning("I/O error({0}): {1}".format(e.errno, e.strerror))
    except OSError as e:
        utils.Logger.warning("OS error({0}): {1}".format(e.errno, e.strerror))

def doPSI46Test(whichtest, temp, timestamp, Directories):
    #-------------start test-----------------
    for Testboard in Testboards:
        #Setup Test Directory
        Testboard.timestamp=timestamp
        Testboard.currenttest=item
        Testboard.testdir=Testboard.parentDir+'/%s_%s_%s/'%(int(time.time()),Testboard.currenttest,temp)
        setupdir(Testboard)
        #Start PSI
        Testboard.busy=True
        #subsystem_utils.client.send(psiSubscription,':prog:TB1:start Pretest,~/supervisor/singleRocTest_TB1,commanderPretest')
        subsystem_utils.client.send(psiSubscription,':prog:TB%s:start %s,%s,commander_%s\n'%(Testboard.slot,Directories['testdefDir']+'/'+ whichtest,Testboard.testdir,whichtest))
        utils.Logger.printn()
        utils.Logger << 'psi46 at Testboard %s is now started'%Testboard.slot

    #wait for finishing
    busy = True
    while subsystem_utils.client.anzahl_threads > 0 and busy:
        sleep(.5)
        packet = subsystem_utils.client.getFirstPacket(psiSubscription)
        if not packet.isEmpty() and not "pong" in packet.data.lower():
            data = packet.data
            Time,coms,typ,msg = decode.decode(data)[:4]
            if coms[0].find('STAT')==0 and coms[1].find('TB')==0 and typ == 'a' and msg=='test:finished':
                index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                #print Testboards[index].tests
                #print Testboards[index].currenttest
                Testboards[index].finished()
                Testboards[index].busy=False
            if coms[0][0:4] == 'STAT' and coms[1][0:2] == 'TB' and typ == 'a' and msg=='test:failed':
                index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                Testboards[index].failed()
                Testboards[index].busy=False

        packet = subsystem_utils.client.getFirstPacket(coolingBoxSubscription)
        if not packet.isEmpty() and not "pong" in packet.data.lower():
            data = packet.data
            Time,coms,typ,msg = decode.decode(data)[:4]
            #nnprint "MESSAGE: %s %s %s %s "%(Time,typ,coms,msg.upper())
            if coms[0].find('STAT')==0 and typ == 'a' and 'ERROR' in msg[0].upper():
                utils.Logger.warning('jumo has error!')
                utils.Logger.warning('\t--> I will abort the tests...')
                utils.Logger.printn()
                for Testboard in Testboards:
                    subsystem_utils.client.send(psiSubscription,':prog:TB%s:kill\n'%Testboard.slot)
                    utils.Logger.warning('\t Killing psi46 at Testboard %s'%Testboard.slot)
                    index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                    Testboard.failed()
                    Testboard.busy=False
        busy=reduce(lambda x,y: x or y, [Testboard.busy for Testboard in Testboards])
    #-------------test finished----------------


    #---------------Test summary--------------
    utils.Logger.printv()
    for Testboard in Testboards:
            subsystem_utils.client.send(psiSubscription,':stat:TB%s?\n'%Testboard.slot)
            received=False
            while subsystem_utils.client.anzahl_threads > 0 and not received:
                sleep(.1)
                packet = subsystem_utils.client.getFirstPacket(psiSubscription)
                if not packet.isEmpty() and not "pong" in packet.data.lower():
                    data = packet.data
                    Time,coms,typ,msg = decode.decode(data)[:4]
                    if coms[0][0:4] == 'STAT' and coms[1][0:3] == 'TB%s'%Testboard.slot and typ == 'a':
                        received=True
                        if msg == 'test:failed':
                            utils.Logger.warning('\tTest in Testboard %s failed! :('%Testboard.slot)
                            powercycle(Testboard, Testboards, timestamp, psiSubscription, Directories)
                        elif msg == 'test:finished':
                            utils.Logger << '\tTest in Testboard %s successful! :)'%Testboard.slot
                        else:
                            utils.Logger << '%s %s %s %s @ %s'%(Time,coms,typ,msg,int(time.time()))
                            utils.Logger.printn()
                            utils.Logger.warning('\tStatus of Testboard %s unknown...! :/'%Testboard.slot)
                            powercycle(Testboard, Testboards, psiSubscription, Directories)
    utils.Logger.printv()
    #---------------iterate in Testloop--------------

def powercycle(Testboard, Testboards, timestamp, psiSubscription, Directories):
    Testboard.timestamp=timestamp
    whichtest='powercycle'
    Testboard.testdir=Testboard.parentDir+'/tmp/'
    setupdir(Testboard)
    utils.Logger << 'Powercycle Testboard at slot no %s'%Testboard.slot
    Testboard.busy=True
    subsystem_utils.client.send(psiSubscription,':prog:TB%s:start %s,%s,commander_%s\n'%(Testboard.slot,Directories['testdefDir']+'/'+ whichtest,Testboard.testdir,whichtest))
    #wait for finishing
    busy = True
    while subsystem_utils.client.anzahl_threads > 0 and busy:
        sleep(.5)
        packet = subsystem_utils.client.getFirstPacket(psiSubscription)
        if not packet.isEmpty() and not "pong" in packet.data.lower():
            data = packet.data
            Time,coms,typ,msg = decode.decode(data)[:4]
            if coms[0].find('STAT')==0 and coms[1].find('TB')==0 and typ == 'a' and msg=='test:finished':
                index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                #Testboards[index].finished()
                Testboards[index].busy=False
                rmtree(Testboard.parentDir+'/tmp/')
            if coms[0][0:4] == 'STAT' and coms[1][0:2] == 'TB' and typ == 'a' and msg=='test:failed':
                index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                #Testboards[index].failed()
                Testboards[index].busy=False
                rmtree(Testboard.parentDir+'/tmp/')
                raise Exception('Could not open Testboard at %s.'%Testboard.slot)
            else:
                pass
        busy=Testboard.busy
    #-------------test finished----------------

def setupParentDir(timestamp, Testboard, Directories):
        Testboard.parentDir=Directories['dataDir']+'/%s_%s_%s/'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",gmtime(timestamp)),timestamp)
        try:
            os.stat(Testboard.parentDir)
        except:
            os.mkdir(Testboard.parentDir)
        return Testboard.parentDir

def get_testboard_list(timestamp, init, config, testlist, Directories, psiSubscription):
    utils.Logger << 'I found the following Testboards with Modules:'
    utils.Logger.printn()
    Testboards=[]
    for tb, module in init.items('Modules'):
        if init.getboolean('TestboardUse',tb):
            Testboards.append(Testboarddefinition(int(tb[2]),module,config.get('TestboardAddress',tb),init.get('ModuleType',tb)))
            Testboards[-1].tests=testlist
            Testboards[-1].defparamdir=Directories['defaultDir']+'/'+config.get('defaultParameters',Testboards[-1].type)
            utils.Logger << '\t- Testboard %s at address %s with Module %s'%(Testboards[-1].slot,Testboards[-1].address,Testboards[-1].module)
            parentDir=setupParentDir(timestamp, Testboards[-1], Directories)

            utils.Logger << 'try to powercycle Testboard...'
            powercycle(Testboards[-1], Testboards, timestamp, psiSubscription, Directories)
    return Testboards

def save_log_directories(directory, Testboards):
    for Testboard in Testboards:
        try:
            copytree(Directories['logDir'],Testboard.parentDir+'logfiles')
        except:
            raise

def remove_tmp_testboard_directories(Testboards):
    for Testboard in Testboards:
        try: rmtree(Testboard.parentDir+'/tmp/')
        except: pass
