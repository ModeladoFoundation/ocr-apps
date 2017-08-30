from pprint import pprint

import sys
import os
import time
import numpy as np
import itertools
import subprocess
import re


#### BKMS / Limitations

# - To profile PSL on computations only, have the profiler exclude the communication worker before-hand


#### TODOs:
# - TODO1: Only supports parsing the TOTAL section
#    - Could implement per node, per thread to help pinpoint stragglers.
#      It would require node and thread dictionaries to accumulate metrics directly in each
# - TODO2: Better parse the colums header, i.e. tolerated single spaces
# - TODO3: Handle profiling discrepancies across runs: missing functions etc...

#
#### Identifiers used to help with parsing a profiler file
#


#### Sections identification
#
LABEL_THREAD = "#### Thread "
START_STRING = "#### TOTAL ####"
LABEL_STRING_MARKER = "%Time"
END_STRING = "Call-graph profile"


#### Columns identification
#
LABEL_SELF_TIME="Self ms"
LABEL_CUM_TIME="Cum ms"
labelTokens=["Time", LABEL_CUM_TIME, LABEL_SELF_TIME, "Calls", "Avg (Cum)", "Std Dev (Cum)", "Function", "Id"]
# LABEL_SELECTED=LABEL_CUM_TIME
LABEL_SELECTED=LABEL_SELF_TIME

#### Pattern matching
#
# Identify non numbers (profiler dooesn't generate exponent notation)
stringMatcher = re.compile('.*[\[a-zA-Z]+.*')

# To help identify how many communication worker there are
commWorkerMatcher = re.compile('.* hc_worker_comm .*')

#### Internal variables/keys to maintain statistics

TOTAL_KEY="totalTime"

# Marks an entry for which we don't have a value
NO_VALUE_MARKER=-1

#### Debugging varialbes
#
DEBUG_PSL=False
# DEBUG_PSL=True


#### Globals
#
nbWorkers = []
nbCommWorkers = []


#
#### Functors
#


def mult(a, b):
    return a*b

def divide(a, b):
    if b == 0.0 or b == 0:
        return a
    return a/b

def substract(a, b):
    return a - b

def overhead(x, y):
    if y == 0.0 or y == 0:
        return 0
    else:
        return x/y

def toPercentStr(a):
    return '{:.3%}'.format(a)


#
#### Implementation
#

# Find and extracts the TOTAL section from the profile output
# input:  a profiler file
# output: a collection of lines
def selectTotalProfileData(profileFile):
    curNbWorkers = 0
    curNbCommWorkers = 0
    total_found = False
    prof_found = False
    thread_found = False
    profile_records = []
    for line in profileFile:
        if line.startswith(LABEL_THREAD):
            curNbWorkers+=1
            thread_found = True
        if thread_found:
            if line == '\n':
                thread_found = False
            if commWorkerMatcher.match(line) != None:
                curNbCommWorkers+=1
        #Begin searching for profile labels.
        if total_found:
            #Explicit check of an empty line, marking end of profile info
            if line == '\n':
                break
            #Once profile labels are found, begin reading numbers
            if prof_found:
                profile_records.append(line)
            if LABEL_STRING_MARKER in line:
                prof_found = True
                #TODO2

        if START_STRING in line:
            total_found = True

    nbWorkers.append(curNbWorkers)
    nbCommWorkers.append(curNbCommWorkers)

    return profile_records

# Parses lines to extract per function statistics
# TODO: to do this correctly and identify missing data from one profile to the other
# we should maintain a running list of functions. At the end of a profile parsing,
# if a function has not been encountered we need to populate a no value marker
def parseFunctionData(dataLines, dictionary, keyPosition):
    for line in dataLines:
        tokens = line.split()
        key = tokens[keyPosition]
        # Retrieve dictionary for that identifier
        metricDict = dictionary.get(key,{})
        if not metricDict:
            dictionary[key] = metricDict
        # Insert values in metric dictionary
        for token, label in zip(tokens, labelTokens):
            number = (stringMatcher.match(token) == None)
            if number:
                values = metricDict.get(label,[])
                if not values:
                    metricDict[label] = values
                values.append(float(token))

# Goes over the funcDict and extract per profile statistics
def extractProfileData(funcDict, profileDict, profileKeys):
    for fid in funcDict:
        metricsDict = funcDict[fid]
        mVector = metricsDict[LABEL_SELECTED]
        # one entry per profile, may be NO_VALUE_MARKER if no data was available
        for t, pid in zip(mVector, profileKeys):
            profileMetricsDict = profileDict.get(pid, {})
            if not profileMetricsDict:
                profileDict[pid] = profileMetricsDict
            values = profileMetricsDict.get(TOTAL_KEY,[])
            if t == NO_VALUE_MARKER:
                t = 0
            if not values:
                profileMetricsDict[TOTAL_KEY] = values
                values.append(t)
            else:
                if t > 0: #filter out negative timings from the profiler
                    values[0] += t

# Assumes sanity check on values has been done beforehand
def computePSL(selfTimes, totalTimePerProfile):
    # average time spent per worker on a given function
    scaleDown = map(divide, selfTimes, nbCompWorkers)
    # average time spent per worker for the whole execution
    ttppSd  = map(divide, totalTimePerProfile, nbCompWorkers)
    # whole execution time difference wrt to first profile
    # this is how much longer a configuration took wrt to the first profile
    ttppSdDiff  = map(substract, ttppSd, [ttppSd[0]] * len(selfTimes))
    # Prepare baseline time array
    baseline = [scaleDown[0]] * len(selfTimes)
    # Compute overhead for each configuration wrt to the baseline
    # If amount of work was constant and can proceed uncontented,
    # the overhead should be zero.
    overHead = map(overhead, baseline, scaleDown)
    # time difference for a given function wrt to the first profile
    timeDiff = map(substract, scaleDown, baseline)
    # Compute PSL absolute value: i.e. of the whole EXCESS execution time
    # a configuration spent, how much of that is attributed to the given function
    pslAbs = map(divide, timeDiff, ttppSdDiff)
    # PSL expressed as a percent of the whole EXCESS execution time the configuration spent
    pslPercent = map(toPercentStr, pslAbs)
    if DEBUG_PSL:
        print scaleDown
        print timeDiff
        print ttppSd
        print ttppSdDiff
        print overHead
        print pslAbs
        print pslPercent
    if LABEL_SELECTED == LABEL_SELF_TIME:
        return pslPercent
    else:
        return map(str, overHead)

# Go over all the profiles and print percent scaling
# loss using the first profile as a baseline
def analyzePercentScalingLoss(profileDict, totalTimePerProfile):
    ignored = []
    nbProfiles = len(totalTimePerProfile)
    # go over the dictionary and
    for funcName in profileDict.keys():
        metricDict = profileDict[funcName]
        values = metricDict[LABEL_SELECTED];
        error = False
        if len(values) != nbProfiles:
            mstr = "IGNORE[incomplete] %s %s %s" % (len(values), nbProfiles, funcName)
            ignored.append(mstr)
            error = True
        for v in values:
            if v < 0:
                mstr = "IGNORE[negative] %s %s" % (funcName, values)
                ignored.append(mstr)
                error = True
        if error:
            continue
        psl = computePSL(values, totalTimePerProfile)
        # print psl
        print("%s %s" % (funcName, ' '.join(psl)))

    for i in ignored:
        print i

def dumpProfileDict(profileDict):
    print "profileDict:", profileDict

def getTotalTimePerProfile(profileDict, nbWorkers):
    total = []
    for nbWorker in nbWorkers:
        total.append(profileDict[nbWorker][TOTAL_KEY][0])
    return total

def main():
    global nbCompWorkers
    global nbCommWorkers
    funcDict = {}
    profileDict = {}
    # Input is a collection of files
    for profileFile in sys.argv[1:len(sys.argv)]:
        try:
            pf = open(profileFile, 'r')
        except:
            print "Unable to open file at:  ", profileFile
            sys.exit(0)
        # Select relevant data
        dataLines = selectTotalProfileData(pf)
        # Parse selected data and accumulate in the dictionary
        parseFunctionData(dataLines, funcDict, 6)
        pf.close()

    nbCompWorkers = map(substract, nbWorkers, nbCommWorkers)
    print "Compute-Workers" , nbCompWorkers

    # Go over function's statistics and build a summary for each profile
    extractProfileData(funcDict, profileDict, nbWorkers)

    totalTimePerProfile = getTotalTimePerProfile(profileDict, nbWorkers)

    # Run analysis on collected data
    analyzePercentScalingLoss(funcDict, totalTimePerProfile)

if __name__ == "__main__":
    main()
