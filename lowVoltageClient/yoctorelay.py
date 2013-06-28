## @file
## Implements the specific low voltage device YoctoRelay from Yoctopuce
## @ingroup lowVoltageClient

import os, sys, time
sys.path.append(os.path.join("yoctopuce","Sources"))
print sys.path
from yocto_api import *
from yocto_relay import *

from lowVoltageDevice import lowVoltageDevice

## Implements the specific low voltage device YoctoRelay from Yoctopuce
##
## The YoctoRelay is just an electronic switch that works together with
## an ordinary (non-programmable) power supply. It uses a USB connection
## to communicate. It cann only switch between state A to state B. The
## default state is A, which should be ON.
class yoctorelay(lowVoltageDevice):
    def __init__(self, device_id):
        lowVoltageDevice.__init__(self)
        self.device_id = device_id
    ## Connects to the YoctoRelay device
    ##
    ## The first YoctoRelay device found is used. The USB device permissions
    ## have to be set correctly in order for this to succeed
    ## @return Boolean, whether the connection to the YoctoRelay device was made
    ## or not
    def connect(self):
        errmsg=YRefParam()
        if YAPI.RegisterHub("usb", errmsg)!= YAPI.SUCCESS:
            return False
        if self.device_id.lower() == "any":
            self.relay = YRelay.FirstRelay()
        else:
            self.relay = YRelay.FindRelay(self.device_id)
        if self.relay is None:
            return False
        if not(self.relay.isOnline()):
            return False
        if self.device_id.lower() == "any":
            print "    |  Connected to " + self.relay.get_hardwareId()
        return True

    ## Gets the state of the YoctoRelay to determine communication
    ##
    ## Sends a command to the YoctoRelay device to ask for its state.
    ## If it answers, communication is established.
    ## @return Boolean, whether communication is established or not
    def test_communication(self):
        state = self.relay.get_state()
        if state == YRelay.STATE_A or state == YRelay.STATE_B:
            return True
        return False

    ## Gets the state of the YoctoRelay device
    ##
    ## Communicates with the YoctoRelay to determine whether the
    ## relay (switch) is open (OFF) or closed (ON).
    ## @return String, whether the relay is "ON" or "OFF"
    def get_output(self):
        state = self.relay.get_state()
        if state == YRelay.STATE_A:
            return "ON"
        else:
            return "OFF"

    ## Sets the state of the YoctoRelay device
    ##
    ## Sends a command to the YoctoRelay device to set the
    ## relay state.
    ## @param val "ON" for turning power on, "OFF" for turning power off
    ## @return Boolean, whether the operation was successful or not
    def set_output(self, val):
        if val.upper() == "ON":
            state = YRelay.STATE_A
        elif val.upper() == "OFF":
            state = YRelay.STATE_B
        else:
            raise Exception("Invalid state given!")
        self.relay.set_state(state)
        return True

    ## Toggles the state of the YoctoRelay
    ##
    ## Gets the current state of the YoctoRelay and sets
    ## the opposite state.
    ## @return Boolean, whether the operation was successful or not
    def toggle_output(self):
        # Get current state
        state = self.get_output()

        # Invert the state
        if state == "ON":
            state = "OFF"
        elif state == "OFF":
            state = "ON"

        # Set the new state
        self.set_output(state)
        return True

    ## Toggles the YoctoRelay off and on again
    ##
    ## Sets the YoctoRelay to an open position (OFF) and after a short
    ## moment closes the switch again (ON)
    ## @return Boolean, whether the operation was successful or not
    def power_cycle(self):
        self.set_output("OFF")
        time.sleep(2)
        self.set_output("ON")
        return True
