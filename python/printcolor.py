
def printi(fg,text):
    pre, post = makefix(3,fg)
    print '    | ' +pre+text+post
    
def printv():
    print '----+-----------------------------------------------------------------------'

def printn():
    print '    |'

def printc(fg,bg,text):
    pre, post = makefix(3,fg)
    one, two = makefix(4,bg)
    pre+=one
    post+=two
    print pre+text+post
    
def makefix(first,color):
    if color == 'black': id=0
    elif color == 'red': id=1
    elif color == 'green': id=2
    elif color == 'yellow': id=3
    elif color == 'blue': id=4
    elif color == 'magenta': id=5
    elif color == 'cyan': id=6
    elif color == 'white': id=7
    else:
        first =0
    if first !=0:
        pre='\033[1;%s%sm'%(first,id)
        post='\033[1;m'
    else:
        pre=''
        post=''
    return pre, post

def printw():
    print '    |'
    print '----+-----------------------------------------------------------------------'
    print '    |'
    print '    | \033[1;34mdBBBBBBBBBBBBBBBBBBP  dBP\033[1;m'
    print '    |\033[1;34mdBP       dBP    dBP  dBP\033[1;m'
    print '    \033[1;34mdBBBBP    dBP    dBBBBBBP\033[1;m'
    print '   \033[1;34mdBP       dBP    dBP  dBP\033[1;m'
    print '  \033[1;34mdBBBBBP   dBP    dBP  dBP\033[1;m  \033[1;30mSwiss Federal Institute of Technology Zurich\033[1;m'
    print '    |'
    print '----+-----------------------------------------------------------------------'
    print '    |'
    print '    |  \033[1;30mEL COMANDANTE\033[1;m - CMS Pixel Detector Module Testing Software'
    print '    |  Developped at ETHZ in 2012'
    print '    |  Felix Bachmair & Philipp Eller'
    print '    |'
    print '----+-----------------------------------------------------------------------'
    print '    |'
