import fileinput
import re
import operator

regex = r"""0x[a-fA-F0-9]+"""

count=0
dictio = {}

dictio['0x0'] = 0
dictio['0x00000000000000'] = 0
count = 1

for line in fileinput.input():
    f = re.compile(regex, 0).findall(line)
    if f:
        for x in f:
            if not x in dictio:
                dictio[x] = count
                count = count + 1
if True:
    sortedx = sorted(dictio.items(), key=operator.itemgetter(1))
    for k in sortedx:
        print( 'DICTIO>G(' + str(k[1]) + ')=' + k[0])
if False:
    #This sorts on the hexadecimal addresses
    it = iter(sorted(dictio.iteritems()))
    for i in range(len(dictio)):
        p = it.next()
        print( 'DICTIO>G(' + str(p[1]) + ')=' + p[0])
if False:
    for k in dictio:
        print( 'DICTIO>G(' + str(dictio[k]) + ')=' + k)

for line in fileinput.input():
    f = re.compile(regex, 0).findall(line)
    if f:
        for x in f:
            line = re.sub(x, 'G(' + str(dictio[x]) + ')', line.rstrip())
    print(line)
