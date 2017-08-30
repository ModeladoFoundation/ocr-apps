import sys
import math

def mean(data):
    sz = len(data)
    if sz == 0:
        return -1
    sum = float(0)
    for x in data:
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
    z = sz /2
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
        if m > x:
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
        y = x - mid
        variance += y*y
    variance /= float(sz-1)

    stdev = math.sqrt(variance)
    return stdev
