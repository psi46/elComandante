#!/usr/bin/env python
import sys
from subprocess import Popen

import os
import os.path
parentDir=sys.argv[1]

print parentDir

def LoopOverDir(parentDir,level):
    dirs = os.listdir(parentDir)
    for dir in dirs:
        #print level,dir
        dirName ='%s/%s'%(parentDir,dir)
        if os.path.isdir(dirName) == False:
            continue
        #print dir
        if 'Fulltest' in dir:
            #print 'analyse'   
            analyseDir = '%s/%s'%(parentDir,dir)
            print '\t',level,'analyse',analyseDir
            Popen(['./Fitter.py',analyseDir])
        else:
            LoopOverDir(dirName,level+1)


LoopOverDir(parentDir,0)
