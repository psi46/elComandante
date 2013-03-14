#colorgenerator.py
#
# Created on: Mar 13, 2013
#     Author: bachmair


#color gadget
def colorGenerator():
    list=['green','blue','magenta','cyan']
    i=0
    while True:
        yield list[i]
        i = (i+1)%len(list)