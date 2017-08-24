import pprint
import re
import subprocess
from optparse import OptionParser
import os.path

def print_line(line):
    pp = pprint.PrettyPrinter()
    pp.pprint(line)

# Collect all template create calls hijacked by SPMD library
def collect_spmdtemplate_stats(line, fptrCountMap):
    rexp = re.compile(r"API\(INFO\) \[.*\] ENTER spmd_ocrEdtTemplateCreate\(\*guid=\w*, funcPtr=(\w*), paramc=[-]*\d+, depc=[-]*\d+, name=\w*\)")
    match = re.search(rexp, line)
    if match:
        print match.group()
        mstr = match.group() # matched string
        fptr = match.group(1) # extract the fptr
        # Count the number of fptr encountered in the template create
        if fptr in fptrCountMap:
            fptrCountMap[fptr] += 1
        else:
            fptrCountMap[fptr] = 1

# Collect all DBK create
def collect_dbk_stats(line, dbkCountMap):
    rexp = re.compile(r"API\(INFO\) \[PD:\w+ W:\w+ EDT:\w+\] ENTER ocrDbCreate\(\*guid=\w+, len=(\d+), flags=\d+, hint=\w+, alloc=\w+\)")
    match = re.search(rexp, line)
    if match:
        mstr = match.group() # matched string
        dbklen = match.group(1) # dbk length
        if dbklen in dbkCountMap:
            dbkCountMap[dbklen] += 1
        else:
            dbkCountMap[dbklen] = 1

# Collect all EDT create
def collect_edt_stats(line, edtCountMap):
    rexp = re.compile(r"API\(INFO\) \[PD:\w+ W:\w+ EDT:\w+\] ENTER ocrEdtCreate\(\*guid=\w+, template=(\w+), paramc=[-]*\w+, paramv=\w+, depc=[-]*\w+, depv=\w+, prop=\w+, hint=\w+, outEvt=\w+\)")
    match = re.search(rexp, line)
    if match:
        mstr = match.group()
        templ = match.group(1)
        if templ in edtCountMap:
            edtCountMap[templ] += 1
        else:
            edtCountMap[templ] = 1

# Collect all EVT create
def collect_evt_stats(line, evtCountMap):
    rexp = re.compile(r"API\(INFO\) \[PD:\w+ W:\w+ EDT:\w+\] ENTER ocrEventCreateParams\(\*guid=\w+, eventType=(\d+), properties=\w+\)")
    match = re.search(rexp, line)
    if match:
        mstr = match.group()
        etype = match.group(1)
        if etype in evtCountMap:
            evtCountMap[etype] += 1
        else:
            evtCountMap[etype] = 1

# Open the file and apply various regex to collect stats
def collect_ocr_object_stats(filename, fptrCountMap, dbkCountMap, edtCountMap, evtCountMap):
    with open(filename) as f:
        # for each line we may want to apply multiple regex
        # and collect stats on individual OCR Objects
        for line in f:
            collect_spmdtemplate_stats(line, fptrCountMap)
            collect_dbk_stats(line, dbkCountMap)
            collect_edt_stats(line, edtCountMap)
            collect_evt_stats(line, evtCountMap)

# Run nm and get symbols from binary
def name_from_nm_output(nm_output):
    rexp = re.compile(r"\w+ (\w) (\w+)")
    match = re.match(rexp, nm_output)
    if match:
        nm_symboltype = match.group(1)
        nm_symbolname = match.group(2)
        if nm_symboltype == "T":
            return nm_symbolname
        else:
            return ""
    else:
        return ""

# Converts each function pointer to their names using nm
def collect_fptr_to_name(fptrCountMap, path_to_binary):
    for key,value in fptrCountMap.items():
        symbol = key
        if key.startswith("0x"):
            symbol = symbol.replace("0x", "", 1)
        syscall = "nm " + path_to_binary + " | grep " + "'" + symbol + "'"
        nm_output = subprocess.check_output(syscall, shell=True)
        fname = name_from_nm_output(nm_output)
        nvalue = [fname, value]
        fptrCountMap[key] = nvalue

def print_dictionary(dict):
    pp = pprint.PrettyPrinter(indent=2)
    pp.pprint(dict)


def print_fptr_stats_in_csv(fptr_dict, csvfile):
    print "spmd_ocrEdtTemplateCreate"
    ntempls = 0
    for key, value in fptr_dict.items():
        print '{0}, {1}'.format(value[0], value[1])
        ntempls += value[1]
    csvfile.write('{0}, '.format(ntempls))

def print_dbk_stats_in_csv(dbklen_dict, csvfile):
    print "ocrDbCreate"
    ndbks = 0
    for key, value in dbklen_dict.items():
        print '{0}, {1}'.format(key, value)
        ndbks += value
    csvfile.write('{0}, '.format(ndbks))

def print_edt_stats_in_csv(edtcount_dict, csvfile):
    print "ocrEdtCreate"
    nedts = 0
    for key, value in edtcount_dict.items():
        print '{0}, {1}'.format(key, value)
        nedts += value
    csvfile.write('{0}, '.format(nedts))

def print_evt_stats_in_csv(evtcount_dict, csvfile):
    print "ocrEvtCreate"
    nevts = 0
    for key, value in evtcount_dict.items():
        print '{0}, {1}'.format(key, value)
        nevts += value
    csvfile.write('{0}'.format(nevts))
    csvfile.write('\n')

################################
# DataStructures to Hold Stats #
################################
fptrCountMap = {}
dbkCountMap = {}
evtCountMap = {}
edtCountMap = {}

filename = ""
path_to_binary = ""
def collect_stats():
    parser = OptionParser(usage="usage: %prog [options]")
    parser.add_option("-f", "--file", action="store_true", dest="filename", default=False, help="file to collect stats from")
    parser.add_option("-p", "--path", action="store_true", dest="path_to_binary", default=False, help="path to the binary")
    (options, args) = parser.parse_args()

    if len(args) != 2:
        parser.error("Specify trace file and path to binary")

    filename = args[0]
    root, path = os.path.splitext(filename)
    csvfilename = root+".csv"
    path_to_binary = args[1]

    collect_ocr_object_stats(filename, fptrCountMap, dbkCountMap, edtCountMap, evtCountMap)
    collect_fptr_to_name(fptrCountMap, path_to_binary)
    csvfile = open(csvfilename, 'w')
    print_fptr_stats_in_csv(fptrCountMap, csvfile)
    print ""
    print_dbk_stats_in_csv(dbkCountMap, csvfile)
    print ""
    print_edt_stats_in_csv(edtCountMap, csvfile)
    print ""
    print_evt_stats_in_csv(evtCountMap, csvfile)

###############
# Entry Point #
###############
collect_stats()
