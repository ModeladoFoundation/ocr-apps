import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    OA.GBL.insert_debug_code = 0

    G=OA.GraphTool.DiGraph()

    OA.setupGraph(G,"BinTree_ForkJoin")

    OA.addHeader(G, '#include "bintreeForkjoin.h" ')  # This will be put at the beginning of the file
    OA.addHeader(G, '#define Nfoliation 2')  # If set to two that makes it a binary tree.

    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'
    dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'gDone';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_gDone = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'TFJiterate';   dbk.count=1;     dbk.type='TFJiterate_t'
    dbk_iterate = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'TFJiterate2';   dbk.count=1;     dbk.type='TFJiterate_t'
    dbk_iterate2 = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'Work';   dbk.count=1;     dbk.type='Work_t'
    dbk_work = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'Work2';   dbk.count=1;     dbk.type='Work_t'
    dbk_work2 = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'RefWork';   dbk.count=1;     dbk.type='Work_t'
    dbk_refwork = dbk

    # ----- NODES
    # NOTE: In as much as doable, the EDT are presented in a bracketing fashion.
    #       That is, assuming we have a sequence of EDTs as follows A->B->C->D->E
    #       Then the EDTs are presented as follows:  A,E,B,D,C
    #       For a linear sequence, that is of little importance; but for a more complex graph, bracketing
    #       makes it clear what are the EDTs acting as inputs and outputs.
    #
    # Always root the starting/root/main EDT at ID=0 = OA.GBL.MAINNODE
    # Always cap all ending EDT by the FinalEDT at ID=1 = OA.GBL.FINALNODE

    nc = OA.GBL.MAINNODE;    taskName = "mainEdt"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)   # To FinalEDT
    dbk = copy.deepcopy(dbk_gDone);    dbk.flight = 'flTAKEOFF'; dbk.localname='o_gDone'; OA.addDataBlocks(G, nc, dbk)   # To SetupBtForkJoin
    toFinalt = 'GUID_ASSIGN_VALUE(*o_gDone, ' + OA.makeGuidEdtname("finalEDT") + ');'
    OA.addCustomText(G, nc, toFinalt)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From mainEdt

    nc += 1;  taskName="SetupBtForkJoin"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)  # Toward ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_refwork); dbk.flight = 'flTAKEOFF'; dbk.localname='o_refWork'; OA.addDataBlocks(G, nc, dbk) # To ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname='o_iterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkIF
    setupt = 'err = setupBtForkJoin(OA_edtTypeNb, OA_DBG_thisEDT, o_iterate, ' + OA.makeGuidEdtname("ConcludeBtForkJoin") + ', add, add_zero, &o_refWork->result);'
    OA.addCustomText(G, nc, setupt)

    nc += 1;  taskName="ConcludeBtForkJoin"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)  # To finalEDT
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone'; OA.addDataBlocks(G, nc, dbk)  # From SetupBtForkJoin
    dbk = copy.deepcopy(dbk_refwork); dbk.flight = 'flLANDING'; dbk.localname = 'in_refwork'; OA.addDataBlocks(G, nc, dbk) # From SetupBtForkJoin
    dbk = copy.deepcopy(dbk_work);    dbk.flight = 'flLANDING'; dbk.localname = 'in_calculated'; OA.addDataBlocks(G, nc, dbk) # From BtJoinELSE
    guid_finalEDT = OA.makeGuidEdtname('finalEDT')
    doneText = 'ocrGuid_t ' + guid_finalEDT + '; GUID_ASSIGN_VALUE(' + guid_finalEDT + ', *in_gDone);'
    OA.addCustomText(G, nc, doneText)
    OA.addCustomText(G, nc, 'err = concludeBtForkJoin(OA_edtTypeNb, OA_DBG_thisEDT, in_refwork->result, in_calculated->result); IFEB;')

    nc += 1;  taskName="BtForkIF"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; OA.addDataBlocks(G, nc, dbk) # From SetupBtForkJoin or BtForkFOR
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_iterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkFOR or BtForkELSE
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)    # To BtJoinIFTHEN
    #dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF';  <-- This is the NULL_GUID used to link to the ELSE clause
    OA.addIFconditionText(G, nc, 'conditionBtFork(OA_edtTypeNb, OA_DBG_thisEDT, in_iterate)')
    iterText = 'err = btForkThen(OA_edtTypeNb, OA_DBG_thisEDT, in_iterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_iterate, o_gDone);'
    OA.addCustomText(G, nc, iterText)

    nc += 1;  taskName="BtForkFOR"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)  # From SetupBtForkJoin or self
    fortext = 'int btFoliationIndex; for(btFoliationIndex=0; btFoliationIndex<Nfoliation; ++btFoliationIndex)'
    OA.addFORconditionText(G, nc, fortext)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_iterate'; OA.addDataBlocks(G, nc, dbk)  # To self
    forkFORt = 'err = btForkFOR(OA_edtTypeNb, OA_DBG_thisEDT, btFoliationIndex, in_iterate, o_iterate);'
    OA.addCustomText(G, nc, forkFORt)

    nc += 1;  taskName="BtForkELSE"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "btFork".
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; OA.addDataBlocks(G,nc,dbk)  # From self BtForkIF
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_iterate';  OA.addDataBlocks(G, nc, dbk)  # To BtForkTransition
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    btForkElseText = 'err = btForkElse(OA_edtTypeNb, OA_DBG_thisEDT, in_iterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_iterate, o_gDone);'
    OA.addCustomText(G, nc, btForkElseText)

    nc += 1;  taskName="BtForkTransition"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_iterate);  dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; OA.addDataBlocks(G,nc,dbk)  # From BtForkELSE
    dbk = copy.deepcopy(dbk_work2); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work2'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    guid_BtJoinIFTHEN = OA.makeGuidEdtname('BtJoinIFTHEN');
    transit_whereToText = 'ocrGuid_t ' + guid_BtJoinIFTHEN + '; GUID_ASSIGN_VALUE(' + guid_BtJoinIFTHEN + ', in_iterate->whereToGoWhenFalse);'
    OA.addCustomText(G, nc, transit_whereToText)
    transitText = 'err = transitionBTFork(OA_edtTypeNb, OA_DBG_thisEDT, in_iterate, o_work, o_work2);'
    OA.addCustomText(G, nc, transitText)

    nc += 1;  taskName="BtJoinIFTHEN"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flLANDING'; dbk.localname = 'in_workLeft'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work2); dbk.flight = 'flLANDING'; dbk.localname = 'in_workRight'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work'; OA.addDataBlocks(G, nc, dbk)
    # dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF';  <-- This is the NULL_GUID used to link to the ELSE clause
    OA.addIFconditionText(G, nc, 'NOTA_BTindex != get_parent_index(in_workLeft->btindex)')
    joinIFTHEN_Text = 'err = joinOperationIFTHEN(OA_edtTypeNb, OA_DBG_thisEDT, *in_workLeft, *in_workRight, o_work);'
    OA.addCustomText(G, nc, joinIFTHEN_Text)
    guid_BtJoinIFTHEN = OA.makeGuidEdtname("BtJoinIFTHEN")
    btJoinIF_whereToGo = 'ocrGuid_t ' + guid_BtJoinIFTHEN + '; GUID_ASSIGN_VALUE(' + guid_BtJoinIFTHEN + ', *in_gDone);'
    OA.addCustomText(G, nc, btJoinIF_whereToGo)
    btJoin_which_slot = 'unsigned int slot_for_work = btCalculateJoinIndex(in_workLeft->btindex);'
    OA.addCustomText(G, nc, btJoin_which_slot)

    nc += 1;  taskName="BtJoinELSE"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flLANDING'; dbk.localname = 'in_workLeft'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work2); dbk.flight = 'flLANDING'; dbk.localname = 'in_workRight'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work'; OA.addDataBlocks(G, nc, dbk)
    joinText = 'err = joinOperationELSE(OA_edtTypeNb, OA_DBG_thisEDT, *in_workLeft, *in_workRight, o_work);'
    OA.addCustomText(G, nc, joinText)
    g_ConcludeBtForkJoin = OA.makeGuidEdtname("ConcludeBtForkJoin")
    btJoinELSE_whereToGo = 'ocrGuid_t ' + g_ConcludeBtForkJoin + '; GUID_ASSIGN_VALUE(' + g_ConcludeBtForkJoin + ', *in_gDone);'
    OA.addCustomText(G, nc, btJoinELSE_whereToGo)

    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'

    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "RefWork")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('RefWork')
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "BtForkIF", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    btForkIF_main_input_edge=ledg[0]  # This takes only the edge out of the labelled edge.

    ledg = OA.graphAddEdge(G, "ConcludeBtForkJoin", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
    OA.getEvent(G, ledg).fertile = False

    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkFOR", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    btJoin_gDone_main_input_edge = ledg[0]

    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    OA.sharedConx(G, ledg[0]).append(btForkIF_main_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    OA.sharedConx(G, ledg[0]).append(btJoin_gDone_main_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')

    ledg = OA.graphAddEdge(G, "BtForkTransition", "BtJoinIFTHEN", "Work2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work2')
    OA.getEvent(G, ledg).fertile = False
    btFork_transition_to_btJoinIF_main_edge2 = ledg[0]
    ledg = OA.graphAddEdge(G, "BtForkTransition", "BtJoinIFTHEN", "Work")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work')
    OA.getEvent(G, ledg).fertile = False
    btFork_transition_to_btJoinIF_main_edge1 = ledg[0]

    ledg = OA.graphAddEdge(G, "BtJoinIFTHEN", "BtJoinELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True
    ledg = OA.graphAddEdge(G, "BtJoinIFTHEN", "BtJoinIFTHEN", "Work")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work')
    OA.getEvent(G, ledg).fertile = False
    OA.getEvent(G, ledg).user_slotnb_text = 'slot_for_work'
    OA.sharedConx(G, ledg[0]).append(btFork_transition_to_btJoinIF_main_edge1)

    ledg = OA.graphAddEdge(G, "BtJoinELSE", "ConcludeBtForkJoin", "Work")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work')
    OA.getEvent(G, ledg).fertile = False

    # ----- WORK
    erri = 0
    while not erri:
        if 1 == 2: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if 1 == 2: OA.printGraph(G)
        if 1 == 1:
            erri = OA.outputDot(G, "z_bintree_forkjoin.dot")
            if erri: break
        if 1 == 1:
            erri = OA.outputOCR(G, "z_bintree_forkjoin.c")
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

