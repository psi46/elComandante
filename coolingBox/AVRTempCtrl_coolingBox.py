import coolingBox

class AVRTempCtrl_coolingBox(coolingBox):
    def __init__(self, device_filename):
        coolingBox.__ini__(self)
        self.serialfile = serial.Serial(port = device_filename, baudrate = 19200)

    def is_cooling(self):
        response = self.serial_send(":GET:STATE:CTRL")
        return response == "COOL"

    def is_heating(self):
        response = self.serial_send(":GET:STATE:CTRL")
        return response == "HEAT"

    def is_flushing(self):
        return False

    def is_unkown(self):
        return False

    def is_final_heating(self):
        return False

    def start_controlling(self):
        response = self.serial_send(":SET:STATE:CTRL COOL")
        return response == "OK"

    def stop_controlling(self):
        response = self.serial_send(":SET:STATE:CTRL OFF")
        return response == "OK"

    def set_setpoint(self, setpoint):
        if setpoint < -30 or setpoint > 18.5)
            return False
        response = self.serial_send(":SET:SETPOINT %5.2f" % setpoint)
        return response == "OK"

    def get_temperature(self):
        response = self.serial_send(":GET:TEMPERATURE0")
        try:
            temperature = float(response)
        except ValueError:
            temperature = None
        return temperature

    def get_relative_humidity(self):
        response = self.serial_send(":GET:HUMIDITY0")
        try:
            humidity = float(response[:-2])
        except ValueError:
            humidity = None
        return humidity

    def do_cycle(self,nCycles,**kwargs): # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        return False

    def is_dry(self):
        response = self.serialfile.write(":GET:STATE:INTERLOCK")
        return response == "SAFE"

    def is_stable(self):
        response = self.serialfile.write(":GET:STEADY")
        return response == "YES"

    def stablize():
        pass

    def check_cycles():
        pass

    def final_heating():
        return False

    def get_dew_point(self):
        response = self.serialfile.write(":GET:DEWPOINT")
        try:
            dewpoint = float(response)
        except ValueError:
            dewpoint = None
        return dewpoint

    def serial_send(self, command):
        if len(command) > 0 and command[-1] != '\n':
                cmd = cmd + "\n"
        #seriallock.acquire()
        self.serialfile.write(cmd)
        response = self.serialfile.readline()
        #seriallock.release()
        return response.strip()
