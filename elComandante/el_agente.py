## @file
## Implements the base class for all elAgentes
## @ingroup elComandante

## Base class for all elAgentes
##
## The class defines the standard interface to elComandante that all
## agents (el_agente/los_agentes) have to implement.
## @ingroup elComandante
class el_agente():
    ## Constructor
    ##
    ## The constructor sets the initial state of the agente
    ## by using the arguments specified.
    ## @param timestamp The timestamp that elComandante has generated
    ## @param log The log handle from elComandante
    ## @param sclient The subsystem client handle from elComandante
    def __init__(self, timestamp, log, sclient):
        ## The timestamp at which elComandante started execution
        self.timestamp = timestamp
        ## Log handle
        self.log = log
        ## Subsystem client handle
        self.sclient = sclient
        ## Flag that specifies whether the agente is enabled or not
        self.active = False
        ## Flag that specifies whether the agente is busy or not
        self.pending = False
        ## Name of the agente
        self.agente_name = "el_agente"
        ## Name of the client
        self.client_name = "client"
        ## Subsystem channel the agente communicates on
        self.subscription = "/el_agente"
        ## Directories to find files
        self.Directories={}
        ## Test that should be executed through the agente
        self.test = None

    ## Allows the agente to read the configuration files of elComandante
    ##
    ## The agente receives the configuration handle and is encuraged to
    ## read the parameters that are relevant to it
    ## @param conf Handle of elComandante's configuration
    ## @return Boolean, whether the operation was successful or not
    def setup_configuration(self, conf):
        return True

    ## Sends a message through the subsystem
    def send(self,message):
        try:
            self.sclient.send(self.subscription,message)
        except:
            pass

    ## Allows the agente to read the initialization files of elComandante
    ##
    ## The agente receives the initialization handle and is encuraged to
    ## read the parameters that are relevant to it
    ## @param init Handle of elComandante's initialization
    ## @return Boolean, whether the operation was successful or not
    def setup_initialization(self, init):
        return True

    ## Allows the agente to setup its own directories
    ##
    ## The agente may need to create directories. For this the base
    ## directories are handed over to it through the parameter.
    ## @param Directories Dictionary of base directories from elComandante
    ## @return None
    def setup_dir(self,Directories):
        self.Directories = Directories

    ## Checks whether the log files of the agente are present in the filesystem
    ##
    ## Some log files of the agente might already be present in the filesystem
    ## due to a previous program crash for example. This function should return
    ## a list of these
    ## @return List of present log files
    def check_logfiles_presence(self):
        # Returns a list of logfiles present in the filesystem
        return []

    ## Checks whether the client that the agente is supervising is running
    ##
    ## The agente should use some mean (such as PID files) to determine
    ## whether its client is running or not.
    ## @return Boolean, whether it is running (True) or not (False)
    def check_client_running(self):
        # Check whether a client process is running
        return False

    ## Starts the client of the agente
    ##
    ## The agente must be able to start its client. It can use the configuration
    ## and initialization to accomplish this (setting the right command line
    ## parameters).
    ## @param timestamp Timestamp from elComandante that should be passed to
    ## the client to guarantee that all the timestamps are the same
    ## @return Boolean, whether the starting was successful (True) or not (False)
    def start_client(self, timestamp):
        self.timestamp = timestamp
        # Start a client process
        return False

    ## Makes the agente subscribe to its subsystem channel
    ##
    ## Makes the agente subscribe to its subsystem channel if
    ## the agente is enabled
    ## @return None
    def subscribe(self):
        if (self.active):
            self.sclient.subscribe(self.subscription)

    ## Checks the subsystem subscription
    ##
    ## Checks whether there is a response from the subsystem
    ## server unless the agente is not enabled.
    ## @return: Boolean, whether the subscription works (True)
    ## or not (False). If the agente is disabled, it should return
    ## True.
    def check_subscription(self):
        # Verify the subsystem connection
        if (self.active):
            return self.sclient.checkSubscription(self.subscription)
        return True

    ## Request the client to exit with a command through subsystem
    ##
    ## Sends a command to the client through the subsystem to ask the
    ## client to exit
    ## @return Boolean, whether the command was received successfully
    ## (True) or not (False)
    def request_client_exit(self):
        return False

    ## Kills the client by using the SIGTERM signal
    ##
    ## Attempts to kill the client using SIGTERM, giving
    ## it still a chance to clean up.
    ## @return Boolean, whether the killing was successful (True)
    ## or not (False)
    def kill_client(self):
        return False

    ## Sets the current test which should be executed
    ##
    ## For preparation, execution, and clean up of a test
    ## the test has to be specified.
    ## @param test The test to be used next
    ## @return None
    def set_test(self, test):
        self.test = test

    ## Prepares the current test
    ##
    ## The agente may need to prepare something
    ## before the test can actually be executed
    ## (for example: cooling down the setup or
    ## turning on x-rays)
    ## @param test The test to be prepared (deprecated)
    ## @param environment The test environment specification
    ## that the agente can chose to look at
    ## @return Boolean, whether it was successful or not
    def prepare_test(self, test, environment):
        # Run before a test is executed
        return False

    ## Execates the current test
    ##
    ## Makes the agente send the correct commands to its
    ## client to execute the current test.
    ## @return Boolean, whether it was successful or not
    def execute_test(self):
        # Initiate a test
        return False

    ## Cleanup the current test
    ##
    ## Makes the agente send the correct command to its
    ## client to cleanup the current test.
    ## @return Boolean, whether it was successful or not
    def cleanup_test(self):
        # Run after a test has executed
        return False

    ## Final cleanup after all tests
    ##
    ## Cleanup after all tests have finished to return
    ## everything to the state before the tests or to a
    ## safe state
    ## @return Boolean, whether it was successful or not
    def final_test_cleanup(self):
        return False

    ## Checks whether the current operation of the client
    ## is finished or not
    ##
    ## Check whether the client has finished its task
    ## but also check for errors and raise an exception
    ## if one occurs.
    ## @return Boolean, whether it if finished (True) or not
    ## (False)
    def check_finished(self):
        return not self.pending
