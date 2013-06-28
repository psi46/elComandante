## @file
## Implements the class which defines the basic interface to
## low voltage devices
## @ingroup lowVoltageClient

## Implements the basic interface to a low voltage device
##
## In order to support multiple different low voltage devices
## it is necessary to define an interface that is common to all
## of them. This is a base class which has all the functions
## that any low voltage device implementation is required to
## define to be used in elComandante. To implement a specific device
## a class has to be written which inherits from this class and replaces
## all the functions with actual code that communicates with the
## device. If a function is not replaced it will cause an exception.
## @ingroup lowVoltageClient
class lowVoltageDevice():
    ## Constructor of the class, doing nothing
    def __init__(self):
        pass

    ## Function that starts a connection to the device
    ##
    ## This function is supposed to be making a connection (through
    ## USB or serial interface for example) to the physical low voltage
    ## device.
    ## @return Boolean, whether the connection succeeded or not
    def connect(self):
        raise Exception("connect() function not implemented")

    ## Function that tests the communication with the device
    ##
    ## This function should attempt communication with the device,
    ## making a query with a known answer.
    ## @return Boolean, whether the connection works or not
    def test_communication(self):
        raise Exception("test_communication() function not implemented")

    ## Function that reads the state of the low voltage device
    ## output
    ##
    ## This function should communicate with the device to determine
    ## the state of the device output
    ## @return Boolean, wheter the output is on or off
    def get_output(self):
        raise Exception("output() function not implemented")

    ## Function that sets the state of the low voltage device
    ## output
    ##
    ## This function should communicate with the device to set the
    ## device output on or off
    ## @param val The state as a string, to which the output should change, either
    ## "ON" or "OFF"
    ## @return Boolean, whether the operation was successful or not
    def set_output(self, val):
        raise Exception("output() function not implemented")

    ## Function that toggles the low voltage device's output
    ##
    ## This function should communicate with the device to determine
    ## the output state and then send a command to change that state
    ## to the opposite
    ## @return Boolean, whether the operation was successful or not
    def toggle_output(self):
        raise Exception("state() function not implemented")

    ## Function that power cycles the low voltage device's output
    ##
    ## This function should communicate with the low voltage device
    ## and set the output off (if it's on) and then after a short
    ## moment, turn it on again. At the end of this operation the
    ## output has to be on.
    ## @return Boolean, whether the operation was successful or not
    def power_cycle(self):
        raise Exception("power_cycle() function not implemented")
