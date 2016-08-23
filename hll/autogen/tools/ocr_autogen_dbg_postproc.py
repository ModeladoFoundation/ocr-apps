# 2016Aug21
# INTENT:
# This program does the post processing of the stdout outputs of the OCR code build by Autogen when Autogen has been
# configure to tool the emitted code with debugging code.
#
# PROGRAM STRUCTURE
#   = A single function, called Pmain, instantiated as pmain, provides the single entry
#     point for the entire code.
#   = All global variables are put in the class named MyGlobals with single instance GBL.
#     Use the g_ mostly for new stuff, then migrate to G.
#   = The file is structured as follows:
#           -globals : This includes all imports
#           -everything not globals, nor Pmain
#           -Pmain
#   = Most functions return an error code.  Zero indicates that all is well.
#     Any non-zero value indicates an error.  Usually the value of the error
#     will be the actual line number where the error occured.


def Usage():
    print(" COMMAND LINE SYNTAX:                                                                           ")
    print("   <OApostprocDBG>.py -i <DBG log file> -o <outputfile> [-s]                                    ")
    print("       =The command passed to <OApostprocDBG>.py follow the following format                    ")
    print("           -<identifier> <whitespace> <payload>                                                 ")
    print("        where <identifier> is usually a single letter, e.g. -e                                  ")
    print("              <whitespace> is a simple ASCII space                                              ")
    print("              <payload>    is the text argument associated with <identifier>                    ")
    print(" NOTE:                                                                                          ")
    print("     Make sure that the application was compiled with                                           ")
    print("         CONFIG_FLAGS = --guid = COUNTED_MAP                                                    ")
    print("     in order to guarantee that all guids are unique.                                           ")
    print(" INPUTS:                                                                                        ")
    print("   =In all cases, a list of 'stuff' on the command line is an actual ASCII file                 ")
    print("    containing one properly terminated line for each entry.                                     ")
    print("    A given line can be commented out if it starts with a # character.                          ")
    print("   =<DBG log file>:                                                                             ")
    print("       -Autogen can be requested to tool the C code it generates by setting                     ")
    print("            GBL.insert_debug_code                                                               ")
    print("        to a non-zero value.  See the Autogen code for more information.                        ")
    print("       -When setup, Autogen will the produced C code with a sequence of PRINTF statement which  ")
    print("        will, at the C code run time, emit a set of statement, formatted as follows:            ")
    print("            OA_DBG> TaskTYPE=<value> TASKID=<value> <Specifi tag> [<properties>                 ")
    print("       -This prost-processor will extract these statements from the provided <DBG log file>     ")
    print("        and will output a DOT file representing the execution and data flows as they unfolded.  ")
    print("   =<outputfile>                                                                                ")
    print("       -Name of the DOT file where all results are to be outputted.                             ")
    print(" OUTPUTS                                                                                        ")
    print("   =A DOT file is produced and outputted in <outfile>.dot                                       ")
    print("   =It will contain the execution and data flows as they unfolded.                              ")

#===============================================================================
#===============================================================================
#=== Put the globals herefater  ================================================
#===============================================================================
#===============================================================================
import optparse  # For command line option parsing

from inspect import currentframe  # for error line handling

class MyGlobals:

    def init(self):
        self.usage = 0  # Set to one if usage command is invoked
        self.infile = ""  # Name of the input file
        self.outfile = ""  # Name of the output file
        self.show_simplified_statements = False
        self.output_fullGuid = False

        self.nota_tasktype = -1
        self.nota_taskID = -2
        self.nota_taskName = 'OA_nota_taskName'
        self.nota_task = (-1,-1)
        self.nota_slot = -1
        self.nota_token = -1
        self.nota_DBKName = 'OA_nota_DBKname'
        self.nota_guid = 'OA_nota_guid'
        self.TABunit = '    '

    def print2stdout(self):
        print("G>usage = " + str(self.usage))
        print("G>TABunit = '" + self.TABunit + "'")
        print("G>infile = " + self.infile)
        print("G>outfile = " + self.outfile)
        print("G>show_simplified_statements = " + str(self.show_simplified_statements))
        print("G>nota_tasktype = " + str(self.nota_tasktype))
        print("G>nota_taskID = " + str(self.nota_taskID))
        print("G>nota_taskName = " + str(self.nota_taskName))
        print("G>nota_task = " + str(self.nota_task))
        print("G>nota_slot = " + str(self.nota_slot))
        print("G>nota_token = " + str(self.nota_token))
        print("G>nota_DBKName = " + str(self.nota_DBKName))
        print("G>nota_guid = " + str(self.nota_guid))

GBL = MyGlobals()
GBL.init()

#===============================================================================
#===============================================================================
#==  Put everything else hereafter  ============================================
#===============================================================================
#===============================================================================
def get_cmdline_info(io_GBL):
    if not __name__ == "__main__":
        return currentframe().f_lineno

    parser = optparse.OptionParser("%prog [--usage] -i <DBGlogfile> -o <outputfile> [-s]")
    parser.add_option("-u", "--usage", dest="usage", action="store_true", help="Invoke Usage instructions and exit.", default=False)
    parser.add_option("-i", "--input", dest="fileOfExamined", type="string" )
    parser.add_option("-o", "--outfile", dest="outfile", type="string")
    parser.add_option("-s", "--sstates", dest="simpleStates", action="store_true", help="Output simplified statements", default=False)
    parser.add_option("-f", "--fullguid", dest="fullGuid", action="store_true", help="Output full GUID in DOT", default=False)

    (option, args) = parser.parse_args()

    if option.usage:
        io_GBL.usage = 1
        return currentframe().f_lineno

    if option.simpleStates:
        io_GBL.show_simplified_statements = True
    else:
        io_GBL.show_simplified_statements = False

    if option.fullGuid:
        io_GBL.output_fullGuid = True
    else:
        io_GBL.output_fullGuid = False

    x = option.fileOfExamined
    if x == None or len(x) <= 0:
        parser.print_help()
        return currentframe().f_lineno
    else:
        io_GBL.infile = x

    x = option.outfile
    if x == None or len(x) <= 0:
        parser.print_help()
        return currentframe().f_lineno
    else:
        io_GBL.outfile = x

    return 0

def isa_null_guid(in_text):
    if in_text == '0x0':
        return True
    return False

def simplify_statements(io_lines, io_guidMap2index):
    erri = 0
    while not erri:
        sz_lines = len(io_lines)
        if sz_lines == 0:
            break

        icount = 0

        for i in range(sz_lines):
            t = io_lines[i].split()
            if len(t) <= 3:
                continue
            if not 'OA_DBG' in t[0]:
                continue
            if 'OA_DBG_PRECOND' in t[0]:
                continue

            if 'TaskID=' in t[2]:
                tid = get_token(t[2], 'TaskID')
                if tid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break
                if tid in io_guidMap2index:
                    index = io_guidMap2index[tid]
                else:
                    if isa_null_guid(tid):
                        index = 0
                        io_guidMap2index[tid] = index
                    else:
                        icount += 1
                        io_guidMap2index[tid] = icount
                        index = icount
                t[2] = 'TaskID' + '=' + str(index)

            #print('DBG169>' +str(t))
            #print('DBG169>' + str(t[3]))

            if 'TaskName' in t[3]:
                pass
            elif 'DBKCreate' in t[3]:
                #print('DBG175>' + str(t))
                tid = get_token(t[4], 'Dguid')
                if tid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break
                if tid in io_guidMap2index:
                    index = io_guidMap2index[tid]
                else:
                    if isa_null_guid(tid):
                        index = 0
                        io_guidMap2index[tid] = index
                    else:
                        icount += 1
                        io_guidMap2index[tid] = icount
                        index = icount
                t[4] = 'Dguid' + '=' + str(index)
                #print('DBG187>' + str(t[4]))

            elif 'EDTCreate' in t[3]:
                tid = get_token(t[5], 'Tguid')
                if tid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break
                if tid in io_guidMap2index:
                    index = io_guidMap2index[tid]
                else:
                    if isa_null_guid(tid):
                        index = 0
                        io_guidMap2index[tid] = index
                    else:
                        icount += 1
                        io_guidMap2index[tid] = icount
                        index = icount
                t[5] = 'Tguid' + '=' + str(index)

            elif 'HOOKUP' in t[3]:
                tid = get_token(t[4], 'Tguid')
                if tid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break
                if tid in io_guidMap2index:
                    index = io_guidMap2index[tid]
                else:
                    if isa_null_guid(tid):
                        index = 0
                        io_guidMap2index[tid] = index
                    else:
                        icount += 1
                        io_guidMap2index[tid] = icount
                        index = icount
                t[4] = 'Tguid' + '=' + str(index)

                tid = get_token(t[6], 'Dguid')
                if tid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break
                if tid in io_guidMap2index:
                    index = io_guidMap2index[tid]
                else:
                    if isa_null_guid(tid):
                        index = 0
                        io_guidMap2index[tid] = index
                    else:
                        icount += 1
                        io_guidMap2index[tid] = icount
                        index = icount
                t[6] = 'Dguid' + '=' + str(index)

            else:
                #print('DBG227>' +str(t))
                erri = currentframe().f_lineno
                break

            #print('DBG232>' + str(t))
            io_lines[i] = " ".join(t)
            #print('DBG234>' + str(io_lines[i]))

        if erri: break  # for i in range(sz_lines)

        break  # while not erri:

    return erri

def get_statements(in_GBL, o_statements, io_edt_type2name, io_edt_name2type, io_guidMap2index):
    try:
        f = open(in_GBL.infile, 'r')
        lines = f.readlines()
    finally:
        f.close()

    erri = 0
    while not erri:

        erri = simplify_statements(lines, io_guidMap2index)
        if GBL.show_simplified_statements:
            for h in lines:
                print(h)
            for k in io_guidMap2index:
                print("#OA_DBG> " + k + ' --> ' + str(io_guidMap2index[k]))

        for x in lines:
            #print('DBG112>' + x)
            x = x.strip()
            if len(x) == 0:
                continue
            if x[0] == '#':
                continue
            t = x.split()
            if not 'OA_DBG' in t[0]:
                continue

            if 'OA_DBG_PRECOND' in t[0]:
                ttype = get_token(t[1], 'Ttype')
                if ttype == GBL.nota_token:
                    continue
                tname = get_token(t[2], 'Tname')
                if tname == GBL.nota_token:
                    continue

                if ttype in io_edt_type2name:
                    if not tname == io_edt_type2name[ttype]:
                        erri = currentframe().f_lineno
                        break
                else:
                    io_edt_type2name[ttype] = tname

            if not 'TaskTYPE=' in t[1]:
                continue
            if not 'TaskID=' in t[2]:
                continue

            o_statements.append(t[1:])
        if erri: break  # for x in lines:

        for ttype in io_edt_type2name:
            tname = io_edt_type2name[ttype]
            io_edt_name2type[tname] = ttype

        break  # while not erri:

    if 1 == 2:
        for s in o_statements:
            print(s)

    return erri

def get_tasktype(in_statement):
    if not 'TaskTYPE' in in_statement:
        return GBL.nota_tasktype
    u = in_statement.split('=')
    if not len(u) == 2:
        return GBL.nota_tasktype
    return u[1]

def get_taskID(in_statement):
    if not 'TaskID' in in_statement:
        return GBL.nota_taskID
    u = in_statement.split('=')
    if not len(u) == 2:
        return GBL.nota_taskID
    return u[1]

def get_taskName(in_statement):
    if not 'TaskName' in in_statement:
        return GBL.nota_taskID
    u = in_statement.split('=')
    if not len(u) == 2:
        return GBL.nota_taskID
    return u[1]

def get_token(in_statement, in_token):
    if not in_token in in_statement:
        return GBL.nota_token
    u = in_statement.split('=')
    if not len(u) == 2:
        return GBL.nota_token
    if not u[0] == in_token:
        return GBL.nota_token
    return u[1]

def parseEDTCreate(in_cmd, in_edt_name2type):
    slotCount = GBL.nota_slot
    newtask = GBL.nota_task
    erri = 0
    while not erri:
        #print('DBG220> ' + str(in_cmd))
        if not len(in_cmd) == 4:
            erri = currentframe().f_lineno
            break
        if not in_cmd[0] == 'EDTCreate':
            erri = currentframe().f_lineno
            break

        slotCount = get_token(in_cmd[1], 'slots')
        if slotCount == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        tguid = get_token(in_cmd[2], 'Tguid')
        if tguid == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        tname = get_token(in_cmd[3], 'Tname')
        if tname == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        if not tname in in_edt_name2type:
            erri = currentframe().f_lineno
            break
        ttype = in_edt_name2type[tname]

        newtask = (ttype, tguid)

        break  # while not erri
    return (erri, slotCount, newtask)

def parseHookup(in_cmd, in_edt_name2type):
    slot = GBL.nota_slot
    newtask = GBL.nota_task
    dbk = (GBL.nota_DBKName, GBL.nota_guid)
    erri = 0
    while not erri:
        #print('DBG259> ' + str(in_cmd))
        if not len(in_cmd) == 6:
            erri = currentframe().f_lineno
            break
        if not in_cmd[0] == 'HOOKUP':
            erri = currentframe().f_lineno
            break

        tguid = get_token(in_cmd[1], 'Tguid')
        if tguid == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        tname = get_token(in_cmd[2], 'Tname')
        if tname == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        if not tname in in_edt_name2type:
            erri = currentframe().f_lineno
            break
        ttype = in_edt_name2type[tname]

        dguid = get_token(in_cmd[3], 'Dguid')
        if dguid == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        dname = get_token(in_cmd[4], 'Dname')
        if dname == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        slot = get_token(in_cmd[5], 'slot')
        if slot == GBL.nota_token:
            erri = currentframe().f_lineno
            break

        newtask = (ttype, tguid)
        dbk = (dname, dguid)

        break  # while not erri

    return (erri, newtask, dbk, slot)

def get_slotcounts(in_statements, o_slotCounts):
    erri = 0
    while not erri:
        if len(in_statements) == 0:
            break

        for s in in_statements:
            if not 'EDTCreate' in s[2]:
                continue

            slots = get_token(s[3], 'slots')
            if slots == GBL.nota_token:
                erri = currentframe().f_lineno
                break

            tguid = get_token(s[4], 'Tguid')
            if tguid == GBL.nota_token:
                erri = currentframe().f_lineno
                break

            if tguid in o_slotCounts:
                if not slots ==  o_slotCounts[tguid]:
                    erri = currentframe().f_lineno
                    break
            else:
                o_slotCounts[tguid] = slots

        if erri: break

        break  # while not erri

    return erri

def buildGraph(in_statements, in_slotCounts, Gglobals, Gnodes, Gedges):
    erri=0
    while not erri:
        nodeCount = 0

        sz_statements = len(in_statements)
        #print('DBG488> len(statements) = ' + str(sz_statements))
        if sz_statements == 0:
            break

        for s in in_statements:
            #print('DBG495>' + str(s))
            if len(s) < 3:
                erri = currentframe().f_lineno
                break

            tt = get_tasktype(s[0])
            if tt == GBL.nota_tasktype:
                erri = currentframe().f_lineno
                break

            tid = get_taskID(s[1])
            if tid == GBL.nota_taskID:
                erri = currentframe().f_lineno

                break

            slots = -1
            if tid in in_slotCounts:
                slots = in_slotCounts[tid]

            #print('DBG515> tid=' + str(tid) + ' tt=' + str(tt) + ' slots=' + str(slots))

            stask = (tid,tt,slots)
            if not stask in Gnodes:
                nodeCount += 1
                Gnodes[stask] = nodeCount

            if not 'HOOKUP' in s[2]:
                # TaskName was handled and is in Gglobals
                # EDTCreate info is in in_slotCounts
                # DBKCreate info is skipped for now TODO
                continue

            if 'HOOKUP' in s[2]:
                newtid = get_token(s[3], 'Tguid')
                if newtid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break

                newtname = get_token(s[4], 'Tname')
                if newtname == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break

                newtt = GBL.nota_tasktype
                if newtname in Gglobals['edt_name2type']:
                    newtt = Gglobals['edt_name2type'][newtname]
                else:
                    print('DBG543> newtname = ' + str(newtname) + '  newtt = ' + str(newtt))
                    erri = currentframe().f_lineno
                    break

                newslots = -1
                if newtid in in_slotCounts:
                    newslots = in_slotCounts[newtid]

                dguid = get_token(s[5], 'Dguid')
                if dguid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break

                dname = get_token(s[6], 'Dname')
                if dguid == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break

                dslot = get_token(s[7], 'slot')
                if dslot == GBL.nota_token:
                    erri = currentframe().f_lineno
                    break

                new_stask = (newtid, newtt, newslots)
                if not new_stask in Gnodes:
                    nodeCount += 1
                    Gnodes[new_stask] = nodeCount

                datum = (dguid, dname, dslot)

                source = Gnodes[stask]
                drain  = Gnodes[new_stask]

                payload = (drain, datum)

                if source in Gedges:
                    Gedges[source].append(payload)
                else:
                    Gedges[source] = [payload]

        if erri: break  # for s in in_statements:

        break #while not erri
    return erri

def taskTriplet_to_label(in_triplet, in_Gglobals):
    nGuid = in_triplet[0]
    nType = in_triplet[1]
    nSlots = in_triplet[2]

    nName = in_Gglobals['edt_type2name'][nType]

    t = '"' + nName + '_sc' + str(nSlots) + '_g' + str(nGuid)  + '"'
    return t

def dataTriplet_to_label(in_triplet):
    dguid = in_triplet[0]
    dname = in_triplet[1]
    dslots = in_triplet[2]

    t = '"' + dname + '_s' + str(dslots) + '_g' + str(dguid) + '"'
    return t

def print2DOT(Gglobals, Gnodes, Gedges, guidMap2index):
    tab = GBL.TABunit
    fo = -1
    erri = 0
    while not erri:
        invGnodes = {}
        for n in Gnodes:
            invGnodes[Gnodes[n]] = n

        invGuidMap = {}
        if GBL.output_fullGuid:
            for g in guidMap2index:
                invGuidMap[str(guidMap2index[g])] = g  # Need to use a string as below it will receive strings

        fo = open(GBL.outfile,'w')
        if fo == -1:
            erri = currentframe().f_lineno
            break

        fo.write("digraph " + str(Gglobals['name']) + " {\n")

        if len(invGnodes) == 0:
            break

        for n in Gnodes:
            nn = n
            if GBL.output_fullGuid:
                id = n[0]
                guid = invGuidMap[id]
                nn = (guid, n[1], n[2])

            t = tab + taskTriplet_to_label(nn, Gglobals) + ';\n'
            fo.write(t)

        for n in Gnodes:
            source = Gnodes[n]
            if not source in Gedges:
                continue

            nn = n
            if GBL.output_fullGuid:
                id = n[0]
                guid = invGuidMap[id]
                nn = (guid, n[1], n[2])
            sourceLabel = taskTriplet_to_label(nn, Gglobals)

            for edg in Gedges[source]:
                drain = edg[0]
                datum = edg[1]

                drainTupl = invGnodes[drain]
                dt = drainTupl
                if GBL.output_fullGuid:
                    id = dt[0]
                    guid = invGuidMap[id]
                    dt = (guid, dt[1], dt[2])
                drainLabel  = taskTriplet_to_label(dt, Gglobals)

                dat = datum
                if GBL.output_fullGuid:
                    id = dat[0]
                    guid = invGuidMap[id]
                    dat = (guid, dat[1], dat[2])
                dataLabel = dataTriplet_to_label(dat)

                t = tab + sourceLabel + ' -> ' + drainLabel + ' [ label=' + dataLabel + '];\n'
                fo.write(t)

        break  # while not erri
    if not fo == -1:
        fo.write("}\n")
        fo.close()
        fo = -1

    return erri

#===============================================================================
#===============================================================================
#==  Below is the actual entry point for the program  ==========================
#===============================================================================
#===============================================================================
def Pmain():
    erri = 0

    while not erri:
        erri = get_cmdline_info(GBL)
        if erri: break
        #GBL.print2stdout()

        statements = []
        edt_type2name = {}
        edt_name2type = {}
        guidMap2index = {}  # This maps <guid in 0x123456 format> --> unique index
        erri = get_statements(GBL, statements, edt_type2name, edt_name2type, guidMap2index)
        if erri: break

        if len(statements) == 0:
            print("No debug statements provided. Exiting.")
            erri =  currentframe().f_lineno
            continue

        slotCounts = {}  # EDT taskid --> slot count at EDTCreate time
        erri = get_slotcounts(statements, slotCounts)

        Gglobals = {}
        Gglobals['name'] = GBL.infile
        Gglobals['edt_type2name'] = edt_type2name
        Gglobals['edt_name2type'] = edt_name2type

        # All nodes have the following format: (TaskID = guid, Tasktype, slotCount)
        Gnodes = {}  #  task --> node number
        Gedges = {}  # For task1 --> DBK1 --> task2 ==>> Gedges[task1] = (task2, DBK1)

        erri = buildGraph(statements, slotCounts, Gglobals, Gnodes, Gedges)
        if erri: break

        if 1 == 2:
            print(Gglobals)
            print(Gnodes)
            print(Gedges)

        erri = print2DOT(Gglobals, Gnodes, Gedges, guidMap2index)
        if erri: break

        break  # while not erri:

    if erri:
        if GBL.usage:
            Usage()
        else:
            print("ERROR: " + str(erri) )

    return erri

Pmain()






