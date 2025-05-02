#!/usr/bin/python
import sys
import argparse
import re
from collections import namedtuple
from subprocess import check_output
from subprocess import CalledProcessError
from operator import itemgetter

def printTuple(tuple):
	return "{0} {1} {2}".format(tuple.name,hex(tuple.start),tuple.size)

parser = argparse.ArgumentParser(description='List all variables or functions and their addresses')
parser.add_argument('-v','--listVariables', action='store_true', default=False,help='List variables')
parser.add_argument('-f','--listFunctions', action='store_true', default=False, help='List functions')
parser.add_argument('inputfile', type=str,help='The binary input file')
parser.add_argument('outputfile', type=argparse.FileType('w'), help='The name of report file')

args = parser.parse_args()
inputfile=args.inputfile
outputfile=args.outputfile
listVariables=args.listVariables
listFunctions=args.listFunctions

print "Processing "+inputfile
print "Output file is "+outputfile.name

MemInfo=namedtuple('MemInfo', ['name','start', 'size'])

try:
	readelf_output = check_output("readelf -s " +inputfile+ "| grep -v 'HIDDEN' | grep -v 'NOTYPE' |grep -v ' ABS' | grep -v 'SECTION'",shell=True)
except:
	print "Error in reading "+inputfile
	sys.exit(1)

readelf_output_lines = readelf_output.split("\n")

variableList=[]
functionList=[]

for readelfLine in readelf_output_lines:
	#print readelfLine
	words=readelfLine.split()
	if len(words)>=7:
		if words[2].startswith('0x'):
			size=int(words[2],16)
		elif re.match(r'\d+$',words[2]):
			size=int(words[2])
		else:
			size=0
		if words[3]=='FUNC' and size>0:
			functionList.append(MemInfo(words[7],int(words[1],16),size));
		elif words[3]=='OBJECT' and size>0:
			variableList.append(MemInfo(words[7],int(words[1],16),size));
variableList=sorted(variableList,key=itemgetter(1))

if listVariables:
	#print "\nList of variables:"
	outputfile.write("{0}\n".format(len(variableList)))
	for info in variableList:
		#print info
		outputfile.write("{0}\n".format(printTuple(info)))


if listFunctions:
	try:
		#print "Executing: "+"arm-none-eabi-objdump -S " +inputfile+ "| grep -e 'bzloop[0-9]*>:'"
		readelf_output=""
		readelf_output = check_output("arm-none-eabi-objdump -S " +inputfile+ "| grep -e 'bzloop[0-9]*>:'",shell=True)
	except:
		try:
			#print "Executing: "+"arm-unknown-linux-gnu-objdump -S " +inputfile+ "| grep -e 'bzloop[0-9]*>:'"
			readelf_output = check_output("arm-unknown-linux-gnu-objdump -S " +inputfile+ "| grep -e 'bzloop[0-9]*>:'",shell=True)
		except:
			readelf_output=""

	readelf_output_lines = readelf_output.split("\n")
	for readelfLine in readelf_output_lines:
		words=readelfLine.split()
		if len(words)>=2:
			size=-1
			functionList.append(MemInfo(words[1],int(words[0],16),size))

	functionList=sorted(functionList,key=itemgetter(1))


	for i in range(0,len(functionList)-1):
		if functionList[i].size<0:
			functionList[i]=MemInfo(functionList[i].name,functionList[i].start,functionList[i+1].start-functionList[i].start);

	for i in range(0,len(functionList)-1):
		if functionList[i].start+functionList[i].size>functionList[i+1].start:
			functionList[i]=MemInfo(functionList[i].name,functionList[i].start,functionList[i+1].start-functionList[i].start);

	#print "\nList of functions:"
	outputfile.write("{0}\n".format(len(functionList)))
	for info in functionList:
		#print info
		outputfile.write("{0}\n".format(printTuple(info)))

outputfile.close()
