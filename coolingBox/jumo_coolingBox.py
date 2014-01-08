
#import struct
#import minimalmodbus
import Imago500
import time
import thread
import sys

from coolingBox import coolingBox



class jumo_coolingBox(coolingBox):
    UNKNOWN = -1
    FLUSHING = 0 
    COOLING = 1
    HEATING = 2
    FINAL_HEATING = 3

    def __init__(self,device,baudrate=9600,slaveaddress=1):
        coolingBox.__init__(self)
        #self.jumo = Imago500(device,slaveaddress)
        self.jumo = Imago500.Imago500('/dev/ttyUSB0',1)
        self.status = self.UNKNOWN
        self.cycles = -1
        self.reachedSetpointTime = time.time()*10
        self.timeUntillStable = 30
        self.isFake = False
        self.last_measurement = 0
        self.measurement_distance = 1
        self.controlling = False
        self.end = False
        self.verbose = 1
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
        self.controlling = False
        self.status = self.UNKOWN
        if not self.isFake:
            self.jumo.activate_auto_mode()
        if not self.isFake:
            self.jumo.stop()
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
        print 'check set_point: %2.1f '%setpoint

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
        if not self.controlling: 
            return
        if self.verbose>2: print '\nstabilize',time.time()
        if self.status == self.UNKNOWN and self.controlling:
            print 'status unkown, start flushing'
            self.flushing()
            if self.verbose>2: print '\tdone',time.time()
            return
        temp = self.get_temperature()
        relHum = self.get_relative_humidity()
        if self.status == self.FLUSHING and not self.is_dry():
            if self.verbose: print 'not dry, temp: %2.1f degC, hum: %2.1f %%, setpoint: %2.1f degC, dewPoint: %2.1f degC'%(temp,relHum,self.setpoint,self.get_dew_point())
            if self.verbose>2: print '\tdone',time.time()
            return

        if not self.is_dry():
            if self.verbose: print 'ERROR: NOT DRY ENOUGH!!!!, temp: %2.1f degC, hum: %2.1f %%, setpoint: %2.1f degC, dewPoint: %2.1f degC'%(temp,relHum,self.setpoint,self.get_dew_point())
            self.flushing()
            if self.verbose>2: print '\tdone',time.time()
            return
        
        self.check_setpoint()
        self.check_cycles()
        if self.setpoint == -9999 and not self.doCycle:
            now = int(time.time())
            if now%10==0:
                if self.verbose: print 'waiting for setpoint.'

            if self.verbose>2: print '\tdone',time.time()
            return
        if self.status == self.FINAL_HEATING:
            if temp > 17:
                self.stop_controlling()
            return

        if temp - self.setpoint > 0:
            #self.deltaT_Max:
            self.cooling()
        elif self.setpoint - temp > self.deltaT_Max:
            self.heating()

        now = time.time()
        if not self.doCycle:
            if abs(temp - self.setpoint) > self.deltaT_Max:
                self.reachedSetpointTime = now * 10
                #print 'temp not good'
            elif now-self.reachedSetpointTime<0:
                if self.verbose: print 'reached setpoint %s'%now
                self.reachedSetpointTime = now
            elif now-self.reachedSetpointTime < self.timeUntillStable:
                if self.verbose >1 :print '%s more seconds to be stable'%int(self.timeUntillStable - (now-self.reachedSetpointTime))
            elif now-self.reachedSetpointTime -self.timeUntillStable < 2:
                print 'stable'
        if self.verbose >2: print '\tdone',time.time()

    def is_stable(self):
        #self.stabilize()
        temp = self.get_temperature()
        now = time.time()
        if self.doCycle:
            if self.cycles==-1:
                print 'end of cycling'
                self.doCycle = False
                self.reachedSetpointTime = now * 10
                return True
            else:
                if self.verbose>1: print 'left cycles: %s'%self.cycles
                return False

        if abs(temp - self.setpoint) > self.deltaT_Max:
            self.reachedSetpointTime = now * 10
            if self.verbose > 1: print 'temp not good'
        elif now-self.reachedSetpointTime<0:
            if self.verbose > 1: print 'reached setpoint'
        if now-self.reachedSetpointTime > self.timeUntillStable:
            if self.verbose > 1: print 'stable'
            return True
        return False
    
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
      jumo = jumo_coolingBox('/dev/ttyUSB0')
      jumo.start_controlling()

      #jumo.set_setpoint(17)
      #while not jumo.is_stable():
      #    time.sleep(1)
      # jumo.stop_controlling()
