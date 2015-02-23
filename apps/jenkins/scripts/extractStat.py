# Following script will extract lines containing desired statistic
# and appends it to the output file
import sys
import os
import csv
import shutil
import glob
import math

def output(statDir, filename, namelist, valuelist):
    csvfile = open(os.path.join(statDir, filename), 'w')
    writer = csv.writer(csvfile)
    writer.writerow(tuple(namelist))
    writer.writerow(tuple(valuelist))
    csvfile.close()

    # Check if jenkins is running the script
    if (os.getenv("WORKSPACE")):
        # Purge old .csv , if exist
        if os.path.isfile(os.path.join(os.getenv("WORKSPACE"), filename)):
            os.remove(os.path.join(os.getenv("WORKSPACE"), filename))

        shutil.move(filename,os.getenv("WORKSPACE"))
    else:
        # Purge old .csv , if exist
        dest = os.path.join(os.path.abspath(os.path.join(os.getenv("JJOB_SHARED_HOME"),os.pardir)), filename)
        if os.path.isfile(dest):
            os.remove(dest)

        shutil.move(filename,os.path.dirname(dest))

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
        for line in fd:
            #example input line from stat file :  user 0.01system 0:00.29elapsed 779%CPU (0avgtext+0avgdata 35452maxresident)k
            time = (line.split()[2]).split("elapsed")
            #after above operation time is equal to 0:00.29 ; 0 min and 00.29 sec
            if len(time) > 1 :
                totaltime = totaltime + float(time[0].split(':')[0])*60 + float(time[0].split(':')[1])
                runcount = runcount + 1
                print(runcount, turns)
            if runcount >= turns :
                ans = math.log((float(totaltime)/float(turns)), 2)
                logY.append(ans)
                runcount = 0
                totaltime = 0
        fd.close()

        # Compute the regression line using linear least-squares fit (NumPy is too picky & non-transparent)
        sumxy = 0
        sumx = 0
        sumy = 0
        sumx2 = 0

        for i in logX:
            sumxy = sumxy + logX[i]*logY[i]
            sumx = sumx + logX[i]
            sumy = sumy + logY[i]
            sumx2 = sumx2 + logX[i]*logX[i]

        scalenameList.append(os.path.basename(file).split(".txt")[0])
        scalevalueList.append(((N*sumxy)-(sumx*sumy))/((N*sumx2)-(sumx*sumx)))


    # Now, record the plots for regression & scaling
    output(statDir, "regressionStat.csv", testnameList, elapsedTimeList)
    output(statDir, "regressionScaling.csv", scalenameList, scalevalueList)

    shutil.rmtree(statDir)

if __name__ == '__main__':
    main()
