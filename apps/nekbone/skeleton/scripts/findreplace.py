import fileinput
import sys

if len(sys.argv) != 4:
    print "USAGE: <exec> <filename> <string to look for> <what to replace it with>"
    exit(1)

filename     = sys.argv[1]
what2lookfor = sys.argv[2]
replacement  = sys.argv[3]

for line in fileinput.FileInput(filename, inplace=1):
    line = line.replace(what2lookfor, replacement)
    #The trailing comma avoid emitting newlines.
    print line,
