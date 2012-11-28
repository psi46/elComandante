class Testboard:
    def __init__(self,slot,module,address,type):
        self.slot=slot
        self.module=module
        self.address=address
        self.type=type
        self.tests=[]
        self.currenttest=''
        self.fintests=[]
        self.failtests=[]
        self.testdir=''
        self.defparamdir=''
        self.powerdup=False
        self.busy=False
        self.timestamp=0
        self.parentdir=''
        
    def dotest(self,test):
        self.currenttest=test

    def finished(self):
        self.fintests.append(self.tests[self.tests.index(self.currenttest)])
        
    def failed(self):
        self.failtests.append(self.tests[self.tests.index(self.currenttest)])
