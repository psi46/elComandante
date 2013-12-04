#!/usr/bin/env python
#####################
# Usage: ./Analyzer.py $parentDir $fullTestDir $ivDir
import ROOT
import sys
import os

class Analyzer():
    def __init__(self,dirName,ivDirName):
        ROOT.gROOT.SetBatch(True)
        gMinuit = ROOT.TMinuit()
        ROOT.gSystem.Load("libpsi46ana.so")
        self.dirName = dirName
        self.ivDirName = ivDirName
        self.type = "m" #'a' or 'b' for half modules
        
    def fitAllPh(self, fitMode = 0):
        phFit = ROOT.PHCalibrationFit(fitMode)
        phFit.FitAllCurves(self.dirName)

    def fitPH(self, chip, col, row, fitMode = 0):
        phFit = ROOT.PHCalibrationFit(fitMode)
        phFit.FitAllCurves(self.dirName, chip, col, row)

    def fitAllSCurve(self):
        sCurve = ROOT.SCurve()
        sCurve.FitSCurves(self.dirName)

    def fitSCurve(self, roc, col, row):
        sCurve = ROOT.SCurve()
        sCurve.FitSCurve(self.dirName, roc, col, row)

    def tmpProfile(self):
        ROOT.gROOT.LoadMacro("tempProfile.C")
        ROOT.readProfile(self.dirName,1) #? (0 = short test, 1 = full test or so)

    def chipSummary(self):
        ROOT.gROOT.LoadMacro("chipSummaryPage.C")
        #chipSummaryPageShort.C
        ROOT.chipSummaries(self.dirName,self.type)

    def moduleSummary(self):
        ROOT.gROOT.LoadMacro("moduleSummaryPage.C")
        #moduleSummaryPageShort.C
        ROOT.moduleSummary(self.dirName,self.type,self.ivDirName)

    def analyzeModule(self):
        try:
            self.fitAllPh()
        except:
            print 'couldnt fit All Ph'
        try:
            self.fitAllPh(3)
        except:
            print "couldn't fit all PH 3"
        try:
            self.fitAllSCurve()
        except:
            print "Couldn't fit all SCurve"
        self.fitAllPh()
        self.fitAllPh(3)
        self.fitAllSCurve()
        #self.chipSummary()
        #self.moduleSummary()

if __name__ == '__main__':
    parentDir = sys.argv[1]
    ivDirName = sys.argv[2]
    for Folder in os.listdir(parentDir):
        if 'Fulltest' in Folder:
            print Folder
            dirName = Folder+'/'
            try:
                analyzer = Analyzer(parentDir+dirName,parentDir+ivDirName)
                analyzer.analyzeModule()
            except:
                pass
    print 'done'
