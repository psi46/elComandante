import time
import utils
import subsystem_utils
import decode

def stablizeTemperature(temp, whichtest):
    #-------------set temp----------------
    stable = False
    utils.Logger << '\t Stablize CoolingBox Temperature @ %s degrees'%temp
    subsystem_utils.client.clearPackets(subsystem_utils.coolingBoxSubscription)
    subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:start\n')
    subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':PROG:TEMP %s\n'%temp)
    subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:stat?\n')
    #subsystem_utils.client.receiveThread()
    time.sleep(3.0)
    subsystem_utils.client.clearPackets(subsystem_utils.coolingBoxSubscription)
    subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:stat?\n')
    i = 0
    while subsystem_utils.client.anzahl_threads > 0 and not stable:
        time.sleep(.5)
        packet = subsystem_utils.client.getFirstPacket(subsystem_utils.coolingBoxSubscription)
        if not packet.isEmpty() and not "pong" in packet.data.lower():
            data = packet.data
            Time,coms,typ,msg = decode.decode(data)[:4]
            if len(coms) > 1:
                if coms[0].find('PROG')>=0 and coms[1].find('STAT')>=0 and typ == 'a' and (msg == 'stable' or msg =='STABLE'):
                    utils.Logger << '\t--> Got information to be stable at %s from packet @ %s'%(int(time.time()),Time)
                    utils.Logger << '\t--> Temp is stable now. I begin with the %s'%(whichtest)
                    stable = True
                elif coms[0][0:4] == 'PROG' and coms[1][0:4] == 'STAT' and typ == 'a':
                    if not i%10:
                        utils.Logger << '\t--> Jumo is in status %s'%(msg)
                    if 'waiting' in msg.lower():
                        subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:start\n')
                        subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':PROG:TEMP %s\n'%temp)
                    i+=1
            else:
                pass
        else:
            subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:stat?\n')
            pass

def doCycle():
        highCycleTemp = float(init.get('Cycle','highTemp'))
        lowCycleTemp = float(init.get('Cycle','lowTemp'))
        nCycles = int(init.get('Cycle','nCycles'))
        subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:cycle:highTemp %s\n'%highCycleTemp)
        subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:cycle:lowTemp %s\n'%lowCycleTemp)
        subsystem_utils.client.send(subsystem_utils.coolingBoxSubscription,':prog:cycle %s\n'%nCycles)
        utils.Logger << 'Temperature cycling with %s cycles between %s and %s'%(nCycles,lowCycleTemp,highCycleTemp)
        cycleDone = False
        while subsystem_utils.client.anzahl_threads >0 and not cycleDone:
            time.sleep(.5)
            packet = subsystem_utils.client.getFirstPacket(subsystem_utils.coolingBoxSubscription)
            if not packet.isEmpty():
                #DONE
                data = packet.data
                Time,coms,typ,msg = decode.decode(data)[:4]
                if len(coms) > 1:
                    if coms[0].find('PROG')>=0 and coms[1].find('CYCLE')>=0 and typ == 'a' and (msg == 'FINISHED'):
                        utils.Logger << '\t--> Cycle FINISHED'
                        cycleDone = True
                    else:
                        pass
                else:
                    pass
                pass
            else:
                pass

"""
#-----------cycle function-----------------------
#------------------------------------------------


    def setupParentDir(timestamp,Testboard):
            Testboard.parentDir=Directories['dataDir']+'/%s_%s_%s/'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",gmtime(timestamp)),timestamp)
            try:
                os.stat(Testboard.parentDir)
            except:
                os.mkdir(Testboard.parentDir)
            return Testboard.parentDir
#
    def doPSI46Test(whichtest,temp):
#-------------start test-----------------
        for Testboard in Testboards:
            #Setup Test Directory
            Testboard.timestamp=timestamp
            Testboard.currenttest=item
            Testboard.testdir=Testboard.parentDir+'/%s_%s_%s/'%(int(time.time()),Testboard.currenttest,temp)
            setupdir(Testboard)
            #Start PSI
            Testboard.busy=True
            #client.send(psiSubscription,':prog:TB1:start Pretest,~/supervisor/singleRocTest_TB1,commanderPretest')
            client.send(psiSubscription,':prog:TB%s:start %s,%s,commander_%s\n'%(Testboard.slot,Directories['testdefDir']+'/'+ whichtest,Testboard.testdir,whichtest))
            utils.Logger.printn()
            utils.Logger << 'psi46 at Testboard %s is now started'%Testboard.slot

        #wait for finishing
        busy = True
        while client.anzahl_threads > 0 and busy:
            time.sleep(.5)
            packet = client.getFirstPacket(psiSubscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
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

            packet = client.getFirstPacket(coolingBoxSubscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
                #nnprint "MESSAGE: %s %s %s %s "%(Time,typ,coms,msg.upper())
                if coms[0].find('STAT')==0 and typ == 'a' and 'ERROR' in msg[0].upper():
                    utils.Logger.warning('jumo has error!')
                    utils.Logger.warning('\t--> I will abort the tests...')
                    utils.Logger.printn()
                    for Testboard in Testboards:
                        client.send(psiSubscription,':prog:TB%s:kill\n'%Testboard.slot)
                        utils.Logger.warning('\t Killing psi46 at Testboard %s'%Testboard.slot)
                        index=[Testboard.slot==int(coms[1][2]) for Testboard in Testboards].index(True)
                        Testboard.failed()
                        Testboard.busy=False
            busy=reduce(lambda x,y: x or y, [Testboard.busy for Testboard in Testboards])
        #-------------test finished----------------


        #---------------Test summary--------------
        utils.Logger.printv()
        for Testboard in Testboards:
                client.send(psiSubscription,':stat:TB%s?\n'%Testboard.slot)
                received=False
                while client.anzahl_threads > 0 and not received:
                    sleep(.1)
                    packet = client.getFirstPacket(psiSubscription)
                    if not packet.isEmpty() and not "pong" in packet.data.lower():
                        data = packet.data
                        Time,coms,typ,msg = decode(data)[:4]
                        if coms[0][0:4] == 'STAT' and coms[1][0:3] == 'TB%s'%Testboard.slot and typ == 'a':
                            received=True
                            if msg == 'test:failed':
                                utils.Logger.warning('\tTest in Testboard %s failed! :('%Testboard.slot)
                                powercycle(Testboard)
                            elif msg == 'test:finished':
                                utils.Logger << '\tTest in Testboard %s successful! :)'%Testboard.slot
                            else:
                                utils.Logger << '%s %s %s %s @ %s'%(Time,coms,typ,msg,int(time.time()))
                                utils.Logger.printn()
                                utils.Logger.warning('\tStatus of Testboard %s unknown...! :/'%Testboard.slot)
                                powercycle(Testboard)
        utils.Logger.printv()
        #---------------iterate in Testloop--------------

#
#-----------IV function-----------------------
    def doIVCurve(temp):
        for Testboard in Testboards:
            Testboard.timestamp=timestamp
            Testboard.currenttest=item
            Testboard.testdir=Testboard.parentDir+'/%s_IV_%s'%(int(time.time()),temp)
            setupdir(Testboard)
            utils.Logger << 'DO IV CURVE for Testboard slot no %s'%Testboard.slot
            #%(Testboard.address,Testboard.module,Testboard.slot),Testboard
            ivStart = float(init.get('IV','Start'))
            ivStop  = float(init.get('IV','Stop'))
            ivStep  = float(init.get('IV','Step'))
            ivDelay = float(init.get('IV','Delay'))
            ivDone = False
            client.send(keithleySubscription,':PROG:IV:START %s'%ivStart)
            client.send(keithleySubscription,':PROG:IV:STOP %s'%ivStop)
            client.send(keithleySubscription,':PROG:IV:STEP %s'%ivStep)
            client.send(keithleySubscription,':PROG:IV:DELA Y%s'%ivDelay)
            client.send(keithleySubscription,':PROG:IV:TESTDIR %s'%Testboard.testdir)
#todo check if testdir exists...
            client.send(psiSubscription,':prog:TB%s:open %s,commander_%s\n'%(Testboard.slot,Testboard.testdir,whichtest))
            sleep(2.0)
            client.send(keithleySubscription,':PROG:IV MEAS\n')
            while client.anzahl_threads >0 and not ivDone:
                    sleep(.5)
                    packet = client.getFirstPacket(keithleySubscription)
                    if not packet.isEmpty() and not "pong" in packet.data.lower():
                        #DONE
                        data = packet.data
                        Time,coms,typ,msg,fullComand = decode(data)
                        if len(coms) > 1:
                            if coms[0].find('PROG')>=0 and coms[1].find('IV')>=0 and typ == 'a' and (msg == 'FINISHED'):
                                utils.Logger << '\t--> IV-Curve FINISHED'
                                ivDone = True
                            elif coms[0].find('IV')==0 and typ == 'q':
                                #print fullComand
                                pass
                        else:
                            pass
                        pass
                    else:
                        pass

        utils.Logger << 'try to close TB'
        client.send(psiSubscription,':prog:TB%s:close %s,commander_%s\n'%(Testboard.slot,Testboard.testdir,whichtest))
        sleep(5)
        powercycle(Testboard)

    def powercycle(Testboard):
        Testboard.timestamp=timestamp
        whichtest='powercycle'
        Testboard.testdir=Testboard.parentDir+'/tmp/'
        setupdir(Testboard)
        utils.Logger << 'Powercycle Testboard at slot no %s'%Testboard.slot
        Testboard.busy=True
        client.send(psiSubscription,':prog:TB%s:start %s,%s,commander_%s\n'%(Testboard.slot,Directories['testdefDir']+'/'+ whichtest,Testboard.testdir,whichtest))
        #wait for finishing
        busy = True
        while client.anzahl_threads > 0 and busy:
            sleep(.5)
            packet = client.getFirstPacket(psiSubscription)
            if not packet.isEmpty() and not "pong" in packet.data.lower():
                data = packet.data
                Time,coms,typ,msg = decode(data)[:4]
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


    def preexec():#Don't forward Signals.
        os.setpgrp()

    for clientName in ["jumoClient","psi46handler","keithleyClient"]:
        if not os.system("ps aux |grep -v grep| grep -v vim|grep -v emacs|grep %s"%clientName):
            raise Exception("another %s is already running. Please Close client first"%clientName)
#open psi46handler in annother terminal
    psiChild = subprocess.Popen("xterm +sb -geometry 120x20+0+900 -fs 10 -fa 'Mono' -e python psi46handler.py ", shell=True,preexec_fn = preexec)
#psiChild = subprocess.Popen("xterm +sb -geometry 160x20+0+00 -fs 10 -fa 'Mono' -e python psi46handler.py ", shell=True)


#open jumo handler
    jumoChild = subprocess.Popen("xterm +sb -geometry 80x25+1200+0 -fs 10 -fa 'Mono' -e '%s/jumoClient -d %s |tee %s/jumo.log'"%(Directories['jumoDir'],config.get("jumoClient","port"),Directories['logDir']), shell=True,preexec_fn = preexec)
#open Keithley handler
    keithleyChild = subprocess.Popen("xterm +sb -geometry 80x25+1200+1300 -fs 10 -fa 'Mono' -e %s/keithleyClient.py -d %s -dir %s -ts %s"%(Directories['keithleyDir'],config.get("keithleyClient","port"),Directories['logDir'],timestamp), shell=True,preexec_fn = preexec)
#check subscriptions?
    utils.Logger<<"Check Subscription of the Clients:"
    time.sleep(2)
    for subscription in subscriptionList:
        if not client.checkSubscription(subscription):
            raise Exception("Cannot read from %s subscription"%subscription)
        else:
            utils.Logger << "%s is answering"%subscription

#-------------SETUP TESTBOARDS----------------
    utils.Logger << 'I found the following Testboards with Modules:'
    utils.Logger.printn()
    Testboards=[]
    for tb, module in init.items('Modules'):
        if init.getboolean('TestboardUse',tb):
            Testboards.append(Testboarddefinition(int(tb[2]),module,config.get('TestboardAddress',tb),init.get('ModuleType',tb)))
            Testboards[-1].tests=testlist
            Testboards[-1].defparamdir=Directories['defaultDir']+'/'+config.get('defaultParameters',Testboards[-1].type)
            #print Testboards[-1].defparamdir
            utils.Logger << '\t- Testboard %s at address %s with Module %s'%(Testboards[-1].slot,Testboards[-1].address,Testboards[-1].module)
            parentDir=setupParentDir(timestamp,Testboards[-1])

            utils.Logger << 'try to powercycle Testboard...'
            powercycle(Testboards[-1])

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
    utils.killChildren()
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
