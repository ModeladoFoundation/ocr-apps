import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():
    OA.GBL.insert_debug_code = 0
    G=OA.GraphTool.DiGraph()

    OA.setupGraph(G,"ptest_FORforkjoin")
    OA.GBL.app_ocr_util_filename = 'app_ocr_util.h'

    OA.addHeader(G, '#include "ptest_FORforkjoin.h"')

    # ----- Data Blocks
    dbk = OA.ocrDataBlock(); dbk.name = 'NULL_GUID'; dbk_nullGuid = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'gDone';     dbk.count = 1; dbk.type='ocrGuid_t';    dbk_gDone = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'gDoneROF';  dbk.count = 1; dbk.type='ocrGuid_t';    dbk_gDoneROF = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'ffjLedger'; dbk.count = 1; dbk.type='FFJ_Ledger_t'; dbk_ffjLedger = dbk

    # ----- NODES
    nc = OA.GBL.MAINNODE; taskName = "mainEdt"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_gDone);    dbk.flight = 'flTAKEOFF'; dbk.localname='o_ffjGDone'; OA.addDataBlocks(G,nc,dbk)
    guid_finalEDT = OA.makeGuidEdtname('finalEDT')
    mainedtTxt = 'err = ffjMainEdt_fcn(&' + guid_finalEDT + ', o_ffjGDone); IFEB;'
    OA.addCustomText(G, nc, mainedtTxt)

    nc = OA.GBL.FINALNODE; taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    finalEDT_txt = 'err = ffjFinalEdt_fcn(); IFEB;'
    OA.addCustomText(G, nc, finalEDT_txt)

    nc += 1; taskName = "ffjSetup"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_ffjLedger); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ffjLedger';OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_gDoneROF);  dbk.flight = 'flTAKEOFF'; dbk.localname='o_gDoneROF';
    OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_gDone);     dbk.flight = 'flTAGO';    OA.addDataBlocks(G,nc,dbk)
    guid_ffjROF = OA.makeGuidEdtname('ffjROF')
    ffjSetup_txt = 'err = ffjSetup_fcn(COUNT_FOR_FORKJOIN, o_ffjLedger, &'+ guid_ffjROF + ', o_gDoneROF); IFEB;'
    OA.addCustomText(G, nc, ffjSetup_txt)

    nc += 1;  taskName="ffjFOR"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_ffjLedger); dbk.flight = 'flLANDING'; dbk.localname='in_ffjLedger'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_gDoneROF);  dbk.flight = 'flLANDING'; dbk.localname='in_gDoneROF'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_ffjLedger); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ffjshared';
    dbk.hint = 'pHintDBK'
    OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_gDoneROF);  dbk.flight = 'flTAKEOFF'; dbk.localname='o_gDoneROF';
    dbk.hint = 'pHintDBK'
    OA.addDataBlocks(G,nc,dbk)
    fortext = 'int rank; for(rank=0; rank<in_ffjLedger->nrank; ++rank)'
    OA.addFORconditionText(G, nc, fortext)
    OA.addFORconditionText_startingclause(G,nc,'unsigned long pdID = calculate_pid(rank, in_ffjLedger->OCR_affinityCount, in_ffjLedger->nrank);')
    OA.addFORconditionText_startingclause(G,nc,'ocrHint_t hintEDT, *pHintEDT=0, hintDBK, *pHintDBK=0;')
    OA.addFORconditionText_startingclause(G,nc,'err = ocrXgetEdtHint(pdID, &hintEDT, &pHintEDT); IFEB;')
    OA.addFORconditionText_startingclause(G,nc,'err = ocrXgetDbkHint(pdID, &hintDBK, &pHintDBK); IFEB;')
    ffjFOR_txt = 'err = ffjFOR_fcn(rank, in_ffjLedger, o_ffjshared, in_gDoneROF, o_gDoneROF); IFEB;'
    OA.addCustomText(G, nc, ffjFOR_txt)

    nc += 1;  taskName="ffjFOR_Transist_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_ffjLedger); dbk.flight = 'flTAGO'; dbk.localname='io_ffjLedger'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_gDoneROF);  dbk.flight = 'flTAGO';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(FFJ_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G,nc,dbk)
    ffjFOR_Transist_start = 'err = ffjFOR_Transist_start_fcn(io_ffjLedger); IFEB;'
    OA.addCustomText(G, nc, ffjFOR_Transist_start)

    nc += 1;  taskName="ffjFOR_Transist_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_ffjLedger); dbk.flight = 'flLANDING'; dbk.localname='in_ffjLedger'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_gDoneROF);  dbk.flight = 'flLANDING'; dbk.localname='in_gDoneROF';  OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nullGuid);  dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G,nc,dbk)
    gdoneROF_txt = 'ocrGuid_t ' + OA.makeGuidEdtname('ffjROF') + ' = *in_gDoneROF;'
    OA.addCustomText(G, nc, gdoneROF_txt)
    ROF_which_slot = 'unsigned int slot_for_ROF = in_ffjLedger->rankid +1; //+1 to accommodate the in_gDone sent from ffjSetup'
    OA.addCustomText(G, nc, ROF_which_slot)
    ffjFOR_Transist_stop = 'err = ffjFOR_Transist_stop_fcn(in_ffjLedger); IFEB;'
    OA.addCustomText(G, nc, ffjFOR_Transist_stop)

    nc += 1;  taskName="ffjROF"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).depc = '(1+COUNT_FOR_FORKJOIN)'
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flLANDING'; dbk.localname='in_gDone'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nullGuid);  dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G,nc,dbk)
    gdone_txt = 'ocrGuid_t ' + OA.makeGuidEdtname('finalEDT') + ' = *in_gDone;'
    OA.addCustomText(G, nc, gdone_txt)
    ffjROF = 'err = ffjROF_fcn(); IFEB;'
    OA.addCustomText(G, nc, ffjROF)

    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NULL_GUID')
    ledg = OA.graphAddEdge(G, "mainEdt", "ffjSetup", "gDone")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')

    ledg = OA.graphAddEdge(G, "ffjSetup", "ffjROF", "gDone")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "ffjSetup", "ffjFOR", "gDoneROF")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDoneROF')
    ledg = OA.graphAddEdge(G, "ffjSetup", "ffjFOR", "ffjLedger")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('ffjLedger')

    ledg = OA.graphAddEdge(G, "ffjFOR", "ffjFOR_Transist_start", "gDoneROF")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDoneROF')
    ledg = OA.graphAddEdge(G, "ffjFOR", "ffjFOR_Transist_start", "ffjLedger")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('ffjLedger')

    ledg = OA.graphAddEdge(G, "ffjFOR_Transist_start", "ffjFOR_Transist_stop", "gDoneROF")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDoneROF')
    ledg = OA.graphAddEdge(G, "ffjFOR_Transist_start", "ffjFOR_Transist_stop", "ffjLedger")
    OA.getEvent(G, ledg).accessmode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('ffjLedger')

    ledg = OA.graphAddEdge(G, "ffjFOR_Transist_stop", "ffjROF", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
    OA.getEvent(G, ledg).fertile = False
    OA.getEvent(G, ledg).user_slotnb_text = 'slot_for_ROF'

    ledg = OA.graphAddEdge(G, "ffjROF", "finalEDT", "NULL_GUID")
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
            erri = OA.outputDot(G, "z_ptest_FORforkjoin.dot")
            if erri: break
        if True:
            erri = OA.outputOCR(G, "z_ptest_FORforkjoin.c")
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

