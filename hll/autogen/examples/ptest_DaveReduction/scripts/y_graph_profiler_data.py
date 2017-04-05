# 2013Jun24
# PROGRAM STRUCTURE
#   = A single function, called Graph_profiler_data, instantiated as Graph_profiler_data, provides the single entry
#     point for the entire code.
#   = All global variables starts with the prefix g_ or they should all be included in the
#     class named MyGlobals with single instance G.
#     Use the g_ mostly for new stuff, then migrate to G.
#   = The file is structured as follows:
#           -globals : This includes all imports
#           -everything not globals, nor Graph_profiler_data
#           -Graph_profiler_data
#   = Most functions return an error code.  Zero indicates that all is well.
#     Any non-zero value indicates an error.  Usually the value of the error
#     will be the actual line number where the error occurred.

def Usage():
    print(" USAGE:")
    print(" This script takes in the output produced by ocr/ocr/scripts/Profiler/analyzeProfile.py.")
    print(" And it converts the data found to a graph based representation.")

# ===============================================================================
# ===============================================================================
# ================= Put the globals herefater  ==================================
# ===============================================================================
# ===============================================================================
import optparse  # For command line option parsing

from inspect import currentframe  # for error line handl

milliTOnano = 1000000  # nanoseconds = milliTOnano * milliseconds

undefRank = -9999999999

class CommandLineArgs:
    def __init__(self):
        self.usage = False  # Set to one if usage command is invoked
        self.infilename = ""
        self.outfilename = ""

    def __repr__(self):
        t="CLA"
        t+= " usage=" + str(self.usage)

        u=self.infilename
        if len(u)<=0:
            u='""'
        t+= " infile=" + u

        u=self.outfilename
        if len(u)<=0:
            u='""'
        t+= " outfile=" + u
        return t
# ===============================================================================
# ===============================================================================
# ===============================================================================
# ===============================================================================
# ===============================================================================
def get_cmdline_info(io_cmdlineopts):
    err = 0

    if not __name__ == "__main__":
        return currentframe().f_lineno

    while not err:
        parser = optparse.OptionParser("%prog [--usage] -i <inputFileName> [-o <outputFileName]")
        parser.add_option("-u", "--usage", dest="usage", action="store_true", help="Invoke Usage instructions and exit.",
                          default=False)
        parser.add_option("-i", "--infile", dest="inputFileName", type="string")
        parser.add_option("-o", "--outfile", dest="outputFileName", type="string")

        (option, args) = parser.parse_args()

        #DBG> print( "in getcml ->" + str(io_cmdlineopts) )

        if option.usage:
            io_cmdlineopts.usage = True
            err = currentframe().f_lineno; break

        x = option.inputFileName
        if x is None or len(x) <= 0:
            print("ERROR: missing --infile option")
            err = currentframe().f_lineno; break
        else:
            io_cmdlineopts.infilename = x

        x = option.outputFileName
        if not x is None:
            if len(x) > 0:
                io_cmdlineopts.outfilename = x

        if len(io_cmdlineopts.infilename) <= 0:
            print("ERROR: --infile option missing.  An input file name must be provided.")
            err = currentframe().f_lineno; break

        if len(io_cmdlineopts.outfilename) <= 0:
            io_cmdlineopts.outfilename = io_cmdlineopts.infilename + '.dot'

        break  # while not err:

    if err and not io_cmdlineopts.usage:
        Usage()

    return err

def getaLine(fileHandle):
    endOfFile_found = False
    do_a_continue = False
    tline=""

    line = fileHandle.readline()
    if len(line)==0:
        endOfFile_found = True
        do_a_continue = True
    else:
        tline = line.strip()
        if len(tline) == 0:
            do_a_continue = True

    return (tline,endOfFile_found,do_a_continue)

def processHeader(in_fileHandle):
    err = 0
    eof = False
    doconti = False

    count = 0

    while not err:
        endOfFile_found = False
        while not endOfFile_found:
            fpos = in_fileHandle.tell()
            (tline, eof, doconti) = getaLine(in_fileHandle)
            count = count + 1
            if eof:
                endOfFile_found = True
            if doconti:
                continue

            if tline.find('Processing file') >=0:
                continue;
            if tline.find('Done processing') >=0:
                continue;
            if tline.find('Done computing') >=0:
                continue;

            in_fileHandle.seek(fpos,0) # Try to undo the last line read.
            break

        doconti = True
        break  # while not err
    print("Number of header lines = " + str(count))
    return (err, eof, doconti)

def processSection(in_fileHandle):
    err = 0
    eof = False
    doconti = False
    title = ""
    indexDictio = {}
    metricGraph = {}

    while not err:
        sectionText = []  # All the lines of text for that section
        titleCount = 0
        endOfFile_found = False
        while not endOfFile_found:
            fpos = in_fileHandle.tell()
            (tline, eof, doconti) = getaLine(in_fileHandle)
            if eof:
                endOfFile_found = True
                if not len(sectionText) == 0:
                    (err, title, indexDictio, metricGraph) = parseSection(sectionText, True)
                    if err: break
            if doconti:
                continue

            if not tline.find('#### ') == -1:
                titleCount = titleCount + 1
                if titleCount == 1:
                    sectionText.append(tline)
                    continue
                else:
                    in_fileHandle.seek(fpos,0) # Try to undo the last line read.
                    (err, title, indexDictio, metricGraph) = parseSection(sectionText, False)
                    if err: break
                    break
            else:
                sectionText.append(tline)
                continue

        if err: break  # while not endOfFile_found

        break  # while not err
    return (err, eof, doconti, title, indexDictio, metricGraph)

def parseSectionHeader(in_sectionText):
    err = 0
    indexDictio = {}  # A mapping index --> a name
                      # This comes from "api_ocrEdtTemplateCreate [55]"
    k = 0

    while not err:
        line = in_sectionText[k]
        if line.find('####') == -1:
            print("ERROR: Missing title in section.")
            err = currentframe().f_lineno; break

        k = 1
        line = in_sectionText[k]
        if line.find('Total measured time') == -1:
            print("ERROR: Missing Total measured time in section.")
            err = currentframe().f_lineno; break

        k = 2
        line = in_sectionText[k]
        if line.find('--- Flat profile ---') == -1:
            print("ERROR: Missing Flat profile title in section.")
            err = currentframe().f_lineno; break

        k = 3
        line = in_sectionText[k]
        if line.find('Calls') == -1:
            print("ERROR: Missing column labels in section.")
            err = currentframe().f_lineno; break

        for i in range(k+1, len(in_sectionText)):
            k = k+1
            line = in_sectionText[k]
            if not line.find('Call-graph profile') == -1:
                #All done
                break
            row = line.split()

            name = row[-2]
            name = name.strip()
            if len(name) == 1 and name == "R":
                name = row[-3]
            tag  = row[-1].replace('[', ' ').replace(']',' ')

            # print( "DICTIO> " + str(int(tag)) + " " + name)
            indexDictio[int(tag)] = name

        break  # while not err

    return (err, indexDictio, k)

def parseCallGraph(in_sectionText, in_indexDictio, in_callGraph_line):

    selfGraph = {}  #  (callerID, calleeID) --> self time
    offset2Self = 0

    err = 0
    while not err:
        k = in_callGraph_line
        line = in_sectionText[k]
        if line.find('Call-graph profile') == -1:
            print("ERROR: Missing call-graph section title")
            err = currentframe().f_lineno; break

        k = k + 1
        line = in_sectionText[k]
        if line.find('Self') == -1:
            print("ERROR: Missing call-graph section Self field")
            err = currentframe().f_lineno; break
        if line.find('Index') == -1:
            print("ERROR: Missing call-graph section Index field")
            err = currentframe().f_lineno; break

        k = k + 1
        while k < len(in_sectionText):
            #----- Get sub-range
            i = k
            j = k
            for t in range(k, len(in_sectionText)):
                if not in_sectionText[t].find('----------------------------') == -1:
                    j = t
                    break
            k = j  # k will be incremented below in order to jump over the separation

            if False:
                for u in range(i,j):
                    line = in_sectionText[u]
                    print(line)
                print("==================================")

            #----- Find current actor
            maxLen = -1
            maxdex = 0
            for t in range(i,j):
                line = in_sectionText[t]
                liste = line.split()
                if maxLen < len(liste):
                    maxLen = len(liste)
                    maxdex = t

            #print("LINE= " + in_sectionText[maxdex] + "\n===========")

            liste = in_sectionText[maxdex].split()
            tag = liste[1].replace(']',' ')
            currActor = int(tag)

            if not currActor in in_indexDictio:
                print("ERROR: unknown called ID: " + liste)
                err = currentframe().f_lineno; break

            #----- Find callees and metrics
            for t in range(i,j):
                if t <= maxdex:
                    # Skip all parents.  Only pick up descendants
                    continue

                liste = in_sectionText[t].split()
                # print(liste); print("========================")

                leeID_text = liste[-1].replace(']',' ').replace('[',' ')
                leeID = int(leeID_text)

                if not leeID in in_indexDictio:
                    print("ERROR: unknown callee ID: " + liste)
                    err = currentframe().f_lineno; break

                selfTime = int( float(liste[offset2Self]) * milliTOnano + 0.5)

                selfGraph[ (currActor, leeID) ] = selfTime

            if err: break
            #-------------------------
            k = k + 1  # while k < len
        if err: break;

        break  # while not err
    return (err, selfGraph)

def parseSection(in_sectionText, in_Itis_the_last_section):
    title = ""
    indexDictio = {}
    selfGraph = {}

    err = 0
    while not err:
        title = in_sectionText[0]

        if in_Itis_the_last_section:
            if title.find('TOTAL') == -1:
                print("ERROR: missing TOTAL section")
                err = currentframe().f_lineno; break

        title = title.replace('#', ' ')
        title = title.strip()

        #print('DBG333 |' + title + '|  length=' + str(len(in_sectionText)))
        if False:
            for lin in in_sectionText:
                print(lin)

        (err, indexDictio, callGraph_line) = parseSectionHeader(in_sectionText)
        if err: break

        (err, selfGraph) = parseCallGraph(in_sectionText, indexDictio, callGraph_line)
        if err: break

        if False:
            print("SELFGRAPH>")
            for key in selfGraph:
                print("\t" + str(key) + "-->" + str(selfGraph[key]) )

        break  # while not err
    return (err, title, indexDictio, selfGraph)

def mergeGraphs(in_threadDictio):
    bigDictio = {}
    mergedGraph = {}

    err = 0
    while not err:
        if False:
            for t in threadDictio:
                print("DBG387> " + t)

        #2017Mar24--> No need for this function.  That is what TOTAL is all about...
        #             if we what to only look at TOTAL.

        for t in in_threadDictio:
            if not t.find("TOTAL") == -1:
                (bigDictio, mergedGraph) = in_threadDictio[t]
                break


        break  # while not err
    return (err, bigDictio, mergedGraph)

def graphStats(in_indexDictio, in_graph):
    ingoing = []  # The number of in-going edges for an operation K at offset K.
    outgoing = []  # The number of out-going edges for an operation K at offset K.

    err = 0
    while not err:

        if False:
            for key in in_graph:
                (From, To) = key
                payl = in_graph[key]
                print( "DBG400> " + str(From) + " " + str(To) + " " + str(payl) )

        clefs = {}
        for key in in_graph:
            (From, To) = key
            if not From in clefs:
                clefs[From] = True
            if not To in clefs:
                clefs[To] = True

        print("STATS> The number of nodes in the graph is " + str(len(clefs)) + " .")

        for i in range(len(clefs) +1):  # +1 because there is no zero offset.
            ingoing.append(0)
            outgoing.append(0)

        for key in in_graph:
            (From, To) = key
            ingoing[To] = ingoing[To] + 1
            outgoing[From] = outgoing[From] + 1

        for i in range(len(clefs) +1):
            ing = ingoing[i]
            outg= outgoing[i]
            #print( "DBG426> [" + str(i) + "] in=" + str(ing) + " out=" + str(outg) )

        roots = []
        for i in range(len(ingoing)):
            if i == 0:
                continue
            x = ingoing[i]
            if x == 0:
                roots.append(i)

        if len(roots) == 0:
            err = currentframe().f_lineno; break
        else:
            print("INFO: There are " + str(len(roots)) + " roots found.")

        leaves = []
        for i in range(len(outgoing)):
            if i == 0:
                continue
            x = outgoing[i]
            if x == 0:
                leaves.append(i)

        if len(leaves) == 0:
            err = currentframe().f_lineno; break
        else:
            print("INFO: There are " + str(len(leaves)) + " leaves found.")

        break  # while not err

    return (err, ingoing)

def dot_graph(in_graphName, in_bigDictio, in_mergedGraph, in_inComing, in_outfileName):
    err = 0
    while not err:

        graphName = in_graphName.replace('.','_').replace('/','_').replace('\\','_')

        fo = open(in_outfileName,'w')
        if fo == -1:
            erri = inspect.currentframe().f_lineno
            break

        #----- Header
        fo.write("digraph " + graphName  + " {\n")

        #----- Body
        for key in in_mergedGraph:
            (From, To) = key
            payload = in_mergedGraph[key]

            payltxt = str(payload)

            t = '"' + str(From) + '" -> "' + str(To) + '" [ label = ' + payltxt + ' ];\n'
            fo.write(t)

        #----- Footer
        if not fo.closed:
            fo.write("}\n")
            fo.close()

        break  # while not err

    if not fo.closed:
        fo.write("}\n")
        fo.close()

    return err

def find_edges_achored_at(in_node, in_mergedGraph):
    edges = []
    for key in in_mergedGraph:
        (From, To) = key
        if From == in_node:
            edges.append(key)
    return edges

def setRanks(in_mergedGraph, in_node_count, in_roots):
    ranks = []
    err = 0
    while not err:
        size = in_node_count + 1  # Offset zero is not used.

        lowestRank = 0

        found = []
        for i in range(size):
            ranks.append(undefRank)
            found.append(False)

        subroots = in_roots

        for node in subroots:
            edges = []
            edges = find_edges_achored_at(node, in_mergedGraph)  # That means node --> X

            if len(edges)==0:
                print("WARNING: Root node " + str(node) + "has node edges.")
                continue

            stack = []
            for e in edges:
                (From, To) = e
                stack.append(e)
                found[From] = True
                ranks[From] = 0

            while( not len(stack) == 0 ):
                currNode = stack.pop()
                (From, To) = currNode
                if not found[From]:
                    err = inspect.currentframe().f_lineno
                if ranks[To] == undefRank:
                    ranks[To] = ranks[From] + 1
                    found[To] = True
                    edges = []
                    edges = find_edges_achored_at(To, in_mergedGraph)
                    for e in edges:
                        stack.append(e)
                else:
                    # The "To" node already has a rank.  So leave it be.
                    # But because it has a rank, it has already been processed.
                    continue
                #while( not len(stack) == 0 ):
            if err: break;

            #for node in in_roots:
        if err: break

        break  # while not err

    return (err, ranks)

def dot_multiGraph(in_graphName, in_bigDictio, in_mergedGraph, in_inComing, in_outfileName):
    fo = open(in_outfileName,'w')
    err = 0
    while not err:
        if fo == -1:
            err = inspect.currentframe().f_lineno
            break

        #----- Pre-processing
        size = len(in_inComing)  # size should be to N +1, where N is the largest id for an operation
        #print("DBG479> size= " + str(size))
        #print(in_inComing)

        found = []
        for i in range(size):
            found.append(False)
        found[0] = False  # Offset zero is not an operation number.

        roots = []
        for i in range(size):
            if i == 0:
                continue
            x = in_inComing[i]
            #print("DBG490> i=" + str(i) + " x=" + str(x))
            if x == 0:
                roots.append(i)

        if len(roots) == 0:
            err = currentframe().f_lineno; break

        (err, ranks) = setRanks(in_mergedGraph, size-1, roots)
        if err: break

        maxRank = -1
        for i in ranks:
            if i > maxRank: maxRank = i

        if False:
            for i in range(len(ranks)):
                if i == 0:
                    continue
                print("DBG592> i=" + str(i) + " rank=" + str(ranks[i]) + " ")

        #----- Header
        tab = "    "
        graphName = in_graphName.replace('.','_').replace('/','_').replace('\\','_')
        fo.write("digraph " + graphName  + " {\n")
        fo.write("rankdir = TB;\n")
        fo.write("subgraph {\n")

        #----- Body
        for key in in_mergedGraph:
            (From, To) = key
            payload = in_mergedGraph[key]
            payltxt = str(payload)

            if ranks[From] == undefRank:
                continue
            if ranks[To] == undefRank:
                continue

            t = '"' + str(From) + '" -> "' + str(To) + '" [ label = ' + payltxt + ' ];\n'
            fo.write(tab + t)

        for r in range(0, maxRank+1):
            sameRanks = []
            for j in range(len(ranks)):
                if j==0: continue
                if ranks[j] == r:
                    sameRanks.append(j)
            fo.write(tab + "{rank = same; ")
            for k in sameRanks:
                fo.write('"' + str(k) +'"; ')
            fo.write("}\n")

        #----- Footer
        if not fo.closed:
            fo.write(tab + "} /*Closing subgraph*/ \n")
            fo.write("}\n")
            fo.close()

        break  # while not err

    if not fo.closed and not fo == -1:
        fo.write("}\n")
        fo.close()

    return err


def processFile(in_cmdlineopts):

    headerDone = False
    err = 0
    while not err:
        fileHandle = open(in_cmdlineopts.infilename, "r")

        endOfFile_found = False

        threadDictio = {}

        while not endOfFile_found:
            if not headerDone:
                (err, eof, doconti) = processHeader(fileHandle)
                if err: break
                if eof:
                    endOfFile_found = True
                if doconti:
                    headerDone = True
                    continue

            (err, eof, doconti, title, indexDictio, metricGraph) = processSection(fileHandle)
            if err: break
            if eof:
                endOfFile_found = True
            threadDictio[title] = (indexDictio, metricGraph)
            if doconti:
                continue

            #print("ONE TURN for DEBUG"); break

        if err: break
        if not fileHandle.closed:
            fileHandle.close()

        (err, bigDictio, mergedGraph) = mergeGraphs(threadDictio)
        if err: break

        (err, ingoing) = graphStats(bigDictio, mergedGraph)
        if err: break

        #err = dot_graph(in_cmdlineopts.infilename, bigDictio, mergedGraph, ingoing, in_cmdlineopts.outfilename)
        err = dot_multiGraph(in_cmdlineopts.infilename, bigDictio, mergedGraph, ingoing, in_cmdlineopts.outfilename)
        if err: break

        break  # while not err

    if not fileHandle.closed:
        fileHandle.close()

    return err

# ===============================================================================
# ===============================================================================
# ===============================================================================
# ===============================================================================
# ===============================================================================
def Graph_profiler_data():
    err = 0

    cla = CommandLineArgs()
    while not err:
        err = get_cmdline_info(cla)
        if err: break
        print( "in GraphPD ->" + str(cla) )

        print( "Units are all in nanoseconds. milliTOnano=" + str(milliTOnano))

        err = processFile(cla)
        if err: break

        break  # while err == 0:

    if err:
        if cla.usage:
            Usage()
        else:
            print("ERROR: " + str(err))

    return err

if __name__ == "__main__":
    Graph_profiler_data()

