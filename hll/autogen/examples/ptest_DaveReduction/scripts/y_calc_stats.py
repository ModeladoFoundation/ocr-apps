import sys
import math

def mean(data):
    sz = len(data)
    if sz == 0:
        return -1
    sum = float(0)
    for x in data:
        if x == -1:
            sz-=1
        else:
            sum += x
    sum /= float(sz)
    return sum

def median(data):
    sz = len(data)
    if sz == 0:
        return -1
    if sz == 1:
        return data[0]
    sortedData = sorted(data)
    # skip -1
    i=0
    for x in data:
        if x != -1:
            break
        i+=1
    sz-=i
    z = (sz /2)+i
    if 2*z == sz:  # even
        dian = (sortedData[z-1] + sortedData[z])/2
    else:
        dian = sortedData[z]

    return dian

def maxi(data):
    sz = len(data)
    if sz == 0:
        return -1
    M = float(-1e30)
    for x in data:
        if M < x:
            M = x
    return M

def mini(data):
    sz = len(data)
    if sz == 0:
        return -1
    m = float(1e30)
    for x in data:
        if m > x and x != -1:
            m = x
    return m

def std(data):
    sz = len(data)
    if sz == 0:
        return -1
    if sz == 1:
        return 0

    mid = mean(data)

    variance = float(0)
    for x in data:
        if x != -1:
            y = x - mid
            variance += y*y
    variance /= float(sz-1)

    stdev = math.sqrt(variance)
    return stdev

def calc_stats(in_filename):

    with open(in_filename) as f:
        content = f.readlines()

    sz = len(content)
    #print('sz=' + str(sz))
    if 1 >= sz:
        return (sz, -1,-1,-1,-1,-1)

    for i,x in enumerate(content):
        x = x.strip()
        if x.isdigit():
            content[i] = float(x)
        else:
            content[i] = -1

    # print(content)
    count   = sz
    moyenne = mean(content)
    mediane = median(content)
    biggest = maxi(content)
    smallest= mini(content)
    stdev   = std(content)

    return (count, mediane,moyenne, stdev, smallest,biggest)

if __name__ == "__main__":
    erri=0
    while not erri:
        argc = len(sys.argv)
        if not argc == 2:
            print('USAGE: <exec> <input data file>')
            print('=This script takes the name of the input file and parse its content')
            print(' in order to calculates some statistics.')
            print('=The content of the file must be:')
            print('    -1 entry per line')
            print('    -Each line contains one number')
            erri = 99
            break

        fname = sys.argv[1]
        count, mediane,moyenne, stdev, smallest,biggest = calc_stats(fname)

        mediane = int(round(mediane))
        moyenne = int(round(moyenne))
        stdev   = int(round(stdev))
        smallest= int(round(smallest))
        biggest = int(round(biggest))

        # print ('[count, median,mean,std,min,max]= ' + str(count) + str(mediane) +' '+ str(moyenne) +' '+ str(stdev) +' '+ str(smallest) +' '+ str(biggest) )
        print '[count,median,mean,std,min,max]= %10d %10d %10d %10d %10d %10d' % (count, mediane, moyenne, stdev, smallest, biggest)

        break  # while not erri:

    if erri:
        print('ERROR: ' + str(erri) + '\n')


