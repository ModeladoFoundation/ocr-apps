# Following script will extract lines containing desired statistic
# and appends it to the output file
import sys
import os
import csv
import shutil
def main():
    if len(sys.argv) !=3 :
        print("ERROR ! Correct Usage :python extractStat.py statFolder\n")
        sys.exit(1)

    statDir = sys.argv[1]
    turns = sys.argv[2]

    statFileList = os.listdir(statDir)
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

    csvfile = open(os.path.join(statDir,"regressionStat.csv"), 'w')
    writer = csv.writer(csvfile)
    writer.writerow(tuple(testnameList))
    writer.writerow(tuple(elapsedTimeList))
    csvfile.close()
    if (os.getenv("WORKSPACE")):
        shutil.move("regressionStat.csv",os.getenv("WORKSPACE"))
    else:
        shutil.move("regressionStat.csv",os.path.abspath(os.path.join(os.getenv("JJOB_SHARED_HOME"),os.pardir)))
    shutil.rmtree(statDir)

if __name__ == '__main__':
    main()
