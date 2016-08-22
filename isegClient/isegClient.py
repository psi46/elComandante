#!/usr/bin/python2
import sys
sys.path.insert(1, "../")
from myutils import sClient, decode, printer, is_float
#from threading import Thread
#import subprocess
import time
import argparse
import isegInterface
import ConfigParser
import os
#import serial
import signal
from time import sleep
from threading import Timer

ON = 1
OFF = 0
End = False

class isegConfigObject:
    def __init__(self):

        #
        self.serialPort = '/dev/iseg'
        self.testDir = '.'

        # sweep
        self.startValue = -100
        self.stopValue = -200
        self.stepValue = 15
        self.nSweeps = 1
        self.delay =.1
        self.maxSweepTries=1
        self.doingSweep = False

        # others
        self.do_leakage_current_measurement = False
        self.leakage_current_dir = '.'
        self.leakage_current_measurement_time = 60


LoggerName = 'IsegLog'
LoggerFileName = 'Iseg.log'

IvLoggerName = 'ivLogCh%d'
IvLoggerFileName = 'IVCh%d.log'

LeakageCurrentLoggerName = 'LeaakgeCurrentCh%d'
LeakageCurrentLoggerFileName = 'LeakageCurrentCh%d.log'


parser = argparse.ArgumentParser()

parser.add_argument("-d", "--device", dest="serialPort",
                       help="serial Port address e.g. /dev/ttyF0",
                       default='/dev/iseg')
parser.add_argument("-c", "--config", dest="configFile",
                       help="config file",
                       default='')
parser.add_argument("-dir","--directory", dest='dataDir',
                       help='directory where LogFilse is Stored',
                       default='.')
parser.add_argument('-ts','--timestamp', dest='timestamp',
                       help='Timestamp for creation of file',
                       default=0)
parser.add_argument('-iV','--immidiateVoltage', dest='immidiateVoltage',
                       help='Bias voltage',
                       default=-150)

args = parser.parse_args()

isegConfig = isegConfigObject()
isegConfig.serialPort = args.serialPort
isegConfig.leakage_current_dir = args.dataDir

try:
    os.stat(args.dataDir)
except:
    os.mkdir(args.dataDir)

#default testDir, should be set (by elComandante)  when doing IV curve
isegConfig.testDir = '%s'%(args.dataDir)

#Setup Logger 
Logger = printer()
Logger.set_name(LoggerName)
Logger.timestamp = float(args.timestamp)
Logger.set_logfile(args.dataDir, LoggerFileName)
Logger.set_prefix('')

#check serial connection
if not os.access(isegConfig.serialPort, os.R_OK):
    Logger.warning('serialPort \'%s\' is not accessible'%isegConfig.serialPort)
    sys.exit()
    raise SystemExit
Logger<<'SerialPort: %s'% isegConfig.serialPort

# handle CTRL+C
def handler(signum, frame):
    Logger << 'Close Connection'
    client.closeConnection()
    Logger << 'Signal handler called with signal %s'%signum
    try:
        iseg.set_output(False, 'all')
    except:
        pass
    if client.isClosed == True:
        Logger << 'client connection closed: kill all'
        End=True
        Logger << 'End: %s'%End
    
signal.signal(signal.SIGINT, handler)

# status output helper
def printStatus():
    global iseg
    activeChannels = iseg.get_list_of_active_channels()
    status = iseg.get_all_channel_status()
    hWidth = 24
    cWidth = 8

    if status and len(status) > 0:
        keys = status[0].keys()
        print "STATUS:"
        print " ".ljust(hWidth),
        for i in range(len(status)):
            if i in activeChannels:
                print ("%d"%i).ljust(cWidth),
            else:
                print ("(%d)"%i).ljust(cWidth),
        print ""

        for key in keys:
            print key.ljust(hWidth),
            for i in range(len(status)):
                if status[i][key]:
                    print "\x1b[32m%s\x1b[0m"%('true'.ljust(cWidth)),
                else:
                    print "\x1b[31m%s\x1b[0m"%('false'.ljust(cWidth)),

            print ""

# establish communication with subserver
clientName = 'isegClient'
serverZiel = '127.0.0.1'
serverPort = 12334
aboName = '/iseg'
IVAbo = '/iseg/IV'
voltageAbo = '/iseg/voltage'
currentAbo = '/iseg/current'
resistanceAbo='/iseg/resistance'
client = sClient(serverZiel, serverPort, clientName)
client.subscribe(aboName)
client.send(aboName,'Connecting {clientName} with Subsystem\n'.format(clientName=clientName))

conf = ConfigParser.ConfigParser()
conf.read(args.configFile)
iseg = isegInterface.ISEG(conf, 1, False)

iseg.set_output(False, 'all')
printStatus()
#Logger << 'status:%s'%iseg.get_all_channel_status()
activeChannels = iseg.get_list_of_active_channels()
Logger << 'active channels: %r'%activeChannels


#start leakage current vs time logger
LeakageCurrentLoggers = {}
for iCh in activeChannels:
    LeakageCurrentLogger = printer()
    LeakageCurrentLogger.set_name(LeakageCurrentLoggerName%iCh)
    LeakageCurrentLogger.set_logfile(args.dataDir, LeakageCurrentLoggerFileName%iCh)
    LeakageCurrentLogger.set_prefix('')
    LeakageCurrentLogger.timestamp = float(args.timestamp)
    LeakageCurrentLogger.disable_print()
    LeakageCurrentLoggers[iCh] = LeakageCurrentLogger

#start IV logger
IVLoggers = {}
for iCh in activeChannels:
    IVLogger = printer()
    IVLogger.set_name(IvLoggerName%iCh)
    IVLogger.set_logfile(args.dataDir,IvLoggerFileName%iCh)
    IVLogger.set_prefix('')
    IVLogger.timestamp = float(args.timestamp)
    IVLogger.disable_print()
    IVLoggers[iCh] = IVLogger


def readCurrentIV():
    global do_leakage_current_measurement
    global client
    global iseg
    global LeakageCurrentLoggers

    print 'measure IV'
    activeChannels = iseg.get_list_of_active_channels()

    if len(activeChannels) > 0:
        timestamp = time.time()
        currents = iseg.read_current(activeChannels)
        voltages = iseg.read_voltage(activeChannels)
        for i in range(len(activeChannels)):
            Logger << "Ch%d: %f V / %f A"%(activeChannels[i], voltages[i], currents[i])
            LeakageCurrentLoggers[activeChannels[i]] << '%s\t%s\t%s' % (voltages[i], currents[i], timestamp)

    '''
                #Logger << '%s: %s V - %s A'%(timestamp,data[0],data[1])

                IVLogger << '%s\t%s\t%s'%(voltage,current,timestamp)
                if do_leakage_current_measurement:
                    LeakageCurrentLogger << '%s\t%s\t%s' % (voltage, current, timestamp)
                #IVLogger << '%s V \t  %s A \t %s'%(data[0],data[1],timestamp)
                client.send(voltageAbo,'%s\n'%voltage)
                client.send(currentAbo,'%s\n'%current)
                client.send(resistanceAbo,'%s\n'%resistance)
        else:
            #Logger << ' could somehow not read data correctly: %s'%data
            pass
    else:
        pass
        #Logger << 'output is off'
    '''

def doLinearSweep(startValue, stopValue, stepValue, nSweeps = 1, delay = 5):
    global isegConfig
    global iseg
    global IVLoggers
    global Logger

    measurements = []

    startValueAbs = abs(startValue)
    stopValueAbs = abs(stopValue)
    stepValueAbs = abs(stepValue)
    if stopValueAbs < startValueAbs:
        stepValueAbs = -stepValueAbs

    # get list of active channels
    activeChannels = iseg.get_list_of_active_channels()
    channelString = iseg.get_channel_string(activeChannels)

    # start IV curve
    startTimestamp = time.time()

    for iSweep in range(nSweeps):
        voltageValue = startValueAbs
        while (startValueAbs <= stopValueAbs and voltageValue <= stopValueAbs) or (startValueAbs > stopValueAbs and voltageValue >= stopValueAbs):
            Logger << "Set Voltage: %d"%voltageValue
            for iCh in activeChannels:
                iseg.set_voltage(voltageValue, iCh)
            sleep(delay)

            timestamp = time.time()
            currents = iseg.read_current(activeChannels)
            voltages = iseg.read_voltage(activeChannels)

            for idxCh in range(len(currents)):
                iCh = activeChannels[idxCh]
                IVLoggers[iCh] << '%+8.3f\t%+11.4e\t%d'%(voltages[idxCh],currents[idxCh],timestamp)
                measurements.append([timestamp, voltages[idxCh], currents[idxCh], iCh])
            Logger << "Measured currents: %s"%(', '.join(["%1.4f"%x for x in currents]))
            voltageValue += stepValueAbs

    endTimestamp = time.time()
    Logger << "IV curve(s) took %d seconds"%(endTimestamp-startTimestamp) 
    return measurements


def sweep():
    global isegConfig
    global iseg
    global aboName
    global Logger

    # save status before sweep
    statusBeforeSweep = iseg.get_all_channel_status()
    isegConfig.doingSweep = True

    client.send(aboName,':MSG! Start with Linear Sweep from %sV to %sV in %sV steps\n'%(isegConfig.startValue, isegConfig.stopValue, isegConfig.stepValue))
    Logger << "TestDirectory is: %s"%isegConfig.testDir
    Logger << "Max Sweep Tries:  %d"%isegConfig.maxSweepTries
    ntries = 0
    while True:
        measurements = doLinearSweep(isegConfig.startValue, isegConfig.stopValue, isegConfig.stepValue, isegConfig.nSweeps, isegConfig.delay)
        ntries+=1
        if measurements or ntries>=isegConfig.maxSweepTries:
            Logger << 'exit while loop'
            break
        voltage = keithley.getLastVoltage()
        msg='Keithley Tripped %s of %s times @ %s\n'%(ntries,maxSweepTries,voltage)
        Logger << msg
        client.send(aboName,msg)
        client.send(IVAbo,msg) 
    Logger << "Done with recording IV Curve"
    client.clearPackets(aboName)
    sleep(1)

    # reset iseg
    Logger << 're-initialize Iseg'
    iseg.init_device(False)

    # restore status
    for iCh,chStatus in enumerate(statusBeforeSweep):
        iseg.set_output(chStatus['On'], iCh)


    isegConfig.doingSweep =  False
    client.send(aboName,':PROG:IV! FINISHED\n')


def finish_leakage_current_measurement():
    global client
    global Logger
    global isegConfig

    if isegConfig.do_leakage_current_measurement:
        client.send(aboName, ':PROG:LEAKAGECURRENT! FINISHED\n')
        Logger << "Finished Leakage Current Meaurement"
        isegConfig.do_leakage_current_measurement = False
    else:
        Logger << "Received Signal for Finished Leakagecurrent Measurement but no measurement active"

def initialise_leakage_current_measurement():
    pass

def exec_leakage_current_measurement():
    #check directory
    global Logger
    global isegConfig
    Logger << "Start Leakage Current Measurement for {mtime} s in directory '{dir}'".format(mtime=isegConfig.leakage_current_measurement_time,
                                                                                           dir=isegConfig.leakage_current_dir)
    outMsg = ":PROG:LEAKAGECURRENT:START! time: {mtime}s, dir: {dir}".format(mtime=isegConfig.leakage_current_measurement_time,
                                                                            dir=isegConfig.leakage_current_dir)
    Logger << outMsg
    outMsg += '\n'
    client.send(aboName, outMsg)
    isegConfig.do_leakage_current_measurement = True
    Timer(isegConfig.leakage_current_measurement_time, finish_leakage_current_measurement, ()).start()
    Logger << "leakage measurement running..."

def parse_leakage_current_testdir(typ, msg):
    global LeakageCurrentLoggers
    global LeakageCurrentLoggerFileName
    global isegConfig

    if typ == 'c':
        Logger << 'LeakageCurrentLogFileDir: "{msg}"'.format(msg=msg)
        new_dir = msg
        try:
            Logger << 'checked Directory {testDir}'.format(testDir=new_dir)
            os.stat(new_dir)
        except:
            outMsg = ':LEAKAGECURRENT:TESTDIR! %s: directory does not exist. Error!' % new_dir
        else:
            isegConfig.leakage_current_dir = new_dir
            outMsg = ':LEAKAGECURRENT:TESTDIR! %s' % isegConfig.leakage_current_dir

            activeChannels = iseg.get_list_of_active_channels()
            for iCh in activeChannels:
                LeakageCurrentLoggers[iCh].set_logfile(isegConfig.leakage_current_dir, LeakageCurrentLoggerFileName%iCh)
    outMsg += '\n'
    client.send(aboName, outMsg)
    pass

def parse_leakage_current_measurement_time(typ, msg):
    global isegConfig
    global Logger
    outMsg = ''

    if typ == 'c':
        Logger << "Setting Leakage Current measurement time {time}s".format(time=msg)
        if is_float(msg):
            new_time = float(msg)
            if new_time > 0:
                isegConfig.leakage_current_measurement_time = float(msg)
                Logger << "Leakage Current measurement time set to {time}s".format(time=isegConfig.leakage_current_measurement_time)
                outMsg = ':LEAKAGECURRENT:TIME! {time}'.format(time=isegConfig.leakage_current_measurement_time)
            else:
                Logger << "Invalid measurement time: {time}".format(time=new_time)
                outMsg = ':LEAKAGECURRENT:TIME! ERROR! Invalid time {time}'.format(time=new_time)
        else:
            Logger << 'Cannot convert leakage current measurement time "{msg}"'.format(msg=msg)
            outMsg = ':LEAKAGECURRENT:TIME! ERROR! Cannot convert "{msg}"'.format(msg, msg)
    else:
        outMsg = 'illegal type: %s'%typ

    outMsg += '\n'
    client.send(aboName, outMsg)
    pass

def analyse_leakage_current(coms, typ, msg):
    global Logger
    if coms[0].find('TIME') >= 0:
        parse_leakage_current_measurement_time(typ, msg)
    elif coms[0].find('TESTDIR') >= 0:
        parse_leakage_current_testdir(typ, msg)
    elif coms[0].find('START') >= 0:
        exec_leakage_current_measurement()

def printHelp():
    print "help menu..."

def  analyseIV(coms,typ,msg):
    global Logger
    global client
    global iseg
    global isegConfig

    if type(coms)==list:
        coms = [x.lower() for x in coms]
    elif type(coms) == str:
        coms = coms.lower()
#    Logger <<'analyse :IV'
    if len(coms)==0:
        if msg.lower().startswith('meas') and typ=='c':
            outMsg= ':MSG! Do Sweep from %.2f V to %.2f'%(isegConfig.startValue, isegConfig.stopValue)
            outMsg+=' in steps of %.2fV with a delay of %.f\n'%(isegConfig.stepValue, isegConfig.delay)
            outMsg+='\tTestDirectory is "%s"\n'%isegConfig.testDir
            Logger << outMsg
            client.send(aboName,outMsg)
            sweep()
        elif typ!='a':
            Logger << 'error'
            printHelp()
    elif len(coms)==1:
#        Logger << 'iv len >0'
        outMsg = 'not Valid Input'
        if coms[0].startswith('testdir'):
            if typ =='c':
#                Logger << '%s: "%s"'%(coms[0],msg)
                isegConfig.testDir = msg
                try:
                    os.stat(testDir)
#                    Logger << 'checked Directory'
                except:
                    outMsg = ':IV:TESTDIR! %s: directory does not exist. Error!'%isegConfig.testDir
                else:
                    outMsg = ':IV:TESTDIR! %s'%isegConfig.testDir

        if coms[0].startswith('start'):
            if typ =='c' and is_float(msg):
                isegConfig.startValue=float(msg)
#                Logger << 'prog-iv-start=%s'%msg
            elif typ =='q':
#                Logger << 'prog-iv-start?'
                pass
            outMsg = ':PROG:IV:START! %s'%isegConfig.startValue
            
        elif coms[0].startswith('stop'):
            if typ =='c'and is_float(msg):
                isegConfig.stopValue = float(msg)
#                Logger << 'prog-iv-stop=%s'%msg
            elif typ =='q':
#                Logger << 'prog-iv-stop?'
                pass
            outMsg = ':PROG:IV:STOP! %s'%isegConfig.stopValue
            
        elif coms[0].startswith('step'):
            if typ =='c'and is_float(msg):
                isegConfig.stepValue=float(msg)
#                Logger << 'prog-iv-step=%s'%msg
            elif typ =='q':
#                Logger << 'prog-iv-step?'
                pass
            outMsg = ':PROG:IV:STEP! %s'%isegConfig.stepValue
            
        elif coms[0].startswith('del'):           
            if typ =='c'and is_float(msg):
                isegConfig.delay=float(msg)
#                Logger << 'prog-iv-delay=%s'%msg
            elif typ =='q':
#                Logger << 'prog-iv-delay?'
                Logger
            outMsg = ':PROG:IV:DELAY! %s'%isegConfig.delay
            
        elif coms[0].startswith('maxtrips'):           
            if typ =='c' and is_float(msg):
                isegConfig.maxSweepTries = float(msg)
#                Logger << 'prog-iv-trip=%s'%msg
            elif typ =='q':
#                Logger << 'prog-iv-trip?'
                pass
            outMsg = ':PROG:IV:TRIP! %s'%isegConfig.maxSweepTries
            
        elif coms[0].startswith('stat') and typ == 'q':
            if isegConfig.doingSweep:
                outMsg = ':PROG:IV:STAT! busy'
            else:
                outMsg = ':PROG:IV:STAT! finished'
            pass
        Logger << outMsg
        outMsg+='\n'
        client.send(aboName,outMsg)
    elif typ != 'a':
#        Logger << 'error prog iv len to long'
        printHelp()
    pass

def analyseProg(coms,typ,msg):
    global Logger
    global client
    global iseg

    if coms[0].find('IV')>=0:
        analyseIV(coms[1:],typ,msg)
    elif coms[0].find('LEAKAGECURRENT') >= 0:
        analyse_leakage_current(coms[1:], typ, msg)
    elif coms[0].find('EXIT')>=0 and typ =='c':
        client.closeConnection();
    else:
        printHelp()
    pass

def analyseOutp(coms,typ,msg):
    global Logger
    global client
    global iseg
    #Logger << 'analyse Output'
    if len(coms)>0 and  typ != 'a':
#        Logger << 'not valid command: %s %s %s '%(coms, typ, msg)
        printHelp()
    else:
        if typ=='q':
            Logger << 'Query for output status'
            activeChannels = iseg.get_list_of_active_channels()
            status = iseg.get_output_status(activeChannels)
            for iCh in range(len(activeChannels)):
                Logger << "CH%d: %r"%(activeChannels(iCh), status[iCh])

            outMsg = ':OUTP! '
            #outMsg+= 'ON' if status else 'OFF'
            #outMsg+='\n'

#            Logger << outMsg
            client.send(aboName,outMsg)
        elif typ=='c':
            if msg in ['1','ON','True']:
                activeChannels = iseg.get_list_of_active_channels()
                iseg.set_output(True, activeChannels)
            elif msg in ['0','OFF','False']:
                activeChannels = iseg.get_list_of_active_channels()
                iseg.set_output(False, activeChannels)
            elif  typ != 'a':
                Logger << 'message of :OUTP not valid: %s, valid messages are \'ON\',\'OFF\''%msg
                printHelp()
        elif  typ != 'a':
            Logger << 'this a non valid typ'
            printHelp()
    pass

def analysePacket(coms,typ,msg):
    global Logger
    global client
    global iseg
    if coms[0].find('PROG')>=0:
        if len(coms[1:])>0:
            analyseProg(coms[1:],typ,msg)
        elif typ != 'a':
            Logger << 'not valid packet: %s'%coms
            printHelp()
        pass
    elif coms[0].find('OUTP')>=0:
        analyseOutp(coms[1:],typ,msg)
        pass
    elif coms[0].find('HELP')>=0 and typ != 'a':
        printHelp()
    elif coms[0]=='K':
        command = ":".join(map(str, coms[1:]))+' '+msg
        Logger << 'send command to iseg: '
        iseg.write(command)
    elif coms[0].lower().startswith('exit') and typ != 'a':
        client.closeConnection()
    else:
        Logger << 'not Valid Packet %s'%coms
        

########################################################################################################################################################
#RECEIVE COMMANDS:

sleep(0.5)
counter = 0 
while client.anzahl_threads > 0 and End == False and client.isClosed == False: 
    
    packet = client.getFirstPacket(aboName)
    
    counter +=1
    if packet.isEmpty():
        sleep(.5)
    else:
        #Logger << 'got Packet: %s'%packet.Print()
        data = packet.data
        timeStamp,coms,typ,msg,command = decode(data)
        # 'T:',timeStamp, 'Comand:',command
        Logger << '%s: %s, %s, %s'%(timeStamp,len(coms),typ,msg)
        dataOut = '%s\n'%packet.Print()
        if command.find(':DOSWEEP')!=-1:
            sweep()
        if len(coms)>0:
            analysePacket(coms,typ,msg)
        else:
            iseg.write(command)
            client.send(aboName,dataOut)
        #string retVal = keithley.setOutput(ON)
            
    if counter%20 == 0:      
        if not isegConfig.doingSweep:
            readCurrentIV()
    pass   
        
        

client.send(aboName,':prog:stat! exit\n')    
Logger << 'exiting...'
client.closeConnection()

#END
while client.anzahl_threads > 0:
    sleep(1)
    pass            

