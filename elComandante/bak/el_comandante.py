#!/usr/bin/python2
from BetterConfigParser import BetterConfigParser
from sclient import *
from decode import *
from time import strftime, gmtime
import time
from shutil import copytree,rmtree
from testboardclass import Testboard as Testboarddefinition
import os,sys
import subprocess
import argparse
from colorprinter import printer

import utils
import config_utils
import subsystem_utils
import interface
import climate_chamber

signal.signal(signal.SIGINT, utils.handler)

try:
    timestamp = int(time.time())

    args = utils.parse_command_line_arguments()
    config_utils.check_config_dir_access(args.configDir)
    config = config_utils.load_config(args.configDir + "/elComandante.conf")
    init = config_utils.load_init(args.configDir + "/elComandante.ini")
    Directories = config_utils.get_config_directories(config, args.configDir)

    utils.create_directory(Directories["dataDir"])
    utils.create_directory(Directories["subserverDir"])

    # FIXME: Make this more concise
    try:
        logFiles = (os.listdir(Directories["logDir"]))
        nLogFiles = len(logFiles)
        print nLogFiles
    except:
        os.mkdir(Directories["logDir"])
        print "mkdir"
    else:
        print nLogFiles
        if nLogFiles>0:
            answer = raw_input("Do you want to overwrite \"%s\"? [y]es or [n]o\n\t" % logFiles)
            if "y" in answer.lower():
                rmtree(Directories["logDir"])
                os.mkdir(Directories["logDir"])
            else:
                raise Exception("LogDir is not empty. Please clean logDir: %s" % Directories["logDir"])

    utils.initialize_logger(timestamp, Directories['logDir'])

    subsystem_utils.start_subserver(Directories['subserverDir'])
    subsystem_utils.get_subsystem_settings(config)
    subsystem_utils.create_subsystem_client()
    subscriptionList = subsystem_utils.make_subscriptions()

    # Welcome message
    utils.Logger.printw()

    testlist = config_utils.get_list_of_tests(init)

    def setupParentDir(timestamp,Testboard):
            Testboard.parentDir=Directories['dataDir']+'/%s_%s_%s/'%(Testboard.module,strftime("%Y-%m-%d_%Hh%Mm",gmtime(timestamp)),timestamp)
            try:
                os.stat(Testboard.parentDir)
            except:
                os.mkdir(Testboard.parentDir)
            return Testboard.parentDir

    subsystem_utils.check_clients_running(["jumoClient","psi46handler","keithleyClient"])
    subsystem_utils.start_clients(timestamp, config, Directories)

    utils.Logger << "Check Subscription of the Clients:"
    time.sleep(2)

    subsystem_utils.check_subscriptions(subscriptionList)

    Testboards = interface.get_testboard_list(timestamp, init, config, testlist, Directories, subsystem_utils.psiSubscription)

    config_utils.print_tests(testlist)


    #--------------LOOP over TESTS-----------------
    for item in testlist:
        sleep(1.0)
        if item == 'Cycle':
            climate_chamber.doCycle()
        else:
            subsystem_utils.client.send(subsystem_utils.keithleySubscription,':OUTP ON\n')
            if item.find('@')>=0:
                whichtest, temp = item.split('@')
            else:
                whichtest = item
                temp =17.0
            utils.Logger.printv()
            utils.Logger << 'I do now the following Test:'
            utils.Logger << '\t%s at %s degrees'%(whichtest, temp)

            climate_chamber.stablizeTemperature(temp, whichtest)
            if whichtest == 'IV':
                doIVCurve(temp)
            else:
                interface.doPSI46Test(whichtest, temp, timestamp, psiSubscription)
        subsystem_utils.client.send(keithleySubscription,':OUTP OFF\n')

    subsystem_utils.exit_clients()

    sleep(1)
    utils.killChildren()
    sleep(1)

    #-------------EXIT----------------
    subsystem_utils.wait_for_clients()

    utils.Logger.printv()
    utils.Logger << 'ciao!'

    del utils.Logger

    try:
        os.stat(Directories['logDir'])
    except:
        raise Exception("Couldn't find logDir %s"%Directories['logDir'])

    utils.killChildren();

    interface.save_log_directories(Directories['logDir'], Testboards)

    rmtree(Directories['logDir'])

    interface.remove_tmp_testboard_directories(Testboards)

except:
    print 'kill Children'
    utils.killChildren()
    print 'DONE'
    raise
    sys.exit(0)
