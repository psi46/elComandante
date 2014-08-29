#!/usr/bin/python2
import sys
sys.path.insert(1, "../")
from myutils import sClient, decode, printer, is_float
#from threading import Thread
#import subprocess
import time
import argparse
import jumo_coolingBox
import os
#import serial
import signal
from time import sleep
ON = 1
OFF = 0
End = False
Logger = printer()
Logger.set_name("CoolingBoxLog")
#sweep parameters TODO anpassen

defSerialPort = '/dev/ttyUSB0'
serialPort = defSerialPort

parser = argparse.ArgumentParser()

parser.add_argument("-d", "--device", dest="serialPort",
                       help="serial Port address e.g. /dev/ttyF0",
                       default=defSerialPort)

parser.add_argument("-dir","--directory", dest='dataDir',
                       help='directory where LogFilse is Stored',
                       default='.')

parser.add_argument('-ts','--timestamp', dest='timestamp',
                       help='Timestamp for creation of file',
                       default=0)
parser.add_argument('-iT','--immidiateTemperature', dest='immidiateTemperature',
                       help='immidiate Temperature',
                       default=-9999)

args = parser.parse_args()
serialPort= args.serialPort
try:
    os.stat(args.dataDir)
except:
    os.mkdir(args.dataDir)
#Setup Logger 
Logger.timestamp = float(args.timestamp)
Logger.set_logfile(args.dataDir,'CoolingBox.log')
Logger.set_prefix('')
#default testDir, should be set (by elComandante)  when doing IV curve
testDir = '%s'%(args.dataDir)
if not os.access(serialPort,os.R_OK):
    Logger.warning('serialPort \'%s\' is not accessible'%serialPort)
    sys.exit()
    raise SystemExit
Logger<<'SerialPort: %s'% serialPort


serverZiel = '127.0.0.1'
serverPort = 12334
aboName = '/jumo'
errorAbo = '/error'
tempAbo = '/temperature/jumo'
humAbo = '/humidity'
dewPointAbo = '/temperature/dewPoint'
client = sClient(serverZiel,serverPort,"coolingboxClient")
client.subscribe(aboName)
client.send(aboName,'Connecting coolingBox Client with Subsystem\n')
counter = 0
initializedJumo = False
while not initializedJumo  and counter < 10:
    try:
        jumo = jumo_coolingBox.jumo_coolingBox(serialPort)
        initializedJumo = True
    except Exception as e:
        counter += 1
        time.sleep(.5)
        print 'Could not initialize Jumo. Try again: %d/10, %s'%(counter,e)
if not initializedJumo:
    raise Exception('Cannot initalize Jumo after 10 tries')
jumo.set_setpoint(float(args.immidiateTemperature))

def handler(signum, frame):
    jumo.stop_controlling()
    Logger << 'Close Connection'
    client.closeConnection()
    Logger << 'Signal handler called with signal %s'%signum
    try:
        pass
        #keithley.setOutput(OFF)
    except:
        pass
    if client.isClosed == True:
        Logger << 'client connection closed: kill all'
        End=True
        Logger << 'End: %s'%End
    
signal.signal(signal.SIGINT, handler)

############################################################################
############################################################################
############################################################################

def printHelp():
    global client
    global Logger
    data = '\n************************************************************************\n'
    data +=  'This is the Help for the python coolingbox client, part of elComandante\n'
    data +=' You can use following SCPI like commands: \n'
    data +='\t:HELP                \tto show this Help\n'
    data +='\n'
    data +='\t:MEAS:TEMP?          \tgets the current temperature \n'
    data +='\t:MEAS:HUM?           \tgets the current relative humidity\n'
    data +='\t:MEAS:DEWPOINT?      \tgets the current dew point\n'
    data +='\n'
    data +='\t:PROG:STAT?          \tto show the status of the cooling box - stable/unstable/cycling\n'
    data +='\t:PROG:START (TEMP)   \tto start controlling the cooling box, optional: temperature to stabibilize to\n'
    data +='\t:PROG:STOP           \tto stop controlling the cooling box\n'
    data +='\t:PROG:SETPOINT TEMP  \tsets the temperature to stabilize to\n'
    data +='\n'
    data +='\t:PROG:CYCLE N        \tstarts cycling doing NCYCLES Cycles, optional arguments: tempLow, tempHigh\n'
    data +='\t:PROG:CYCLE:LOWTEMP  \tdefines low temperature for cycling\n'
    data +='\t:PROG:CYCLE:HIGHTEMP \tdefines high temperature for cycling\n'
    data += '************************************************************************\n'
    Logger << data
    client.sendData(aboName,data)
    

def analyseCycle(coms,typ,msg):
    if len(coms) == 0:
        if typ == 'c':
            msg = msg.split()
            if len(msg) == 1:
                nCycles = int(msg[0])
                jumo.do_cycle(nCycles)
                pass
            elif len(msg) == 3:
                nCycles = int(msg[0])
                lowTemp = float(msg[1])
                highTemp = float(msg[2])
                jumo.do_cycle(nCycles,tempLow = lowTemp,tempHigh = highTemp)
            pass
        elif typ == 'q':
            client.send(aboName,':PROG:CYCLE! %s\n'%jumo.doCycle)
    elif coms[0].startswith('HIGHTEMP'):
        if typ == 'q':
            client.send(aboName,':PROG:CYCLE:HIGHTEMP! %2.1f\n'%jumo.cycleHigh)
        elif typ == 'c':
            try:
                jumo.cycleHigh = float(msg)
            except:
                print 'cannot convert msg "%s" for cycle High Temp'%msg

    elif coms[0].startswith('LOWTEMP'):
        if typ =='q':
            client.send(aboName,':PROG:CYCLE:LOWTEMP! %2.1f\n'%jumo.cycleLow)
        elif typ =='c':
            try:
                jumo.cycleLow = float(msg)
            except:
                print 'cannot convert msg "%s" for cycle Low Temp'%msg


def analyseProg(coms,typ,msg):

    #print ' analyse PROG: ',coms,typ,msg
    if coms[0].startswith('START') and typ =='c':
        print 'start controlling %s'%msg
        if msg != 'unknown':
            try:
                temp = float(msg)
                jumo.set_setpoint(temp)
            except:
                print 'cannot convert msg to temp: "%s"'%msg
        jumo.start_controlling()
    elif coms[0].startswith('STOP') and typ =='c':
        print 'stop controlling'
        jumo.stop_controlling()
    elif coms[0].upper().startswith('FINAL_HEAT') and typ =='c':
        jumo.final_heating()
        pass

    elif coms[0].startswith('STAT') and typ == 'q':
        if not jumo.controlling or jumo.is_unkown():
            status = 'waiting'
        elif jumo.doCycle:
            if jumo.is_stable():
                client.send(aboName,':PROG:CYCLE! FINISHED\n')
                return
            status = 'CYCLING %s'%jumo.cycles
        elif jumo.is_final_heating():
            status = 'FINAL_HEATING'
        elif jumo.is_stable():
            status = 'STABLE'
        else:
            status = 'UNSTABLE'
        client.send(aboName,':PROG:STAT! %s\n'%status)
    elif coms[0].startswith('SETPOINT') or coms[0].startswith('TEMP'):
        if typ == 'c':
            temp = float(msg)
            jumo.set_setpoint(temp)
        elif typ == 'q':
            temp = jumo.setpoint
            client.send(aboName,':PROG:%s! %2.1f\n'%(coms[0],temp))
    elif coms[0].startswith('CYCLE'):
        analyseCycle(coms[1:],typ,msg)
        pass
    pass

def analyseMeasure(coms,typ,msg):
    if typ != 'q': 
        return
    if coms[0].upper().startswith('TEMP'):
        temp = jumo.get_temperature()
        client.send(aboName,':MEAS:TEMP! %2.1f'%temp)
        pass
    elif coms[0].upper().startswith('HUM'):
        hum = jumo.get_relative_humidity()
        client.send(aboName,':MEAS:HUM! %2.1f'%hum)
        pass
    elif coms[0].upper().startswith('DEWPOINT'):
        dewpoint = jumo.get_dew_point()
        client.send(aboName,':MEAS:DEWPOINT! %2.1f'%dewpoint)
    pass

def analysePacket(coms,typ,msg):
    global Logger
    global client
    global jumo
    if coms[0].find('PROG')>=0:
        if len(coms[1:])>0:
            analyseProg(coms[1:],typ,msg)
        elif typ != 'a':
            Logger << 'not valid packet: %s'%coms
            printHelp()
        pass
    elif coms[0].upper().startswith('MEAS'):
        analyseMeasure(coms[1:],typ,msg)
    elif coms[0].find('HELP')>=0 and typ != 'a':
        printHelp()
    elif coms[0].lower().startswith('exit') and typ != 'a':
        print 'stop controlling'
        jumo.stop_controlling()
        time.sleep(1)
        client.closeConnection()
    else:
        Logger << 'not Valid Packet %s'%coms
        
def sendMeasurements():
    temp = jumo.get_temperature()
    hum = jumo.get_relative_humidity()
    client.send(tempAbo,'%2.2f\n'%temp)
    client.send(humAbo,'%2.2f\n'%hum)
    client.send(dewPointAbo,'%2.2f\n'%jumo.get_dew_point())

def check_if_dry(isDry):
    if not jumo.is_dry():
        if isDry:
            hum = jumo.get_relative_humidity()
            temp = jumo.get_temperature() 
            client.send(aboName,'ERROR: Box not dry anymore, %2.1f %%, %2.1f degC'%(hum,temp))
            client.send(errorAbo,'coolingbox: not dry anymore!: %2.1f %%, %2.1f degC '%(hum,temp))
        isDry = False
    else:
        isDry = True
        pass
    return isDry


def mainLoop():

    sleep(0.5)
    counter = 0 
    lastMeasurementSend = time.time()
    isDry = False
    while client.anzahl_threads > 0 and End == False and client.isClosed == False: 
        isDry = check_if_dry(isDry)
        

        packet = client.getFirstPacket(aboName)

        now = time.time()
        if now - lastMeasurementSend > 5:
            sendMeasurements()
            lastMeasurementSend = now
        
        if packet.isEmpty():
            sleep(.5)
            continue
        counter +=1

        #Logger << 'got Packet: %s'%packet.Print()
        data = packet.data
        timeStamp,coms,typ,msg,command = decode(data)
        # 'T:',timeStamp, 'Comand:',command
        #Logger << '%s: %s, %s, %s'%(timeStamp,len(coms),typ,msg)
        #dataOut = '%s\n'%packet.Print()
        if len(coms)>0:
            analysePacket(coms,typ,msg)
        pass   
    client.send(aboName,':prog:stat! exit\n')    
    Logger << 'exiting...'
    jumo.stop_controlling()
    client.closeConnection()


def endClient():
    jumo.stop_controlling()
    while client.anzahl_threads > 0:
        sleep(1)
        pass            
    Logger << 'ciao!'
############################################################################
############################################################################
############################################################################
#RECEIVE COMMANDS:

try:
    mainLoop()
    endClient()
except Exception,e: 
    print str(e)
    jumo.stop_controlling()
    handler(1,1)
    endClient()
