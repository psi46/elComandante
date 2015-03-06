#!/usr/bin/python

import sys

HELP = "usage: sort_ini.py file.ini"

def do_sort(filename, outputname):
	iniFile = file(filename)
	lines = iniFile.readlines()
	iniFile.close()
	currentSection = ""
	sections = {}
	for line in lines:
		line = line.strip()
		if line.startswith("[") and line.endswith("]"):
			currentSection = line[1:-1]
			sections.update({currentSection: [line,]})
		else:
			if len(line) > 0:
				sections[currentSection].append(line)

	sortedSections = sections.keys()
	sortedSections.sort()

	f = open(outputname, 'w')
	for section in sortedSections:
		for line in sections[section]:
			f.write(line)
			f.write("\n")
		f.write("\n")
	f.close()

if "--help" in sys.argv:
	print HELP
else:
	if len(sys.argv) == 3:
		do_sort(sys.argv[1], sys.argv[2])
	elif len(sys.argv) ==2:
		do_sort(sys.argv[1], sys.argv[1])
	else:
		print "wrong syntax\n"
		print HELP	
