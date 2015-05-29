import datetime
#import struct
#import minimalmodbus
import Imago500
import time
import thread
import sys

from coolingBox import coolingBox



class jumo_coolingBox(coolingBox):
    buffer_length = 100

    def __init__(self,device,baudrate=9600,slaveaddress=1):
        coolingBox.__init__(self)
        #self.jumo = Imago500(device,slaveaddress)
        self.jumo = Imago500.Imago500(device,1)
        self.status = self.UNKNOWN
        self.cycles = -1
        self.reachedSetpointTime = time.time()*10
        self.timeUntillStable = 120
        self.isFake = False
        self.last_measurement = 0
        self.measurement_distance = 1
        self.controlling = False
        self.end = False
        self.verbose = 0
        self.start_controlling()
        self.stabilize()
        thread.start_new(self.check_status,())

    def __del__(self):
        self.stop_controlling()
        print 'close '
        self.end =  True



    def start_controlling(self):
        if not self.controlling:
            self.jumo.initialise_controlling()
            self.controlling = True
            self.set_setpoint(self.setpoint)
            if self.status != self.COOLING and self.status != self.HEATING:
                self.flushing()
            if self.verbose > 0: print 'start'
            self.stabilize()

    def stop_controlling(self):
        if not self.controlling:
            return
        if not self.isFake:
            self.jumo.initialise_controlling()
        print 'stop controlling'
        self.controlling = False
        self.jumo.stop()
        print 'controlling',self.controlling
        self.status = self.UNKNOWN
        if not self.isFake:
            self.jumo.activate_auto_mode()
        if self.verbose > 0: print 'stop'

    def flushing(self):
        if self.status != self.FLUSHING:
            if self.verbose > 0: 
                print 'start flushing'
            if not self.isFake:
                self.jumo.flush_air()
            self.status = self.FLUSHING

    def cooling(self):
        if self.status != self.COOLING:
            if self.setpoint == -9999:
                return
            print 'start cooling, %2.1f degC'%self.setpoint
            #self.jumo.set_setpoint(self.setpoint)
            if not self.isFake:
                self.jumo.cooling()
            self.status = self.COOLING

    def heating(self):
        if self.status != self.HEATING:
            #self.jumo.set_setpoint(self.setpoint)
            if self.setpoint == -9999:
                return
            print 'start heating %2.1f'%self.setpoint
            if not self.isFake:
                self.jumo.heating()
            self.status = self.HEATING

    def final_heating(self):
        print 'start final heating'
        self.set_setpoint(20)
        self.heating()
        self.status = self.FINAL_HEATING


    def check_status(self):
        try:
            while not self.end:
                if self.controlling:
                    if self.verbose>2: print 'check  status',time.time()
                    self.stabilize()
                time.sleep(.5)
        except Exception,e:
            print 'EXCEPTION in check_status: ',e
            pass

    def check_setpoint(self):
        if not self.controlling:
            return
        now = int(time.time())
        if now % 10 != 0:
            return
        setpoint = self.setpoint

       # if self.doCycle:
       #     temp = self.get_temperature()
       #     if self.cycles == -1:
       #         return
       #     if temp >self.cycleHigh:
       #         setpoint = self.cycleLow
       #     if temp < self.cycleLow:
       #         setpoint = self.cycleHigh
        self.jumo.set_setpoint(setpoint)

    def check_cycles(self):
        if self.cycles == -1:
            return 
        temp = self.get_temperature()
        if self.verbose>2: print 'temp: %2.1f, cycles: %s, cycleLow: %2.1f, cycleHigh: %2.1f'%(temp,self.cycles,self.cycleLow,self.cycleHigh)
        if self.cycles == 0:
            if temp > self.cycleHigh:
                self.cycles = -1
                self.set_setpoint(self.cycleHigh)
                self.doCycles = False
                if self.verbose >0: print 'cycles done'

        if temp < self.cycleLow and self.cycles%1 == 0.5 and self.cycles>0:
            if self.verbose >1: print 'start cycle heating'
            self.cycles -= 0.5
            self.set_setpoint(self.cycleHigh+self.cycleAdditionalTemp)
        if temp > self.cycleHigh and self.cycles%1 == 0.0 and self.cycles > 0:
            if self.verbose > 0: print 'start cycle cooling'
            self.cycles -= 0.5
            self.set_setpoint(self.cycleLow-self.cycleAdditionalTemp)

    def stabilize(self):
        d = datetime.datetime.now()
        log_message = '\r{date} '.format(date=d.strftime("%y-%m-%d %H:%M:%S"))
        if not self.controlling: 
            log_message += 'deactivated'
            sys.stdout.write('\r'+' '*self.buffer_length)
            sys.stdout.write(log_message+ ' '*20)
            sys.stdout.flush()
            return
        if self.verbose>2: print '\nstabilize',time.time()
        if self.status == self.UNKNOWN and self.controlling:
            print 'status unkown, start flushing'
            self.flushing()
            log_message += 'flushing'
            if self.verbose>2: print '\tdone',time.time()
            sys.stdout.write('\r'+' '*self.buffer_length)
            sys.stdout.write(log_message)
            sys.stdout.flush()
            return
        temp = self.get_temperature()
        relHum = self.get_relative_humidity()
        try:
            log_message +=' hum: {hum:4.1f}%, temp: {temp:+5.1f} degC\t'.format(temp=temp,hum=relHum)
        except:
            log_message += ' unknownTemp/Hum\t'
        if self.status == self.FLUSHING and not self.is_dry():
            if self.verbose: print 'not dry, temp: %2.1f degC, hum: %2.1f %%, setpoint: %2.1f degC, dewPoint: %2.1f degC'%(temp,relHum,self.setpoint,self.get_dew_point())
            if self.verbose>2: print '\tdone',time.time()
            log_message += 'Drying'
            sys.stdout.write('\r'+' '*self.buffer_length)
            sys.stdout.write(log_message)
            sys.stdout.flush()
            return

        if not self.is_dry():
            if self.verbose: print 'ERROR: NOT DRY ENOUGH!!!!, temp: %2.1f degC, hum: %2.1f %%, setpoint: %2.1f degC, dewPoint: %2.1f degC'%(temp,relHum,self.setpoint,self.get_dew_point())
            self.flushing()
            if self.verbose>2: print '\tdone',time.time()
            log_message += 'REDrying'
            sys.stdout.write('\r'+' '*self.buffer_length)
            sys.stdout.write(log_message)
            sys.stdout.flush()
            return
        
        self.check_setpoint()
        self.check_cycles()
        if self.setpoint == -9999 and not self.doCycle:
            now = int(time.time())
            if now%10==0:
                if self.verbose: print 'waiting for setpoint.'

            if self.verbose>2: print '\tdone',time.time()
            log_message += 'waiting for Setpoint'
            sys.stdout.write('\r'+' '*self.buffer_length)
            sys.stdout.write(log_message)
            sys.stdout.flush()
            return
        if self.status == self.FINAL_HEATING:
            if temp > 17:
                self.stop_controlling()
            log_message += 'Stop Controlling'
            sys.stdout.write('\r'+' '*self.buffer_length)
            sys.stdout.write(log_message)
            sys.stdout.flush()
            return

        if temp - self.setpoint > 0 and abs(temp - self.setpoint) > self.deltaT_Max:
            #self.deltaT_Max:
            self.cooling()
            log_message += 'Cooling'
        elif self.setpoint - temp > self.deltaT_Max and abs( self.setpoint - temp) > self.deltaT_Max:
            self.heating()
            log_message += 'Heating'
        log_message += '@ {setpoint:+5.1f} degC \t'.format(setpoint=self.setpoint)

        now = time.time()
        if not self.doCycle:
            if abs(temp - self.setpoint) > self.deltaT_Max:
                self.reachedSetpointTime = now * 10
                log_message += 'Not Yet Reached'
                #print 'temp not good'
            elif now-self.reachedSetpointTime<0:
                if self.verbose: print 'reached setpoint %s'%now
                log_message += 'Reached Setpoint'
                self.reachedSetpointTime = now
            elif now-self.reachedSetpointTime < self.timeUntillStable:
                if self.verbose >1 :print '%s more seconds to be stable'%int(self.timeUntillStable - (now-self.reachedSetpointTime))
                log_message += 'UnStable - time left: {time}s'.format(time=int(self.timeUntillStable - (now-self.reachedSetpointTime)))
            elif now-self.reachedSetpointTime -self.timeUntillStable < 2:
                log_message += 'Stable'
            else:
                log_message += 'Stable'
        sys.stdout.write('\r'+' '*self.buffer_length)
        sys.stdout.write(log_message)
        sys.stdout.flush()

    def is_stable(self):
        #self.stabilize()
        temp = self.get_temperature()
        #status_log = '\rCurrentTemperature {temp:+5.1f}\t'.format(temp=temp)
        now = time.time()
        if self.doCycle:
            #status_log += 'Cycling'
            if self.cycles==-1:
                #status_log +=  ' --> end of cycling\n'
                self.doCycle = False
                self.reachedSetpointTime = now * 10
                #print status_log
                return True
            else:
                if self.verbose>1: 
                    print 'left cycles: %s'%self.cycles
                #status_log += 'left cycles: {cycles}'.format(cycles=self.cycles)
                #print status_log,
                return False

        if abs(temp - self.setpoint) > self.deltaT_Max:
            self.reachedSetpointTime = now * 10
            #status_log += 'Setpoint {setpoint:+5.1f}'.format(setpoint= self.setpoint)
            if self.verbose > 1: print 'temp not good'
        elif now-self.reachedSetpointTime<0:
            #status_log += 'Setpoint {setpoint:+5.1f}, left time: {time} '.format(setpoint= self.setpoint,
                                                                        #time=int(self.timeUntillStable -(now-self.reachedSetpointTime)))
            if self.verbose > 1: print 'reached setpoint'
        if now-self.reachedSetpointTime > self.timeUntillStable:
            if self.verbose > 1: print 'stable'
            return True
        return False

    def get_current(self):
        self.update_measurements()
        cur = self.jumo.get_current()
        return float(cur)
    
    def get_temperature(self):
        self.update_measurements()
        temp = self.jumo.get_temperature()
        return float(temp)

    def get_relative_humidity(self):
        hum = self.jumo.get_relative_humidity()
        return float(hum)

    def update_measurements(self):
        now = time.time()
        delta = now - self.last_measurement
        if delta > self.measurement_distance:
            if self.isFake:
                self.jumo.currentHum = raw_input('hum:')
                self.jumo.currentTemp= raw_input('temp:')
                pass
            else:
                self.jumo.read_conditions()
            self.last_measurement = time.time()

    def set_setpoint(self,temp):
        if self.setpoint != temp:
            self.setpoint = temp
            print 'new setpoint: %2.1f degC' %self.setpoint
            if temp == -9999:
                self.flushing()
            elif not self.isFake:
                self.jumo.set_setpoint(self.setpoint)
                self.stabilize()
            time.sleep(.5)


if __name__=='__main__':
      jumo = jumo_coolingBox('/dev/ttyJUMO')
      jumo.start_controlling()

      #jumo.set_setpoint(17)
      #while not jumo.is_stable():
