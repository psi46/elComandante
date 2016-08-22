# Base class for all High Voltage interfaces

# ============================
# IMPORTS
# ============================
from string import maketrans

# ============================
# CONSTANTS
# ============================
ON = 1
OFF = 0


# ============================
# MAIN CLASS
# ============================
class HVInterface:
    def __init__(self, config, device_no, hot_start):
        self.nchannels = 1
        self.hot_start = hot_start
        self.device_no = device_no
        self.target_voltage = 0
        self.config = config
        self.section_name = 'HV%d' % self.device_no
        if self.config.has_option(self.section_name, 'module_name'):
            self.module_name = self.config.get(self.section_name, 'module_name')
        else:
            self.module_name = ''
        self.model_number = self.config.get(self.section_name, 'model')
        self.name = self.config.get(self.section_name, 'name')
        self.model = ''
        # ramping
        self.can_ramp = False
        self.ramp_fast = False
        self.started_ramping = False
        pass

    def get_n_channels(self):
        return self.nchannels

    def set_to_manual(self, status):
        raise Exception("set_to_manual not yet implemented")

    def set_output(self, status):
        raise Exception("set_output not yet implemented")

    def set_bias(self, voltage):
        raise Exception("set_bias not yet implemented")

    def get_output(self):
        raise Exception("get_output not yet implemented")

    def read_current(self):
        raise Exception("read_current not yet implemented")

    def read_voltage(self):
        raise Exception("read_voltage not yet implemented")

    def read_iv(self):
        raise Exception("read_iv not yet implemented")

    def set_voltage(self, value):
        return self.set_bias(value)

    def set_on(self):
        return self.set_output(ON)

    def set_off(self):
        return self.set_output(OFF)

    def get_set_voltage(self):
        return self.target_voltage

    def get_target_voltage(self):
        return self.target_voltage

    def get_device_name(self, log=0):
        space = ("_" if log else " ")
        out = self.section_name + space + self.name + space + self.model_number
        return out

    def get_model_name(self):
        print 'blasd'
        pass

    @staticmethod
    def is_number(s):
        try:
            int(s)
            return True
        except ValueError:
            return False

    @staticmethod
    def is_float(s):
        try:
            float(s)
            return True
        except ValueError:
            return False

    @staticmethod
    def clear_string(data):
        data = data.translate(None, '\r\n\x00\x13\x11\x10')
        data = data.translate(maketrans(',', ' '))
        return data.strip()
