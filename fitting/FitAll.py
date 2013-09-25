#!/usr/bin/env python
import sys
from subprocess import Popen

parentDir=sys.argv[1]

#for dirs in ['001_Fulltest_m10','003_Fulltest_m10','005_Fulltest_p17']:
for dirs in ['000_Fulltest_p17','002_Fulltest_m10']:
    #print './Fitter.py %s/%s'%(parentDir,dirs)
    Popen(['./Fitter.py','%s/%s'%(parentDir,dirs)])
