#!/usr/bin/python 
from sclient import *
from decode import *
from printcolor import printc
from threading import Thread
import subprocess
import time
import argparse
import keithleyInterface
import os
import sys
ON = 1
OFF = 0
End = False


#sweep parameters TODO anpassen
startValue = -100
stopValue = -200
stepValue = 15
nSweeps = 1
delay =.1
maxSweepTries=1
doingSweep = False

defSerialPort = '/dev/tty.usbserial-FTG7MJTY'
serialPort = defSerialPort
#optParser = argparse.ArgumentParser(description='keithley communication client for Subsytem')
parser = argparse.ArgumentParser()

parser.add_argument("-d", "--device", dest="serialPort",
                       help="serial Port address e.g. /dev/ttyF0",
                       default=defSerialPort)
#                       type=string,
#                        type=file,
#                       action='store_const',
#                       const = serialPort)
#                       default=defSerialPort)
#                       action='store_const',
#                       metavar="SERIALPORT")
args = parser.parse_args()
print args.serialPort
serialPort= args.serialPort
if not os.access(serialPort,os.R_OK):
    print 'serialPort \'%s\' is not accessible'%serialPort
    sys.exit()
    raise SystemExit
print serialPort

def handler(signum, frame):
    print 'Close Connection'
    client.closeConnection()
    print 'Signal handler called with signal', signum
    if client.isClosed == True:
        print 'client connection closed: kill all'
        End=True
        End=True
        print End
    
signal.signal(signal.SIGINT, handler)


serverZiel = '127.0.0.1'
serverPort = 12334
aboName = '/keithley'
IVAbo = '/keithley/IV'
voltageAbo = '/keithley/voltage'
currentAbo = '/keithley/current'
resistanceAbo='/keithley/resistance'
client = sClient(serverZiel,serverPort,"keithleyClient")
client.subscribe(aboName)
client.send(aboName,'Connecting Keithley Client with Subsystem\n')
keithley=keithleyInterface.keithleyInterface(serialPort)
keithley.setOutput(ON)
print 'status:%s'%keithley.getOutputStatus()
keithley.setOutput(OFF)
print 'status:%s'%keithley.getOutputStatus()

def readCurrentIV():
    if keithley.getOutputStatus():
        data= keithley.getAnswerForQuery(':READ?').split(' ')
        timestamp = time.time()
        if len(data)==5:
            if is_float(data[0]) and is_float(data[1]) and is_float(data[2]):
                voltage = float(data[0])
                current = float(data[1])
                resistance = float(data[2])
                print '%s: %s V - %s A'%(timestamp,data[0],data[1])
                client.send(voltageAbo,'%s\n'%voltage)
                client.send(currentAbo,'%s\n'%current)
                client.send(resistanceAbo,'%s\n'%resistance)
        else:
            print ' could somehow not read data correctly',data
    else:
        print 'output is off'
        
def sweep():
    global doingSweep
    doingSweep = True
    outputStatus = keithley.getOutputStatus()
    client.send(aboName,'Start with Linear Sweep from %sV to %sV in %sV steps\n'%(startValue,stopValue,stepValue))
    ntries = 0
    while True:
        retVal = keithley.doLinearSweep(startValue, stopValue, stepValue, nSweeps, delay)
        print 'keithley RetVal: %s'%retVal
        ntries+=1
        if retVal!=0 or ntries>=maxSweepTries:
            print 'exit while loop'
            break
        voltage = keithley.getLastVoltage()
        msg='Keithley Tripped %s of %s times @ %s\n'%(ntries,maxSweepTries,voltage)
        print msg
        client.send(aboName,msg)
        client.send(IVAbo,msg) 
    client.send(aboName,'Results of Linear Sweep:\n')
    client.send(IVAbo,'Results Start:\n')
    client.sendData(voltageAbo,'Sweep Data\n')
    client.sendData(currentAbo,'Sweep Data\n')
    client.sendData(resistanceAbo,'Sweep Data\n')
    client.send(aboName,':PROG:IV! RESULTS\n')
    npoint = 0
    nPoints = len(keithley.measurments)
    while len(keithley.measurments)>0:
        npoint +=1
        measurement = keithley.measurments.popleft()
        measurement[0]=int(measurement[0])
        timestamp = measurement[0]
        voltage = float(measurement[1])
        current = float(measurement[2])
#        resistance = float(measurement[3])
        client.sendData(aboName,':IV! %s/%s'%(npoint,nPoints) +" ".join(map(str, measurement[:3]))+'\n')
        client.sendData(IVAbo," ".join(map(str, measurement[:3]))+'\n')
        client.sendData(voltageAbo,'%s %s\n'%(timestamp,voltage))
        client.sendData(currentAbo,'%s %s\n'%(timestamp,current))
#        client.sendData(resistanceAbo,'%s %s\n'%(timestamp,resistance))
    client.send(IVAbo,'Results End\n')
    client.send(aboName,':PROG:IV! FINISHED\n')
    client.send(voltageAbo,'Sweep Data Done\n')
    client.send(currentAbo,'Sweep Data Done\n')
    client.send(resistanceAbo,'Sweep Data Done\n')
    sleep(1)
    keithley.initKeithley()
    keithley.setOutput(outputStatus)
    doingSweep =  False



############################################################################
############################################################################
############################################################################

def printHelp():
    data = '\n************************************************************************\n'
    data +=  'This is the Help for the python keithley client, part of elComandante\n'
    data +=' You can use following SCPI like commands: \n'
    data +='\t:HELP                \tto show this Help\n'
    data +='\t:OUTPut ON/OFF       \tto switch the Output of the Keithley ON/OFF\n'
    data +='\t:OUTPut?             \tto query the current status of the Output of the Keithley\n'
    data +='\t:PROG:IV MEASURE    \tto make an IV Curve for the current device\n'
    data +='\t:PROG:IV:START    XX\tto make an IV Curve for the current device\n'
    data +='\t:PROG:IV:STOP     XX\tto make an IV Curve for the current device\n'
    data +='\t:PROG:IV:STEP     XX\tto make an IV Curve for the current device\n'
    data +='\t:PROG:IV:MAXTRIPS XX\tto set maximum tries if keithley is tripping\n'
    data +='\t:PROG:RESISTANCE  XX\tto enable/disable 4-Wire Resistance Measurement\n'
    data += '************************************************************************\n'
    print data
    client.sendData(aboName,data)
    
def  analyseIV(coms,typ,msg):
    global startValue
    global stopValue
    global delay
    global stepValue
    global maxSwepTries
    global nSweeps
    print'analyse :IV'
    if len(coms)==0:
        if msg.find('MEAS')>=0 and typ=='c':
            outMsg= 'Do Sweep from %.2f V to %.2f'%(startValue,stopValue)
            outMsg+=' in steps of %.2fV with a delay of %.f\n'%(stepValue,delay)
            print outMsg
            client.send(aboName,outMsg)
            sweep()
        else:
            print 'error'
            printHelp()
    elif len(coms)==1:
        print 'iv len >0'
        outMsg = 'not Valid Input'
        if coms[0].find('START')>=0:
            if typ =='c' and is_float(msg):
                startValue=float(msg)
                print 'prog-iv-start=%s'%msg
            elif typ =='q':
                print 'prog-iv-start?'
            outMsg = ':PROG:IV:START! %s'%startValue
        elif coms[0].find('STOP')>=0:
            if typ =='c'and is_float(msg):
                stopValue = float(msg)
                print 'prog-iv-stop=%s'%msg
            elif typ =='q':
                print 'prog-iv-stop?'
            outMsg = ':PROG:IV:STOP! %s'%stopValue
        elif coms[0].find('STEP')>=0:
            if typ =='c'and is_float(msg):
                stepValue=float(msg)
                print 'prog-iv-step=%s'%msg
            elif typ =='q':
                print 'prog-iv-step?'
            outMsg = ':PROG:IV:STEP! %s'%stepValue
        elif coms[0].find('DEL')>=0:           
            if typ =='c'and is_float(msg):
                delay=float(msg)
                print 'prog-iv-delay=%s'%msg
            elif typ =='q':
                print 'prog-iv-delay?'
            outMsg = ':PROG:IV:DELAY! %s'%delay
        elif coms[0].find('MAXTRIPS')>=0:           
            if typ =='c' and is_float(msg):
                maxSweepTries=float(msg)
                print 'prog-iv-trip=%s'%msg
            elif typ =='q':
                print 'prog-iv-trip?'
            outMsg = ':PROG:IV:TRIP! %s'%delay
        outMsg+='\n'
        client.send(aboName,outMsg)
    else:
        print 'error prog iv len to long'
        printHelp()
    pass
        
def analyseProg(coms,typ,msg):
    print 'analyse :PROG'
    print coms
    if coms[0].find('IV')>=0:
        analyseIV(coms[1:],typ,msg)
        pass
    elif coms[0].find('RESISTANCE')>=0 and typ =='c':
        if msg in ['ON','TRUE','1']:
            keithley.initFourWireResistensMeasurement()
        elif msg in ['OFF','FALSE','0']:
            keithley.initKeithley()
    elif coms[0].find('EXIT')>=0 and typ =='c':
        print 'end program'
        client.closeConnection();
        
    else:
        printHelp()
    pass

def analyseOutp(coms,typ,msg): #pretty much ok
    print 'analyse Output'
    if len(coms)>0:
        print 'not valid command: ',coms, typ, msg
        printHelp()
    else:
        if typ=='q':
            print 'Query for output status'
            status = keithley.getOutputStatus()
            outMsg = ':OUTP! '
            outMsg+= 'ON' if status else 'OFF'
            outMsg+='\n'
            print outMsg
            client.send(aboName,outMsg)
        elif typ=='c':
            if msg in ['1','ON','True']:
                keithley.setOutput(ON)
            elif msg in ['0','OFF','False']:
                keithley.setOutput(OFF)
            else: 
                print 'message of :OUTP not valid: %s, valid messages are \'ON\',\'OFF\''%msg
                printHelp()
        else:
            print 'this a non valid typ'
            printHelp()
    pass

def analysePacket(coms,typ,msg):
    if coms[0].find('PROG')>=0:
        if len(coms[1:])>0:
            analyseProg(coms[1:],typ,msg)
        else:
            print 'not valid packet: ',coms
            printHelp()
        pass
    elif coms[0].find('OUTP')>=0:
        analyseOutp(coms[1:],typ,msg)
        pass
    elif coms[0].find('HELP')>=0:
        printHelp()
    elif coms[0]=='K':
        command = ":".join(map(str, coms[1:]))+' '+msg
        print 'send command to keithley: '
        keithley.write(command)
    else:
        print 'not Valid Packet'
        


############################################################################
############################################################################
############################################################################
#RECEIVE COMMANDS:

sleep(0.5)
counter = 0 
while client.anzahl_threads > 0 and End == False and client.isClosed == False: 
    
    counter +=1
    sleep(.5)
    packet = client.getFirstPacket(aboName)
    
    if not packet.isEmpty():

        #print 'read a new packet from abo', packet.aboName,':'
        print 'got Packet:', packet.Print()
        data = packet.data
        timeStamp,coms,typ,msg,command = decode(data)
        print 'T:',timeStamp, 'Comand:',command
        print '%s: %s, %s, %s'%(timeStamp,len(coms),typ,msg)
        dataOut = '%s\n'%packet.Print()
        if command.find(':DOSWEEP')!=-1:
            sweep()
        if len(coms)>0:
            analysePacket(coms,typ,msg)
        else:
            keithley.write(command)
            client.send(aboName,dataOut)
        #string retVal = keithley.setOutput(ON)
    else:
       # print client.getNumberOfPackets(),' packets are left in queue'
       pass
    if counter%10 == 0:      
        if not doingSweep:
            readCurrentIV()
    pass   
        
        

client.send(aboName,':prog:stat! exit\n')    
print 'exiting...'
client.closeConnection()

#END
while client.anzahl_threads > 0:
    sleep(1)
    pass            
print 'ciao!'

