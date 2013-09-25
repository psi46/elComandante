#!/usr/bin/env python
#####################
# Usage: ./Analyzer.py $parentDir $fullTestDir $ivDir
import ROOT
import sys

class Analyzer():
    def __init__(self,dir):
        ROOT.gROOT.SetBatch(True)
        gMinuit = ROOT.TMinuit()
        ROOT.gSystem.Load("libpsi46ana.so")
        self.dirName = dir
        
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

    def analyzeModule(self):
        self.fitAllPh()
        self.fitAllPh(3)
        self.fitAllSCurve()

if __name__ == '__main__':
    dir = sys.argv[1]
    analyzer = Analyzer(dir)
    analyzer.analyzeModule()
