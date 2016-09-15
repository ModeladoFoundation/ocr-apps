def nek_cubic2(in_processCount): #This is from NEKbone 2.1.2
    mx = in_processCount
    my = 1
    mz = 1
    ratio = in_processCount

    iroot3 = int(in_processCount **(1/3.0) + 0.000001)
    for i in range(iroot3, 0, -1):
        iz = i
        myx = in_processCount//iz
        nrem = in_processCount - myx * iz

        if nrem == 0:
            iroot2 = int(myx **(1/2.0) + 0.000001)
            for j in range(iroot2,0,-1):
                iy = j
                ix = myx //iy
                nrem = myx-ix*iy
                if nrem == 0:
                    break

            if ix < iy :
                it = ix
                ix = iy
                iy = it
            if ix < iz :
                it = ix
                ix = iz
                iz = it
            if iy < iz :
                it = iy
                iy = iz
                iz = it
            if float(ix)/iz < ratio:
                ratio = float(ix)/iz
                mx = ix
                my = iy
                mz = iz
    return (mx,my,mz)

#===============================================================================
import optparse #For command line option parsing

from inspect import currentframe # for error line handling

erri =0
while not erri:
    #----- Command line
    parser = optparse.OptionParser("%prog -t <totalElementCount> (-r <rank count> | -e <element per rank>)")
    parser.add_option("-t", "--totalElement", dest="totalEleCount", type="int")
    parser.add_option("-r", "--rankCount", dest="rankCount", type="int")
    parser.add_option("-e", "--elementPerRank",  dest="elementPerRank", type="int")

    (option, args) = parser.parse_args()

    totalElement = -1
    rankc = -1
    elementc = -1
    divideWarning ="     "

    x = option.totalEleCount
    if x==None :
        parser.print_help()
        break
    else:
        if x<=0:
            print("ERROR: totalElement must be bigger than zero.")
            erri=currentframe().f_lineno;
            break
        totalElement = x

    x = option.rankCount
    if x!=None :
        if x<=0:
            print("ERROR: rankCount must be bigger than zero.")
            erri=currentframe().f_lineno;
            break
        rankc = x

    x = option.elementPerRank
    if x!=None :
        if x<=0:
            print("ERROR: elementPerRank must be bigger than zero.")
            erri=currentframe().f_lineno;
            break
        elementc = x

    if rankc!=-1:
        x = totalElement // rankc
        if x==0:
            print("ERROR: rankCount is too big, must be smaller than totalElement.")
            erri=currentframe().f_lineno;
            break
        y = totalElement - x * rankc
        if y!=0:
            rankc = totalElement // x
            divideWarning = "WarnR"
        elementc = totalElement // rankc
    elif elementc!=-1:
        x = totalElement // elementc
        if x==0:
            print("ERROR: elementPerRank is too big, must be smaller than totalElement.")
            erri=currentframe().f_lineno;
            break
        y = totalElement - x * elementc
        if y!=0:
            elementc = totalElement // x
            divideWarning = "WarnE"
        rankc = totalElement // elementc
    else:
        print("ERROR: At least one of (--rankCount|--elementPerRank) must be specified.")
        erri=currentframe().f_lineno;
        break

    #----- Actual work
    effectiveTE = rankc*elementc

    tripletR = nek_cubic2(rankc)
    tripletE = nek_cubic2(elementc)

    maxR = tripletR[0]
    minR = tripletR[0]
    valueof2 = 0

    for v in tripletR:
        if maxR < v: maxR = v
        if minR > v: minR = v
        if v<=2: valueof2 = 1

    ratio = float(maxR) / minR

    check = 'ok     '
    if ratio > 5.0:
        check = 'badRatio'
    if valueof2 == 1:
        check = 'bad2   '
        if ratio > 5.0:
            check = 'badAll '

    print(divideWarning),
    print(" "+ check),

    print(" TE=" + str(totalElement)),
    print(" effTE=" + str(effectiveTE) +' '),
    print(" Rc="+ str(rankc)),
    print(" Ec="+ str(elementc)),

    print("\tranks= "+ str(tripletR)),
    print(" elements="+ str(tripletE)),
    print(" ratio="+ str(ratio)),

    print("")


    break #while not erri:

if erri:
    print("ERROR: " +str(erri))
