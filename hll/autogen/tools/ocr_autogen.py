# NOTE:
# 2016may9: Quick analysis of what data flow we have
#   Source dvillen
#
# = In trying to understand the handling of data, the following concepts were made explicit.
#   They symbolize the interaction between data and a task, and the lifetime management of data:
#   There are 6 ways for any data to transit in code with respect to a given task (EDT).
#   These 6 ways are:
#           1) the take off
#                   The data is created within the task.  It is packaged to be shipped out.
#           2) The landing
#                   The data enters the task.  The data is destroyed within the task.
#           3) the touch & go
#                   The data enters the task.  It is locally modified. The data exit the graph.
#           4) the pass-through
#                   The data enters and exit the task, without any modifications.
#           5) the hop
#                   The data is created and destroyed within the task
#           6) the fly-over
#                   The fly-over has not direct interaction with the task at all.
#                   Indirectly it can influence the task.  For example, a fly-over data
#                   could use resource (tasks or memory) in such a way as to limit what
#                   could be done within a task, e.g. no memory for a hop.
#
#                     ------------ fly over -------------------->
#
#                            ==============================
#                            |                            |
#                     ------------ pass through ---------------->
#                            |                            |
#                            |                            |
#                     ------------ touch & go -\   /------------>
#                            |                  \ /       |
#                            |                   V        |
#                            |                            |
#                     --landing-\                 /--take off--->
#                            |   \               /        |
#                            |    \             /         |
#                            |                            |
#                            |                            |
#                            |       /---hop----\         |
#                            |      /            \        |
#                            |     /              \       |
#                            |                            |
#                            ==============================
#
#   The data can be presented in two states:
#       literal   -> It is a guid and no auxiliary accompanies it, e.g. template guid
#       dereferenced -> The guid or its accompanying local pointer are required, e.g. ocrDep_t for a data block.
#
#       Note that only data blocks can be either literal or dereferenced.
#       Some other entities can only be literal, e.g. template.
#       Also, currently, there is no way to convert, within a task, a literal Guid to its dereferenced form.
#
#   The data life time, creation or destruction , is implicit in the type of data flow one has.
#
#   When writing an EDT, one must classify its data with respect to the above classification.
#   In code, the above seems to give 4 bins:
#       Upon entrance:  literal or dereferenced
#                           pass-through
#                           touch & go
#                           landing
#       Upon exit: literal or dereferenced
#                           pass-through
#                           touch & go
#                           take-off
#
#   the exception being the data flow called "hop", as it has a pure local creation/destruction process.
#   So a creation and destruction process will also need to be coded.
#
#   There seems to be a limit to how many paramv one can pass into an Edt, on some systems.
#   So, at the risk of being sub-optimal, but portable, the literals will need to be stuffed
#   in a data block; ironically this data block of literals will have to be referenced.
#
# 2016may: More deliberation on the life management of globals
#           This started as BKM1: How to make ocrEdtTemplate available, similar to how to handle a global
#  Source: dvillen
#   - There are three ways to make an ocrEdtTemplate's guid available for the creation of an ocrEdt:
#     Three issues occurs: creation, deletion, transmission
#       Creation Issue:
#           option C1: create wherever needed
#           option C2: create in an appropriate centralized Edt
#           option C3: Never create, let the ocrEdtCreate worry about it
#       Deletion Issue:
#           option D1: Delete after use
#           option D2: Delete when it is known it is no longer useful, at least in, or before, FinalEdt
#           option D3: Never delete, let the system do automatic garbage collection
#       Transmission Issue:
#           Option T1: Never transmit from one Edt to another
#           Option T2: Transmit using the point-to-point method, i.e. a multi-cast,
#                      to where the template's guid is needed.
#           Option T3: Transmit using a chain-of-responsibility type of propagation
#
#   - From a user of OCR, the easiest to code is options C3-D3-T1, of which only D3 is currently doable
#
#   - C3 is not currently supported, and C2 usually disqualifies T1
#     D1 only works if the template is used once, or if C1 is used.
#     D1 forces T1, as the template guid cannot be sent if destroyed.
#     D2 requires a transmission and a synchronization.
#
#   - So C1 & C2 remains.
#     D1 implies C1.  D1 also implies T1.
#           ==>> First possible strategy:  C1-D1-T1
#   - Using C2, now T1 is out. So the only viable choices are: C2-T2 and C2-T3
#     The choice of D2 or D3 seems to be an after thought in the context of C2.
#     Assume that we are good citizen, and clean up after use: This removes D3.
#     T2 only works if the target EDTs are known a-priori
#     ==>> So (C1|C2)-T2-D2  --> point-to-point
#     ==>> So (C1|C2)-T3-D2  --> Chain-of-responsibility
#   -Under chain-of-responsibility, one must not feed, all the time, a template's guid as a depv argument.
#    It has to be transported in a data block whose guid is propagated.
#       So, under T3, one must properly modulate the guids between literal and dereferenced, if that is possible.
#       Also T3 does impose a penalty of carrying around literal guids as everything needs to be passed down the lines.
#
#   -Finally, in the case of templates guids, because these can be scattered throughout the code,
#    one may have to decide where to destroy them.  The simplest is to destroy in the final EDT.
#
#   ==> Considering the above, from an app API point-of-view, the simplest is to use
#           C1-D1-T1
#       PROs are:
#           No scatter, nor gather needed for templates
#           Wrapping EDT creation in one function completely removes templates from the API
#       CONs:
#           The accounting done by the app-side has a bigger counterpart on the kernel-side of OCR,
#           then when templates are managed within the app.
#
#
# 2016may11: BKM1: Handling of literals
#  Source: Donald Villeneuve
#       There seems to be a limit to how many paramv one can pass into an Edt, on some systems.
#       So, at the risk of being sub-optimal, but portable, the literals will need to be stuffed
#       in a data block; ironically this data block of literals will have to be referenced.
#   - Put all literals in at least one data block called, say, "Literals"
#   - One may split the Literals block between "LiteralsCONST" and "LiteralsRW".
#     "LiteralsCONST" are literals guaranteed to be either ReadOnly or Constant in the current EDT.
#     "LiteralsRW" are literals that are either Read-Write or Exclusive-Write
#
# 2016may6: BKM2: For general creation of EDTs and their templates
#  Source: Donald Villeneuve
#   - Avoid the "return EdtGuid" mechanism at the exit of an EDT
#   - Use C1-D1-T1 as presented by the provided function app_ocr_util.h::ocrEdtXCreate(...)
#
# 2016may11: BKM3: Release data blocks only if they have been dereferenced
#   - As they are two types of data motions: literals and dereferenced, not counting paramv,
#     ocrDbRelease is only applied to dereferenced data blocks.
#     This is a shortcut for now.  Data blocks could be released from only their guids;
#     so one could stuff them in the Literals set.  But it is felt that
#     data blocks passed in as literals are on a pass-through flight, and should
#     not be released in that state.
#     It seems like a good idea at this time.  Time will tell...

import inspect  # for error line handling & the inspection stack
# from inspect import currentframe,stack # for error line handling only
from collections import deque
from sys import exit
import copy

import networkx as GraphTool

# ------------------------------------------------------------------------------
def whereAmI():
    print("//================== DEV> Currently in: " + inspect.stack()[1][3] )
# ------------------------------------------------------------------------------
# The class Globals contains all global variables and constants.
# It forms a singleton named G.
class Globals:
    def __init__(self):
        self.MAINNODE = 0
        self.FINALNODE= 1
        self.NOTA_NODE_ID = -1
        self.NOTA_OFFSET = -1
        self.genAUTO = "_auto_"  #Use genAUTO to let the system determine the dependency count.
        self.TABunit = "    "
        self.app_ocr_util_filename = '../../tools/app_ocr_util.h'

        self.TMPL_structName_type = 'OCRmetaData_t'
        self.TMPL_structName      = 'OCRmetaData'

        self.DBK_structName_type = 'OCR_DBKs_t'
        self.DBK_structName      = 'OCR_DBKs'

        self.EDT_longform_print = False   # Set to True in order to get everything
                                          # outputted during ToDictio conversion, i.e. printout.
        self.DBK_longform_print = False  # Set to True in order to get everything
                                         # outputted during ToDictio conversion, i.e. printout.

        self.use_long_edge_name = 0

        self.display_literals_content = 1

        self.notaSlot = -1  # The undefined value for a slot offset

        self.nota_multiplicity = -1  # The not-in-use value for multiplicity: mAtOrigin & mAtDest

        self.debug_DBK_flights = False

        self.use_verbose_way_to_eventsText = False

        self.use_old_sizeName = False # Set to True: When creating a DBK, its byte size will be printed out
                                      #              using self.text_for_sizeName() + '*sizeof(' + self.type + ')'
                                      # Otherwise:  The value of ocrDataBlock.count has been overloaded as follows:
                                      #                 if isinstance(self.count, int):
                                      #                     bytesize = str(self.count) + '*sizeof(' + self.type + ')'
                                      #                 elif isinstance(self.count, str):
                                      #                     bytesize = self.count
                                      #             When count is a string, the text for bytsize is to be provided
                                      #             directly by the user.

        self.insert_debug_code = 0  # IF set to a non-zero non-negative integer, debug code will be inserted in order
                                    # to hopefully help with debugging.

GBL = Globals()
# ------------------------------------------------------------------------------
def makeGuidTemplateName(in_name):
    return "gt_" + in_name
# ------------------------------------------------------------------------------
def makeGuidEdtname(in_name):
    return "ga_" + in_name
# ------------------------------------------------------------------------------
def makeGuidEventname(in_name):
    if in_name == 'NULL_GUID':
        return in_name
    return "ge_" + in_name
# ------------------------------------------------------------------------------
def makeGuidDataBlockname(in_name):
    return "gd_" + in_name
# ------------------------------------------------------------------------------
def makeNameDataBlock(in_name):
    return "db_" + in_name
# ------------------------------------------------------------------------------
def makeNameDataBlockDep(in_name):  #For ocrEdtDep_t
    return "dd_" + in_name
# ------------------------------------------------------------------------------
def makeNameTemplate(in_name):
    return "tmpl_" + in_name
# ------------------------------------------------------------------------------
def makeNameEvent(in_name):
    return "evt_" + in_name
# ------------------------------------------------------------------------------
def makeEdtDepName(in_name):
    return "dep_" + in_name
# ------------------------------------------------------------------------------
def makeNameTask(in_name):
    return "tsk_" + in_name
# ------------------------------------------------------------------------------
def makeEdgeName(in_G, in_edgeTuple):
    # in_edgeTuple = edge where edge is (nodeA,nodeB) and node(A|B) are node numbers.
    if GBL.use_long_edge_name:
        n0 = getMyTask(in_G, in_edgeTuple[0]).aname
        n1 = getMyTask(in_G, in_edgeTuple[1]).aname
        t = 'E'+ n0 +'_'+ n1
    else:
        t = 'E'+str(in_edgeTuple[0]) +'_'+ str(in_edgeTuple[1])
    return t;
# ------------------------------------------------------------------------------
def makeEdgeNameStruct(in_G, in_edgeTuple):
    t = makeEdgeName(in_G, in_edgeTuple)
    return (t+'_t')
# ------------------------------------------------------------------------------
def makeInLiteralsName(in_G, in_edgeTuple):
    litname = 'IN_literals_' + makeEdgeName(in_G, in_edgeTuple)
    return litname
# ------------------------------------------------------------------------------
def makeInLiteralsNameStruct(in_G, in_edgeTuple):
    slitname = 'IN_literals_' + makeEdgeNameStruct(in_G, in_edgeTuple)
    return slitname
# ------------------------------------------------------------------------------
def makeInDerefsName(in_G, in_edgeTuple):
    derefname = 'IN_derefs_' + makeEdgeName(in_G, in_edgeTuple)
    return derefname
# ------------------------------------------------------------------------------
def makeOutLiteralsName(in_G, in_edgeTuple):
    litname = 'OUT_literals_' + makeEdgeName(in_G, in_edgeTuple)
    return litname
# ------------------------------------------------------------------------------
def makeOutLiteralsNameStruct(in_G, in_edgeTuple):
    slitname = 'OUT_literals_' + makeEdgeNameStruct(in_G, in_edgeTuple)
    return slitname
# ------------------------------------------------------------------------------
def makeAllSlotsName(in_G, in_edgeTuple):
    ssname = 'allSlots_' + makeEdgeName(in_G, in_edgeTuple)
    return ssname
# ------------------------------------------------------------------------------
def makeOutDerefsName(in_G, in_edgeTuple):
    derefname = 'OUT_derefs_' + makeEdgeName(in_G, in_edgeTuple)
    return derefname
# ------------------------------------------------------------------------------
class contextedGuid:
    def __init__(self, in_structName, in_guidName, in_ocrObjectEnumText):
        self.structName = in_structName # Leave in_structName as an empty string if the guid is not part of a C struct.
        self.guidName = in_guidName
        self.ocrType = in_ocrObjectEnumText
        if ocrObjectEnum_is_bad(self.ocrType):
            self.ocrType = 'otUNDEF'

    def toName(self):
        t = self.structName + self.guidName
        return t

    def getDestroyText(self):
        t = ""
        g = self.structName
        g += self.guidName
        if self.ocrType == 'DBK':
            t = 'err = ocrDbDestroy( '+ g +' ); IFEB;'
        elif self.ocrType == 'TMPL':
            t = 'err = ocrEdtTemplateDestroy( '+ g +' ); IFEB;'
        elif self.ocrType == 'TASK':
            t = 'err = ocrEdtDestroy( '+ g +' ); IFEB;'
        elif self.ocrType == 'EVT':
            t = 'err = ocrEventDestroy( '+ g +' ); IFEB;'
        return t

    def getReleaseText(self):
        t = ""
        g = self.structName
        g += self.guidName
        if self.ocrType == 'DBK':
            t = 'err = ocrDbRelease(' + g + ' ); IFEB;'
        return t

    def toTuple(self):
        return self.structName, self.guidName, self.ocrType
# ------------------------------------------------------------------------------
#NOTE: All data blocks are to be released before any ocrEventSatisfy
class ocrDataBlock:
    def __init__(self):
        self.name         = ""  # The unique name given  to the data block.
                                # NOTE: The name 'NULL_GUID' is reserved to indicate a pure virtual
                                #      data block which can be used to satisfy an event,
                                #      i.e. ocrEventSatisfy
        self.count        = 0  # If it is an integer >=0,
                               #    the number of bytes used in ocrDbCreate will be str(self.count) * sizeof(self.type)
                               # If it is a text string, i.e. Python type str, then that provided string
                               #    will be used as-is
                               # If count is set to a negative integer, then data block creation will be omitted
                               #    entirely and will be the responsibility of the user.
        self.type         = 'char' # text string of the Type T of the elements in the block
        self.flags        = 'DB_PROP_NONE'
        self.hint         = 'NULL_HINT'
        self.allocator    = 'NO_ALLOC'
        self.flight       = 'flUNDEF'  # See flightEnum_is_bad(in_flightEnum) for options.
        self.localname    = ""  # This is the name the customer would like to use upon LANDing
                                # or flTAGOing.  So basically how to call the ocrEdtDep_t.ptr.
                                # The pointer generated will be of type (void*).
                                # If not specified, no customer friendly named pointer will be inserted.

        self.localnameGuid  = ""  # This is the ocrGuid_t name the customer would like to use upon LANDing
                                  # or flTAGOing.  So basically how to call the ocrEdtDep_t.guid.
                                  # If not specified, no customer friendly guid name will be inserted.

        self.localText = []  # This is closely related to self.localname field. It is to store a list of cutomer OCR
                             # code to be placed in close proximity to self.localname.  For example, it can
                             # further help with the unpacking of the DBK.
                             # This only works for flLANDING and flTAGO DBKs.

        self.delayReleaseDestroy = False # This is a way to modulate the destruction/release sequence of a data
                                         # block up until a certain pattern occured.
                                         #2016aug16:  Used only with the FOR pattern, in order to delay
                                         #            until after the FOR loop.

        self.user2destroyORrelease = False # If set to True, the user is now held
                                           # responsible for the release or destruction
                                           # of the data block.

    def toDictio(self):
        longform = GBL.DBK_longform_print
        d = {}
        if longform:
            d["count"] = self.count
            d["type"] = self.type
            d["flags"] = self.flags
            d["hint"] = self.hint
            d["allocator"] = self.allocator
            d["localText"] = self.localText
        d["name"] = self.name
        d["flight"] = self.flight
        d["localname"] = self.localname
        d["localnameGuid"] = self.localnameGuid
        d["delayReleaseDestroy"] = self.delayReleaseDestroy
        d["user2destroyORrelease"] = self.user2destroyORrelease

        return d

    def addLocalText(self, in_text):
        self.localText.append(in_text)
    def getLocalTexts(self):
        return self.localText

    def text_for_sizeName(self):
        t = 'OA_Count_' + self.name
        return t

    def text_for_ocrDbCreate(self, in_tab, in_True_to_use_old_sizeName):
        if self.name == 'NULL_GUID':
            t = in_tab + '//Skipping creation of a NULL_GUID data block.'
            return t

        gd_name = makeGuidDataBlockname(self.name)

        db_name = makeNameDataBlock(self.name)
        if len(self.localname) >0:
            db_name = self.localname

        if isinstance(self.count, int) and self.count < 0:
            t = in_tab + '//Creating for data block "" is the user\'s responsibility.'
            t = in_tab + 'ocrGuid_t ' + gd_name + '= NULL_GUID;\n'
            t += in_tab + self.type + ' * ' + db_name + '=NULL;\n'
            return t

        t = in_tab + 'ocrGuid_t ' + gd_name + '= NULL_GUID;\n'
        t += in_tab + self.type + ' * ' + db_name + '=NULL;\n'

        t += in_tab + 'err = ocrDbCreate( &' + gd_name + ', (void**)&'+ db_name

        bytesize = ""
        if in_True_to_use_old_sizeName:
            bytesize = self.text_for_sizeName() + '*sizeof(' + self.type + ')'
        else:
            if isinstance(self.count, int):
                bytesize = str(self.count) + '*sizeof(' + self.type + ')'
            elif isinstance(self.count, str):
                bytesize = self.count
            else:
                bytesize = 'ERROR type unknown'

        t += ', ' + bytesize
        t += ', ' + self.flags
        t += ', ' + self.hint # This should be NULL_HINT as it is not in current use (2016May5)
        t += ', ' + self.allocator +'); IFEB;\n'

        return t

    def dbg_text_for_ocrDbCreate(self, in_tab, io_file):
        if not GBL.insert_debug_code:
            return
        gd_name = makeGuidDataBlockname(self.name)
        if self.name == 'NULL_GUID':
            gd_name = '0g'
        t = in_tab + 'PRINTF("OA_DBG> TaskTYPE=%d TaskID="GUIDF" DBKCreate Dguid="GUIDF" Dname=%s'
        t += '\\n'
        t += '", OA_edtTypeNb, GUIDA(OA_DBG_thisEDT), GUIDA('
        t += gd_name + '), STRINGIZE(' + self.name + '));\n'
        io_file.write(t)

# ------------------------------------------------------------------------------
class ocrEdtTemplate:
    def __init__(self):
        self.name       = GBL.genAUTO #On GBL.genAUTO, the template name and guid is derived from the name of its first EDT.
        self.funcPtrName= GBL.genAUTO
        self.paramc     = GBL.genAUTO
        self.depc       = GBL.genAUTO
    def toDictio(self):
        d={}
        d["name"]=self.name
        d["funcPtrName"] = self.funcPtrName
        d["paramc"] = self.paramc
        d["depc"] = self.depc
        return d
    def setToFullAuto(self):
        self.funcPtrName=GBL.genAUTO
        self.paramc = GBL.genAUTO
        self.depc = GBL.genAUTO
# ------------------------------------------------------------------------------
class ocrEdt:
    def __init__(self):
        self.aname          = "" #The name of this EDT (The a is for task)
        # For the creation of the EDT
        #   = The values of self.paramc & self.depc are always going to be 'EDT_PARAM_DEF'
        #     at the ocrEdtCreate point for this particular ocrEdt.
        #   = The actual values of self.paramc & self.depc will be set either automatically (use GBL.genAUTO)
        #     or by the user (a non-zero integer).
        #     These values will be used for the creation of this Edt's template
        #     and for slot accounting.
        self.paramc         = GBL.genAUTO   # The choices are: an non-negative integer or GBL.genAUTO
        self.paramv         = GBL.genAUTO
        self.depc           = GBL.genAUTO   # The choices are: an non-negative integer or GBL.genAUTO
        self.depv           = GBL.genAUTO
        self.aflags         = 'EDT_PROP_NONE'  # Also: 'EDT_PROP_FINISH' 'EDT_PARAM_UNK' 'EDT_PARAM_DEF'
        self.hint           = 'NULL_HINT'
        self.outputEvent    = 'NULL'

    def toDictio(self):
        longform = GBL.EDT_longform_print
        d={}
        d["aname"]           = self.aname
        d["depc"]            = self.depc
        d["depv"]            = self.depv
        if longform:
            d["paramc"] = self.paramc
            d["paramv"] = self.paramv
            d["flags"]       = self.aflags
            d["hint"]        = self.hint
            d["outputEvent"] = self.outputEvent
        return d

    def text_for_ocrEdtCreate(self, in_tab):
        ga_name = makeGuidEdtname(self.aname)

        t  = in_tab+ 'ocrGuid_t '+ ga_name +' = NULL_GUID;\n'
        t += in_tab+ 'err = ocrEdtXCreate(' + self.aname
        t += ', '+ str(self.paramc) #BKM2
        t += ', '+ self.paramv #BKM2
        t += ', '+ str(self.depc)   #BKM2
        t += ', '+ self.depv #BKM2 #<-- During creation, depv is of type ocrGuid_t
        t += ', '+ self.aflags
        t += ', '+ self.hint
        t += ', &'+ ga_name
        t += ', '+ self.outputEvent
        t += '); IFEB;\n'
        return t

    def dbg_text_for_ocrEdtCreate(self, in_tab, io_file):
        if not GBL.insert_debug_code:
            return
        ga_name = makeGuidEdtname(self.aname)
        t = in_tab + 'PRINTF("OA_DBG> TaskTYPE=%d TaskID="GUIDF" EDTCreate slots=%d  Tguid="GUIDF"  Tname=%s\\n"'
        t += ', OA_edtTypeNb, GUIDA(OA_DBG_thisEDT), ' + str(self.depc) + ', GUIDA('
        t += ga_name + '), STRINGIZE(' + self.aname + '));\n'
        io_file.write(t)

# ------------------------------------------------------------------------------
class ocrEvent:
    def __init__(self):
        #For ocrEventCreate
        self.etype = 'OCR_EVENT_ONCE_T' # Other options are 'OCR_EVENT_ONCE_T'  'OCR_EVENT_IDEM_T' 'OCR_EVENT_STICKY_T' 'OCR_EVENT_LATCH_T'
        self.eflag = 'EVT_PROP_TAKES_ARG' # Other options are "EVT_PROP_NONE" 'EVT_PROP_TAKES_ARG'

        #For ocrAddDependence
        self.accessMode = 'DB_MODE_RW' #All options are: 'DB_MODE_RW', 'DB_MODE_EW', 'DB_MODE_RO', 'DB_MODE_CONST'. 'DB_MODE_NULL'

        #For ocrEventSatisfySlot & ocrAddDependence
        self.calculatedSlot = GBL.notaSlot

        self.user_slotnb_text = ""  # This is the user provided text which would replace self.calculatedSlot

        #For ocrEventSatisfySlot
        self.satisfy ="" # If len(self.satisfy)==0 --> No need for an ocrEventSatisfy statement
                         # The valid entry are
                         #      'NULL_GUID'
                         #      a guid by textual name
                         #      a contextedGuid

        self.fertile = True  # Set to True if the parent EDT will create the children EDT; False otherwise.
                             # Here the parent EDT refers to the EDT where this event starts;
                             # and the children EDT is where this event ends.

        self.isTheLead = True # Set to True if this edge DBK is the one to consider for deletion during flLANDING.
                              # This is used to avoid deletion of the same data block multiple times.

        # The following is intended for the compact graph representation.
        self.mAtOrigin = GBL.nota_multiplicity  # Edge multiplicity at the emitting EDT, i.e.
                                                # how many edges does the originating EDT emits.
        self.mAtDest   = GBL.nota_multiplicity  # Edge multiplicity at the receiving EDT, i.e.
                                                # how many edges does the destination EDT receives.

    def toDictio(self):
        d={}
        d["etype"] = self.etype
        d["eflag"] = self.eflag
        d["accessMode"] = self.accessMode
        d["calculatedSlot"] = self.calculatedSlot
        d["user_slotnb_text"] = self.user_slotnb_text
        d["satisfy"] = self.satisfy
        d["fertile"] = self.fertile
        d["isTheLead"] = self.isTheLead
        d["mAtOrigin"] = self.mAtOrigin
        d["mAtDest"] = self.mAtDest
        return d

    def text_for_ocrEventCreate(self, in_tab, in_eventGuidName):
        t = in_tab + 'ocrGuid_t ' + in_eventGuidName +' = NULL_GUID;\n'
        t += in_tab + 'err = ocrEventCreate( &' + in_eventGuidName
        t += ', ' + self.etype
        t += ', ' + self.eflag + '); IFEB;\n'
        return t

    def text_for_ocrAddDependence(self, in_tab, in_sourceEventGuidName, in_targetEdtGuidName):
        t = in_tab + 'err = ocrAddDependence( ' + in_sourceEventGuidName
        t += ', ' + in_targetEdtGuidName
        slottext = str(self.calculatedSlot)
        if not self.user_slotnb_text == "":
            slottext = self.user_slotnb_text
        t += ', ' + slottext
        t += ', ' + self.accessMode + '); IFEB;\n'
        return t

    def text_for_ocrEventSatisfySlot(self, in_tab, in_eventGuidName, in_dataGuid, in_true_to_useEventSlots=None):
        t = ""
        if in_true_to_useEventSlots == None:
            in_true_to_useEventSlots = False
        if in_true_to_useEventSlots:
            t = in_tab + 'err = ocrEventSatisfySlot( ' + in_eventGuidName
            t += ', ' + in_dataGuid
            t += ', ' + str(0) + '); IFEB;\n'  # self.calculatedSlot is not an event slot
        else:
            t = in_tab + 'err = ocrEventSatisfy( ' + in_eventGuidName
            t += ', ' + in_dataGuid + '); IFEB;\n'
        return t

    def text_for_ocrHookUp(self, in_tab, in_targetEdtGuidName, in_dataGuid):
        t = in_tab
        t += 'err = ocrXHookup(' + self.etype + ', ' + self.eflag + ', ' + in_targetEdtGuidName + ', '
        slottext = str(self.calculatedSlot)
        if not self.user_slotnb_text == "":
            slottext = self.user_slotnb_text
        t += slottext + ', ' + self.accessMode + ', ' + in_dataGuid + '); IFEB;\n'
        return t

    def dbg_text_for_ocrHookUp(self, in_tab, in_targetEdtGuidName, in_targetEdtName, in_dataGuid, in_dataID, io_file):
        if not GBL.insert_debug_code:
            return
        slottext = str(self.calculatedSlot)
        if not self.user_slotnb_text == "":
            slottext = self.user_slotnb_text

        t = in_tab
        t += 'PRINTF("OA_DBG> TaskTYPE=%d TaskID="GUIDF" HOOKUP Tguid="GUIDF" Tname=%s Dguid="GUIDF" Dname=%s  slot=%d\\n"'
        t += ', OA_edtTypeNb, GUIDA(OA_DBG_thisEDT), GUIDA('
        t += in_targetEdtGuidName + '), STRINGIZE(' + in_targetEdtName + ')'
        t += ', GUIDA(' + in_dataGuid + '), STRINGIZE(' + in_dataID + '), ' + slottext
        t += ');\n'
        io_file.write(t)

# ------------------------------------------------------------------------------
def errmsg(in_erri, in_text=""):
    if not in_erri:
        return
    if len(in_text)==0:
        print("ERROR: " + str(in_erri))
    else:
        print("ERROR: " + str(in_erri) +" : "+ in_text)
# ------------------------------------------------------------------------------
def warnmsg(in_erri, in_text=""):
    if len(in_text) == 0:
        print("WARNING: " + str(in_erri))
    else:
        print("WARNING: " + str(in_erri) + " : " + in_text)
# ------------------------------------------------------------------------------
def setupGraph(io_G, in_name):
    io_G.graph['name'] =in_name
    io_G.graph['nodeIDs'] = {}
    io_G.graph['customerHeader'] = []
# ------------------------------------------------------------------------------
# This list of text will be put at the beginning of the file
def addHeader(io_G, in_text):
    io_G.graph['customerHeader'].append(in_text)
# ------------------------------------------------------------------------------
def getHeader(io_G):
    return io_G.graph['customerHeader']
# ------------------------------------------------------------------------------
def getNode(io_G,in_nodeNumber ):
    return io_G.node[in_nodeNumber]
# ------------------------------------------------------------------------------
def getNodeNames(in_G):
    return in_G.graph['nodeIDs']
# ------------------------------------------------------------------------------
def getNodebyitsName(in_G, in_name):
    if in_name in getNodeNames(in_G):
        return getNodeNames(in_G)[in_name]
    else:
        return GBL.NOTA_NODE_ID
# ------------------------------------------------------------------------------
def getNodeNamebyitsNumber(in_G, in_nodeNumber):
    return getMyTask(in_G, in_nodeNumber).aname
# ------------------------------------------------------------------------------
def getNodeparents(io_G,in_nodeNumber):
    return io_G.node[in_nodeNumber]["parents"]
# ------------------------------------------------------------------------------
def getMyTask(io_G, in_nodeNumber):
    return getNode(io_G,in_nodeNumber)["myTask"]
# ------------------------------------------------------------------------------
def getEdts(io_G, in_nodeNumber):
    return getNode(io_G,in_nodeNumber)["edts"]
# ------------------------------------------------------------------------------
def getDataBlocks(io_G, in_nodeNumber):
    return getNode(io_G,in_nodeNumber)["datablock"]
# ------------------------------------------------------------------------------
def getDestroys(io_G, in_nodeNumber):
    return getNode(io_G,in_nodeNumber)["destroy"]
# ------------------------------------------------------------------------------
def getReleases(io_G, in_nodeNumber):
    return getNode(io_G,in_nodeNumber)["release"]
# ------------------------------------------------------------------------------
def getOutgoings(io_G, in_nodeNumber):
    return getNode(io_G,in_nodeNumber)["outgoing"]
# ------------------------------------------------------------------------------
def addCustomText(io_G, in_nodeNumber, in_text):
    getNode(io_G, in_nodeNumber)["customerText"].append(in_text)
# ------------------------------------------------------------------------------
def getCustomTexts(io_G, in_nodeNumber):
    return getNode(io_G, in_nodeNumber)["customerText"]
# ------------------------------------------------------------------------------
def addFORconditionText(io_G, in_nodeNumber, in_text):
    getNode(io_G, in_nodeNumber)["FORconditionText"] = in_text
# ------------------------------------------------------------------------------
def getFORconditionText(io_G, in_nodeNumber):
    return getNode(io_G, in_nodeNumber)["FORconditionText"]
# ------------------------------------------------------------------------------
def addFORconditionText_startingclause(io_G, in_nodeNumber, in_text):
    getNode(io_G, in_nodeNumber)["FORconditionText_startingclause"].append(in_text)
# ------------------------------------------------------------------------------
def getFORconditionText_startingclause(io_G, in_nodeNumber):
    return getNode(io_G, in_nodeNumber)["FORconditionText_startingclause"]
# ------------------------------------------------------------------------------
def addIFconditionText(io_G, in_nodeNumber, in_text):
    getNode(io_G, in_nodeNumber)["IFconditionText"] = in_text
# ------------------------------------------------------------------------------
def getIFconditionText(io_G, in_nodeNumber):
    return getNode(io_G, in_nodeNumber)["IFconditionText"]
# ------------------------------------------------------------------------------
def graphAddNode(io_G,in_nodeNumber, in_nodename):
    erri=0
    while not erri:
        if not 'nodeIDs' in io_G.graph:
            erri=inspect.currentframe().f_lineno
            errmsg(erri, "Missing nodeIDs dictionary in the provided graph.")
            break

        io_G.add_node(in_nodeNumber)
        io_G.graph['nodeIDs'][in_nodename]=in_nodeNumber

        getNode(io_G, in_nodeNumber)["parents"] = []    # A list of node numbers whose edge (parent->thisNode) points
                                                        # to this node.

        getNode(io_G, in_nodeNumber)["datablock"] = []  # a list of ocrDataBlock
        getNode(io_G, in_nodeNumber)["destroy"] = []    # a list of contextedGuid
        getNode(io_G, in_nodeNumber)["release"] = []  # a list of contextedGuid
        getNode(io_G, in_nodeNumber)["edts"] = []  # a list of ocrEdt objects that needs to be created by the current EDT.
                                                   # It can contain reference to specific EDT by their graph node name.

        getNode(io_G, in_nodeNumber)["outgoing"] = []  # This is for data blocks which are flTAGO or flPASSTRU.
                                                       # It stores (DBK name, DBK guid)

        getNode(io_G, in_nodeNumber)["customerText"] = []  # This is the text line given by the customer/programmer
                                                           # will be added.  They usually are for code.

        getNode(io_G, in_nodeNumber)["FORconditionText"] = ""  # - If not empty, the EDT is a FOR pattern; otherwise not.
                                                               # - It is mutually exclusive with a IF-THEN-ELSE pattern
                                                               # - The text must follow the following format:
                                                               #        for(<init>; <condition>; <increment>)
                                                               #    = It is the same format as the C equivalent IF statement.
                                                               #    = The parenthesis are required, as is the for.

        getNode(io_G, in_nodeNumber)["FORconditionText_startingclause"] = []  # - If not empty, this is a list of text which will be
                                                                              #   added after the opening '{' if, and only if
                                                                              #   the "FORconditionText" content is not empty.

        getNode(io_G, in_nodeNumber)["IFconditionText"] = ""  # - If not empty, this indicates that this nodes if the THEN
                                                              # part of the IF-THEN-ELSE pattern.
                                                              # - It is mutually exclusive with a FOR pattern
                                                              # - For the ELSE part, see leads_to_ElseClause on the edges.
                                                              # - This text will literally use as follows:
                                                              #         if( IFconditionText ){
                                                              #             MyTask  # From getMyTask
                                                              #         } else{
                                                              #             EDT from edge marked with leads_to_ElseClause
                                                              #         }

        getNode(io_G, in_nodeNumber)["istheELSEclause"] = False  # This is the counterpart to ["IFconditionText"]
                                                                 # If ["IFconditionText"] is not empty, then there must
                                                                 # be another EDT which will be the ELSEclause.
                                                                 # That other EDT must have its ["istheELSEclause"]
                                                                 # field set to True. See leads_to_ElseClause on the edges.

        getNode(io_G, in_nodeNumber)["myTask"] = ocrEdt()  # This is used for the creation of this Edt encapsulated by this node.
        getMyTask(io_G, in_nodeNumber).aname = in_nodename

        break #while not erri
    return erri
# ------------------------------------------------------------------------------
def flightEnum_is_bad(in_flightEnum):
    q = ['flUNDEF', 'flPASSTRU', 'flTAGO', 'flLANDING', 'flTAKEOFF', 'flHOP', 'flFLYOVER']

    if in_flightEnum in q:
        if in_flightEnum == 'flFLYOVER':
            return True # Mark 'flFLYOVER' as bad  because it is not currently in use
        return False
    return True
# ------------------------------------------------------------------------------
def ocrObjectEnum_is_bad(in_ocrType):
    q = [ 'otUNDEF', 'DBK', 'TMPL', 'TASK', 'EVT' ]
    if in_ocrType in q:
        if in_ocrType == 'otUNDEF':
            return 1 #Mark as bad 'otUNDEF' as it should not be used
        return 0 # Zero means false
    return 1
# ------------------------------------------------------------------------------
def addDataBlocks(io_G, in_nodeNumber, in_ocrDataBlock):
    getDataBlocks(io_G, in_nodeNumber).append(in_ocrDataBlock)
# ------------------------------------------------------------------------------
def addToBeDestroyed(io_G, in_nodeNumber, in_contextedGuid):
    getNode(io_G, in_nodeNumber)["destroy"].append(in_contextedGuid)
# ------------------------------------------------------------------------------
def addToOutgoing(io_G, in_nodeNumber, in_dnk_name_guid_2tuple):
    getNode(io_G, in_nodeNumber)["outgoing"].append(in_dnk_name_guid_2tuple)
# ------------------------------------------------------------------------------
def addToBeReleased(io_G, in_nodeNumber, in_contextedGuid):
    getNode(io_G, in_nodeNumber)["release"].append(in_contextedGuid)
# ------------------------------------------------------------------------------
def edgeNames2tuple(in_G, in_from_node_name, in_to_node_name):
    edgetuple = (GBL.NOTA_NODE_ID, GBL.NOTA_NODE_ID)
    if not 'nodeIDs' in in_G.graph:
        return edgetuple
    fromId = in_G.graph['nodeIDs'][in_from_node_name]
    toId   = in_G.graph['nodeIDs'][in_to_node_name]
    return (fromId, toId)
# ------------------------------------------------------------------------------
def getEdge(io_G, in_edgeTuple):
    # in_edgeTuple = edge where edge is (nodeA,nodeB) and node(A|B) are node numbers.
    return io_G.edge[in_edgeTuple[0]][in_edgeTuple[1]]
# ------------------------------------------------------------------------------
def getEvents(io_G, in_edgeTuple):
    # in_edgeTuple = edge where edge is (nodeA,nodeB) and node(A|B) are node numbers.
    return getEdge(io_G, in_edgeTuple)["events"]
# ------------------------------------------------------------------------------
def literals(io_G, in_edgeTuple):
    # in_edgeTuple = edge where edge is (nodeA,nodeB) and node(A|B) are node numbers.
    return getEdge(io_G, in_edgeTuple)["literals"]
# ------------------------------------------------------------------------------
def dereferenciables(io_G, in_edgeTuple):
    # in_edgeTuple = edge where edge is (nodeA,nodeB) and node(A|B) are node numbers.
    return getEdge(io_G, in_edgeTuple)["derefs"]
# ------------------------------------------------------------------------------
def getEvent(io_G, in_labeled_edgeTuple):
    # in_labeled_edgeTuple = ( edge, label) where edge is (nodeA,nodeB) and label is a string
    return getEdge(io_G, in_labeled_edgeTuple[0])["events"][in_labeled_edgeTuple[1]]
# ------------------------------------------------------------------------------
def sharedConx(io_G, in_edgeTuple):
    # in_edgeTuple = edge where edge is (nodeA,nodeB) and node(A|B) are node numbers.
    return getEdge(io_G, in_edgeTuple)["sharedConx"]
# ------------------------------------------------------------------------------
def graphAddEdge(io_G, in_from_nodeName, in_to_nodeName, in_label):
    #whereAmI()
    e=edgeNames2tuple(io_G, in_from_nodeName, in_to_nodeName)
    io_G.add_edge(e[0], e[1])
    if "events" in getEdge(io_G, e):
        getEdge(io_G, e)["events"].update({in_label: ocrEvent()})
    else:
        getEdge(io_G, e)["events"] = {in_label: ocrEvent()}
    if not "literals" in getEdge(io_G, e):
        getEdge(io_G, e)["literals"] = [] # A list by text name of the data blocks that needs to be received as literals
        getEdge(io_G, e)["derefs"] = [] # A list by text name of the data blocks that needs to be received and dereferenced.
        getEdge(io_G, e)["slotCount"] = GBL.notaSlot  # A non-zero positive integer counting how many slots an edge requires.
                                                      # The slot count of an EDT is the sum(slotCount, over all edges incident on that EDT).

        # -An edge can share a connection with another edge.  This means that both edges will be using
        # the same slot.  The current edge, the one with the valid "sharedConx" should be the one
        # holding the overloading the connection.
        # -A connection is made up of all slots an edge uses upon entry in a node.
        # -If a connection is shared between two edges, then both edges must have
        #  identically matching paramv and depv.
        # -For example, a shared edge can occur during a recursion pattern.
        #==>"sharedConx" is used to break cycles in the Graph in order to have a simple DAG.
        #    The graph will be broken on the edge having the "sharedConx" set.
        #2016Sept12: The restriction of a sharedConx to the entire set of events on an edge
        #            is too restrictive.  --> See comments from 2016aug17 for example
        getEdge(io_G,e)["sharedConx"] = []  # A list of (node source, node target) 2-tuple
                                            # This list should have a length of at most one.
                                            # I had to use a list in order to get a nice accessor sharedConx()

        getEdge(io_G,e)["leads_to_ElseClause"] = False  # - This is intended for the IF-THE-ELSE pattern.
                                                        # - Set to True is this leads to the False Clause
                                                        #   of the IF-THEN_ELSE pattern.

    return e, in_label
# ------------------------------------------------------------------------------
def printGraph(in_G):
    whereAmI()

    tab="  "
    tab2 = tab + GBL.TABunit
    print("Graph name = " + in_G.graph['name'])

    if len(in_G.graph) >0:
        for a in in_G.graph:
            print(tab),
            print(str(a) +" = "),
            print(in_G.graph[a])

    nlen=len(in_G.nodes())
    print(tab + "node count = " + str(nlen))
    if nlen >0:
        for n in in_G.nodes():
            print(tab + "node = " + str(n))
            attributes = getNode(in_G,n)
            print(tab2),
            print(attributes)

            mytask = getMyTask(in_G, n)
            print(tab2 +"EDT:"+ str(hex(id(mytask))) ),
            print(mytask.toDictio())

            edts = getEdts(in_G,n)
            for edt in edts:
                if isinstance(edt,ocrEdt):
                    print(tab2 +"EDT:"+ str(hex(id(edt))) ),
                    print(edt.toDictio())
                else:
                    print(tab2 + 'EDT: "' + edt + '"')

            if len(getDataBlocks(in_G,n))>0:
                for db in getDataBlocks(in_G,n):
                    print(tab2 +"DBK:"+ str(hex(id(db))) ),
                    print(db.toDictio())

            if len(getDestroys(in_G, n))>0:
                for troy in getDestroys(in_G, n):
                    print(tab2 +"Destroys: " + str(hex(id(troy))) + ' ' + str(troy.toTuple()) )

            if len(getReleases(in_G, n))>0:
                for rel in getReleases(in_G, n):
                    print(tab2 +"Releases: " + str(hex(id(rel))) + ' ' + str(rel.toTuple()) )

            if len(getOutgoings(in_G, n)) > 0:
                for outg in getOutgoings(in_G, n):
                    print(tab2 + "Outgoings: " + str(hex(id(outg))) + ' ' + str(outg))

    elen=len(in_G.edges())
    print(tab + "edge count = " + str(elen))
    if nlen >0:
        for e in in_G.edges():
            print(tab + "edge = " + str(e))
            attributes = getEdge(in_G,e)
            print(tab2),
            print(attributes)

            events = getEvents(in_G,e)
            for name, evt in events.iteritems():
                print(tab2 +name +"  EVT:"+ str(hex(id(evt))) ),
                print(evt.toDictio())

# ------------------------------------------------------------------------------
def outputDot(in_G, in_filename):
    whereAmI()

    edgeStyleList = []
    edgeStyleList.append(' style=dashed,color=black')
    edgeStyleList.append(' style=dotted,color=black')
    edgeStyleList.append(' style=bold,color=black')
    edgeStyleList.append(' style=dashed,color=red')
    edgeStyleList.append(' style=dotted,color=red')
    edgeStyleList.append(' style=bold,color=red')
    edgeStyleList.append(' style=dashed,color=green')
    edgeStyleList.append(' style=dotted,color=green')
    edgeStyleList.append(' style=bold,color=green')
    edgeStyleList.append(' style=dashed,color=blue')
    edgeStyleList.append(' style=dotted,color=blue')
    edgeStyleList.append(' style=bold,color=blue')
    edgeStyleList.append(' style=dashed,color=orange')
    edgeStyleList.append(' style=dotted,color=orange')
    edgeStyleList.append(' style=bold,color=orange')
    sz_edgeStyleList = len(edgeStyleList)

    tab = GBL.TABunit
    fo = -1
    erri = 0
    while not erri:
        fo = open(in_filename,'w')
        if fo == -1:
            erri = inspect.currentframe().f_lineno
            break

        fo.write("digraph " + str(in_G.graph['name']) + " {\n")

        if len(in_G.nodes()) == 0:
            break

        for n in in_G.nodes():
            fo.write(tab + '"' + getMyTask(in_G, n).aname + '"')
            fo.write(' [idnb=' + str(n) + '];\n')

        if len(in_G.edges()) == 0:
            break

        sharedEvt = {}
        styleDex = -1
        for e in in_G.edges():
            if len(sharedConx(in_G, e)) == 0:
                continue
            n = e[1]
            shared_conx = ()
            for cx in sharedConx(in_G, e):
                if n == cx[1]:
                    shared_conx = cx  # So the current edge shares the connection cx
                    break
            if shared_conx == ():
                continue
            #print('DBG901..............> ' + str(e) + '  ' + str(shared_conx))
            styleDex += 1
            styleDex %= sz_edgeStyleList
            sharedEvt[e] = edgeStyleList[styleDex]
            sharedEvt[cx] = edgeStyleList[styleDex]

        for e in in_G.edges():
            events = getEvents(in_G,e)
            is_elseClause = getEdge(in_G,e)["leads_to_ElseClause"]
            for name, evt in events.iteritems():
                From = getMyTask(in_G, e[0]).aname
                To   = getMyTask(in_G, e[1]).aname
                nom = name
                if nom == 'NULL_GUID':
                    nom = '0G'
                if is_elseClause:
                    nom = 'ELSEclause'

                style = ""
                if e in sharedEvt:
                    style = sharedEvt[e]
                if not style == "":
                    style = ',' + style

                etxt = tab + '"' + From + '" -> "' + To
                etxt += '" [ label="' + nom + '"' + style + '];\n'
                fo.write(etxt)

        break
    if not fo == -1:
        fo.write("}\n")
        fo.close()
        fo = -1

    return erri
# ------------------------------------------------------------------------------
def check_for_cycles(io_G):
    # NOTES:
    # 1) One needs to break cycles on shared connections. See sharedConx().
    # 2) Make sure nodes with a ELSE edge properly handled. See leads_to_ElseClause.

    #When it is done: whereAmI()

    erri=0
    while not erri:
        if len(io_G.nodes())==0: break;

        ## TODO: Although that I was hopeful that the cycle algorithm below was correct.  It is wrong.
        #
        # # Strategy: First annotate the edges, then find the back edge, finally check
        # #           that all back edges have a shared connection.
        #
        # visited = {}  # A dictionary (parent nodeid, nodeid)
        # ages = {}  # The dictionary (nodeID -> age) where ages tells you when the node was first encountered.
        # # The root node is age = 0; all other are older.  i.e. DFS order.
        # currage = 0
        # stack = []
        # stack.append((GBL.NOTA_NODE_ID,GBL.MAINNODE))
        # ages[GBL.MAINNODE] = 0
        #
        # while len(stack) > 0:
        #     cur = stack.pop();
        #     k = cur[1]
        #     if k in visited:
        #         visited[k].append(cur)
        #         continue
        #     else:
        #         visited[k] = [cur]
        #
        #     bors = io_G.neighbors(k)
        #     # print('cur = ' + str(k) + ' B= ' + str(bors) )
        #     for b in bors:
        #         if not b in ages:
        #             ages[b] = ages[k] + 1
        #     for b in bors:
        #         stack.append((k, b))
        #
        # if 1 == 1:
        #     for k in ages:
        #         print("k= " + str(k) + "  ages[k]= " + str(ages[k]))
        #
        #     print("All edges :")
        #     for k in visited:
        #         print("k = " + str(k) + " : "),
        #         print(visited[k])
        #
        # backedges = {}  # node id -> back edge list
        # for k in visited:
        #     if len(visited[k]) == 0:
        #         continue
        #     if len(visited[k]) < 2:
        #         v = visited[k][0]
        #         if v[0] == v[1]:
        #             erri = inspect.currentframe().f_lineno
        #             errmsg(erri, 'Self edge ' + str(v) + 'has no one to share the connection to node "' + str(k) + '".\n')
        #             break
        #         continue
        #     for v in visited[k]:
        #         if ages[v[0]] > ages[v[1]] or v[0] == v[1]:
        #             if k in backedges:
        #                 backedges[k].append(v)
        #             else:
        #                 backedges[k] = [v]
        # if erri: break
        # if 1 == 1:
        #     print("All back edges: "),
        #     print(backedges)


        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
def check_for_FOR_and_IFTHEN_mutual_exclusion(io_G):
    erri=0
    while not erri:
        if len(io_G.nodes()) == 0: break

        for n in io_G.nodes():
            forpart = getFORconditionText(io_G, n)
            ifpart  = getIFconditionText(io_G, n)

            if len(forpart) and len(ifpart):
                erri = inspect.currentframe().f_lineno
                errmsg(erri, 'EDT "' + getMyTask(io_G, n).aname + '" cannot host both a IF-THEN-ELSE pattern and a FOR pattern.\n')

        if erri: break

        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
def check_flight_enum(in_G):
    whereAmI()
    erri=0
    while not erri:
        if len(in_G.nodes())==0: break;

        for n in in_G.nodes():
            blocks = getDataBlocks(in_G, n)
            for b in blocks:
                if flightEnum_is_bad(b.flight):
                    erri=inspect.currentframe().f_lineno
                    errmsg(erri, "Flight data for data block "+ b.name +" in node "+ str(n) +" is not valid: "+b.flight+"\n")
                    break
            if erri: break
        if erri: break

        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
def isa_DBK_access_code(in_access_code):
    if in_access_code in ['DB_MODE_CONST', 'DB_MODE_RW', 'DB_MODE_EW', 'DB_MODE_RO', 'DB_MODE_NULL']:
        return 1
    return 0
# ------------------------------------------------------------------------------
def check_datablock_access_code(in_G):
    whereAmI()
    erri=0
    while not erri:
        if len(in_G.nodes())==0: break;

        for n in in_G.nodes():
            parents = getNodeparents(in_G,n)
            for p in parents:
                edg = (p,n)
                events = getEvents(in_G, edg)
                for evtName in events:
                    if evtName == 'NULL_GUID':
                        continue  # One cannot collide NULL_GUIDs.
                    evt = events[evtName]
                    edghex = hex(id(evt))
                    if not isa_DBK_access_code(evt.accessMode):
                        erri = inspect.currentframe().f_lineno
                        t = 'Edge ' + str(edg) + ' ' + str(edghex) + ' has an invalid access code: '
                        t += str(evt.accessMode) +'\n'
                        errmsg(erri, t)
                        break

                if erri: break  # for evtName in events:
            if erri: break  # for p in parents:
        if erri: break  # n in in_G.nodes():

        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
# An EDT has multiple incoming edges, each edge potentially carrying multiple DBKs.
# By name, each event must uniquely map to a single DBK in the receiving EDT.
# An exception has been made for DBK which lands and take-offs within the same EDT,
# as this is a classic mean of insuring locality and ownership transfer.
def check_datablock_integrity(in_G):
    whereAmI()
    erri=0
    while not erri:
        if len(in_G.nodes())==0: break;

        for n in in_G.nodes():
            parents = getNodeparents(in_G,n)
            for p in parents:
                edg = (p,n)
                events = getEvents(in_G, edg)
                for evtName in events:
                    if evtName == 'NULL_GUID':
                        continue  # One cannot collide NULL_GUIDs.

                    evt = events[evtName]
                    edghex = hex(id(evt))  # This provide a very unique identifier

                    flights = []
                    count = 0
                    for db in getDataBlocks(in_G, n):
                        if db.name == evtName:
                            count += 1
                            flights.append(db.flight)
                    if not count == 1:
                        if count == 0:
                            erri = inspect.currentframe().f_lineno
                            t = 'DBK "' + evtName + '" on Edge ' + str(edg) + ' ' + edghex
                            t += ' has no flight data in the receiving EDT.\n'
                            errmsg(erri, t)
                            break
                        else:
                            exception_found = False
                            if count == 2:
                                fcheck = 0
                                if 'flLANDING' in flights:
                                    fcheck += 1
                                if 'flTAKEOFF' in flights:
                                    fcheck += 1
                                if fcheck == 2:
                                    exception_found = True
                            if exception_found:
                                # NOTE 802: 2016jun15: It is assumed that both a flLANDING DBK and a
                                #                      flTAKEOFF DBK can share a DBK name because
                                #                      the landing DBK will have its name-mangled
                                #                      upon entry.  Thus the flTAKEOFF DBK can take
                                #                      that name without worrying about name collision.
                                t = 'DBK "' + evtName + '" in EDT called "' + getMyTask(in_G, n).aname + '"'
                                t += ' both LANDs and TAKEOFFs. Exception made.\n'
                                warnmsg(inspect.currentframe().f_lineno, t)
                            else:
                                erri = inspect.currentframe().f_lineno
                                t = 'DBK "' + evtName + '" in EDT called "' + getMyTask(in_G, n).aname + '"'
                                t += ' must be unique with the EDT. So only one DBK can be called by that name.\n'
                                errmsg(erri, t)
                                break
                if erri: break
            if erri: break
        if erri: break

        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
# inputs: DB_MODE_CONST, DB_MODE_RW, DB_MODE_EW, DB_MODE_RO, DB_MODE_NULL
# Outputs:
#   0 --> all is ok.
#   positive number --> error
#   negative number --> warning because workable
def compare_DBK_access(in_access_left, in_access_right):
    if not isa_DBK_access_code(in_access_left):
        erri = inspect.currentframe().f_lineno
        return erri
    if not isa_DBK_access_code(in_access_right):
        erri = inspect.currentframe().f_lineno
        return erri

    choice_table = {}
    # Use 0 for ok, 1 for error, -1 for warning
    choice_table['DB_MODE_CONST'] = {'DB_MODE_CONST': 0, 'DB_MODE_RO':-1, 'DB_MODE_RW': 1, 'DB_MODE_EW': 1, 'DB_MODE_NULL': 1}
    choice_table['DB_MODE_RO']    = {'DB_MODE_CONST':-1, 'DB_MODE_RO': 0, 'DB_MODE_RW':-1, 'DB_MODE_EW':-1, 'DB_MODE_NULL': 1}
    choice_table['DB_MODE_RW']    = {'DB_MODE_CONST': 1, 'DB_MODE_RO':-1, 'DB_MODE_RW': 0, 'DB_MODE_EW': 1, 'DB_MODE_NULL': 1}
    choice_table['DB_MODE_EW']    = {'DB_MODE_CONST': 1, 'DB_MODE_RO':-1, 'DB_MODE_RW': 1, 'DB_MODE_EW': 0, 'DB_MODE_NULL': 1}
    choice_table['DB_MODE_NULL']  = {'DB_MODE_CONST': 1, 'DB_MODE_RO': 1, 'DB_MODE_RW': 1, 'DB_MODE_EW': 1, 'DB_MODE_NULL': 0}

    return choice_table[in_access_left][in_access_right]
# ------------------------------------------------------------------------------
def check_for_colliding_data_flights(io_G):
    whereAmI()
    erri=0
    while not erri:
        if len(io_G.nodes())==0: break;

        for n in io_G.nodes():
            # First collect the info on all incoming data blocks
            founds = {}
            parents = getNodeparents(io_G, n)
            for p in parents:
                edg = (p,n)
                events = getEvents(io_G, edg)
                for evtName in events:
                    if evtName == 'NULL_GUID':
                        continue  # One cannot collide NULL_GUIDs.
                    if not evtName in founds:
                        founds[evtName] = []
                    evt = events[evtName]
                    edghex = hex(id(evt))  # This will be used as a more precise edge ID.

                    flight = "flUNDEF"
                    for dbk in getDataBlocks(io_G, n):  # A data block is local to an EDT
                        if dbk.name == evtName:
                            if flight == 'flUNDEF':
                                flight = dbk.flight
                            elif flight == 'flLANDING' and dbk.flight == 'flTAKEOFF':
                                # The effect of NOTE 802 is felt here: One DBK by name lands, another takes off.
                                flight = 'flLANDING'  # Bias toward landing
                            elif dbk.flight == 'flLANDING' and flight == 'flTAKEOFF':
                                # The effect of NOTE 802 is felt here: One DBK by name lands, another takes off.
                                flight = dbk.flight # Bias toward landing
                            else:
                                erri = inspect.currentframe().f_lineno
                                # I do not think we should be able to get here,due to previous checks.
                                break
                    if erri: break
                    founds[evtName].append([edg, edghex, evt.accessMode, flight])
                if erri: break
            if erri: break

            # Then analyze what we got
            iedg = 0
            ihex = 1
            iaccessm  = 2
            ifl  = 3
            for evtName in founds:
                sameDBKs = founds[evtName]
                if len(sameDBKs) == 1:
                    continue

                landings = []

                # Compare every DBK of a given name with every other DBK of the same name.
                for i in range(len(sameDBKs)):
                    d1 = sameDBKs[i]
                    if d1[ifl] == 'flLANDING':
                        landings.append(d1)
                    for j in range(i + 1, len(sameDBKs)):
                        d2 = sameDBKs[j]
                        if not d1[ifl] == d2[ifl]:
                            erri = inspect.currentframe().f_lineno
                            t = str(d1) + ' flight plan differs from ' + str(d2) + '\n'
                            errmsg(erri, t)
                            break

                        check = compare_DBK_access(d1[iaccessm], d2[iaccessm])
                        if check:
                            if check == -1:
                                t = 'DBK "' + evtName + '" : EVT' + str(d1) + ' has different DBK access code from EVT' + str(d2) + '. Exception made.\n'
                                warnmsg(inspect.currentframe().f_lineno, t)
                            else:
                                erri = inspect.currentframe().f_lineno
                                t = 'DBK "' + evtName + '" : EVT' + str(d1) + ' has an incompatible DBK access code from EVT' + str(d2) + '\n'
                                errmsg(erri, t)
                                break
                    if erri: break  # for j in range(i
                if erri: break  # for i in range

                if len(landings) > 1:
                    # Only one of these landings can perform the deletion
                    for i in range(len(landings)):
                        the_edg = landings[i][iedg]
                        labeled_edge = (the_edg, evtName)
                        if i == 0:
                            getEvent(io_G, labeled_edge).isTheLead = True
                            t = 'The LANDINGs DBK "' + evtName + '" from edge "' + str(landings[i]) + '" has been chosen to be the leading edge.\n'
                            warnmsg(inspect.currentframe().f_lineno, t)
                        else:
                            getEvent(io_G, labeled_edge).isTheLead = False

            if erri: break  # for evtName in founds:
        if erri: break  # for n in io_G.nodes():

        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
#set_parents add a field on each node with the ID of the parent
# DEV NOTE: This algorithm should work even with directed multi-graphs.
def set_parents(io_G):
    whereAmI()
    erri=0
    while not erri:
        if len(io_G.nodes())==0: break;

        for n in io_G.nodes():
            bors=io_G.neighbors(n)
            for b in bors:
                if not n in getNodeparents(io_G, b):
                    getNodeparents(io_G, b).append(n)

        debug = False
        if debug == True:
            for n in io_G.nodes():
                print( 'n=' + str(n) + '  parents=' + str(getNodeparents(io_G, n)))

        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
def init_slotCounts_for_param(io_G):
    whereAmI()
    erri=0
    while not erri:
        if len(io_G.nodes())==0: break;

        for n in io_G.nodes():
            getMyTask(io_G, n).paramc = 0
            getMyTask(io_G, n).paramv = 'NULL'

        break # while not erri:
    return erri
# ------------------------------------------------------------------------------
# 1) Find out what leaves node A,
# 2) Find out how it is used in B,
# 3) Classify depending on B usage
#NOTE: This function will not capture the info incoming into an ELSE EDT, because
#      the IF-THEN edt will handle that.
def find_IN_literals_and_dereferenceds(io_G):
    whereAmI()
    debug1 = GBL.debug_DBK_flights
    erri=0
    while not erri:
        for n in io_G.nodes():

            bors = io_G.neighbors(n)
            for b in bors:
                edg = (n,b)

                if debug1: print('DBG1160> find lits derefs>  node=' + str(n) + '  bor=' + str(b))

                events = getEvents(io_G, edg)
                for evtName in events:
                    if evtName == 'NULL_GUID':
                        literals(io_G, edg).append(evtName)  # @MARK709  Search for this mark for more details
                        continue

                    # DEV DBG> evt = events[evtName]; print("DEV680>" + evtName +"  "+ str(evt.toDictio()) )
                    if debug1: print('DBG1> Node='+str(n)+ ' Edge=' +str(edg) + '  DBK= ' + evtName)

                    if 1 == 1:
                        # Get the info from the starting node
                        # NOTE:710: When looking for literals and derefs, begin_flight info is not pertinent to analysis.
                        # NOTE:710: But it serves here as a good check.
                        begin_flight = 'flUNDEF'
                        if len(getDataBlocks(io_G,n))>0:
                            for db in getDataBlocks(io_G,n):
                                if db.name == evtName:
                                    begin_flight = db.flight
                                    break

                        if debug1: print('\tbegin_flight=' + begin_flight)

                        if begin_flight in ['flLANDING, flHOP', 'flFLYOVER']:
                            # If landing,hop or flyover, it should not be on an outgoing edge
                            erri=inspect.currentframe().f_lineno
                            errmsg(erri, 'Node='+str(n)+ ' Edge=' +str(edg)+ ' : Data block "'+evtName+'" of type '+ begin_flight +' cannot be on outgoing edge.\n')
                            break

                        if begin_flight in ['flUNDEF']:
                            erri=inspect.currentframe().f_lineno
                            errmsg(erri, 'Node='+str(n)+ ' Edge=' +str(edg)+ ' : Data block "'+evtName+'" of type '+ begin_flight +' needs to be defined.\n')
                            break

                    # Get the info from the ending node
                    end_flight = 'flUNDEF'
                    if len(getDataBlocks(io_G,b))>0:
                        for db in getDataBlocks(io_G,b):
                            if db.name == evtName:
                                end_flight = db.flight
                                break

                    if debug1: print('\tend_flight=' + end_flight)

                    if end_flight in ['flTAKEOFF', 'flHOP','flFLYOVER']:
                        # If landing,hop or flyover, it should not be on an outgoing edge
                        erri=inspect.currentframe().f_lineno
                        errmsg(erri, 'Node='+str(n)+ ' Edge=' +str(edg)+ ' : Data block "'+evtName+'" of type '+ end_flight +' cannot be on outgoing edge.\n')
                        break

                    if end_flight in ['flUNDEF']:
                        erri=inspect.currentframe().f_lineno
                        errmsg(erri, 'Node='+str(n)+ ' Edge=' +str(edg)+ ' : Data block "'+evtName+'" of type '+ end_flight +' needs to be defined.\n')
                        break

                    # Decide what to do
                    # begin q = [           'flPASSTRU', 'flTAGO',            'flTAKEOFF',                 ]
                    # end   q = [           'flPASSTRU', 'flTAGO', 'flLANDING',                            ]
                    if end_flight == 'flPASSTRU':
                        #
                        # --> DEV not finished yet.  So flag them as errors for now.
                        erri = inspect.currentframe().f_lineno
                        txt = 'The handling of flPASSTRU data blocks has not been fully implemented yet.'
                        txt += ' Please use TAGO instead for now.'
                        errmsg(erri, txt)
                        #  literals(io_G, edg).append(evtName)
                        # if has_ELSEedt:  --> Fill up the rest
                    elif end_flight == 'flTAGO' or end_flight == 'flLANDING':
                        dereferenciables(io_G, edg).append(evtName)
                    else:
                        erri=inspect.currentframe().f_lineno
                        break
                if erri: break  # for evtName in events:

                if debug1:
                    print('>DBG1160> find lits derefs>  node=' + str(n) + '  bor=' + str(b))
                    print('\t edg=' + str(edg))
                    print('\t lits=' + str(literals(io_G, edg)))
                    print('\t derefs=' + str(dereferenciables(io_G, edg)))

            if erri: break  # for b in bors:
        if erri: break  # for n in io_G.nodes():

        # So now, each edge has literals and derefs content.

        break # while not erri:

    return erri
# ------------------------------------------------------------------------------
# inputs: 'flUNDEF', 'flPASSTRU', 'flTAGO', 'flLANDING', 'flTAKEOFF', 'flHOP', 'flFLYOVER'
#           See flightEnum_is_bad(in_flightEnum) for details
#         In this context, 'flUNDEF' means that the flight was not found, e.g. that DBK is not present.
# Outputs:
#   0 --> all is ok.
#   positive number --> error
#   negative number --> warning because workable
# NOTE:
#  2016jun: The matrix is symmetric for now.  That seems to make sense at this time.
def flight_are_compatible_for_IFTHENELSE(in_ThenFlight, in_ElseFlight):
    choice_table = {}
    # Use 0 for ok, 1 for error, -1 for warning

    # Note: Two 'flUNDEF' --> How do one get there?  --> Error.
    # Note: Only flHOP and flTAKEOFF seems to make sense.
    choice_table['flUNDEF']   = {'flUNDEF':1, 'flPASSTRU':1, 'flTAGO':1, 'flLANDING':1, 'flTAKEOFF':0, 'flHOP':0, 'flFLYOVER':1}

    choice_table['flPASSTRU'] = {'flUNDEF':1, 'flPASSTRU':0, 'flTAGO':0, 'flLANDING':0, 'flTAKEOFF':1, 'flHOP':1, 'flFLYOVER':1}
    choice_table['flTAGO']    = {'flUNDEF':1, 'flPASSTRU':0, 'flTAGO':0, 'flLANDING':0, 'flTAKEOFF':1, 'flHOP':1, 'flFLYOVER':1}
    choice_table['flLANDING'] = {'flUNDEF':1, 'flPASSTRU':0, 'flTAGO':0, 'flLANDING':0, 'flTAKEOFF':1, 'flHOP':1, 'flFLYOVER':1}
    choice_table['flTAKEOFF'] = {'flUNDEF':0, 'flPASSTRU':1, 'flTAGO':1, 'flLANDING':1, 'flTAKEOFF':0, 'flHOP':0, 'flFLYOVER':1}

    # Note: flHOP is local, and it should not interfere, or be interfered with, other data blocks.
    choice_table['flHOP']     = {'flUNDEF':0, 'flPASSTRU':1, 'flTAGO':1, 'flLANDING':1, 'flTAKEOFF':0, 'flHOP':0, 'flFLYOVER':1}
    # Note: Anything flFLYOVER should be an error.  No clue how it would get here.
    choice_table['flFLYOVER'] = {'flUNDEF':1, 'flPASSTRU':1, 'flTAGO':1, 'flLANDING':1, 'flTAKEOFF':1, 'flHOP':1, 'flFLYOVER':1}

    return choice_table[in_ThenFlight][in_ElseFlight];

# ------------------------------------------------------------------------------
def validate_IFthenELSE(io_G):
    whereAmI()
    erri=0
    while not erri:

        # First find all IF-THEN-ELSE edges
        ifedges = []
        for e in io_G.edges():
            if getEdge(io_G,e)["leads_to_ElseClause"]:
                ifedges.append(e)

        if len(ifedges)==0:
            break  # Not much to do

        # Process I-T-E edges for validity
        for edg in ifedges:
            FROM = edg[0]
            TO = edg[1]

            # Making we have an IF conditional
            if len(getIFconditionText(io_G, FROM))==0:
                erri=inspect.currentframe().f_lineno
                t = "Missing IF condition for the IF-THEN-ELSE edge = " + str(edg)
                errmsg(erri,t)
                break

            countDBK_from = len(getDataBlocks(io_G,FROM))
            done_FROM=[]
            for i in range(countDBK_from):
                done_FROM.append(False)

            countDBK_to = len(getDataBlocks(io_G, TO))
            done_TO=[]
            for i in range(countDBK_to):
                done_TO.append(False)

            for f, bf in enumerate(getDataBlocks(io_G,FROM)):
                vol = 'flUNDEF'
                name = ""
                for t, bt in enumerate(getDataBlocks(io_G, TO)):
                    if done_TO[t]:
                        continue
                    name = bt.name
                    if bf.name == name:
                        done_TO[t] = True
                        vol = bt.flight
                        break

                check = flight_are_compatible_for_IFTHENELSE(bf.flight, vol)
                if check:
                    erri = inspect.currentframe().f_lineno
                    t = 'On the IF-THEN-ELSE edge = ' + str(edg) + ', the data blocks "' + bf.name + '" '
                    t += 'and "' + name + '" have incompatible flights, repectively: ' + bf.flight + '" "'
                    t += vol + '".'
                    errmsg(erri, t)
            if erri: break  # for f, bf in

            for t, bt in enumerate(getDataBlocks(io_G,TO)):
                vol = 'flUNDEF'
                name = ""
                for f, bf in enumerate(getDataBlocks(io_G, FROM)):
                    if done_FROM[f]:
                        continue
                    name = bf.name
                    if bt.name == name:
                        done_FROM[f] = True
                        vol = bf.flight
                        break

                check = flight_are_compatible_for_IFTHENELSE(bt.flight, vol)
                if check:
                    erri = inspect.currentframe().f_lineno
                    t = 'On the IF-THEN-ELSE edge = ' + str(edg) + ', the data blocks "' + bf.name + '" '
                    t += 'and "' + name + '" have incompatible flights, repectively: ' + bf.flight + '" "'
                    t += vol + '".'
                    errmsg(erri, t)
                if erri: break  # for t, bt in

            if erri: break  # for f, bF in e
        if erri: break  # for edg in ifedges:

        break # while not erri:

    return erri
# ------------------------------------------------------------------------------
#NOTE: For ELSE edt, because they do not have explicit literals nor dereferentiables (they are handled by the IF edt),
#      shared connections cannot be put on ELSE EDTs.  <-- Not sure. See Note 2016Aug17
#   2016Aug17: When implementing  binaryTree_forkjoin, I had an IF-THEN edt and a ELSE edt both emitting a DBK
#              to the same other edt BtJoin.  Autogen saw that has two separate inputs; when in fact it was only one.
#              So I used a shared connection to tell Autogen that both the IF-THEN and its ELSE are sharing the input
#              into the BtJoin edt.
def check_shared_connections(in_G):
    erri=0
    while not erri:
        for n in in_G.nodes():
            for p in getNodeparents(in_G,n):
                edg = (p, n)
                lits   = literals(in_G, edg)
                derefs = dereferenciables(in_G, edg)

                if len(sharedConx(in_G,edg)) == 0:
                    continue
                shared_conx = ()
                for cx in sharedConx(in_G,edg):
                    if n == cx[1]:
                        shared_conx = cx  # So the current edge shares the connection cx
                        break
                if shared_conx == ():
                    continue

                litsCX = literals(in_G, shared_conx)
                derefsCX = dereferenciables(in_G, shared_conx)

                if not sorted(lits) == sorted(litsCX):
                    erri = inspect.currentframe().f_lineno
                    t = 'Edge "' + str(edg) + '" must have the same literals as its shared connection "'
                    t += str(shared_conx) + '".\n'
                    errmsg(erri,t)

                if not sorted(derefs) == sorted(derefsCX):
                    # erri = inspect.currentframe().f_lineno
                    warni = inspect.currentframe().f_lineno
                    t = 'Edge "' + str(edg) + '" must have the same dereferentiables as its shared connection "'
                    t += str(shared_conx) + '".\n'
                    # errmsg(erri,t)
                    #2016aug17: In a join operation over binary tree, a node will emit a DBK to one or the two inputs
                    #           it is expected.
                    #                                   Transition1         Transition2
                    #                                       |   |
                    #                                       \   /
                    #                                         B3            B4              Compactly
                    #                                           \           /
                    #                                            \         /                Transition
                    #                                             \       /                  |       |
                    #                                              \     /          -->           B    <--
                    #                                                 B1                           |     |
                    #                                                  |                           v     |
                    #                                                  |                            -->--
                    #                                                   B0
                    #           So it can happen that although an EDT starts with 2 inputs, only only slot
                    #           will be shared.  In the above graph, one of Transition->B3 will have to shared
                    #           with B3->B1.
                    warnmsg(warni,t)

                if erri: break
            if erri: break  # for p in getNodeparents(in_G,n):

        if erri: break  # for n in in_G.nodes():

        break  # while not erri:
    return erri
# ------------------------------------------------------------------------------
def assign_slots_for_literals_and_dereferenceds2(io_G):
    erri=0
    while not erri:
        #2016Sept14: Faulty do  not use yet. Stillunder development.
        erri = inspect.currentframe().f_lineno
        break

        for n in io_G.nodes():
            local_slot_offset = 0

            #Get the slot count per edg.
            for dbk in getDataBlocks(io_G, n):
                if dbk.flight in ['flUNDEF', 'flFLYOVER']:
                    # No DBK should have these flight paths at this point.
                    erri = inspect.currentframe().f_lineno
                    break
                if dbk.flight in ['flTAKEOFF', 'flHOP']:
                    # These cannot be coming in.
                    continue
                if dbk.flight in 'flPASSTRU':
                    erri = inspect.currentframe().f_lineno
                    print('ERROR: DBK with flPASSTRU are not handled yet.\n')
                    break

                if not dbk.flight in ['flTAGO', 'flLANDING']:
                    # Just in case a new type of flight is added.
                    erri = inspect.currentframe().f_lineno
                    break

                if dbk.name == 'NULL_GUID' and dbk.flight == 'flLANDING':
                    thisIs_theNullGuid_for_IFTHENELSE = False
                    for p in getNodeparents(io_G, n):
                        if len(getIFconditionText(io_G, n)):
                            edgif = (p,n)
                            if getEdge(io_G, edgif)["leads_to_ElseClause"]:
                                thisIs_theNullGuid_for_IFTHENELSE = True
                                if not len(getDataBlocks(io_G, n)) == 1:
                                    erri = inspect.currentframe().f_lineno
                                    t = 'NULL_GUID linking a IF-THEN edt and a ELSE edt is not along. Possible error.\n'
                                    warnmsg(erri, t)
                                    erri = 0
                                break
                    if thisIs_theNullGuid_for_IFTHENELSE:
                        continue

                #First check among the non-shared edges
                parent = GBL.NOTA_NODE_ID
                for pa in getNodeparents(io_G, n):
                    edg = (pa, n)

                    is_shared = False
                    for cx in sharedConx(io_G, edg):
                        if n == cx[1]:
                            # This connection is shared with another
                            is_shared = True
                            break
                    if is_shared:
                        continue

                    events = getEvents(io_G, edg)
                    for evtName in events:
                        if evtName == dbk.name:
                            parent = pa
                            break
                    if not parent == GBL.NOTA_NODE_ID:  # 2016Sept13-2229: Added this as a fix, i.e. stop at first found.
                        break

                if parent == GBL.NOTA_NODE_ID:
                    # Look in the shared edge to find it
                    erri = inspect.currentframe().f_lineno
                    t = 'Looking in the shared edges for DBK "' + dbk.name + '" on node ' + str(n) + '.\n'
                    warnmsg(erri, t)
                    erri = 0
                    for pa in getNodeparents(io_G, n):
                        edg = (pa, n)

                        is_shared = False
                        for cx in sharedConx(io_G, edg):
                            if n == cx[1]:
                                # This connection is shared with another
                                is_shared = True
                                break
                        if not is_shared:
                            continue

                        events = getEvents(io_G, edg)
                        for evtName in events:
                            if evtName == dbk.name:
                                parent = pa
                                break
                        if not parent == GBL.NOTA_NODE_ID:  # 2016Sept13-2229: Added this as a fix, i.e. stop at first found.
                            break

                if parent == GBL.NOTA_NODE_ID:
                    #We really cannot find an incoming event for this DBK.
                    erri = inspect.currentframe().f_lineno
                    t = 'The DBK "' + dbk.name + '" in node ' + str(n) + ' has no incoming event for it.\n'
                    errmsg(erri, t)
                    break

                edg = (parent, n)
                if getEdge(io_G, edg)["slotCount"] == GBL.notaSlot:
                    getEdge(io_G, edg)["slotCount"] = 0
                getEdge(io_G, edg)["slotCount"] += 1

            if erri: break  # for dbk in getDataBlocks(io_G, n)

            #Now assign slot numbers to non-shared edges
            for dbk in getDataBlocks(io_G, n):
                if dbk.flight in ['flUNDEF', 'flFLYOVER']:
                    # No DBK should have these flight paths at this point.
                    erri = inspect.currentframe().f_lineno
                    break
                if dbk.flight in ['flTAKEOFF', 'flHOP']:
                    # These cannot be coming in.
                    continue
                if dbk.flight in 'flPASSTRU':
                    erri = inspect.currentframe().f_lineno
                    print('ERROR: DBK with flPASSTRU are not handled yet.\n')
                    break

                if not dbk.flight in ['flTAGO', 'flLANDING']:
                    # Just in case a new type of flight is added.
                    erri = inspect.currentframe().f_lineno
                    break

                if dbk.name == 'NULL_GUID' and dbk.flight == 'flLANDING':
                    thisIs_theNullGuid_for_IFTHENELSE = False
                    for p in getNodeparents(io_G, n):
                        if len(getIFconditionText(io_G, n)):
                            edgif = (p, n)
                            if getEdge(io_G, edgif)["leads_to_ElseClause"]:
                                thisIs_theNullGuid_for_IFTHENELSE = True
                                if not len(getDataBlocks(io_G, n)) == 1:
                                    erri = inspect.currentframe().f_lineno
                                    t = 'NULL_GUID linking a IF-THEN edt and a ELSE edt is not along. Possible error.\n'
                                    warnmsg(erri, t)
                                    erri = 0
                                break
                    if thisIs_theNullGuid_for_IFTHENELSE:
                        continue

                # Find the non-shared edge to which the DBK belong
                parent = GBL.NOTA_NODE_ID
                for pa in getNodeparents(io_G, n):
                    edg = (pa, n)

                    is_shared = False
                    for cx in sharedConx(io_G, edg):
                        if n == cx[1]:
                            # This connection is shared with another
                            is_shared = True
                            break
                    if is_shared:
                        continue

                    events = getEvents(io_G, edg)
                    for evtName in events:
                        if evtName == dbk.name:
                            parent = pa
                            break
                    if not parent == GBL.NOTA_NODE_ID:  # 2016Sept13-2229: Added this as a fix, i.e. stop at first found.
                        break

                if parent == GBL.NOTA_NODE_ID:
                    # We really connat find an incoming event for this DBK.
                    erri = inspect.currentframe().f_lineno
                    t = 'The DBK "' + dbk.name + '" in node ' + str(n) + ' has no incoming event for it.\n'
                    errmsg(erri, t)
                    break

                edga = (parent, n)
                eventsa = getEvents(io_G, edga)
                eventsa[dbk.name].calculatedSlot = local_slot_offset
                local_slot_offset += 1

            if erri: break  # for dbk in getDataBlocks(io_G, n)

            if getMyTask(io_G, n).depc == GBL.genAUTO:
                getMyTask(io_G, n).depc = local_slot_offset
            getMyTask(io_G, n).depv = 'NULL'  # Using events to fill the slots

        if erri: break  # for n in io_G.nodes():

        #Now handle the shared connections
        for n2 in io_G.nodes():
            for p2 in getNodeparents(io_G,n2):
                edg = (p2, n2)
                lits   = literals(io_G, edg)
                derefs = dereferenciables(io_G, edg)
                # print("\nDBG1516> n = " + str(n2))
                # print('DBG1516> edg='+ str(edg) + '  lits= '+ str(lits) + '  derefs=' + str(derefs))

                shared_conx = ()
                for cx in sharedConx(io_G,edg):
                    if n2 == cx[1]:
                        shared_conx = cx  # So the current edge shares the connection cx
                        break
                if shared_conx == ():
                    continue

                events = getEvents(io_G, edg)
                sharedEV = getEvents(io_G, shared_conx)

                if 1 == 1:
                    print('DBG1745> events   = ' + str(events))
                    print('DBG1745> sharedEV = ' + str(sharedEV))
                    print('DBG1745> lits = ' + str(lits))
                    print('DBG1745> derefs = ' + str(derefs))

                if lits:
                    for lit in lits:
                        events[lit].calculatedSlot = sharedEV[lit].calculatedSlot
                if derefs:
                    for deref in derefs:
                        if deref in sharedEV:  # 2016Sept12: This is a hack in order to allow the proper functioning of shared edges
                                               #             without having the full labeled edge info.
                            events[deref].calculatedSlot = sharedEV[deref].calculatedSlot
                            print('DBG1758> events[deref].calculatedSlot = ' + str(events[deref].calculatedSlot))

            if erri: break  # for p2 in getNodeparents(io_G,n):

        if erri: break  # for n2 in io_G.nodes():

        break  # while not erri:
    return erri
# ------------------------------------------------------------------------------
# Version 2 exists and is above and it is faulty.
def assign_slots_for_literals_and_dereferenceds(io_G):
    erri=0
    while not erri:
        for n in io_G.nodes():
#            if n == 15:
#                import pdb
#                pdb.set_trace()
#                print("DBG1891> At node 15\n")

            local_slot_offset = 0
            for p in getNodeparents(io_G,n):
                edg = (p,n)

                derefs = dereferenciables(io_G, edg)

                shared_conx = False
                for cx in sharedConx(io_G,edg):
                    if n == cx[1]:
                        shared_conx = True
                        break
                if shared_conx: continue
                #Shared connection will be handle separately, below, after these n:p loops.

                lits = literals(io_G, edg)
                if len(lits) > 1:
                    # 2016jun29: lits are expected to be empty or contain a single 'NULL_GUID',
                    #            So I put the check in order to ascertain that.
                    #            See @MARK709 for more details
                    erri = inspect.currentframe().f_lineno
                    break

                getEdge(io_G, edg)["slotCount"] = 0
                for lt in lits:
                    if lt == 'NULL_GUID':
                        getEdge(io_G, edg)["slotCount"] += 1  # All literals will end up in one struct in one data block.
                                                              # NOTE: That I put 'NULL_GUID's in literals
                                                              # @MARK709  Search for this mark for more details
                    else:
                        # Incomplete code --> No clue how to handle non-NULL_GUID events.
                        erri = inspect.currentframe().f_lineno
                        break
                if erri: break

                getEdge(io_G, edg)["slotCount"] += len(derefs) # One slot per deref

                events = getEvents(io_G, edg)

                # print('DBG1499> lits = ' + str(lits))
                # print('DBG1499> derefs = ' + str(derefs))

                if lits:
                    for lit in lits:
                        events[lit].calculatedSlot = local_slot_offset
                        # print(lit + '  ' + str(local_slot_offset))

                        # Lits are all bunched up in one struct so they get only one slot.
                        #But we could have multiple NULL_GUIDs. # @MARK709
                        local_slot_offset += 1

                if derefs:
                    for deref in derefs:
                        events[deref].calculatedSlot = local_slot_offset
                        # print('DBG1328> events[deref].calculatedSlot = ' + str(events[deref].calculatedSlot))
                        # print(deref + '  ' + str(local_slot_offset))
                        local_slot_offset += 1
            if erri: break  # for p in getNodeparents(io_G,n):

            if getMyTask(io_G, n).depc == GBL.genAUTO:
                getMyTask(io_G, n).depc = local_slot_offset
            getMyTask(io_G, n).depv = 'NULL'  # Using events to fill the slots

        if erri: break  # for n in io_G.nodes():

        #Now handle the shared connections
        for n2 in io_G.nodes():
            for p2 in getNodeparents(io_G,n2):
                edg = (p2, n2)
                lits   = literals(io_G, edg)
                derefs = dereferenciables(io_G, edg)
                # print("\nDBG1516> n = " + str(n2))
                # print('DBG1516> edg='+ str(edg) + '  lits= '+ str(lits) + '  derefs=' + str(derefs))

                shared_conx = ()
                for cx in sharedConx(io_G,edg):
                    if n2 == cx[1]:
                        shared_conx = cx  # So the current edge shares the connection cx
                        break
                if shared_conx == ():
                    continue

                events = getEvents(io_G, edg)
                sharedEV = getEvents(io_G, shared_conx)

                if 1 == 1:
                    print('DBG1745> events   = ' + str(events))
                    print('DBG1745> sharedEV = ' + str(sharedEV))
                    print('DBG1745> lits = ' + str(lits))
                    print('DBG1745> derefs = ' + str(derefs))

                if lits:
                    for lit in lits:
                        events[lit].calculatedSlot = sharedEV[lit].calculatedSlot
                if derefs:
                    for deref in derefs:
                        if deref in sharedEV:  # 2016Sept12: This is a hack in order to allow the proper functioning of shared edges
                                               #             without having the full labeled edge info.
                            events[deref].calculatedSlot = sharedEV[deref].calculatedSlot
                            print('DBG1758> events[deref].calculatedSlot = ' + str(events[deref].calculatedSlot))

            if erri: break  # for p2 in getNodeparents(io_G,n):

        if erri: break  # for n2 in io_G.nodes():

        break  # while not erri:
    return erri
# ------------------------------------------------------------------------------
def set_the_ELSEclause_field(io_G):
    erri=0
    while not erri:
        for n in io_G.nodes():
            if len(getIFconditionText(io_G,n)) == 0:
                continue
            linking_event_name = ""
            count_evt = 0
            count_edg = 0
            for bor in io_G.neighbors(n):
                edg = (n, bor)
                if not getEdge(io_G, edg)["leads_to_ElseClause"]:
                    continue
                for evtName in getEvents(io_G, edg):
                    getEvents(io_G, edg)[evtName].fertile = False  # ELSE edt are merged with the IF-THEN edt.
                    count_evt += 1
                    linking_event_name = evtName
                getNode(io_G,bor)["istheELSEclause"] = True
                count_edg += 1
            if not count_edg == 1:
                erri = inspect.currentframe().f_lineno
                t = 'On node ' + str(n) + ', IF EDT found.  But invalid ELSE condition count found :' + str(count_edg)
                t += '. Only single count, i.e. edge from IF-THEN to ELSE, of an ELSE condition is allowed.'
                errmsg(erri, t)
                break
            if not count_evt == 1:
                erri = inspect.currentframe().f_lineno
                t = 'On node ' + str(n) + ', IF EDT found.  Only one event is allowd on the edge leading to '
                t += 'the ELSE EDT: ' + str(count_evt) + ' events were found.'
                errmsg(erri, t)
                break
            if not linking_event_name == 'NULL_GUID':
                erri = inspect.currentframe().f_lineno
                t = 'On node ' + str(n) + ', IF EDT found.  Only a NULL_GUID DBK can be used on the event linking '
                t += 'the IF-THEN EDT to the ELSE EDT: Found = ' + linking_event_name
                errmsg(erri, t)
                break
        if erri: break

        break  # while not erri:
    return erri
# ------------------------------------------------------------------------------
def OCRanalysis(io_G):
    whereAmI()
    erri = 0
    while not erri:
        erri = check_flight_enum(io_G)
        if erri: break
        erri = check_for_cycles(io_G)
        if erri: break
        erri = check_for_FOR_and_IFTHEN_mutual_exclusion(io_G)
        if erri: break
        erri = set_parents(io_G)
        if erri: break
        erri = set_the_ELSEclause_field(io_G)  # This must be before find_IN_literals_and_dereferenceds(io_G)
        if erri: break
        erri = init_slotCounts_for_param(io_G)
        if erri: break
        erri = check_datablock_access_code(io_G)  # Keep this before check_datablock_integrity
        if erri: break
        erri = check_datablock_integrity(io_G)
        if erri: break
        erri = check_for_colliding_data_flights(io_G)  # In addition to checking, this also sets the "isTheLead" field.
        if erri: break
        erri = find_IN_literals_and_dereferenceds(io_G)
        if erri: break
        erri = check_shared_connections(io_G)
        if erri: break
        erri = assign_slots_for_literals_and_dereferenceds(io_G)
        if erri: break
        break
    return erri
# ------------------------------------------------------------------------------
def find_destroy_outgoing_release_forELSEedt(io_G, in_nodeIndex):
    whereAmI()
    erri = 0
    while not erri:
        if not getNode(io_G, in_nodeIndex)["istheELSEclause"]:
            break

        print("DEV-DBG1849> hook for node 17\n")
        if in_nodeIndex == 17:
            #import pdb
            #pdb.set_trace()
            print("DBG1851> At node 17\n")

        #Since this is an ELSE edt, first find its IF-THEN edt
        nodeIF = GBL.NOTA_NODE_ID
        for p in getNodeparents(io_G,in_nodeIndex):
            edg = (p,in_nodeIndex)
            if getEdge(io_G,edg)["leads_to_ElseClause"]:
                nodeIF = p
                break
        if nodeIF == GBL.NOTA_NODE_ID:
            erri = inspect.currentframe().f_lineno
            nameELSE = getNodeNamebyitsNumber(io_G, in_nodeIndex)
            errmsg(erri, 'The IF clause node for node "' + nameELSE + '" could not be found.')
            break

        #print('DBG1722> IF= ' + getNodeNamebyitsNumber(io_G, nodeIF) + '  ELSE= ' + getNodeNamebyitsNumber(io_G, in_nodeIndex))
        #print('DBG1722> IF= ' + str(nodeIF) + '  ELSE= ' + str(in_nodeIndex))

        for dbk in getDataBlocks(io_G, in_nodeIndex):

            if dbk.flight in ['flUNDEF', 'flFLYOVER']:
                erri = inspect.currentframe().f_lineno
                errmsg(erri, 'Invalid DBK flight found: "' + dbk.flight + '"')
                break

            if dbk.name =='NULL_GUID':
                # 'NULL_GUID' do not need to be released, nor destroyed, and they are always outgoing.
                continue

            #print('DBG1736> ELSE DBK = ' + dbk.name)
            #print('DBG1736> node parent of IF = ' + str(getNodeparents(io_G, nodeIF)))

            parentIF = GBL.NOTA_NODE_ID
            for pi in getNodeparents(io_G, nodeIF):
                edg = (pi, nodeIF)

                is_shared = False
                for cx in sharedConx(io_G, edg):
                    if nodeIF == cx[1]:
                        # This connection is shared with another
                        is_shared = True
                        break
                if is_shared:
                    continue  # Only kept the true connection, not those that piggybacks.
                              # 2016Sept13-2229: For now see below

                events = getEvents(io_G, edg)
                for evtName in events:
                    if evtName == dbk.name:
                        parentIF = pi
                        break
                if not parentIF == GBL.NOTA_NODE_ID:  # 2016Sept13-2229: Added this as a fix, i.e. stop at first found.
                    break
            if parentIF == GBL.NOTA_NODE_ID:
                # 2016Sept13-2229: I've addwed the search into the shared connection because the shared connection
                #                  definition is too restrictive.
                #TODO: Review the concept of shared connections and make it specific to a labeled edge, not an entiore edge.
                for pi in getNodeparents(io_G, nodeIF):
                    edg = (pi, nodeIF)

                    is_shared = False
                    for cx in sharedConx(io_G, edg):
                        if nodeIF == cx[1]:
                            # This connection is shared with another
                            is_shared = True
                            break
                    if not is_shared:
                        continue  # Only kept the shared connection since we arfe still missing a DBK

                    events = getEvents(io_G, edg)
                    for evtName in events:
                        if evtName == dbk.name:
                            parentIF = pi
                            break
                    if not parentIF == GBL.NOTA_NODE_ID:  # 2016Sept13-2229: Added this as a fix, i.e. stop at first found.
                        break

            if parentIF == GBL.NOTA_NODE_ID:
                name_fromIndex = getNodeNamebyitsNumber(io_G, in_nodeIndex)
                nameIF = getNodeNamebyitsNumber(io_G, nodeIF)
                t = 'The DBK "' + dbk.name + '" of node "' + name_fromIndex + '"  could not be found among its IF '
                t += 'clause EDT (node="' + nameIF + '")'
                warni = inspect.currentframe().f_lineno
                warnmsg(warni, t)
                break

            edgIF = (parentIF, nodeIF)
            edgName = makeInDerefsName(io_G, edgIF)
            drname = edgName + '_' + makeEdtDepName(dbk.name)

            if getEvent(io_G, (edgIF, dbk.name)).isTheLead:
                if dbk.flight in ['flPASSTRU', 'flTAGO']:
                    if not dbk.user2destroyORrelease:
                        addToBeReleased(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                    addToOutgoing(io_G, in_nodeIndex, (dbk.name, drname+'.guid'))
                elif dbk.flight in ['flLANDING', 'flHOP']:
                    if not dbk.user2destroyORrelease:
                        addToBeDestroyed(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                elif dbk.flight in ['flTAKEOFF']:
                    # This will be handle directly in release_and_destroy_DBKs
                    pass
                else:
                    erri = inspect.currentframe().f_lineno
                    break
        if erri: break

        debug = False
        if debug:
            print('DBG1714>node = ' + str(in_nodeIndex))
            print('DBG1714>\tDESTROY list:'),
            print(getDestroys(io_G,in_nodeIndex))
            print('DBG1714>\tRELEASE list:'),
            print(getReleases(io_G, in_nodeIndex))
            print('DBG1714>\tOUTGOING list:'),
            print(getOutgoings(io_G, in_nodeIndex))

        break  # while not erri:
    return erri
# ------------------------------------------------------------------------------
def find_destroy_outgoing_release2(io_G, in_nodeIndex):
    whereAmI()
    #print('DBG1714> nod=' + str(in_nodeIndex) )

    erri = 0
    while not erri:

        isaELSEedt = getNode(io_G, in_nodeIndex)["istheELSEclause"]
        if isaELSEedt:
            erri = find_destroy_outgoing_release_forELSEedt(io_G, in_nodeIndex)
            break

        for dbk in getDataBlocks(io_G, in_nodeIndex):
            if dbk.flight in ['flUNDEF', 'flFLYOVER']:
                erri = inspect.currentframe().f_lineno
                errmsg(erri, 'Invalid DBK flight found: "' + dbk.flight + '"')
                break
            if dbk.name == 'NULL_GUID':
                # 'NULL_GUID' do not need to be released, nor destroyed, and they are always outgoing.
                continue

            if not dbk.name in ['flTAGO', 'flLANDING']:
                  continue

            parent = GBL.NOTA_NODE_ID
            for pi in getNodeparents(io_G, in_nodeIndex):
                edg = (pi, in_nodeIndex)

                is_shared = False
                for cx in sharedConx(io_G, edg):
                    if in_nodeIndex == cx[1]:
                        # This connection is shared with another
                        is_shared = True
                        break
                if is_shared:
                    continue  # Only kept the true connection, not those that piggybacks.
                    # 2016Sept13-2229: For now see below

                events = getEvents(io_G, edg)
                for evtName in events:
                    if evtName == dbk.name:
                        parent = pi
                        break
                if not parent == GBL.NOTA_NODE_ID:  # 2016Sept13-2229: Added this as a fix, i.e. stop at first found.
                    break
            if parent == GBL.NOTA_NODE_ID:
                # 2016Sept13-2229: I've addwed the search into the shared connection because the shared connection
                #                  definition is too restrictive.
                # TODO: Review the concept of shared connections and make it specific to a labeled edge, not an entiore edge.
                for pi in getNodeparents(io_G, in_nodeIndex):
                    edg = (pi, in_nodeIndex)

                    is_shared = False
                    for cx in sharedConx(io_G, edg):
                        if in_nodeIndex == cx[1]:
                            # This connection is shared with another
                            is_shared = True
                            break
                    if not is_shared:
                        continue  # Only kept the shared connection since we arfe still missing a DBK

                    events = getEvents(io_G, edg)
                    for evtName in events:
                        if evtName == dbk.name:
                            parent = pi
                            break
                    if not parent == GBL.NOTA_NODE_ID:  # 2016Sept13-2229: Added this as a fix, i.e. stop at first found.
                        break

            if parent == GBL.NOTA_NODE_ID:
                name_fromIndex = getNodeNamebyitsNumber(io_G, in_nodeIndex)
                namee = getNodeNamebyitsNumber(io_G, in_nodeIndex)
                t = 'The DBK "' + dbk.name + '" of node "' + name_fromIndex + '"  could not be found among its IF '
                t += 'clause EDT (node="' + namee + '")'
                warni = inspect.currentframe().f_lineno
                errmsg(warni, t)
                break

            edgee = (parent, in_nodeIndex)
            edgName = makeInDerefsName(io_G, edgee)
            drname = edgName + '_' + makeEdtDepName(dbk.name)

            if getEvent(io_G, (edgee, dbk.name)).isTheLead:
                if dbk.flight in ['flPASSTRU', 'flTAGO']:
                    if not dbk.user2destroyORrelease:
                        addToBeReleased(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                    addToOutgoing(io_G, in_nodeIndex, (dbk.name, drname + '.guid'))
                elif dbk.flight in ['flLANDING', 'flHOP']:
                    if not dbk.user2destroyORrelease:
                        addToBeDestroyed(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                elif dbk.flight in ['flTAKEOFF']:
                    # This will be handle directly in release_and_destroy_DBKs
                    pass
                else:
                    erri = inspect.currentframe().f_lineno
                    break

        if erri: break  # for dbk in getDataBlocks(io_G, in_nodeIndex):

        for p in getNodeparents(io_G, in_nodeIndex):
            edg = (p, in_nodeIndex)

            is_shared = False
            for cx in sharedConx(io_G, edg):
                if in_nodeIndex == cx[1]:
                    # This connection is shared with another
                    is_shared = True
                    break
            if is_shared:
                continue

            derefs = dereferenciables(io_G, edg)

            if len(derefs) > 0:
                # NOTE: flTAKEOFF are taken care directly in release_and_destroy_DBKs
                edgName = makeInDerefsName(io_G, edg)
                for k, deref in enumerate(derefs):
                    drname = edgName + '_' + makeEdtDepName(deref)

                    for dbk in getDataBlocks(io_G, in_nodeIndex):
                        if not dbk.name == deref:
                            continue
                        if getEvent(io_G, (edg, dbk.name)).isTheLead:
                            if dbk.flight in ['flLANDING', 'flHOP']:
                                if not dbk.user2destroyORrelease:
                                    addToBeDestroyed(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                            if dbk.flight in ['flTAGO', 'flPASSTRU']:
                                if not dbk.user2destroyORrelease:
                                    addToBeReleased(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                                addToOutgoing(io_G, in_nodeIndex, (deref, drname+'.guid'))

        if erri: break  # for p in getNodeparents(in_G,n):

        debug = False
        if debug:
            print('DBG2041>\tDESTROY list:'),
            print(getDestroys(io_G,in_nodeIndex))
            print('DBG2041>\tRELEASE list:'),
            print(getReleases(io_G, in_nodeIndex))
            print('DBG2041>\tOUTGOING list:'),
            print(getOutgoings(io_G, in_nodeIndex))

        break  # while not erri:

    return erri
# ------------------------------------------------------------------------------
# Version 2 exist and is above
def find_destroy_outgoing_release(io_G, in_nodeIndex):
    whereAmI()
    #print('DBG1714> nod=' + str(in_nodeIndex) )

    erri = 0
    while not erri:

        isaELSEedt = getNode(io_G, in_nodeIndex)["istheELSEclause"]
        if isaELSEedt:
            erri = find_destroy_outgoing_release_forELSEedt(io_G, in_nodeIndex)
            break

        for p in getNodeparents(io_G, in_nodeIndex):
            edg = (p, in_nodeIndex)

            is_shared = False
            for cx in sharedConx(io_G, edg):
                if in_nodeIndex == cx[1]:
                    # This connection is shared with another
                    is_shared = True
                    break
            if is_shared:
                continue

            lits   = literals(io_G, edg)
            derefs = dereferenciables(io_G, edg)

            len_lits = len(lits)
            for lit in lits:
                if lit == 'NULL_GUID':
                    len_lits += -1

            if len_lits > 0:
                errmsg(inspect.currentframe().f_lineno, "DEV> Handling of literals not fully debugged yet.\n")

                edgName = makeInLiteralsName(io_G, edg)
                litsOffset = -1
                for k, lit in enumerate(lits):
                    if lit == 'NULL_GUID':
                        continue
                    if litsOffset == -1:
                        events = getEvents(io_G, edg)
                        litsOffset = events[lit].calculatedSlot
                    for dbk in getDataBlocks(io_G, in_nodeIndex):
                        if not dbk.name == lit:
                            continue
                        if getEvent(io_G, (edg, dbk.name)).isTheLead:
                            litName = edgName + '.' + makeGuidDataBlockname(lit)
                            if dbk.flight in ['flLANDING', 'flHOP']:
                                if not dbk.user2destroyORrelease:
                                    addToBeDestroyed(io_G, in_nodeIndex, contextedGuid('', litName, 'DBK'))
                            if dbk.flight in ['flTAGO', 'flPASSTRU']:
                                addToOutgoing(io_G, in_nodeIndex, (lit, litName))

            if len(derefs) > 0:
                # NOTE: flTAKEOFF are taken care directly in release_and_destroy_DBKs
                edgName = makeInDerefsName(io_G, edg)
                for k, deref in enumerate(derefs):
                    drname = edgName + '_' + makeEdtDepName(deref)

                    for dbk in getDataBlocks(io_G, in_nodeIndex):
                        if not dbk.name == deref:
                            continue
                        if getEvent(io_G, (edg, dbk.name)).isTheLead:
                            if dbk.flight in ['flLANDING', 'flHOP']:
                                if not dbk.user2destroyORrelease:
                                    addToBeDestroyed(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                            if dbk.flight in ['flTAGO', 'flPASSTRU']:
                                if not dbk.user2destroyORrelease:
                                    addToBeReleased(io_G, in_nodeIndex, contextedGuid('', drname + '.guid', 'DBK'))
                                addToOutgoing(io_G, in_nodeIndex, (deref, drname+'.guid'))

        if erri: break  # for p in getNodeparents(in_G,n):

        debug = False
        if debug:
            print('DBG2041>\tDESTROY list:'),
            print(getDestroys(io_G,in_nodeIndex))
            print('DBG2041>\tRELEASE list:'),
            print(getReleases(io_G, in_nodeIndex))
            print('DBG2041>\tOUTGOING list:'),
            print(getOutgoings(io_G, in_nodeIndex))

        break  # while not erri:

    return erri
# ------------------------------------------------------------------------------
def setup_inputs_structs2(io_G, in_nodeIndex, in_tab, io_file, io_delayed_releaseDestroy):
    whereAmI()
    tab2 = in_tab + GBL.TABunit

    #TODO: Implement the handling of literals in setup_inputs_structs
#    print("DEV-DBG2200> hook for node 15\n")
#    if in_nodeIndex == 15:
#        import pdb
#        pdb.set_trace()
#        print("DBG2200> At node 15\n")

    erri = 0
    while not erri:
        io_file.write(in_tab + '//----- Setup input data structs\n')

        for dbk in getDataBlocks(io_G, in_nodeIndex):
            if dbk.name == 'NULL_GUID':
                continue
            if dbk.flight in ['flUNDEF', 'flFLYOVER']:
                erri = inspect.currentframe().f_lineno
                break
            if dbk.flight in ['flTAKEOFF', 'flHOP']:
                continue
            if dbk.flight in 'flPASSTRU':
                erri = inspect.currentframe().f_lineno
                print('ERROR: DBK with flPASSTRU are not handled yet.\n')
                break

            if not dbk.flight in ['flTAGO', 'flLANDING']:
                erri = inspect.currentframe().f_lineno
                break

            #First look for DBK in the incoming edges which are not shared
            goodEdge = ()
            for p in getNodeparents(io_G, in_nodeIndex):
                edg = (p, in_nodeIndex)

                is_shared = False
                for cx in sharedConx(io_G, edg):
                    if in_nodeIndex == cx[1]:
                        # This connection is shared with another
                        is_shared = True
                        break
                if is_shared:
                    continue

                events = getEvents(io_G, edg)
                for evtName in events:
                    if evtName == dbk.name:
                        goodEdge = edg
                        break  # Pick the first that works
                if not goodEdge == ():
                    break

            if goodEdge == ():
                # Now search among the shared edges
                for p in getNodeparents(io_G, in_nodeIndex):
                    edg = (p, in_nodeIndex)

                    is_shared = False
                    for cx in sharedConx(io_G, edg):
                        if in_nodeIndex == cx[1]:
                            # This connection is shared with another
                            is_shared = True
                            break
                    if not is_shared:
                        continue

                    events = getEvents(io_G, edg)
                    for evtName in events:
                        if evtName == dbk.name:
                            goodEdge = edg
                            break  # Pick the first that works
                    if not goodEdge == ():
                        break

            if goodEdge == ():
                erri = inspect.currentframe().f_lineno
                print("ERROR: On node " + str(in_nodeIndex) + ', no incoming event found for DBK = ' + dbk.name + '\n')
                break

            edgName = makeInDerefsName(io_G, goodEdge)

            calcSlot = getEvents(io_G, goodEdge)[dbk.name].calculatedSlot
            drname = edgName + '_' + makeEdtDepName(dbk.name)

            if dbk.delayReleaseDestroy:
                io_delayed_releaseDestroy.append(drname)

            text = in_tab + 'ocrEdtDep_t ' + drname + ' = depv[' + str(calcSlot) + '];\n'
            io_file.write(text)

            if len(dbk.localname) > 0:
                text = in_tab + dbk.type + ' * ' + dbk.localname + ' = ' + drname + '.ptr;\n'
                io_file.write(text)

            if len(dbk.localnameGuid) > 0:
                text = in_tab + 'ocrGuid_t ' + dbk.localnameGuid + ' = ' + drname + '.guid;\n'
                io_file.write(text)

            if len(dbk.getLocalTexts()) > 0:
                for lt in dbk.getLocalTexts():
                    text = in_tab + lt + '\n'
                    io_file.write(text)

        if erri: break  # for dbk in getDataBlocks(io_G, in_nodeIndex):

        io_file.write('\n')

        break  # while not erri:

    return erri
# ------------------------------------------------------------------------------
# Version 2 exist and is above
def setup_inputs_structs(io_G, in_nodeIndex, in_tab, io_file, io_delayed_releaseDestroy):
    whereAmI()
    #print('DBG1885> node=' + str(in_nodeIndex) )
    tab2 = in_tab + GBL.TABunit
#    print("DEV-DBG2200> hook for node 15\n")
#   if in_nodeIndex == 15:
#        import pdb
#        pdb.set_trace()
#        print("DBG2200> At node 15\n")

    erri = 0
    while not erri:
        io_file.write(in_tab + '//----- Setup input data structs\n')

        for p in getNodeparents(io_G, in_nodeIndex):
            edg = (p, in_nodeIndex)
            lits   = literals(io_G, edg)
            derefs = dereferenciables(io_G, edg)
            #print('DBG1895> Ein  '+ str(edg) +'  '+ str(lits) +'  '+ str(derefs))

            is_shared = False
            for cx in sharedConx(io_G, edg):
                if in_nodeIndex == cx[1]:
                    # This connection is shared with another
                    is_shared = True
                    break
            if is_shared:
                #print('DBG1832> Ein  '+ str(edg) +'  '+ str(lits) +'  '+ str(derefs))
                continue

            len_lits = len(lits)
            for lit in lits:
                if lit == 'NULL_GUID':
                    len_lits += -1

            if len_lits > 0:
                erri = inspect.currentframe().f_lineno
                errmsg(erri, "DEV> Handling of literals not fully implemented yet.\n")
                break

            if len(derefs) > 0:
                edgName = makeInDerefsName(io_G, edg)
                for k, deref in enumerate(derefs):
                    calcSlot = getEvents(io_G, edg)[deref].calculatedSlot
                    drname = edgName + '_' + makeEdtDepName(deref)
                    text = in_tab + 'ocrEdtDep_t ' + drname + ' = depv[' + str(calcSlot) + '];'
                    text += '\n'
                    io_file.write(text)
                    for dbk in getDataBlocks(io_G, in_nodeIndex):
                        if not dbk.name == deref:
                            continue
                        if dbk.delayReleaseDestroy:
                            io_delayed_releaseDestroy.append(drname)
                        if dbk.flight == 'flLANDING':
                            if len(dbk.localname) > 0:
                                text = in_tab + dbk.type + ' * ' + dbk.localname + ' = ' + drname + '.ptr;\n'
                                io_file.write(text)
                            if len(dbk.getLocalTexts()) > 0:
                                for lt in dbk.getLocalTexts():
                                    text = in_tab + lt + '\n'
                                    io_file.write(text)
                        if dbk.flight in ['flTAGO', 'flPASSTRU']:
                            if dbk.flight in 'flTAGO':
                                if len(dbk.localname) > 0:
                                    text = in_tab + dbk.type + ' * ' + dbk.localname + ' = ' + drname + '.ptr;\n'
                                    io_file.write(text)
                                if len(dbk.getLocalTexts()) > 0:
                                    for lt in dbk.getLocalTexts():
                                        text = in_tab + lt + '\n'
                                        io_file.write(text)
                if erri: break

        if erri: break  # for p in getNodeparents(in_G,n):

        #print('DBG1951> '),
        #print(io_delayed_releaseDestroy)

        io_file.write('\n')

        break  # while not erri:

    return erri
# ------------------------------------------------------------------------------
def create_dataBlocks(in_G, in_nodeIndex, in_tab, io_file):
    use_old_sizeName = GBL.use_old_sizeName

    erri = 0
    while not erri:
        if len(in_G.nodes()) == 0:
            erri = inspect.currentframe().f_lineno
            break
        if len(getDataBlocks(in_G, in_nodeIndex)) == 0:
            break
        io_file.write(in_tab + '//----- Create_dataBlocks\n')
        if True:  # use_old_sizeName:  # Change to True as some application will need explicit counts.
            for dbk in getDataBlocks(in_G, in_nodeIndex):
                if dbk.name == 'NULL_GUID':
                    continue
                if not dbk.flight in ['flTAKEOFF', 'flHOP']:
                    continue
                t = in_tab + 'const u64 ' + dbk.text_for_sizeName() + ' = ' + str(dbk.count) + ';\n'
                io_file.write(t)
            io_file.write('\n')
        for dbk in getDataBlocks(in_G, in_nodeIndex):
            if dbk.name == 'NULL_GUID':
                continue
            if not dbk.flight in ['flTAKEOFF', 'flHOP']:
                continue
            t = dbk.text_for_ocrDbCreate(in_tab, use_old_sizeName)
            io_file.write(t)
            dbk.dbg_text_for_ocrDbCreate(in_tab, io_file)

        io_file.write('\n')

        break  # while not erri
    return erri
# ------------------------------------------------------------------------------
def write_user_section(in_G, in_nodeIndex, in_tab, io_file):
    io_file.write(in_tab + '//----- User section\n')

    customs = getCustomTexts(in_G, in_nodeIndex);
    if len(customs) > 0:
        for line in customs:
            io_file.write(in_tab + line +'\n')
    else:
        io_file.write('\n')
    io_file.write('\n')
# ------------------------------------------------------------------------------
def write_children_EDT(in_G, in_nodeIndex, in_tab, io_file):
    erri=0
    while not erri:
        if len(in_G.nodes()) ==0:
            erri=inspect.currentframe().f_lineno
            break

        io_file.write(in_tab+ "//----- Create children EDTs\n")

        bors=in_G.neighbors(in_nodeIndex)

        for b in bors:
            edg = (in_nodeIndex, b)
            events = getEvents(in_G,edg)
            createThechild = False
            for evt in events:
                if events[evt].fertile:
                    createThechild = True
                    break
            if createThechild:
                t = getMyTask(in_G, b).text_for_ocrEdtCreate(in_tab)
                io_file.write(t)
                getMyTask(in_G, b).dbg_text_for_ocrEdtCreate(in_tab, io_file)

        edts = getEdts(in_G, in_nodeIndex)
        for edt in edts:
            if isinstance(edt, ocrEdt):
                t = edt.text_for_ocrEdtCreate(in_tab)
                io_file.write(t)
                edt.dbg_text_for_ocrEdtCreate(in_tab, io_file)
            elif isinstance(edt,str):
                nodn = getNodebyitsName(in_G, edt)
                if nodn == GBL.NOTA_NODE_ID:
                    erri = inspect.currentframe().f_lineno
                    break
                else:
                    childedt = getMyTask(in_G, nodn)
                    t = childedt.text_for_ocrEdtCreate(in_tab)
                    childedt.text_for_ocrEdtCreate(in_tab, io_file)
                    io_file.write(t)
            else:
                erri=inspect.currentframe().f_lineno
                errmsg(erri,"An EDT can only create a child EDT by its name or by its ocrEdt class")
                break
            if erri: break
        io_file.write('\n')
        break #while not erri
    return erri
# ------------------------------------------------------------------------------
def release_and_destroy_DBKs(in_G, in_nodeIndex, in_tab, io_file,
                             in_delayed_releaseDestroy,  # This formed the delay set
                             in_onlyInDelaySet):  # Set to True in order to process only the elements in the DelaySet
                                                  # Set to False in order to process all elements NOt in the DelaySet
    whereAmI()
    debug = False
    erri = 0
    while not erri:
        if len(in_G.nodes()) == 0:
            erri = inspect.currentframe().f_lineno
            break

        io_file.write(in_tab + '//----- Release or destroy data blocks\n')

        if debug:
            print('DBG2006> n = ' + str(in_nodeIndex) + ' DBKs= '+ str(getDataBlocks(in_G, in_nodeIndex)))

        for dbk in getDataBlocks(in_G, in_nodeIndex):
            if dbk.flight in ['flUNDEF', 'flFLYOVER']:
                erri=inspect.currentframe().f_lineno
                break

            fcn = 'ocrDbRelease'
            cguid = contextedGuid("", '', 'DBK')
            cguid.guidName = makeGuidDataBlockname(dbk.name)

            if dbk.flight == 'flLANDING':
                if dbk.name == 'NULL_GUID':
                    io_file.write(in_tab + '//Skipping destruction of a NULL_GUID data block.\n')
                    continue
                continue # Use the section on "Destruction for landing blocks" below
            elif dbk.flight == 'flHOP':
                if dbk.name == 'NULL_GUID':
                    erri=inspect.currentframe().f_lineno
                    break
                fcn = 'ocrDbDestroy'
                cguid.structName = ""
            elif dbk.flight == 'flTAKEOFF':
                if dbk.name == 'NULL_GUID':
                    io_file.write(in_tab + '//Skipping release of a NULL_GUID data block.\n')
                    continue
                fcn = 'ocrDbRelease'
                cguid.structName = ""
            elif dbk.flight in ['flPASSTRU', 'flTAGO']:
                continue  # Use the Release section below
            else:
                erri=inspect.currentframe().f_lineno
                break

            if len(in_delayed_releaseDestroy):
                x_name = cguid.toName()
                if x_name in in_delayed_releaseDestroy:
                    if not in_onlyInDelaySet:
                        continue
                else:
                    if in_onlyInDelaySet:
                        continue

            if not dbk.user2destroyORrelease:
                io_file.write(in_tab + 'err = '+ fcn + '(' + cguid.toName() + '); IFEB;\n')
        if erri: break

        if debug:
            print('DBG2006> node = ' + str(in_nodeIndex))
            print('DBG2006> To be Destroyed = '),
            print(getDestroys(in_G, in_nodeIndex))
            print('DBG2006> To be Released = '),
            print(getReleases(in_G, in_nodeIndex))

        # Destruction for blocks
        for roy in getDestroys(in_G, in_nodeIndex):
            if len(in_delayed_releaseDestroy):
                x_name = roy.toName()
                y_name = x_name.split('.')[0]  #2016aug16: I found out that the name in 'x_name' is actually a compound name
                                               #           of  the form abcdef.guid.  So I could not match names to this
                                               #           because of the '.guid' part.
                                               # TODO: When doing ocrDestroy, find a nice way to label the guid name
                if y_name in in_delayed_releaseDestroy:
                    if not in_onlyInDelaySet:
                        continue
                else:
                    if in_onlyInDelaySet:
                        continue
            t = roy.getDestroyText()
            io_file.write(in_tab + t + '\n')

        # Release of dereferenced data blocks
        for rel in getReleases(in_G, in_nodeIndex):
            if len(in_delayed_releaseDestroy):
                x_name = rel.toName()
                y_name = x_name.split('.')[0]   #2016aug16: I found out that the name in 'x_name' is actually a compound name
                                                #           of  the form abcdef.guid.  So I could not match names to this
                                                #           because of the '.guid' part.
                                                # TODO: When doing ocrDestroy, find a nice way to label the guid name
                if y_name in in_delayed_releaseDestroy:
                    if not in_onlyInDelaySet:
                        continue
                else:
                    if in_onlyInDelaySet:
                        continue
            t = rel.getReleaseText()
            io_file.write(in_tab + t + '\n')

        io_file.write('\n')
        break  # while not erri
    return erri
# ------------------------------------------------------------------------------
def write_events2children_EDT(in_G, in_nodeIndex, in_tab, io_file):
    tab2 = in_tab + GBL.TABunit
    debug = False
    erri = 0
    while not erri:
        if len(in_G.nodes()) == 0:
            erri = inspect.currentframe().f_lineno
            break

        io_file.write(in_tab + '//----- Link to other EDTs using Events\n')

        bors = in_G.neighbors(in_nodeIndex)
        #  print('DBG2310> in_nodeIndex =' + str(in_nodeIndex))
        if debug and in_nodeIndex == 17:
            print('DBG2313> At the check point')
        #  print('DBG2310> bors =' + str(bors))
        for b in bors:
            if debug: print('DBG2314> evt2child> n=' + str(in_nodeIndex) + '  b= ' + str(b) + '  isELSE=' + str(getNode(in_G, b)["istheELSEclause"]) )
            if getNode(in_G, b)["istheELSEclause"]:
                continue
            edg = (in_nodeIndex, b)
            lits   = literals(in_G, edg)
            derefs = dereferenciables(in_G, edg)
            if debug: print('DBG2039> evt2child> Eout  ' + str(edg) + '  ' + str(lits) + '  ' + str(derefs))

            if len(lits) > 0 and GBL.display_literals_content:
                len_lits = len(lits)
                for lit in lits:
                    if lit == 'NULL_GUID':
                        len_lits -= 1
                if len_lits > 0:
                    outLitName = makeOutLiteralsName(in_G, edg)
                    io_file.write(in_tab + '// Content for Literals for : ' + outLitName + '\n')
                    t = ''
                    for lit in lits:
                        comment = ''
                        if lit == 'NULL_GUID':
                            continue
                        t += in_tab + '// ' + GBL.TABunit + lit + comment + '\n'
                    io_file.write(t)
                    io_file.write('\n')

            # First setup the structs
            slots_entries = []  # A list of 3-tuple (dbk_identifier, dbk_guid, slot number)
            if len(lits) > 0:
                len_lits = len(lits)
                for lit in lits:
                    if lit == 'NULL_GUID':
                        calcSlot = getEvents(in_G, edg)[lit].calculatedSlot
                        slots_entries.append(('NULL_GUID', 'NULL_GUID', calcSlot))
                        len_lits -= 1
                if len_lits > 0:
                    outLitName = makeOutLiteralsName(in_G, edg)
                    outLitName_t = makeOutLiteralsNameStruct(in_G, edg)
                    io_file.write(in_tab + 'typedef struct ' + outLitName_t + '{\n')

                    litsOffset = GBL.NOTA_OFFSET

                    for k, lit in enumerate(lits):
                        if lit == 'NULL_GUID':
                            continue
                        endword = ',\n'
                        if k + 1 == len_lits:
                            endword = '\n'
                        io_file.write(tab2 + 'ocrGuid_t ' + makeGuidDataBlockname(lit) + endword)
                        if litsOffset == GBL.NOTA_OFFSET:
                            events = getEvents(in_G, edg)
                            litsOffset = events[lit].calculatedSlot
                    if erri: break

                    io_file.write(in_tab + '} ' + outLitName_t + ';\n')
                    io_file.write(in_tab + outLitName_t + ' ' + outLitName + ';\n')

                    slots_entries.append(('literals', outLitName, litsOffset))  # TODO: remove 'literals

            if erri: break

            #  print('DBG2372> len(derefs)=' + str(len(derefs)))
            if len(derefs) > 0:
                #  print('DBG2374> ' + str(derefs))
                for deref in derefs:
                    calcSlot = getEvents(in_G, edg)[deref].calculatedSlot
                    drname = ''
                    for dbk in getDataBlocks(in_G, in_nodeIndex):
                        if not dbk.name == deref:
                            continue
                        if dbk.flight == 'flTAKEOFF':
                            drname = makeGuidDataBlockname(deref)
                        elif dbk.flight in ['flTAGO', 'flPASSTRU'] :
                            outs = getOutgoings(in_G, in_nodeIndex)
                            if debug: print('DBG2389> deref= ' + str(deref) + '  outs= ' + str(outs))
                            for dbkID,dbkGuid in outs:
                                if deref == dbkID:
                                    drname = dbkGuid
                                    break
                        else:
                            outDerefName = makeOutDerefsName(in_G, edg)
                            drname = outDerefName + '_' + makeGuidDataBlockname(deref)
                    if drname == '':
                        erri = inspect.currentframe().f_lineno
                        break
                    slots_entries.append((deref, drname, calcSlot))
                if erri: break

            # print('DBG1779> len(slots_entries)=' + str(len(slots_entries)))
            if len(slots_entries) > 0:
                for dbkID, guidName, calcSlot in slots_entries:
                    targetEdtName = getMyTask(in_G, b).aname
                    targetEdtGuid = makeGuidEdtname(targetEdtName)

                    edgName = makeEdgeName(in_G, edg) + '_' + dbkID
                    ge_event = makeGuidEventname(edgName)

                    events = getEvents(in_G, edg)
                    evt = events[dbkID]
                    if GBL.use_verbose_way_to_eventsText:
                        t = evt.text_for_ocrEventCreate(in_tab, ge_event)
                        io_file.write(t)

                        t = evt.text_for_ocrAddDependence(in_tab, ge_event, targetEdtGuid)
                        io_file.write(t)

                        t = evt.text_for_ocrEventSatisfySlot(in_tab, ge_event, guidName)
                        io_file.write(t)
                    else:
                        t = evt.text_for_ocrHookUp(in_tab, targetEdtGuid, guidName)
                        io_file.write(t)
                        evt.dbg_text_for_ocrHookUp(in_tab, targetEdtGuid, targetEdtName, guidName, dbkID, io_file)

                    io_file.write('\n')
                if erri: break
            if erri: break  # for b in bors:
        break  # while not erri
    return erri
# ------------------------------------------------------------------------------
def write_finalEDT(in_G, in_tab, io_file):
    whereAmI()
    erri=0
    while not erri:
        if len(in_G.nodes()) == 0:
            erri=inspect.currentframe().f_lineno
            break
        io_file.write(in_tab + "ocrShutdown();\n")

        break #while not erri
    return erri
# ------------------------------------------------------------------------------
def write_the_header(in_G, io_file):
    whereAmI()
    io_file.write("#include <ocr.h>\n")
    io_file.write('#include "' + GBL.app_ocr_util_filename +'"\n' )

    if GBL.insert_debug_code:
        io_file.write('#include <extensions/ocr-runtime-itf.h>  //For currentEdtUserGet\n')

    customs = getHeader(in_G)
    for aline in customs:
        io_file.write(aline + '\n')

    io_file.write("\n")
# ------------------------------------------------------------------------------
def outputOCR_writeSubclause(io_G, in_nodeIndex, in_tab, io_file):
    erri = 0
    while not erri:
        erri = create_dataBlocks(io_G, in_nodeIndex, in_tab, io_file)
        if erri: break

        erri = write_children_EDT(io_G, in_nodeIndex, in_tab, io_file)
        if erri: break

        write_user_section(io_G, in_nodeIndex, in_tab, io_file)

        empty_delay = []
        release_and_destroy_DBKs(io_G, in_nodeIndex, in_tab, io_file, empty_delay, False)

        erri = write_events2children_EDT(io_G, in_nodeIndex, in_tab, io_file)
        if erri: break

        if in_nodeIndex == GBL.FINALNODE:
            write_finalEDT(io_G, in_tab, io_file)
            if erri: break

        break  # while not erri
    return erri

# ------------------------------------------------------------------------------
def outputOCR_writeFOR_Subclause(io_G, in_nodeIndex, in_tab, io_file, in_delayed_releaseDestroy):
    tab2 = in_tab + GBL.TABunit
    erri = 0
    while not erri:
        #print('DBG2306--->'),
        #print(in_delayed_releaseDestroy)
        forclause = getFORconditionText(io_G, in_nodeIndex)

        fortext = forclause + '{\n'
        io_file.write(in_tab + fortext)

        startClause = getFORconditionText_startingclause(io_G, in_nodeIndex)
        if not len(startClause) == 0:
            for tl in startClause:
                tl_text = tab2 + tl + '\n'
                io_file.write(tl_text)

        erri = create_dataBlocks(io_G, in_nodeIndex, tab2, io_file)
        if erri: break

        erri = write_children_EDT(io_G, in_nodeIndex, tab2, io_file)
        if erri: break

        write_user_section(io_G, in_nodeIndex, tab2, io_file)

        release_and_destroy_DBKs(io_G, in_nodeIndex, tab2, io_file, in_delayed_releaseDestroy, False)

        erri = write_events2children_EDT(io_G, in_nodeIndex, tab2, io_file)
        if erri: break

        io_file.write(in_tab + '}; IFEB;\n')

        release_and_destroy_DBKs(io_G, in_nodeIndex, in_tab, io_file, in_delayed_releaseDestroy, True)

        if in_nodeIndex == GBL.FINALNODE:
            write_finalEDT(io_G, in_tab, io_file)
            if erri: break

        break  # while not erri
    return erri

# ------------------------------------------------------------------------------
def outputOCR_writeEDT(io_G, in_nodeIndex, in_tab, io_file):
    tab2 = in_tab + GBL.TABunit
    erri = 0
    while not erri:
        if in_nodeIndex == GBL.MAINNODE:
            pass

        delayed_releaseDestroy = []
        erri = setup_inputs_structs2(io_G, in_nodeIndex, in_tab, io_file, delayed_releaseDestroy)
        if erri: break

        #print('DBG2363> '),
        #print(delayed_releaseDestroy)

        if len(getIFconditionText(io_G, in_nodeIndex)) == 0:
            forclause = getFORconditionText(io_G, in_nodeIndex)
            if len(forclause):
                erri = outputOCR_writeFOR_Subclause(io_G, in_nodeIndex, in_tab, io_file, delayed_releaseDestroy)
            else:
                erri = outputOCR_writeSubclause(io_G, in_nodeIndex, in_tab, io_file)
            if erri: break
        else:
            io_file.write(in_tab + 'if( ')
            t = getIFconditionText(io_G, in_nodeIndex)
            io_file.write(t + ' ){\n')

            erri = outputOCR_writeSubclause(io_G, in_nodeIndex, tab2, io_file)

            io_file.write(in_tab + '}else{\n')

            for bor in io_G.neighbors(in_nodeIndex):
                edg = (in_nodeIndex, bor)
                if not getEdge(io_G,edg)["leads_to_ElseClause"]:
                    continue
                erri = outputOCR_writeSubclause(io_G, bor, tab2, io_file)
                if erri:
                    t = 'outputOCR_writeSubclause failed for IF-THEN node "' + str(in_nodeIndex) +'" '
                    t += 'and ELSE node "' + str(bor) + '"'
                    errmsg(erri, t)
                    break
                break  # Only output the first one as there should only be one.
            if erri: break

            io_file.write(in_tab + '}\n')

        break  # while not erri
    return erri
# ------------------------------------------------------------------------------
def outputOCR(in_G, in_filename):
    whereAmI()
    tab = "    "
    tab2 = tab + GBL.TABunit
    tab3 = tab2 + GBL.TABunit
    erri = 0
    while not erri:
        if len(in_G.nodes()) ==0:
            # Only output code if we really have something to output.
            erri=inspect.currentframe().f_lineno
            break

        try:
            fo = open(in_filename, 'w')
        except:
            erri=inspect.currentframe().f_lineno
            break

        write_the_header(in_G, fo)

        # ----- Forward declarations
        for n in in_G.nodes():
            if getNode(in_G, n)["istheELSEclause"]:
                # Skip any ELSEclause nodes as they will be handled by their
                # corresponding IFconditionText & THEN node.
                continue
            # ----- start of the task
            name = getMyTask(in_G, n).aname
            fo.write("ocrGuid_t " + name + "(EDT_ARGS);\n")
        fo.write("\n")

        if GBL.insert_debug_code:
            t = 'void OA_DBG_task_type_name(){\n'
            fo.write(t)
            for n in in_G.nodes():
                tname = getMyTask(in_G, n).aname
                ttype = str(n)
                t = tab + 'PRINTF("OA_DBG_PRECOND> Ttype=%d Tname=%s\\n",' + ttype + ', "' + tname + '");\n'
                fo.write(t)
            fo.write('}\n\n')

        # ----- Write the EDTs
        for n in in_G.nodes():
            #print('DBG2265> ====== node = ' + str(n))
            if not getNode(in_G, n)["istheELSEclause"]:
                erri = find_destroy_outgoing_release2(in_G, n)
                if erri: break
            if not len(getIFconditionText(in_G, n)) == 0:  # We are on the IF-THEN edt.
                # We have to also process the ELSE edt here before it enters outputOCR_writeEDT(...)
                elseEDT = GBL.NOTA_NODE_ID
                bors = in_G.neighbors(n)
                for b in bors:
                    edg = (n,b)
                    if getEdge(in_G,edg)["leads_to_ElseClause"]:
                        elseEDT = b
                        break
                if elseEDT == GBL.NOTA_NODE_ID:
                    erri = inspect.currentframe().f_lineno
                    break
                erri = find_destroy_outgoing_release2(in_G, elseEDT)
                if erri: break

            if getNode(in_G, n)["istheELSEclause"]:  # Skip any ELSEclause nodes as they will be handled by their
                                                     # corresponding IFconditionText & THEN node.
                continue
            # ----- start of the task
            name = getMyTask(in_G, n).aname
            fo.write("ocrGuid_t " + name + "(EDT_ARGS)\n")
            fo.write("{\n")

            t = tab + 'typedef enum ocr_constants {\n'
            t += tab2 + 'OA_edtTypeNb = ' + str(n) + '\n'
            t += tab + '} ocr_constants_t;\n'
            fo.write(t)

            if n == 0 and GBL.insert_debug_code:
                fo.write('\n' + tab + 'OA_DBG_task_type_name();\n\n')

            t = tab + 'ocrGuid_t OA_DBG_thisEDT = NULL_GUID;\n'
            fo.write(t)
            if GBL.insert_debug_code:
                t = tab + 'currentEdtUserGet(&OA_DBG_thisEDT);\n'
                fo.write(t)
                t = tab + 'PRINTF("OA_DBG> TaskTYPE=%d TaskID="GUIDF" TaskName=%s\\n", OA_edtTypeNb, GUIDA(OA_DBG_thisEDT), "'
                t += getMyTask(in_G, n).aname + '");\n'
                fo.write(t)
            fo.write("\n")

            fo.write(tab + "Err_t err=0;\n")
            fo.write(tab + "while(!err){\n")

            erri = outputOCR_writeEDT(in_G, n, tab2, fo)
            if erri: break

            # -----
            fo.write(tab2+ "break; //while(!err)\n")
            fo.write(tab + "}\n")

            fo.write(tab + "EDT_ERROR(err);\n")
            fo.write(tab + "return NULL_GUID;\n")

            fo.write("}\n")
        if erri:
            fo.close()
            break

        fo.close()

        break  # while not erri
    return erri
