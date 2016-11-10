import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    SLOT4REDUCTION = 4

    OA.GBL.insert_debug_code = 0

    G=OA.GraphTool.DiGraph()

    OA.setupGraph(G,"utest_DaveReduction")
    OA.GBL.app_ocr_util_filename = 'app_ocr_util.h'

    OA.addHeader(G, ' ')  # Just a blank line
    OA.addHeader(G, '#define ENABLE_EXTENSION_LABELING')
    OA.addHeader(G, '#include "extensions/ocr-labeling.h"')
    OA.addHeader(G, ' ')  # Just a blank line
    OA.addHeader(G, '#include "utest_reduction.h"')
    OA.addHeader(G, '#include "reduction.h"')
    OA.addHeader(G, ' ')  # Just a blank line
    OA.addHeader(G, '#define NRANK 2')
    OA.addHeader(G, '#define SzVEC 3')
    OA.addHeader(G, ' ')  # Just a blank line

    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'; dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'workLeft';  dbk.count = 'SzVEC*sizeof(ReducValue_t)'; dbk.type='ReducValue_t'; dbk_workLeft = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'workRight'; dbk.count = 'SzVEC*sizeof(ReducValue_t)'; dbk.type='ReducValue_t'; dbk_workRight = dbk

    dbk=OA.ocrDataBlock(); dbk.name = 'doneLeft';  dbk.count=1; dbk.type='ocrGuid_t'; dbk_doneLeft = dbk
    dbk=OA.ocrDataBlock(); dbk.name = 'doneRight'; dbk.count=1; dbk.type='ocrGuid_t'; dbk_doneRight = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'sum';  dbk.count = 1; dbk.type='ReducSum_t'; dbk_sum = dbk

    dbk=OA.ocrDataBlock(); dbk.name = 'sharedG'; dbk.count=1; dbk.type='DReduct_shared_t'; dbk_sharedG = dbk
    dbk=OA.ocrDataBlock(); dbk.name = 'sharedR'; dbk.count=1; dbk.type='DReduct_shared_t'; dbk_sharedR = dbk
    dbk=OA.ocrDataBlock(); dbk.name = 'sharedL'; dbk.count=1; dbk.type='DReduct_shared_t'; dbk_sharedL = dbk

    dbk=OA.ocrDataBlock(); dbk.name = 'ReducPrivate'; dbk.count=1; dbk.type='reductionPrivate_t'; dbk_ReducPrivate = dbk

    # ----- NODES
    nc = OA.GBL.MAINNODE;    taskName = "mainEdt"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_workLeft); dbk.flight = 'flTAKEOFF'; dbk.localname='o_workLeft'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_workRight); dbk.flight = 'flTAKEOFF'; dbk.localname='o_workRight'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneLeft); dbk.flight = 'flTAKEOFF'; dbk.localname='o_doneLeft'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneRight); dbk.flight = 'flTAKEOFF'; dbk.localname='o_doneRight'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_sharedG); dbk.flight = 'flTAKEOFF'; dbk.localname='o_sharedG'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_sharedL); dbk.flight = 'flTAKEOFF'; dbk.localname='o_sharedL'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_sharedR); dbk.flight = 'flTAKEOFF'; dbk.localname='o_sharedR'; OA.addDataBlocks(G,nc,dbk)
    toFinalLeft = 'GUID_ASSIGN_VALUE(*o_doneLeft, ' + OA.makeGuidEdtname("finalEDT") + ');';   OA.addCustomText(G, nc, toFinalLeft)
    toFinalRight = 'GUID_ASSIGN_VALUE(*o_doneRight, ' + OA.makeGuidEdtname("finalEDT") + ');'; OA.addCustomText(G, nc, toFinalRight)
    mainedtTxt = 'err = utest_mainedt(NRANK, 1, SzVEC, o_workLeft,o_workRight, o_sharedG,o_sharedL,o_sharedR); IFEB;'
    OA.addCustomText(G, nc, mainedtTxt)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_sharedG); dbk.flight = 'flLANDING'; dbk.localname='in_shared'; OA.addDataBlocks(G,nc,dbk)
    finalEDTtxt = 'err = destroy_DReduct_shared(in_shared); IFEB;'
    OA.addCustomText(G, nc, finalEDTtxt)

    nc += 1;  taskName="LeftEDT_init"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_sharedL);  dbk.flight = 'flLANDING'; dbk.localname='io_sharedL';  OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_workLeft); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneLeft); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_ReducPrivate); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ReducPrivate'; OA.addDataBlocks(G,nc,dbk)
    rankTxt = 'unsigned long rank = 0;'; OA.addCustomText(G, nc, rankTxt)
    leftEDT_init = 'err = reduction_init(rank, io_sharedL, o_ReducPrivate); IFEB;'
    OA.addCustomText(G, nc, leftEDT_init)

    nc += 1;  taskName="RightEDT_init"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_sharedR);  dbk.flight = 'flLANDING'; dbk.localname='io_sharedR';  OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_workRight); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneRight); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_ReducPrivate); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ReducPrivate'; OA.addDataBlocks(G,nc,dbk)
    rankTxt = 'unsigned long rank = 1;'; OA.addCustomText(G, nc, rankTxt)
    rightEDT_init = 'err = reduction_init(rank, io_sharedR, o_ReducPrivate); IFEB;'
    OA.addCustomText(G, nc, rightEDT_init)

    nc += 1;  taskName="LeftEDT_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_workLeft); dbk.flight = 'flTAGO'; dbk.localname='io_workLeft'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneLeft); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_ReducPrivate); dbk.flight = 'flTAGO'; dbk.user2destroyORrelease = True
    dbk.localname='io_ReducPrivate'; dbk.localnameGuid='guid_ReducPrivate_left';
    dbk.addLocalText('//It is expected that the reduction library will release the ReducPrivate data block.')
    dbk.addLocalText('//-->????Should the ReducPrivate data block be sent to the ...stop EDT? What if we want to use it again?')
    OA.addDataBlocks(G,nc,dbk)
    guid_LeftEDT_stop = OA.makeGuidEdtname('LeftEDT_stop')
    doLStart = 'err = compute_start(guid_ReducPrivate_left, io_ReducPrivate, SzVEC, io_workLeft, SLOT4REDUCTION-1,' + guid_LeftEDT_stop + '); IFEB;'
    OA.addCustomText(G, nc, doLStart)

    nc += 1;  taskName="RightEDT_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_workRight); dbk.flight = 'flTAGO'; dbk.localname='io_workRight'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneRight); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_ReducPrivate); dbk.flight = 'flTAGO'; dbk.user2destroyORrelease = True
    dbk.localname='io_ReducPrivate'; dbk.localnameGuid='guid_ReducPrivate_right';
    dbk.addLocalText('//It is expected that the reduction library will release the ReducPrivate data block.')
    dbk.addLocalText('//-->????Should the ReducPrivate data block be sent to the ...stop EDT? What if we want to use it again?')
    OA.addDataBlocks(G,nc,dbk)
    guid_RightEDT_stop = OA.makeGuidEdtname('RightEDT_stop')
    doRStart = 'err = compute_start(guid_ReducPrivate_right, io_ReducPrivate, SzVEC, io_workRight, SLOT4REDUCTION-1, ' + guid_RightEDT_stop + '); IFEB;'
    OA.addCustomText(G, nc, doRStart)

    nc += 1;  taskName="LeftEDT_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).depc = SLOT4REDUCTION
    dbk = copy.deepcopy(dbk_workLeft); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneLeft); dbk.flight = 'flLANDING'; dbk.localname='in_doneLeft'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_ReducPrivate); dbk.flight = 'flLANDING'; dbk.localname='in_ReducPrivate';
    dbk.addLocalText('ocrEdtDep_t IN_BY_USER_depv_sum = depv[SLOT4REDUCTION-1];  //This will be LANDING here.')
    dbk.addLocalText('ocrGuid_t in_sum_guid = IN_BY_USER_depv_sum.guid;')
    dbk.addLocalText('ReducSum_t * in_sum = IN_BY_USER_depv_sum.ptr;')
    OA.addDataBlocks(G,nc,dbk)
    guid_finalEDT = OA.makeGuidEdtname('finalEDT')
    doneText = 'ocrGuid_t ' + guid_finalEDT + '; GUID_ASSIGN_VALUE(' + guid_finalEDT + ', *in_doneLeft);'
    OA.addCustomText(G, nc, doneText)
    resultTxt = 'err = leftEDT_stop(in_ReducPrivate, in_sum, in_sum_guid); IFEB;';
    OA.addCustomText(G, nc, resultTxt)

    nc += 1;  taskName="RightEDT_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).depc = SLOT4REDUCTION
    dbk = copy.deepcopy(dbk_workRight); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_doneRight); dbk.flight = 'flLANDING'; dbk.localname='in_doneRight'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_ReducPrivate); dbk.flight = 'flLANDING'; dbk.localname='in_ReducPrivate';
    dbk.addLocalText('ocrEdtDep_t IN_BY_USER_depv_sum = depv[SLOT4REDUCTION-1];  //This will be LANDING here.')
    dbk.addLocalText('ocrGuid_t in_sum_guid = IN_BY_USER_depv_sum.guid;')
    dbk.addLocalText('ReducSum_t * in_sum = IN_BY_USER_depv_sum.ptr;')
    OA.addDataBlocks(G,nc,dbk)
    guid_finalEDT = OA.makeGuidEdtname('finalEDT')
    doneText = 'ocrGuid_t ' + guid_finalEDT + '; GUID_ASSIGN_VALUE(' + guid_finalEDT + ', *in_doneRight);'
    OA.addCustomText(G, nc, doneText)
    resultTxt = 'err = rightEDT_stop(in_ReducPrivate, in_sum, in_sum_guid); IFEB;';
    OA.addCustomText(G, nc, resultTxt)

    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "sharedG")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('sharedG')

    ledg = OA.graphAddEdge(G, "mainEdt", "LeftEDT_init", "workLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('workLeft')
    ledg = OA.graphAddEdge(G, "mainEdt", "LeftEDT_init", "doneLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('doneLeft')
    ledg = OA.graphAddEdge(G, "mainEdt", "LeftEDT_init", "sharedL")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('sharedL')

    ledg = OA.graphAddEdge(G, "mainEdt", "RightEDT_init", "workRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('workRight')
    ledg = OA.graphAddEdge(G, "mainEdt", "RightEDT_init", "doneRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('doneRight')
    ledg = OA.graphAddEdge(G, "mainEdt", "RightEDT_init", "sharedR")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('sharedR')

    ledg = OA.graphAddEdge(G, "LeftEDT_init", "LeftEDT_start", "workLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('workLeft')
    ledg = OA.graphAddEdge(G, "LeftEDT_init", "LeftEDT_start", "doneLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('doneLeft')
    ledg = OA.graphAddEdge(G, "LeftEDT_init", "LeftEDT_start", "ReducPrivate")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('ReducPrivate')

    ledg = OA.graphAddEdge(G, "RightEDT_init", "RightEDT_start", "workRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('workRight')
    ledg = OA.graphAddEdge(G, "RightEDT_init", "RightEDT_start", "doneRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('doneRight')
    ledg = OA.graphAddEdge(G, "RightEDT_init", "RightEDT_start", "ReducPrivate")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('ReducPrivate')

    ledg = OA.graphAddEdge(G, "LeftEDT_start", "LeftEDT_stop", "workLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('workLeft')
    ledg = OA.graphAddEdge(G, "LeftEDT_start", "LeftEDT_stop", "doneLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('doneLeft')
    ledg = OA.graphAddEdge(G, "LeftEDT_start", "LeftEDT_stop", "ReducPrivate")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('ReducPrivate')

    ledg = OA.graphAddEdge(G, "RightEDT_start", "RightEDT_stop", "workRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('workRight')
    ledg = OA.graphAddEdge(G, "RightEDT_start", "RightEDT_stop", "doneRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('doneRight')
    ledg = OA.graphAddEdge(G, "RightEDT_start", "RightEDT_stop", "ReducPrivate")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('ReducPrivate')

    ledg=OA.graphAddEdge(G,  "LeftEDT_stop", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
    OA.getEvent(G, ledg).fertile = False
    ledg=OA.graphAddEdge(G,  "RightEDT_stop","finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
    OA.getEvent(G, ledg).fertile = False

    # ----- WORK
    erri = 0
    while not erri:
        if True: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if False: OA.printGraph(G)
        if True:
            erri = OA.outputDot(G, "z_utest_DaveReduction.dot")
            if erri: break
        if True:
            erri = OA.outputOCR(G, "z_utest_DaveReduction.c")
            if erri: break

        if False: OA.printGraph(G)

        break  # while not erri
    OA.errmsg(erri)
    return erri
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
if __name__ == "__main__":
    errm = theMain()
    if errm:
        exit(1)
    else:
        exit(0)

