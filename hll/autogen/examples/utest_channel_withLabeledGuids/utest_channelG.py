import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    OA.GBL.insert_debug_code = 0

    G=OA.GraphTool.DiGraph()

    OA.setupGraph(G,"utest_channel")

    OA.addHeader(G, '#include "utest_channelG.h" ')
    OA.addHeader(G, ' ')

    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'rightLedger'; dbk.count=1; dbk.type='utcLedger_t'
    dbk_rightLedger = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'leftLedger';  dbk.count=1; dbk.type='utcLedger_t'
    dbk_leftLedger = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'refLedger';  dbk.count=1; dbk.type='utcLedger_t'
    dbk_refLedger = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'channelEnvelop'; dbk.count=1; dbk.type='ocrGuid_t'
    dbk_channelEnvelop = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'labelEventEnvelop'; dbk.count=1; dbk.type='ocrGuid_t'
    dbk_labelEventEnvelop = dbk

#    dbk = OA.ocrDataBlock();  dbk.name = 'neighborChannel';  dbk.count=1; dbk.type='ocrGuid_t'
#    dbk_neighborChannel = dbk

    # ----- NODES
    # Always root the starting/root/main EDT at ID=0 = OA.GBL.MAINNODE
    # Always cap all ending EDT by the FinalEDT at ID=1 = OA.GBL.FINALNODE
    nc = OA.GBL.MAINNODE;    taskName = "mainEdt"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flTAKEOFF'; dbk.localname='o_leftLedger';  OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_rightLedger);dbk.flight = 'flTAKEOFF'; dbk.localname='o_rightLedger';  OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_refLedger);  dbk.flight = 'flTAKEOFF'; dbk.localname='o_refLedger';  OA.addDataBlocks(G,nc,dbk)
    Ma = 'err = Userfcn_MainEdt( OA_DEBUG_OUTVARS, &' + OA.makeGuidEdtname("finalEDT") + ', '
    Ma += 'o_leftLedger,o_rightLedger,o_refLedger); IFEB;'
    OA.addCustomText(G, nc, Ma)

    nc = OA.GBL.FINALNODE; taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_refLedger); dbk.flight = 'flLANDING'; dbk.localname='in_refLedger';  OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flLANDING'; dbk.localname='in_leftLedger';  OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_rightLedger); dbk.flight = 'flLANDING'; dbk.localname='in_rightLedger';  OA.addDataBlocks(G,nc,dbk)
    finalEDTa = 'err = Userfcn_FinalEdt(OA_DEBUG_OUTVARS, in_refLedger, in_leftLedger, in_rightLedger); IFEB;'
    OA.addCustomText(G, nc, finalEDTa)

    nc += 1;  taskName="BeginInit_Left"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flLANDING'; dbk.localname='in_ledger'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ledger'; OA.addDataBlocks(G,nc,dbk)
    EndInit_Left_guid = OA.makeGuidEdtname('EndInit_Left')
    BILc  = 'err = Userfcn_BeginInit(OA_DEBUG_OUTVARS, in_ledger, o_ledger, &'
    BILc += EndInit_Left_guid + ', SLOTCNT_EndInit-1 ); IFEB;'
    OA.addCustomText(G, nc, BILc)

    nc += 1;  taskName="BeginInit_Right"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_rightLedger); dbk.flight = 'flLANDING'; dbk.localname='in_ledger'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_rightLedger); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ledger'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_channelEnvelop); dbk.flight = 'flTAKEOFF'; dbk.localname='o_chanEnvelop'; OA.addDataBlocks(G,nc,dbk)
    EndInit_Right_guid = OA.makeGuidEdtname('EndInit_Right')
    BIRc = 'err = Userfcn_BeginInit(OA_DEBUG_OUTVARS, in_ledger, o_ledger, &'
    BIRc += EndInit_Right_guid + ', SLOTCNT_EndInit-1); IFEB;'
    OA.addCustomText(G, nc, BIRc)

    nc += 1;  taskName="EndInit_Left"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).depc = 'SLOTCNT_EndInit'
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flLANDING'; dbk.localname='in_ledger';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_neighborLabelEvt = depv[SLOTCNT_EndInit-1];  //This will be LANDING here.')
    OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ledger';  OA.addDataBlocks(G,nc,dbk)
    EndInit_Lefta = ' err = Userfcn_EndInit(OA_DEBUG_OUTVARS, in_ledger, o_ledger, IN_BY_USER_depv_neighborLabelEvt); IFEB;'
    OA.addCustomText(G, nc, EndInit_Lefta)

    nc += 1;  taskName="EndInit_Right"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).depc = 'SLOTCNT_EndInit'
    dbk = copy.deepcopy(dbk_rightLedger); dbk.flight = 'flLANDING'; dbk.localname='in_ledger';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_neighborLabelEvt = depv[SLOTCNT_EndInit-1];  //This will be LANDING here.')
    OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_rightLedger); dbk.flight = 'flTAKEOFF'; dbk.localname='o_ledger'; OA.addDataBlocks(G,nc,dbk)
    EndInit_Righta = ' err = Userfcn_EndInit(OA_DEBUG_OUTVARS, in_ledger, o_ledger, IN_BY_USER_depv_neighborLabelEvt); IFEB;'
    OA.addCustomText(G, nc, EndInit_Righta)

    nc += 1;  taskName="Work_Left_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flTAGO'; dbk.localname='io_ledger'; OA.addDataBlocks(G,nc,dbk)
    Work_Left_stop_guid = OA.makeGuidEdtname('Work_Left_stop')
    Work_Left_startA = 'err = work_start(OA_DEBUG_OUTVARS, io_ledger, &' + Work_Left_stop_guid +', SLOTCNT_WorkStop-1); IFEB;'
    OA.addCustomText(G, nc, Work_Left_startA)

    nc += 1;  taskName="Work_Right_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_rightLedger); dbk.flight = 'flTAGO'; dbk.localname='io_ledger'; OA.addDataBlocks(G,nc,dbk)
    Work_Right_stop_guid = OA.makeGuidEdtname('Work_Right_stop')
    Work_Right_startA = 'err = work_start(OA_DEBUG_OUTVARS, io_ledger, &' + Work_Right_stop_guid +', SLOTCNT_WorkStop-1); IFEB;'
    OA.addCustomText(G, nc, Work_Right_startA)

    nc += 1;  taskName="Work_Left_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).depc = 'SLOTCNT_WorkStop'
    dbk = copy.deepcopy(dbk_leftLedger); dbk.flight = 'flTAGO'; dbk.localname='io_ledger';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_neighborChanEvt = depv[SLOTCNT_WorkStop-1];  //This will be LANDING here.')
    OA.addDataBlocks(G,nc,dbk)
    finalEdt_guid = OA.makeGuidEdtname('finalEDT')
    guid_text = 'ocrGuid_t ' + finalEdt_guid + ';';  OA.addCustomText(G, nc, guid_text)
    Work_Left_stopA = 'err = work_stop(OA_DEBUG_OUTVARS, io_ledger, &' + finalEdt_guid + ', IN_BY_USER_depv_neighborChanEvt); IFEB;'
    OA.addCustomText(G, nc, Work_Left_stopA)

    nc += 1;  taskName="Work_Right_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).depc = 'SLOTCNT_WorkStop'
    dbk = copy.deepcopy(dbk_rightLedger); dbk.flight = 'flTAGO'; dbk.localname='io_ledger';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_neighborChanEvt = depv[SLOTCNT_WorkStop-1];  //This will be LANDING here.')
    OA.addDataBlocks(G,nc,dbk)
    finalEdt_guid = OA.makeGuidEdtname('finalEDT')
    guid_text = 'ocrGuid_t ' + finalEdt_guid + ';';  OA.addCustomText(G, nc, guid_text)
    Work_Right_stopA = 'err = work_stop(OA_DEBUG_OUTVARS, io_ledger, &' + finalEdt_guid + ', IN_BY_USER_depv_neighborChanEvt); IFEB;'
    OA.addCustomText(G, nc, Work_Right_stopA)

    # ----- EDGES
    ledg = OA.graphAddEdge(G,"mainEdt", "finalEDT", 'refLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('refLedger')

    ledg=OA.graphAddEdge(G, "mainEdt", "BeginInit_Left", 'leftLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('leftLedger')

    ledg=OA.graphAddEdge(G, "mainEdt", "BeginInit_Right", 'rightLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('rightLedger')

    ledg=OA.graphAddEdge(G, "BeginInit_Left", "EndInit_Left", 'leftLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('leftLedger')
#    OA.getEvent(G, ledg).user_slotnb_text = '0'

    ledg=OA.graphAddEdge(G, "BeginInit_Right", "EndInit_Right", 'rightLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('rightLedger')
#    OA.getEvent(G, ledg).user_slotnb_text = '0'

    ledg=OA.graphAddEdge(G, "EndInit_Left", "Work_Left_start", 'leftLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('leftLedger')

    ledg=OA.graphAddEdge(G, "EndInit_Right", "Work_Right_start", 'rightLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('rightLedger')

    ledg=OA.graphAddEdge(G, "Work_Left_start", "Work_Left_stop", 'leftLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('leftLedger')

    ledg=OA.graphAddEdge(G, "Work_Right_start", "Work_Right_stop", 'rightLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('rightLedger')

    ledg=OA.graphAddEdge(G, "Work_Left_stop", "finalEDT", 'leftLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('leftLedger')
    OA.getEvent(G, ledg).fertile = False

    ledg=OA.graphAddEdge(G, "Work_Right_stop", "finalEDT", 'rightLedger')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('rightLedger')
    OA.getEvent(G, ledg).fertile = False

    # ----- WORK
    erri = 0
    while not erri:
        if True: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if False: OA.printGraph(G)
        if True:
            erri = OA.outputDot(G, "z_utest_channelG.dot")
            if erri: break
        if True:
            erri = OA.outputOCR(G, "z_utest_channelG.c")
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

