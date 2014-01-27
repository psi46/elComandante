import minimalmodbus
import time


class Imago500( minimalmodbus.Instrument ):
        """Instrument class for JUMO IMAG) 500  PID controller
            ARGS: 
                    * portname (str): port name
                    * slaveaddress (int): slave address in the range 1 to 247

       """
        nControllers = 4
        nSetPoints = 4
        polarity = 1<<0|1<<15
        safty = 1<<1|1<<15
        water = 1<<4|1<<15
        flush = 1<<5|1<<15
        switch1 = 1 << 6|1<<15
        switch2 = 1<< 7|1<<15
        def __init__(self, portname, slaveaddress):
            minimalmodbus.TIMEOUT=.5
            minimalmodbus.Instrument.__init__(self, portname, slaveaddress)
            self.serial.baudrate = 9600
            self.debug=False
            time.sleep(.1)
            self.currentHum = 100
            self.currentTemp = 9999
            self.lock = False

        def initialise_controlling(self):
            self.set_Binary_control_contacts(False,False,True,False)
            self.set_parameter_set(1,0)
            self.set_parameter_set(1,0)
            self.write_program_buffer()

        def wait_for_unlock(self):
            i = 0
            locked = time.time()
            while self.lock:
                time.sleep(.1)
                i+=1
                if i%100 == 0:
                    now = time.time()
                    print 'reste unlock: after %s seconds'%(now-locked)
                    self.lock = False


        def get_register(self,address):
            ntries = 0
            while True:
                ntries +=1
                try:
                    self.wait_for_unlock()
                    self.lock = True
                    a = self.read_register(address)
                    self.lock = False
                    break
                except IOError:
                    self.lock = False
                    print 'IOError'
                    pass
                except ValueError:
                    self.lock = False
                    print 'ValueError'
                    pass
                if ntries>20:
                    print 'cannot get_register after',ntries
                    a = 0
                time.sleep(.1)
            return a

        def get_registers(self,address,number):
            ntries = 0
            while True:
                ntries += 1
                try:
                    self.wait_for_unlock()
                    self.lock = True
                    a = self.read_registers(address,number)
                    self.lock = False
                    break
                except IOError:
                    self.lock = False
                    print 'IOError'
                    pass
                except ValueError:
                    self.lock = False
                    print 'ValueError'
                    pass
                if ntries>20:
                    print 'cannot get_registers after',ntries
                    a = number*[0]
                time.sleep(.1)
            return a


        def get_long(self,address):
            ntries = 0
            while True:
                ntries +=1
                try:
                    self.wait_for_unlock()
                    self.lock = True
                    a = self.read_long(address)
                    self.lock = False
                    break
                except IOError:
                    self.lock = False
                    print 'IOError'

                    pass
                except ValueError:
                    self.lock = False
                    print 'ValueError'
                    pass
                if ntries>20:
                    print 'cannot get_long after',ntries
                    a = 0
                time.sleep(.1)
            return a
        

        def set_register(self,address,value):
            try:
                self.wait_for_unlock()
                self.lock = True
                self.write_register(address,value)
                self.lock = False
            except ValueError:
                self.lock = False
                print 'ValueError'
                pass

        def set_registers(self,address,value):
            try:
                self.wait_for_unlock()
                self.lock = True
                self.write_registers(address,value)
                self.lock = False
            except ValueError:
                self.lock = False
                print 'ValueError'
                pass

        def set_float(self,address,value):
            a = minimalmodbus._bytestringToValuelist(minimalmodbus._floatToBytestring(value),2)
            b = [a[1],a[0]]
            ntries = 0
            while True:
                ntries += 1
                try:
                    self.set_registers(address,b)
                    break
                except IOError:
                    print 'IOError'
                    currentValue = self.get_float(address)
                    if abs(currentValue-value)<1e-1:
                        break
                    else:
                        print 'float not set correctly, retry, %s!=%s'%(currentValue,value)
                    pass
                if ntries>20:
                    print 'cannot set_float after',ntries
                time.sleep()

        def start(self):
            print 'start'
            try:
                self.set_register(0x01B4,0x1000)
            except ValueError:
                pass

        #def stop(self):
        #    print 'stop'
        #    try:
        #        self.set_register(0x0172,0x0800)
        #    except ValueError:
        #        pass

        def next(self):
            print 'next'
            try:
                self.set_register(0x0172,0x0004)
            except ValueError:
                pass


        def get_parameter_set_numbers(self):
            """CX: parameter set number """
            return self.get_registers(0x0118,4)

        def get_parameter_set_no(self,channel):
            if not channel in range(1,5):
                return -1
            address = 0x0118
            address += (channel-1)
            return self.get_register(address)

     
        def get_segment_no(self,channel):
            if not channel in range(1,5):
                return -1
            address = 0x011D
            address += (channel-1)
            return self.get_register(address)

        def get_segment_numbers(self):
            return self.get_registers(0x011D,4)

        def get_program_number(self):
            return self.get_register(0x011C)

        def get_last_segment(self,channel):
            if not channel in range(1,5):
                return -1
            address = 0x0121
            address += (channel-1)
            return self.get_register(address)

        def get_float(self,address):
            a = self.get_registers(address,2)
            try:
                retVal = self.convert_MODbus_to_standard_float(a)
            except:
                retVal = -9999999
            return retVal

        def convert_MODbus_to_standard_float(self,a):
            if len(a) != 2:
                raise Exception('INVALID input')
            b = [a[1],a[0]]
            return minimalmodbus._bytestringToFloat("".join([minimalmodbus._numToTwoByteString(i) for i in b]))


        def read_conditions(self):
            bReadConditions = False
            i = 0
            while not bReadConditions and i < 10:
                if i>3:
                    print 'read conditions: ',i
                i+= 1
                try:
                    a = self.get_registers(0x00A6,6)
                    temp1 = self.convert_MODbus_to_standard_float(a[0:2])
                    temp2 = self.convert_MODbus_to_standard_float(a[2:4])
                    hum = self.convert_MODbus_to_standard_float(a[4:6])
                    self.currentTemp = temp2
                    self.currentHum = hum
                    bReadConditions = True
                    time.sleep(.3)
                except:
                    pass


        
        def get_generator_setpoint(self,channel):
            """PChX: generator setpoint according to operating mode """
            if not channel in range(1,5):
                return -1
            address = 0x0127
            address += (channel-1)*2
            return self.get_float(address)
    
        def get_control_contacts(self):
            """Control contacts (INT),
                    Bit 00:  Control contact 01
                    ...
                    Bit 15:  Control contact 16"""
            a =  self.get_register(0x012F)
            b = [] 
            for i  in range(0,16):
                b.append(a&1<<i==1<<i)
            return b
        

        def get_control_contact(self,bit):
            if bit in range (0,16):
                a = self.get_control_contacts()
                return a[bit]
            return -1

        def get_polarity_contact(self):
            return self.get_control_contact(0)

        def get_air_flush_contact(self):
            return self.get_control_contact(1)

        def get_air_and_water_flow_contact(self):
            return self.get_control_contact(2)

        def get_safty_indicator_contact(self):
            return self.get_control_contact(3)

        def get_program_runtime(self):
            return self.get_long(0x0136)

        def get_remaining_program_runtime(self):
            return self.get_long(0x0138)

        def get_setpoint_address(self,controllerNo,setPointNo):
             if controllerNo<1 or controllerNo>self.nControllers:
                 return -1
             if setPointNo<1 or setPointNo >self.nSetPoints:
                 return -1
             controllerNo -= 1
             setPointNo -= 1
             floatLength = 2
             address = 0x083C
             address +=  (controllerNo*self.nSetPoints+setPointNo)*floatLength
             return address
 

        def get_setpoint(self,channel,number):
            address = self.get_setpoint_address(channel,number)
            if address == -1:
                raise Exception
            return self.get_float(address)

        def activate_manual_mode_controller(self,controllerNo):
            print 'manual mode'
            if controllerNo < 1 or controllerNo > self.nControllers:
                raise Exception
            address = 0x0173
            address += (controllerNo-1)
            self.set_register(address,0x200)

        def activate_auto_mode_controller(self,controllerNo):
            print 'auto mode'
            if controllerNo < 1 or controllerNo > self.nControllers:
                raise Exception
            address = 0x0173
            address += (controllerNo-1)
            self.set_register(address,0x100)

            

        def activate_manual_mode(self):
            print 'manual mode'
            self.set_register(0x0172,0x200)
            self.set_register(0x0173,0x200)
            self.set_register(0x0174,0x200)
            self.set_register(0x0175,0x200)
            self.set_register(0x0176,0x200)


        def activate_auto_mode(self):
            print 'auto mode'
            self.set_register(0x0172,0x100)
            self.set_register(0x0173,0x100)
            self.set_register(0x0174,0x100)
            self.set_register(0x0175,0x100)
            self.set_register(0x0176,0x100)

#        def set_temperature_controller(self,controllerNo,setPointNo,temp):
#             address = self.get_setpoint_address(controllerNo,setPointNo)
#             if address == -1:
#                 raise Exception
#                 return
#             self.write_float(address,temp)


        def set_temperature_controller(self,controllerNo,setpointNo,temp):
            self.activate_manual_mode_controller(controllerNo)
            address = self.get_setpoint_address(controllerNo,setpointNo)
            if address == -1:
                raise Exception
                return
            self.set_float(address,temp)
            self.switch_to_setpoint(controllerNo,setpointNo)
            self.activate_auto_mode_controller(controllerNo)


        def switch_to_setpoint(self,controllerNo,setpointNo):
            if controllerNo<1 or controllerNo>self.nControllers:
                raise Exception
            if setpointNo<1 or setpointNo >self.nSetPoints:
                raise Exception
            address = 0x09F7+(controllerNo-1)*3
            self.set_register(address,setpointNo)


        def write_program_buffer(self):
            self.set_register(0x01C1,0x200)

        def convert_Binary_control_contacts(self,bFlush,bFlow,bSafty,bPolarity):
            retVal = 0
            if bFlush:
                retVal = retVal | 0x200
            if bFlow:
                retVal = retVal | 0x0400
            if bSafty:
                retVal = retVal | 0x800
            if bPolarity:
                retVal = retVal | 0x100
            return retVal

    
        def get_Binary_control_contacts(self):
            register = self.get_register(0x01CA)
            contacts = [0x200,0x0400,0x800,0x100]
            retVal = [register&i == i for i in contacts]
            return retVal

        def set_Binary_control_contacts(self,bFlush,bFlow,bSafty,bPolarity):
            self.set_register(0x01CA,self.convert_Binary_control_contacts(bFlush,bFlow,bSafty,bPolarity))
            #self.write_program_buffer()

        def set_parameter_set(self,channel,setNumber):
            if channel>=1 and channel <=4 and setNumber>=1 and setNumber<=2:
                self.set_register(0x01CB+channel-1,setNumber-1)
            #self.write_program_buffer()

        def activate_controllers(self,controller1,controller2,controller3=False,controller4=False):
            
            register = (controller1)+(controller2<<1)+(controller3<<2)+(controller4<<3)
            self.set_register(0x01CF,register)
            #self.write_program_buffer()



        def stop(self):
            self.activate_controllers(False,False)
            self.set_Binary_control_contacts(False,False,False,False)
            self.write_program_buffer()

        def flush_air(self):
            self.activate_controllers(False,False)
            self.set_Binary_control_contacts(True,False,True,False)
            self.write_program_buffer()
            pass

        def cooling(self):
            #self.set_parameter_set(1,0)
            #self.set_parameter_set(2,1)
            self.activate_controllers(True,False)
            self.set_Binary_control_contacts(False,True,True,False)
            self.write_program_buffer()
        

        def heating(self):
            #self.set_parameter_set(1,1)
            #self.set_parameter_set(2,0)
            self.activate_controllers(False,True)
            self.set_Binary_control_contacts(True,True,True,True)
            self.write_program_buffer()

        def set_setpoint(self,temp):
            for i in range(0,2):
                self.set_float(0x01C2+i*2,temp)
            self.set_float(0x083E,temp)
            self.write_program_buffer()
        
        def get_temperature(self):
            return self.currentTemp

        def get_relative_humidity(self):
            return self.currentHum

        #def read_conditions(self):
        #    self.currentHum = 20
        #    self.currentTemp = 20
        #    pass

if __name__=='__main__':
    jumo = Imago500('/dev/ttyUSB0',1)



