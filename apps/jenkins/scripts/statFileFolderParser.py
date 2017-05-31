# Following script parses all stat files ( 1 per build )
# present in archive dir and generates an input file
# that contains trend line statistics. This is fed
# to plotgraph.py which generates trendline plots
#
# Generated stat file has following format:
# testname1 buildNo:time1,buildNo:time2,buildNo:time3
# testname2 buildNo:time1,buildNo:time2
# .
# .
# .

import sys
import os
import datetime
import csv
import re

# Import test_baseval dict which contains values
# against which execution times will be normalized
# Each entry in test_baseline.py looks like :
# {'testname':value_used_for_normalization,}
import baselineVal

def main():
    if len(sys.argv) !=3 :
        print("ERROR ! Correct Usage :python statFileFolderParser archiveDir outStatFile\n")
        sys.exit(1)

    archiveDir = sys.argv[1]
    oFile = sys.argv[2]
    statFileList = os.listdir(archiveDir)
    #sorting from https://blog.codinghorror.com/sorting-for-humans-natural-sort-order/
    convert = lambda text: int(text) if text.isdigit() else text
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ]
    statFileList.sort(key=alphanum_key)
    testdict = dict()

    for filename in statFileList:
        fd = open(os.path.join(archiveDir,filename), 'r')
        reader = csv.reader(fd)

        # x axis labels. Jenkins plot will reflect build number with date , manual won't.
        if (os.getenv("WORKSPACE")):
            # Jenkins running infra
            modifiedFileName = os.path.splitext(filename)[0]
        else:
            modifiedFileName = "b#" + os.path.splitext(filename)[0] + "(" + datetime.datetime.now().strftime("%b%d") + ")"
        # Extract test data from the stat file ( one per build ) in Archive dir
        for row in reader:
            key                    = row[0]                      # Test name - Key
            # Only consider entries available in the baseline dictionary
            if key in baselineVal.test_baseval:
                baseval = baselineVal.test_baseval[key]
                filenameExecTimeTuple  = (modifiedFileName , (float(row[1])/float(baseval))) # Value of key , Normalized runtimes
                if key not in testdict:
                    testdict.setdefault(key,[filenameExecTimeTuple])
                else:
                    testdict[key].append(filenameExecTimeTuple)
        fd.close()

    # Purge old output file , if exist
    if os.path.isfile(oFile):
        os.remove(oFile)

    num_points = int(os.getenv("OCR_APPS_PLOT_POINTS") or 0)
    if (num_points <= 0):
        num_points = 99
    fd = open(oFile, 'w')
    for key in testdict:
        buildtimeTupleList = []
        buildList = []
        execTimeList = []
        for ele in testdict[key]:
            buildList.append(ele[0])
            execTimeList.append(str(ele[1]))
        fd.write(key + " " + ",".join(buildList[-num_points:]) + " " + ",".join(execTimeList[-num_points:])+'\n')
    fd.close()


if __name__ == '__main__':
    main()
