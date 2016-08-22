# implementation of ISEG NIM Power supplies
# based on HV_interface class

# ============================
# IMPORTS
# ============================
import ConfigParser
import os
import sys
import warnings
import inspect

currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0, parentdir)
from HV_interface import HVInterface
import serial
from time import sleep, time

# ============================
# CONSTANTS
# ============================
ON = 1
OFF = 0


# ============================
# MAIN CLASS
# ============================
class ISEG(HVInterface):
    def __init__(self, config, device_no=1, hot_start=False):
        self.last_write = ''
        self.nchannels = 6
        self.last_iv_measurement = {'time': -1, 'ivs': None}
        self.last_channel_status = {'time': -1, 'status': None}
        self.Busy = False
        self.commandEndCharacter = '\r\n'
        self.readSleepTime = .1
        self.writeSleepTime = .1
        HVInterface.__init__(self, config, device_no, hot_start)
        self.bOpen = False
        self.config = config
        self.read_config()
        self.lastVoltage = 0
        self.serial = None
        # self.model = self.get_model_name()
        self.identifier = None
        self.answer_time = 0.1
        self.open_serial_port()
        self.can_ramp = True
        self.init_device(hot_start)
        pass

    def open_serial_port(self):
        try:
            self.serial = serial.Serial(
                port=self.serialPortName,
                baudrate=9600,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                timeout=.5,
            )
            self.bOpen = True
            print 'Open serial port: \'%s\'' % self.serialPortName
        except serial.SerialException:
            print 'Could not open serial Port: \'%s\'' % self.serialPortName
            self.bOpen = False
            pass

    def init_device(self, hot_start):
        if hot_start:
            sleep(1)
            self.clear_buffer()
            self.identify()
            self.clear_error_queue()
            self.configure_ramp_speed_voltage()
            sleep(1)
        else:
            sleep(.1)
            self.set_emergency_off('all')
            self.set_emergency_clear('all')
            self.configure_ramp_speed_voltage()
            self.reset()
            self.clear_buffer()
            self.identify()
            self.set_max_voltage()
            self.clear_error_queue()
            sleep(.1)
        return

    def read_config(self):
        self.serialPortName = self.config.get(self.section_name, 'address')
        pass

    def identify(self):
        self.identifier = self.get_answer_for_query('*IDN?')
        self.get_model_name()

    def is_valid_voltage(self, voltage, channel):
        if channel < 0 or channel > 5:
            raise AttributeError('Invalid voltage ')
        # todo - write function
        return True

    def is_valid_voltage_bound(self, voltage, channel):
        # todo - write function
        return True

    def is_valid_current(self, current, channel):
        # todo
        return True

    def is_valid_current_bound(self, current_bound, channel):
        # todo
        return True

    def is_valid_channel_string(self, channel):
        channel = str(channel)
        if channel == 'all':
            return True
        try:
            ch = int(channel)
            if 0 <= ch <= 5:
                return True
        except:
            pass
        raise AttributeError('invalid channel no %s' % channel)

    def get_channel_string(self, channel):
        # print 'get channel string:', channel
        ret_val = None
        if type(channel) is str:
            if channel.lower() == 'all':
                ret_val = '0-5'
        else:
            if not type(channel) == list:
                channel = [channel]
            valid_channels = [0 <= ch <= 5 for ch in channel]
            if not all(valid_channels):
                raise AttributeError('Invalid channel in list')
            channel = [str(x) for x in channel]
            ret_val = ','.join(channel)
        return '(@{ch})'.format(ch=ret_val)

    # ============================
    # DEVICE FUNCTIONS
    def set_output(self, status, channel=None):
        ch_str = self.get_channel_string(channel)
        if status:
            data = ':VOLT ON,' + ch_str
            return self.write(data)
        else:
            self.set_emergency_off(channel)
            return self.set_emergency_clear(channel)

    def set_channel_voltage(self, voltage, channel=-1):
        self.is_valid_channel_string(channel)
        self.is_valid_voltage(voltage, channel)
        ch_str = self.get_channel_string(channel)
        data = ':VOLT %.3f,%s' % (voltage, ch_str)
        # print 'set Voltage of channel %d to %.3f V' % (channel, voltage)
        return self.write(data)

    def set_pos_voltage(self, voltage):
        assert voltage > 0, 'You have to enter a positive voltage'
        for ch in range(3):
            self.set_channel_voltage(voltage, ch)

    def set_neg_voltage(self, voltage):
        assert voltage <= 0, 'You have to enter a negative voltage'
        for ch in range(3, 6):
            self.set_channel_voltage(voltage, ch)

    def set_emergency_off(self, channel='all'):
        self.is_valid_channel_string(channel)
        ch_str = self.get_channel_string(channel)
        print 'Emergency off for channel(s) %s' % channel
        data = ':VOLT EMCY OFF,{ch}'.format(ch=ch_str)
        return self.write(data)

    def set_emergency_clear(self, channel='all'):
        self.is_valid_channel_string(channel)
        ch_str = self.get_channel_string(channel)
        print 'Emergency clear for channel(s) %s' % channel
        data = ':VOLT EMCY CLR,{ch}'.format(ch=ch_str)
        return self.write(data)

    def set_channel_voltage_bound(self, voltage_bound, channel=-1):
        self.is_valid_channel_string(channel)
        self.is_valid_voltage_bound(voltage_bound, channel)
        data = ':VOLT:BOU %.3f (@%s)' % (voltage_bound, channel)
        return self.write(data)

    def set_channel_current(self, current, channel=-1):
        self.is_valid_channel_string(channel)
        self.is_valid_current(current, channel)
        data = ':CURR %.3f (@%d)' % (current, channel)
        print 'set Current of channel %d to %.3f V' % (channel, current)
        return self.write(data)

    def set_channel_current_bound(self, current_bound, channel=-1):
        self.is_valid_channel_string(channel)
        self.is_valid_current_bound(current_bound, channel)
        data = ':CURR:BOU %.3e (@%s)' % (current_bound, channel)
        print 'set current bound of %d to %.3e A' % (channel, current_bound)
        return self.write(data)

    def clear_channel_events(self, channel=-1):
        self.is_valid_channel_string(channel)
        data = ':EV:CLEAR (@%s)' % channel
        print 'Clear channel events @ch' % channel
        return self.write(data)

    def set_channel_event_mask(self, mask_word, channel=-1):
        self.is_valid_channel_string(channel)
        self.is_valid_channel_mask(mask_word)
        print 'set channel event mask for ch %s to "%s"' % (channel, bin(mask_word))
        data = ':EV:MASK %s' % mask_word
        return self.write(data)

    # todo :SET FASTOFF
    # todo :CONFIGURE:TRIP
    # todo :CONFIGURE:INH

    def configure_ramp_speed(self, ramp_type, speed=None):
        if speed is None:
            ramp = self.config.getint(self.section_name, 'ramp')
            speed = ramp / 20.
        else:
            speed = speed / 20.
        if len(ramp_type) > 4:
            ramp_type = ramp_type[:4]
        ramp_type = ramp_type.upper()
        assert ramp_type in ['VOLT', 'CURR'], 'Invalid Type of ramp speed'
        assert 0 < speed <= 100, 'Invalid speed for ramp speed'
        data = ':CONF:RAMP:%s %.2f' % (ramp_type, speed)
        print 'Set {type} ramp speed to {0:3.1f} {type}/s'.format(speed * 20, type=ramp_type)
        return self.write(data)

    def configure_ramp_speed_voltage(self, speed=None):
        self.configure_ramp_speed('VOLT', speed)

    def configure_ramp_speed_current(self, speed=None):
        self.configure_ramp_speed('CURR', speed)

    def configure_average(self, filterSteps):
        valid_steps = [1, 16, 64, 256, 512, 1024]
        if not filterSteps in valid_steps:
            raise AttributeError('FilterSteps must be in %s' % valid_steps)
        data = ':CONF:AVER %d' % filterSteps
        print 'Set Average Filter Steps to %d' % filterSteps
        return self.write(data)

    def get_average_filter_steps(self):
        return int(self.get_answer_for_query(':CONF:AVER?'))

    def set_kill_function(self, value):
        value = bool(value)
        data = ':CONF:KILL %s'
        if value:
            retval = 'ENABLE'
        else:
            retval = 'DISABLE'
        print '%s kill function' % retval.lower()
        return self.write(data % retval)

    def is_kill_function_active(self):
        retval = self.get_answer_for_query(':CONF:KILL?')
        return bool(retval)

    # todo :CONF:EV:CLEAR
    # todo :CONF:EV <WORD>
    # todo :CONF:EV:MASK
    # todo :CONF:EV:CHANMASK

    def read_current(self, channel=-1):
        ch = self.get_channel_string(channel)
        retVal = (self.get_answer_for_query(':MEAS:CURR?%s' % ch)).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def read_voltage(self, channel=-1):
        ch = self.get_channel_string(channel)
        retVal = (self.get_answer_for_query(':MEAS:VOLT?%s' % ch)).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def read_iv(self):
        now = time()
        if now - self.last_iv_measurement['time'] > .5:
            self.clear_buffer()
            currents = self.read_current("all")
            voltages = self.read_voltage("all")
            channels = list(range(self.nchannels))
            ivs = map(lambda x, y, z: {"voltage": x, "current": y, "channel": z}, voltages, currents, channels)
            self.last_iv_measurement = {'time': now, 'ivs': ivs}
        return self.last_iv_measurement['ivs']

    def reset(self):
        self.write('*RST')
        self.serial.readall()
        return self.serial.inWaiting()

    def clear_error_queue(self):
        self.write('*CLS')
        self.serial.readall()
        return self.serial.inWaiting()

    def clear_buffer(self, warning=True, command=''):
        busy = self.Busy
        self.Busy = True
        retval = ''
        if self.bOpen:
            while self.serial.inWaiting():
                while self.serial.inWaiting():
                    retval += self.__read()
                sleep(self.readSleepTime)
        else:
            pass
        if retval != '' and warning:
            msg = 'Buffer was not empty when reading  %s: "%s"' % (command, retval)
            msg += ',\n\t last command: "%s"' % self.last_write
            warnings.warn(msg)
        self.Busy = busy
        return self.serial.inWaiting()

    # ============================
    # ACCESS FUNCTIONS
    def wait_for_unbusy(self):
        now = time()
        while self.Busy:
            sleep(.1)
            if time() - now > 20:
                self.Busy = False
                warnings.warn('Device stucked. Waiting for more than 20sec to unbusy - Reset Busy')

    def get_answer_for_query(self, data, minlength=1):
        self.wait_for_unbusy()
        self.Busy = True
        self.clear_buffer(command=data)
        self.__write(data)
        sleep(self.readSleepTime)
        data = self.__read(minlength)
        self.Busy = False
        return self.clear_string(data)

    def write(self, data):
        self.wait_for_unbusy()
        self.Busy = True
        self.__write(data)
        self.Busy = False

    def __write(self, data):
        # print 'write: "%s"' % data
        data += self.commandEndCharacter
        self.last_write = data
        if self.bOpen:
            output = self.serial.write(data)
        else:
            output = True
        sleep(self.writeSleepTime)
        return output == len(data)

    def read(self, min_lenght=0):
        self.wait_for_unbusy()
        self.Busy = True
        self.__read(min_lenght)
        self.Busy = False

    def __read(self, min_lenght=0):
        # if not self.serial.inWaiting():
        #     print 'there is nothing in the queue'
        #     return
        out = ''
        if not self.bOpen:
            if not self.bOpenInformed:
                print 'cannot read since Not serial port is not open'
                self.bOpenInformed = False
            return ''
        ts = time()
        max_time = 300
        k = 0
        clear_first = False
        while True:
            while self.serial.inWaiting() > 0 and time() - ts < max_time and not out.endswith(self.commandEndCharacter):
                out += self.serial.read(1)
                k += 1
            if out == '':
                break
            if out.endswith(self.commandEndCharacter):
                if out.startswith('*') or out.startswith(':'):
                    out = ''
                    continue
                else:
                    break
            if time() - ts > max_time:
                break
            if 0 < min_lenght <= len(out):
                break
            sleep(self.readSleepTime)
        if time() - ts > max_time:
            print "Tried reading for %s seconds." % (time() - ts), out
            try:
                print ord(out[-2]), ord(out[-1]), ord(self.commandEndCharacter[0]), ord(self.commandEndCharacter[1])
            except IndexError:
                print "Error trying: 'print ord(out[-2]),ord(out[-1])," \
                      "ord(self.commandEndCharacter[0]),ord(self.commandEndCharacter[1]),len(out)'"
            return ''
        # print 'received after %s tries: %s' % (k, out)
        return out

    # ============================
    # GET-FUNCTIONS
    def get_model_name(self):
        self.model = 9999
        if self.identifier == '':
            self.identify()
        else:
            ident_list = self.identifier.split()
            if len(ident_list) > 5:
                mod = ident_list[3] + ident_list[4]
                self.model = int(mod) if self.is_number(mod) else mod
            print 'Connected iseg model', self.model
        self.set_max_voltage()

    def get_channel_voltage(self, ch=-1):
        return self.query_set_voltage(ch)

    def get_output_status(self, ch=None):
        valid_output_status = False
        retval = None
        while not valid_output_status:
            try:
                try:
                    channel_status = self.get_channel_status(ch)
                except Exception as e:
                    print e, e.args
                    raise Exception('get_output_status: Cannot get channel status,%s' % e)
                try:
                    l_ch_status = list(channel_status)
                except Exception as e:
                    print 'conversion', e, e.args
                    raise Exception('get_output_status: Cannot convert channel status to list "%s"' % channel_status)
                try:
                    retval = [k['On'] for k in l_ch_status]
                except Exception as e:
                    print e, e.args, l_ch_status
                    raise Exception('get_output_status: Cannot extract channel status from list "%s"' % l_ch_status)
                valid_output_status = True
            except Exception as e:
                print e
            if not valid_output_status:
                print 'invalid output status'
            else:

                pass
                # print 'status:',ch,retval
        return retval

    def query_set_voltage(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:VOLT?%s' % ch_str).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def query_voltage_limit(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:VOLT:LIM?%s' % ch_str).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def query_voltage_nominal(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:VOLT:NOM?%s' % ch_str).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def query_voltage_bounds(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:VOLT:BOU?%s' % ch_str).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def query_channel_on(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:VOLT:ON?%s' % ch_str).split()
        retVal = [bool(k) for k in retVal]
        return retVal

    def query_emergency_bit(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:VOLT:EMCY?%s' % ch_str).split()
        retVal = [bool(k) for k in retVal]
        return retVal

    def query_set_current(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:CURR?%s' % ch_str).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def query_set_current_limit(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:CURR:LIM?%s' % ch_str).split()
        retVal = [float(k[:-1]) for k in retVal]
        return retVal

    def query_set_current_nominal(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retval = self.get_answer_for_query(':READ:CURR:NOM?%s' % ch_str).split()
        retval = [float(k[:-1]) for k in retval]
        return retval

    def query_set_current_bounds(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retval = self.get_answer_for_query(':READ:CURR:BOU?%s' % ch_str).split()
        retval = [float(k[:-1]) for k in retval]
        return retval

    '''
        Unit: %/s TODO
    '''

    def query_module_voltage_ramp_speed(self):
        retval = self.get_answer_for_query(':READ:RAMP:VOLT?')
        retval = float(retval[:-3])
        return retval

    '''
        Unit: V/s
    '''

    def query_channel_voltage_ramp_speed(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retval = self.get_answer_for_query(':READ:RAMP:VOLT?%s' % ch_str).split()
        retval = [float(k[:-3]) for k in retval]
        return retval

    '''
        Unit: %/s TODO
    '''

    def query_module_current_ramp_speed(self):
        retval = self.get_answer_for_query(':READ:RAMP:CURR?')
        retval = float(retval[:-3])
        return retval

    '''
        Unit: A/s
    '''

    def query_channel_current_ramp_speed(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retval = self.get_answer_for_query(':READ:RAMP:CURR?%s' % ch_str).split()
        retval = [float(k[:-3]) for k in retval]
        return retval

    def query_module_supply_voltage_p24(self):
        return float(self.get_answer_for_query(':READ:MOD:SUP:P24V?')[:-1])

    def query_module_supply_voltage_n24(self):
        return float(self.get_answer_for_query(':READ:MOD:SUP:N24V?')[:-1])

    def query_module_supply_voltage_p5(self):
        return float(self.get_answer_for_query(':READ:MOD:SUP:P5V?')[:-1])

    def query_module_temperature(self):
        return float(self.get_answer_for_query(':READ:MOD:TEMP?')[:-1])

    def query_module_channels(self):
        return int(self.get_answer_for_query(':READ:MOD:CHAN?'))

    def query_firmware_name(self):
        return self.get_answer_for_query(':READ:FIRM:NAME?')

    def query_firmware_release(self):
        return self.get_answer_for_query(':READ:FIRM:REL?')

    def get_all_channel_status(self):
        now = time()
        # delta_t = now - self.last_channel_status['time']
        # print 'get all channel status, delta t ', delta_t,
        if now - self.last_channel_status['time'] > 1:
            ch_str = self.get_channel_string('all')
            valid_answer = False
            ret_val = None
            while not valid_answer:
                try:
                    ret_val = self.get_answer_for_query(':READ:CHAN:STAT?%s' % ch_str).split()
                    ret_val = [int(k) for k in ret_val]
                    ret_val = [self.convert_channel_status(i) for i in ret_val]
                    valid_answer = True
                except:
                    print 'no valid channel status, retry'
                    self.clear_buffer(warning=False)
                    pass
            if self.last_channel_status['status']:
                # print time(),'valid,status',len(self.last_channel_status['status'])
                pass
            else:
                # print time(),'valid,status'
                pass
            self.last_channel_status['status'] = ret_val
            self.last_channel_status['time'] = now
            # print 'updated status'
        else:
            pass
            # print 'do not update status'
        return self.last_channel_status['status']

    def get_channel_status(self, ch=None):
        self.get_all_channel_status()
        while not self.last_channel_status['status'] or len(self.last_channel_status['status']) == 0:
            self.get_all_channel_status()
        try:
            if type(ch) == int:
                return [self.last_channel_status['status'][ch]]
            if type(ch) == list:
                return [self.last_channel_status['status'][i] for i in ch]
            if ch == 'all':
                return self.last_channel_status['status']
            ch = ch.split()
            return [self.last_channel_status['status'][int(k)] for k in ch]
        except Exception as e:
            # if type(self.last_channel_status['status']) == list:
                # print 'len', len(self.last_channel_status['status']),

            return self.last_channel_status['status']

    ''' Channel Event Status (read-write access)
            :READ:CHANnel:EVent:STATus?
    An event bit is permanently set if the status bit is '1' or is changing to '1'. Different to the status bit an
    event bit isn't automatically reset. A reset has to be done by the user by writing '1' to this event bit.
    '''

    def get_channel_event_status(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:CHAN:EV:STAT?%s' % ch_str).split()
        retVal = [int(k) for k in retVal]
        retVal = [self.convert_channel_event_status(i) for i in retVal]
        return retVal

    ''' Channel Event Mask (write access, read access)
        :CONF:EVent:MASK?
        :READ:CHANnel:EVent:MASK?
    '''

    def get_channel_event_mask(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:CHAN:EV:MASK?%s' % ch_str).split()
        retVal = [int(k) for k in retVal]
        retVal = [self.convert_channel_event_mask(i) for i in retVal]
        return retVal

    ''' Channel Control (read access)
    The signals SetOn and SetEmergencyOff control are basic functions of the channel. The signal SetOn
    is switching ON the HV of the channel and is a precondition for giving voltage to the output. As far as
    a VoltageSet has been set and no event has occurred and is not registered yet (in minimum, bit 10 to
    15 of the register Channel Event Status must be 0), a start of a HV ramp will be synchronized (a ramp
    is a software controlled, time proportionally increase / decrease of the output voltage ).
    '''

    def get_channel_control(self, ch=-1):
        ch_str = self.get_channel_string(ch)
        retVal = self.get_answer_for_query(':READ:CHANnel:CONTrol?%s' % ch_str).split()
        retVal = [int(k) for k in retVal]
        retVal = [self.convert_channel_control(i) for i in retVal]
        return retVal

    ''' Module Status (read access)
        :READ:MODule:STATus?
        The status bits as there are IsTemperatureGood, IsSupplyGood, IsModuleGood, IsEventActive,
        IsSafetyLoopGood, IsNoRamp and IsNoSumError indicate the single status for the complete module.
    '''

    def get_module_status(self):
        retVal = int(self.get_answer_for_query(':READ:MODule:STATus?'))
        return self.convert_module_status(retVal)

    ''' Module Event Status (read access)
        :READ:MODule:EVent:STATus?
    '''

    def get_module_event_status(self):
        retVal = int(self.get_answer_for_query(':READ:MODule:EV:STATus?'))
        return self.convert_module_event_status(retVal)

    ''' Module Event Mask (read-write access)
        :CONF:EVent:MASK?
        :READ:MODule:EVent:MASK?
    All bits of the EventMask register are set to '1' after the power on reset.
    Module in mode KILL enable: If a bit of the EventStatus register is set to '1' and the corresponding bit in the
    EventMask register is '0' no reset of the EventStatus bits is necessary before switch on the HV of any channel again.
    If a bit of the EventMask register is set to '1' and if the corresponding bit in the EventStatus register is set to '1'
    by the module, a reset of the corresponding EventStatus bits is necessary before a switch on of any channel is
    possible
    '''

    def get_module_event_mask(self):
        retVal = int(self.get_answer_for_query(':READ:MODule:EV:MASK?'))
        return self.convert_module_event_mask(retVal)

    ''' Module Event Channel Status (read-write access)
        :READ:MODule:EVent:CHANSTATus?
    The n-th bit of the register is set, if an event is active in the n-th channel and the associated bit in the EventMask
    register of the n-th channel is set too.
    CHn = EventStatus[n] & EventMask[n]
    Reset of a bit is done by writing a 1 to this bit
    '''

    def get_module_event_channel_status(self):
        retVal = int(self.get_answer_for_query(':READ:MODule:EVent:CHANSTATus?'))
        # TODO
        return bin(retVal)

    ''' Module Control (read-write access)
        :READ:MODule:CONTRol?
    '''

    def get_module_control(self):
        retVal = int(self.get_answer_for_query(':READ:MODule:CONTRol?'))
        return self.convert_module_control(retVal)

    # ============================
    # SET-FUNCTIONS
    def set_max_voltage(self):
        # TODO
        self.max_voltage = 3000

    def set_voltage(self, value, chan=None):
        return self.set_channel_voltage(value, chan)

    def set_bias(self, voltage, chan=None):
        return self.set_channel_voltage(voltage, chan)

    # ============================
    # STATIC CONVERSION FUCNTIONS
    @staticmethod
    def check_bit(status, bit):
        return (status & (1 << bit)) == (1 << bit)

    @staticmethod
    def convert_channel_status(status):
        # Bit15  Bit14   Bit13   Bit12 Bit11 Bit10 Bit9 Bit8 Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
        retVal = {
            "VoltageLimitExceeded": ISEG.check_bit(status, 15),  # 0 for OK, 1 the hardware voltage limit is exceeded
            "CurrentLimitExceeded": ISEG.check_bit(status, 14),  # 0 for OK, 1 the hardware current limit is exceeded
            "TripExceeded": ISEG.check_bit(status, 13),
            # 0 for OK, 1 VO is shut off to 0V without ramp because the channel has tripped.
            "ExtInhibit": ISEG.check_bit(status, 12),  # 0 for OK, 1 External Inhibit was scanned
            "VoltageBoundsExceeded": ISEG.check_bit(status, 11),  # 0 for OK, 1 |Vmeas - Vset| > Vbounds
            "CurrentBoundsExceeded": ISEG.check_bit(status, 10),  # 0 for OK, 1 |Imeas - Iset| > Ibounds
            "Reserved2": ISEG.check_bit(status, 9),
            "LowCurrentRange": ISEG.check_bit(status, 8),  #
            "ControlledVoltage": ISEG.check_bit(status, 7),  # 1 Channel is in state of voltage control
            "ControlledCurrent": ISEG.check_bit(status, 6),  # 1 channel is in state of current contro
            "EmergencyOff": ISEG.check_bit(status, 5),
            # 1 channel is in state of emergency off, VO has been shut off to 0V without ramp
            "Ramping": ISEG.check_bit(status, 4),
            # 0, no voltage is in change, 1 voltage is in change with the stored ramp speed value
            "On": ISEG.check_bit(status, 3),  # 0 channel is off, 1  channel voltage follows the Vset value
            "InputError": ISEG.check_bit(status, 2),  # 0 no input-error, 1 incorrect message to control the module
            "IsPositive": ISEG.check_bit(status, 0),  # 0 negative polarity, 1 positive polarity
            "Reserved": ISEG.check_bit(status, 1),

        }
        # VLIM   CLIM    TRP     EINH VBND CBND res LCR CV CC EMCY RAMP ON IERR res POS
        return retVal

    @staticmethod
    def convert_channel_event_status(status):
        bits = ["Reserved",  # 0
                "Reserved1",  # 1
                "EventInputError",  # 2
                "EventOnToOff",  # 3
                "EventEndOfRamp",  # 4
                "EventEmergencyOff",  # 5
                "EventControlledCurrent",  # 6
                "EventControlledVoltage",  # 7
                "Reserved2",  # 8
                "Reserved3",  # 9
                "EventCurrentBounds",  # 10
                "EventVoltageBounds",  # 11
                "EventExtInhibit",  # 12
                "EventTrip",  # 13
                "EventCurrentLimit",  # 14
                "EventVoltageLimit",  # 15
                ]
        retVal = {}
        for i in range(len(bits)):
            if bits[i].startswith('Reserved'):
                continue
            retVal[bits[i]] = ISEG.check_bit(status, i)
        return retVal

    @staticmethod
    def convert_channel_event_mask(status):
        bits = ["Reserved",  # 0
                "Reserved1",  # 1
                "MaskEventInputError",  # 2
                "MaskEventOnToOff",  # 3
                "MaskEventEndOfRamp",  # 4
                "MaskEventEmergencyOff",  # 5
                "MaskEventControlledCurrent",  # 6
                "MaskEventControlledVoltage",  # 7
                "Reserved2",  # 8
                "Reserved3",  # 9
                "MaskEventCurrentBounds",  # 10
                "MaskEventVoltageBounds",  # 11
                "MaskEventExtInhibit",  # 12
                "MaskEventTrip",  # 13
                "MaskEventCurrentLimit",  # 14
                "MaskEventVoltageLimit",  # 15
                ]
        retVal = {}
        for i in range(len(bits)):
            if bits[i].startswith('Reserved'):
                continue
            retVal[bits[i]] = ISEG.check_bit(status, i)
        return retVal

    @staticmethod
    def convert_channel_control(status):
        bits = ["Reserved",  # 0
                "Reserved1",  # 1
                "Reserved2",  # 2
                "SetOn",  # 3
                "Reserved3",  # 4
                "SetEmergencyOff",  # 5
                "Reserved6",  # 6
                "Reserved7",  # 7
                "Reserved8",  # 8
                "Reserved9",  # 9
                "Reserved10",  # 10
                "Reserved11",  # 11
                "Reserved12",  # 12
                "Reserved13",  # 13
                "Reserved14",  # 14
                "Reserved15",  # 15
                ]
        retVal = {}
        for i in range(len(bits)):
            if bits[i].startswith('Reserved'):
                continue
            retVal[bits[i]] = ISEG.check_bit(status, i)
        return retVal

    @staticmethod
    def convert_module_status(status):
        bits = ["IsFineAdjustment",  # 0  0: Fine adjustment is off.
                "Reserved",  # 1
                "Reserved2",  # 2
                "Reserved3",  # 3
                "IsService",  # 4          1: Hardware failure detected
                "isHwVLgd",  # 5
                "IsInputError",  # 6       0: no input error in connection with a module access
                "Reserved7",  # 7
                "IsNoSumError",
                # 8       1: evaluation of the "Channel Status" over all channels to a sum error flag ==> no errors
                "IsNoRamp",  # 9           1: no channel is ramping
                "IsSafetyLoopGood",  # 10  1: safety loop is closed
                "IsEventActive",  # 11     1: any Event is active
                "IsModuleGood",
                # 12      1: module is good, that means (isnoSERR AND NOT(ETMPngd OR ESPLYngd ORESFLPngd))==1
                "IsSupplyGood",  # 13      1: supply voltages are within range
                "IsTemperatureGood",  # 14 1: module temperature is within working range
                "IsKillEnable",  # 15  -   0: Module in state kill disable
                ]
        retVal = {}
        for i in range(len(bits)):
            if bits[i].startswith('Reserved'):
                continue
            retVal[bits[i]] = ISEG.check_bit(status, i)
        return retVal

    @staticmethod
    def convert_module_event_status(status):
        bits = ["Reserved0",  # 0
                "Reserved1",  # 1
                "Reserved2",  # 2
                "Reserved3",  # 3
                "EventService",  # 4
                "EventHardwareVoltageLimitNotGood",  # 5
                "EventInputError",  # 6
                "Reserved7",  # 7
                "Reserved8",  # 8
                "Reserved9",  # 9
                "EventTemperatureNotGood",  # 10
                "Reserved11",  # 11
                "Reserved12",  # 12
                "EventSupplyNotGood",  # 13
                "EventSafetyLoopNotGood",  # 14
                "Reserved15",  # 15
                ]
        retVal = {}
        for i in range(len(bits)):
            if bits[i].startswith('Reserved'):
                continue
            retVal[bits[i]] = ISEG.check_bit(status, i)
        return retVal

    @staticmethod
    def convert_module_event_mask(status):
        bits = ["Reserved0",  # 0
                "Reserved1",  # 1
                "Reserved2",  # 2
                "Reserved3",  # 3
                "Reserved4",  # 4
                "MaskEventHardwareVoltageLimitNotGood",  # 5
                "MaskEventInputError",  # 6
                "Reserved7",  # 7
                "Reserved8",  # 8
                "Reserved9",  # 9
                "MaskEventSafetyLoopNotGood",  # 10
                "Reserved11",  # 11
                "Reserved12",  # 12
                "MaskEventSupplyNotGood",  # 13
                "MaskEventTemperatureNotGood",  # 14
                "Reserved15",  # 15
                ]
        retVal = {}
        for i in range(len(bits)):
            if bits[i].startswith('Reserved'):
                continue
            retVal[bits[i]] = ISEG.check_bit(status, i)
        return retVal

    @staticmethod
    def convert_module_control(status):
        bits = ["Reserved0",  # 0
                "Reserved1",  # 1
                "Reserved2",  # 2
                "Reserved3",  # 3
                "Reserved4",  # 4
                "Reserved5",  # 5
                "ClearKill",  # 6 - Hardware ClearKill signal and clear all event signals of the module and the channels
                "Reserved7",  # 7
                "Reserved8",  # 8
                "Reserved9",  # 9
                "Reserved10",  # 10
                "Endian",  # 11  Order of bytes in word: 0 = Little Endian (INTEL); 1 = Big Endian (MOTOROLA)
                "Adjust",  # 12 Switch ON of fine adjustment
                "Reserved13",  # 13
                "KillEnable",  # 14
                "Reserved15",  # 15
                ]
        retVal = {}
        for i in range(len(bits)):
            if bits[i].startswith('Reserved'):
                continue
            retVal[bits[i]] = ISEG.check_bit(status, i)
        return retVal

    def get_list_of_active_channels(self):
        mask = self.config.getint(self.section_name, 'active_channels')
        n_channels = self.query_module_channels()
        retval = []
        for i in range(n_channels):
            if (mask & (1 << i) == (1 << i)):
                retval.append(i)
        return retval


if __name__ == '__main__':
    conf = ConfigParser.ConfigParser()
    conf.read('../config/keithley.cfg')
    d = ISEG(conf, 7, False)
