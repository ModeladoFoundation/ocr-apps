import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    OA.GBL.insert_debug_code = 0

    G=OA.GraphTool.DiGraph()

    OA.setupGraph(G,"utest_labelEdt")

    OA.addHeader(G, ' ')  # Just a blank line
    OA.addHeader(G, '#define ENABLE_EXTENSION_LABELING  // For labeled EDTs')
    OA.addHeader(G, '#include "extensions/ocr-labeling.h"  // For labeled EDTs')
    OA.addHeader(G, ' ')  # Just a blank line

    OA.addHeader(G, '#define N 5')
    N = 5

    OA.addHeader(G, 'typedef struct Work { int x[N]; ocrGuid_t emap;} Work_t;')

    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'
    dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'WorkLeft';   dbk.count = 1; dbk.type='Work_t'
    dbk_workLeft = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'WorkRight';   dbk.count = 1; dbk.type='Work_t'
    dbk_workRight = dbk

    dbk=OA.ocrDataBlock();  dbk.name = 'edtMap';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_edtMap = dbk

    dbk=OA.ocrDataBlock();  dbk.name = 'wDoneLeft';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_wdoneLeft = dbk
    dbk=OA.ocrDataBlock();  dbk.name = 'wDoneRight';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_wdoneRight = dbk

    # ----- NODES
    nc = OA.GBL.MAINNODE;    taskName = "mainEdt"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_edtMap); dbk.flight = 'flTAKEOFF'; dbk.localname='o_emap'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_workLeft); dbk.flight = 'flTAKEOFF'; dbk.localname='o_wleft'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_workRight); dbk.flight = 'flTAKEOFF'; dbk.localname='o_wright'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wdoneLeft); dbk.flight = 'flTAKEOFF'; dbk.localname='o_doneLeft'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wdoneRight); dbk.flight = 'flTAKEOFF'; dbk.localname='o_doneRight'; OA.addDataBlocks(G,nc,dbk)
    toFinalLeft = 'GUID_ASSIGN_VALUE(*o_doneLeft, ' + OA.makeGuidEdtname("finalEDT") + ');'
    OA.addCustomText(G, nc, toFinalLeft)
    toFinalRight = 'GUID_ASSIGN_VALUE(*o_doneRight, ' + OA.makeGuidEdtname("finalEDT") + ');'
    OA.addCustomText(G, nc, toFinalRight)
    emapText = 'ocrGuid_t emap = NULL_GUID;'; OA.addCustomText(G, nc, emapText)
    emapText = 'err = ocrGuidRangeCreate(&emap, 3, GUID_USER_EDT); IFEB; // 3 for fun'; OA.addCustomText(G, nc, emapText)
    emapText = 'GUID_ASSIGN_VALUE(*o_emap, emap);'; OA.addCustomText(G, nc, emapText)

    wleft_text = 'int i;'; OA.addCustomText(G, nc, wleft_text)
    wleft_text = 'for(i=0;i<N;++i) o_wleft->x[i]=i+1;'; OA.addCustomText(G, nc, wleft_text)
    wleft_text = 'GUID_ASSIGN_VALUE(o_wleft->emap, emap);'; OA.addCustomText(G, nc, wleft_text)
    wright_text = 'for(i=0;i<N;++i) o_wright->x[i]=N+1+i;'; OA.addCustomText(G, nc, wright_text)
    wright_text = 'GUID_ASSIGN_VALUE(o_wright->emap, emap);'; OA.addCustomText(G, nc, wright_text)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_edtMap); dbk.flight = 'flLANDING'; dbk.localname='in_edtMap'; OA.addDataBlocks(G,nc,dbk)
    destroy_emap = 'err = ocrGuidMapDestroy(*in_edtMap); IFEB;'; OA.addCustomText(G, nc, destroy_emap)

    nc += 1;  taskName="LeftEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_workLeft); dbk.flight = 'flTAGO';  dbk.localname='io_workL'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wdoneLeft); dbk.flight = 'flTAGO'; dbk.localname='io_doneL'; OA.addDataBlocks(G,nc,dbk)
    nextedt = 'ocrGuid_t nextEDT = NULL_GUID;'; OA.addCustomText(G, nc, nextedt)
    nextedt = 'unsigned long index = 1;'; OA.addCustomText(G, nc, nextedt)
    nextedt = 'err = ocrXIndexedEdtCreate(SumEDT, 0, NULL, 4, EDT_PROP_NONE, NULL_HINT, index, & io_workL->emap, &nextEDT, NULL); IFEB;'
    OA.addCustomText(G, nc, nextedt)
    guid_sumEDT = OA.makeGuidEdtname("SumEDT")
    guid_sumEDT_text = 'GUID_ASSIGN_VALUE(' + guid_sumEDT + ', nextEDT);'; OA.addCustomText(G, nc, guid_sumEDT_text)

    nc += 1;  taskName="RightEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_workRight); dbk.flight = 'flTAGO';  dbk.localname='io_workR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wdoneRight); dbk.flight = 'flTAGO'; dbk.localname='io_doneR'; OA.addDataBlocks(G,nc,dbk)
    nextedt = 'ocrGuid_t nextEDT = NULL_GUID;'; OA.addCustomText(G, nc, nextedt)
    nextedt = 'unsigned long index = 1;'; OA.addCustomText(G, nc, nextedt)
    nextedt = 'err = ocrXIndexedEdtCreate(SumEDT, 0, NULL, 4, EDT_PROP_NONE, NULL_HINT, index, & io_workR->emap, &nextEDT, NULL); IFEB;'
    OA.addCustomText(G, nc, nextedt)
    guid_sumEDT = OA.makeGuidEdtname("SumEDT")
    guid_sumEDT_text = 'GUID_ASSIGN_VALUE(' + guid_sumEDT + ', nextEDT);'; OA.addCustomText(G, nc, guid_sumEDT_text)

    nc += 1;  taskName="SumEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_wdoneLeft); dbk.flight = 'flLANDING'; dbk.localname='in_doneL'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wdoneRight); dbk.flight = 'flLANDING'; dbk.localname='in_doneR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_workLeft); dbk.flight = 'flLANDING'; dbk.localname='in_wleft'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_workRight); dbk.flight = 'flLANDING'; dbk.localname='in_wright'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G,nc,dbk)

    sumText = 'int i, sum=0;'; OA.addCustomText(G, nc, sumText)
    sumText = 'for(i=0; i<N; ++i) sum += in_wleft->x[i];'; OA.addCustomText(G, nc, sumText)
    sumText = 'for(i=0; i<N; ++i) sum += in_wright->x[i];'; OA.addCustomText(G, nc, sumText)
    sumText = 'PRINTF("TESTIO> sum = %d\\n", sum);'; OA.addCustomText(G, nc, sumText)

    guid_finalEDT = OA.makeGuidEdtname('finalEDT')
    doneText = 'ocrGuid_t ' + guid_finalEDT + '; GUID_ASSIGN_VALUE(' + guid_finalEDT + ', *in_doneL);'
    OA.addCustomText(G, nc, doneText)

    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "edtMap")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_EW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('edtMap')

    ledg = OA.graphAddEdge(G, "mainEdt", "LeftEDT", "WorkLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('WorkLeft')
    ledg = OA.graphAddEdge(G, "mainEdt", "LeftEDT", "wDoneLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('wDoneLeft')

    ledg = OA.graphAddEdge(G, "mainEdt", "RightEDT", "WorkRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('WorkRight')
    ledg = OA.graphAddEdge(G, "mainEdt", "RightEDT", "wDoneRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('wDoneRight')

    ledg = OA.graphAddEdge(G, "RightEDT", "SumEDT", "WorkRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('WorkRight')
    ledg = OA.graphAddEdge(G, "RightEDT", "SumEDT", "wDoneRight")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('wDoneRight')

    ledg = OA.graphAddEdge(G, "LeftEDT", "SumEDT", "WorkLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('WorkLeft')
    ledg = OA.graphAddEdge(G, "LeftEDT", "SumEDT", "wDoneLeft")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('wDoneLeft')


    ledg=OA.graphAddEdge(G,  "SumEDT", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
    OA.getEvent(G, ledg).fertile = False

    # ----- WORK
    erri = 0
    while not erri:
        if 1 == 2: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if 1 == 2: OA.printGraph(G)
        if 1 == 1:
            erri = OA.outputDot(G, "z_utest_labeledt.dot")
            if erri: break
        if 1 == 1:
            erri = OA.outputOCR(G, "z_utest_labeledt.c")
            if erri: break

        if 1 == 2: OA.printGraph(G)

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

