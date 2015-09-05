# Following script will extract lines containing desired statistic
# and appends it to the output file
# Generated file format is
#
# testname1,runtime
# testname2,runtime
# .
# .
#

import sys
import os
import csv
import shutil
import glob
import math
import datetime

def output(statDir, filename, namelist, valuelist,archiveDir):
    # Generate stat file
    testnameValPair = zip(namelist,valuelist)
    csvfile = open(os.path.join(statDir, filename), 'w')
    writer = csv.writer(csvfile)
    for pair in testnameValPair:
        writer.writerow(tuple(pair))
    csvfile.close()

    # Generate Archive dir path depending on who is running infra. Jenkins or Manual ?
    archivedirPath = ""
    if (os.getenv("WORKSPACE")):
        # Jenkins running infra
        archivedirPath = os.path.join(os.getenv("WORKSPACE"), archiveDir)
    else:
        # Manual exec. of infra
        archivedirPath = os.path.join(os.path.abspath(os.path.join(os.getenv("JJOB_SHARED_HOME"),os.pardir)), archiveDir)

    # Check if stat dir exists . if not , create one
    if not os.path.exists(archivedirPath):
        print("ERROR ! Archived dir (" + archivedirPath +" ) for this regression run does not exist . Inspect jenkins PBS configuration if jenkins running the framework or statCollector.sh if framework is being run manully.\n")
        sys.exit(1)

    # Purge old .csv , if exist
    if os.path.isfile(os.path.join(archivedirPath, filename)):
        msg = "Purging old version of " + filename +"\n"
        print(msg)
        os.remove(os.path.join(archivedirPath, filename))

    # Move stat file in Archive dir
    shutil.move(os.path.join(statDir, filename),archivedirPath)

def main():
    if len(sys.argv) !=3 :
        print("ERROR ! Correct Usage :python extractStat.py <directory containing running times> <number of runs for each case>\n")
        sys.exit(1)

    statDir = sys.argv[1]
    turns = int(sys.argv[2])

    # First read the running times for regression runs
    statFileList = glob.glob(statDir+"/time*.txt")
    statFileList.sort()
    os.chdir(statDir)
    elapsedTimeList = []
    testnameList = []

    for file in statFileList:
        fd = open(file,'r')
        totaltime = 0
        avgtime = 0
        for line in fd:
            #example input line from stat file :  user 0.01system 0:00.29elapsed 779%CPU (0avgtext+0avgdata 35452maxresident)k
            time = (line.split()[2]).split("elapsed")
            #after above operation time is equal to 0:00.29 ; 0 min and 00.29 sec
            if len(time) > 1 :
               totaltime = totaltime + float(time[0].split(':')[0])*60 + float(time[0].split(':')[1])
        fd.close()
        testnameList.append(os.path.basename(file).split(".txt")[0])
        avgtime = totaltime/float(turns)
        elapsedTimeList.append(avgtime)

    # Next, read the running times for scaling runs
    scaleFileList = glob.glob(statDir+"/scale*.txt")
    scalenameList = []
    scalevalueList = []
    logX = [0,1,2,3,4]
    logY = []
    N = 5
    for file in scaleFileList:
        fd = open(file,'r')
        totaltime = 0
        runcount = 0
        logY = []
        for line in fd:
            #example input line from stat file :  user 0.01system 0:00.29elapsed 779%CPU (0avgtext+0avgdata 35452maxresident)k
            time = (line.split()[2]).split("elapsed")
            #after above operation time is equal to 0:00.29 ; 0 min and 00.29 sec
            if len(time) > 1 :
                totaltime = totaltime + float(time[0].split(':')[0])*60 + float(time[0].split(':')[1])
                runcount = runcount + 1
            if runcount >= turns:
                if totaltime > 0.0:
                    ans = math.log((float(totaltime)/float(turns)), 2)
                else:
                    ans = 0
                logY.append(ans)
                runcount = 0
                totaltime = 0
        fd.close()

        # Compute the regression line using linear least-squares fit (NumPy is too picky & non-transparent)
        sumxy = 0
        sumx = 0
        sumy = 0
        sumx2 = 0

        if(len(logY) == N):
            for i in logX:
                sumxy = sumxy + logX[i]*logY[i]
                sumx = sumx + logX[i]
                sumy = sumy + logY[i]
                sumx2 = sumx2 + logX[i]*logX[i]

            scalevalueList.append(-1 * (((N*sumxy)-(sumx*sumy))/((N*sumx2)-(sumx*sumx))))
        else:
            scalevalueList.append(0)  # Erroneous run, just give a scaling of 0

        scalenameList.append(os.path.basename(file).split(".txt")[0])
        print(scalenameList, scalevalueList)

    # Use build no as stat file name if jenkins running infra else use date time
    if (os.getenv("WORKSPACE")):
        filename = os.getenv("BUILD_NUMBER")+".csv"
    else:
        filename = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")+ ".csv"

    # Now, record the plots for regression & scaling
    output(statDir, filename, testnameList, elapsedTimeList,"regressionResults/NightlyRegressionStat")
    output(statDir, filename, scalenameList, scalevalueList,"regressionResults/NightlyScalingStat")
    shutil.rmtree(statDir)

if __name__ == '__main__':
    main()
