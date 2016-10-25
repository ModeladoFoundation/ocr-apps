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
    OA.addHeader(G, '#include "global_data.h"')
    OA.addHeader(G, '#include "reduction2ary.h"')
    OA.addHeader(G, ' ')  # Just a blank line
    OA.addHeader(G, '#define ENABLE_EXTENSION_LABELING  // For labeled EDTs')
    OA.addHeader(G, '#include "extensions/ocr-labeling.h"  // For labeled EDTs')
    OA.addHeader(G, ' ')  # Just a blank line
    OA.addHeader(G, '#define Nfoliation 2 // This is used by the bintreeForkJoin')
    OA.addHeader(G, '#define reductNaryFoliation 2 // This is used by the reductionBy2ary')

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

    # 'GlobalsRef' is the one we will properly destroy and release its ressources
    # 'Globals' is the one we will move around and clear when done.
    dbk = OA.ocrDataBlock();  dbk.name = 'Globals';   dbk.count=1;     dbk.type='GlobalData_t'
    dbk_globals = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'GlobalsRef';   dbk.count=1;     dbk.type='GlobalData_t'
    dbk_globalsRef = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'LedgerR2ary';   dbk.count=1;     dbk.type='LedgerReduc2ary_t'
    dbk_ledgerR2ary = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'LedgerR2ary2';  dbk.count=1;     dbk.type='LedgerReduc2ary_t'
    dbk_ledgerR2ary2 = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'R2aryValue';   dbk.count=1;     dbk.type='R2aryValue_t'
    dbk_valueR2ary = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'R2aryWhenDone';   dbk.count=1;     dbk.type='R2aryWhenDone_t'
    dbk_whenDoneR2ary = dbk

    # =========================================================================
    # =========================================================================
    # =========================================================================
    # =========================================================================
    # =========================================================================
    # ----- NODES -- Fork-Join Tree
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
    dbk = copy.deepcopy(dbk_globals);  dbk.flight = 'flTAKEOFF'; dbk.localname='o_globals'; OA.addDataBlocks(G, nc, dbk)
    toFinalt = 'GUID_ASSIGN_VALUE(*o_gDone, ' + OA.makeGuidEdtname("finalEDT") + ');'
    OA.addCustomText(G, nc, toFinalt)
    initGlobals_text = 'create_globalData(OA_DEBUG_OUTVARS, o_globals);'; OA.addCustomText(G, nc, initGlobals_text)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From mainEdt

    nc += 1;  taskName="SetupBtForkJoin"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)  # Toward ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flTAGO'; dbk.localname='io_globals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globalsRef); dbk.flight = 'flTAKEOFF'; dbk.localname='o_refGlobals'; OA.addDataBlocks(G, nc, dbk) # To ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_refwork); dbk.flight = 'flTAKEOFF'; dbk.localname='o_refWork'; OA.addDataBlocks(G, nc, dbk) # To ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname='o_iterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkIF
    setupt = 'err = setupBtForkJoin(OA_DEBUG_OUTVARS, o_iterate, '
    setupt += OA.makeGuidEdtname("ConcludeBtForkJoin") + ', add, add_zero, io_globals, &o_refWork->result);'
    OA.addCustomText(G, nc, setupt)
    transferGlobal_text = 'copy_globalData(OA_DEBUG_OUTVARS, io_globals, o_refGlobals);'
    OA.addCustomText(G, nc, transferGlobal_text)

    nc += 1;  taskName="ConcludeBtForkJoin"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)  # To finalEDT
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone'; OA.addDataBlocks(G, nc, dbk)  # From SetupBtForkJoin
    dbk = copy.deepcopy(dbk_refwork); dbk.flight = 'flLANDING'; dbk.localname = 'in_refwork'; OA.addDataBlocks(G, nc, dbk) # From SetupBtForkJoin
    dbk = copy.deepcopy(dbk_globalsRef); dbk.flight = 'flLANDING'; dbk.localname = 'in_refGlobals'; OA.addDataBlocks(G, nc, dbk) # From SetupBtForkJoin
    dbk = copy.deepcopy(dbk_work);    dbk.flight = 'flLANDING'; dbk.localname = 'in_calculated'; OA.addDataBlocks(G, nc, dbk) # From BtJoinELSE
    guid_finalEDT = OA.makeGuidEdtname('finalEDT')
    doneText = 'ocrGuid_t ' + guid_finalEDT + '; GUID_ASSIGN_VALUE(' + guid_finalEDT + ', *in_gDone);'
    OA.addCustomText(G, nc, doneText)
    OA.addCustomText(G, nc, 'err = concludeBtForkJoin(OA_DEBUG_OUTVARS, in_refwork->result, in_calculated->result); IFEB;')
    GlobalDone_text = 'err = destroy_globalData(OA_DEBUG_OUTVARS, in_refGlobals); IFEB;'
    OA.addCustomText(G, nc, GlobalDone_text)

    nc += 1;  taskName="BtForkIF"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_globals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_globals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; OA.addDataBlocks(G, nc, dbk) # From SetupBtForkJoin or BtForkFOR
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_iterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkFOR or BtForkELSE
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)    # To BtJoinIFTHEN
    #dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF';  <-- This is the NULL_GUID used to link to the ELSE clause
    OA.addIFconditionText(G, nc, 'conditionBtFork(OA_DEBUG_OUTVARS, in_iterate)')
    iterText = 'err = btForkThen(OA_DEBUG_OUTVARS, in_iterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_iterate, o_gDone);'
    OA.addCustomText(G, nc, iterText)
    copy_globals_text = 'copy_globalData(OA_DEBUG_OUTVARS, in_globals, o_globals);'
    OA.addCustomText(G, nc, copy_globals_text)

    nc += 1;  taskName="BtForkFOR"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)  # From SetupBtForkJoin or self
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_iterate'; OA.addDataBlocks(G, nc, dbk)  # To self
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_globals'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_globals'; OA.addDataBlocks(G, nc, dbk)
    fortext = 'int btFoliationIndex; for(btFoliationIndex=0; btFoliationIndex<Nfoliation; ++btFoliationIndex)'
    OA.addFORconditionText(G, nc, fortext)
    forkFORt = 'err = btForkFOR(OA_DEBUG_OUTVARS, btFoliationIndex, in_iterate, o_iterate);'
    OA.addCustomText(G, nc, forkFORt)
    copy_globals_text = 'copy_globalData(OA_DEBUG_OUTVARS, in_globals, o_globals);'
    OA.addCustomText(G, nc, copy_globals_text)

    nc += 1;  taskName="BtForkELSE"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "btFork".
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_iterate';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_globals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_globals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)
    btForkElseText = 'err = btForkElse(OA_DEBUG_OUTVARS, in_iterate, '
    btForkElseText += OA.makeGuidEdtname("BtJoinIFTHEN") +', o_iterate, o_gDone);'
    OA.addCustomText(G, nc, btForkElseText)
    copy_globals_text = 'copy_globalData(OA_DEBUG_OUTVARS, in_globals, o_globals);'
    OA.addCustomText(G, nc, copy_globals_text)

    nc += 1;  taskName="BtForkTransition_Start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAGO'; dbk.localname = 'io_FJiterate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flTAGO'; dbk.localname = 'io_globals'; OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_ledgerR2ary);  dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_ledgerR2ary'; OA.addDataBlocks(G, nc, dbk)
    btForkTransitStart_text = 'err = btForkTransitStart(OA_DEBUG_OUTVARS, io_FJiterate, io_globals,'
    btForkTransitStart_text += '&' + OA.makeGuidEdtname("BtForkTransition_Stop")
    btForkTransitStart_text += ', o_ledgerR2ary); IFEB;'
    OA.addCustomText(G, nc, btForkTransitStart_text)

    nc += 1;  taskName="BtForkTransition_Stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_iterate);  dbk.flight = 'flLANDING'; dbk.localname = 'in_iterate'; OA.addDataBlocks(G,nc,dbk)  # From BtForkELSE
    dbk = copy.deepcopy(dbk_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_globals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work2); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work2'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN

    dbk = copy.deepcopy(dbk_valueR2ary);  dbk.flight = 'flLANDING'; dbk.localname = 'in_valueR2ary'; OA.addDataBlocks(G,nc,dbk)

    guid_BtJoinIFTHEN = OA.makeGuidEdtname('BtJoinIFTHEN')
    transit_whereToText = 'ocrGuid_t ' + guid_BtJoinIFTHEN + '; GUID_ASSIGN_VALUE(' + guid_BtJoinIFTHEN + ', in_iterate->whereToGoWhenFalse);'
    OA.addCustomText(G, nc, transit_whereToText)

    transitText = 'err = transitionBTFork(OA_DEBUG_OUTVARS, in_iterate, o_work, o_work2); IFEB;'
    OA.addCustomText(G, nc, transitText)

    check_valueR2ary_text = 'err = R2ary_ValidateValue_in_btForkTransit_stop(OA_DEBUG_OUTVARS, in_globals, in_valueR2ary, in_iterate);'
    OA.addCustomText(G, nc, check_valueR2ary_text)

    clearGlobals_text = 'err = clear_globalData(OA_DEBUG_OUTVARS, in_globals); IFEB; //The destroy_globalData() has to be done globally.'
    OA.addCustomText(G, nc, clearGlobals_text)

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

    # ----- NODES -- Reduction2ary
    nc += 1;  taskName="R2arySetup"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_ledgerR2ary);  dbk.flight = 'flLANDING'; dbk.localname = 'in_ledgerR2ary'; OA.addDataBlocks(G, nc, dbk) # From BtForkTransition_Start
    dbk = copy.deepcopy(dbk_ledgerR2ary2); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_ledgerR2ary'; OA.addDataBlocks(G, nc, dbk) # To R2aryJoinIF
    dbk = copy.deepcopy(dbk_whenDoneR2ary);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_r2aWdone'; OA.addDataBlocks(G, nc, dbk) # To R2aryConclusion
    guid0_R2aryJoinIF_text = OA.makeGuidEdtname('R2aryJoinIF')
    text_R2ary_nextEDT = 'ocrGuid_t ' + guid0_R2aryJoinIF_text + ';';  OA.addCustomText(G, nc, text_R2ary_nextEDT)

    guid_R2aryConclusion_text = OA.makeGuidEdtname("R2aryConclusion")
    text_R2ary_dslot_to_use = 'unsigned int ra2ary_ledgerSlot2use;'; OA.addCustomText(G, nc, text_R2ary_dslot_to_use)
    text_for_R2arySetup = 'err = setup_reduction2ary(OA_DEBUG_OUTVARS, o_ledgerR2ary, o_r2aWdone, in_ledgerR2ary, &'
    text_for_R2arySetup += guid_R2aryConclusion_text + ', R2aryJoinIF, &'
    text_for_R2arySetup += guid0_R2aryJoinIF_text + ', &ra2ary_ledgerSlot2use); IFEB;'
    OA.addCustomText(G, nc, text_for_R2arySetup)

    nc += 1;  taskName="R2aryConclusion"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_valueR2ary); dbk.flight = 'flLANDING'; dbk.localname = 'in_value'; OA.addDataBlocks(G, nc, dbk)  # From R2aryForkELSE_FOR
    dbk = copy.deepcopy(dbk_whenDoneR2ary); dbk.flight = 'flLANDING'; dbk.localname = 'in_r2aWdone'; OA.addDataBlocks(G, nc, dbk) # From R2arySetup
    dbk = copy.deepcopy(dbk_valueR2ary); dbk.flight = 'flTAKEOFF'; dbk.localname = "o_value"; OA.addDataBlocks(G, nc, dbk)  # To BtForkTransition_Stop
    initValue_reductBy2ary_text = 'init_R2aryValue(o_value);'; OA.addCustomText(G, nc, initValue_reductBy2ary_text)
    copyValue_r2aryConclude_text = 'copy_R2aryValue(in_value, o_value);'; OA.addCustomText(G, nc, copyValue_r2aryConclude_text)
    guid_BtForkTransition_Stop = OA.makeGuidEdtname("BtForkTransition_Stop")
    R2aryConclusion_whereToGo = 'ocrGuid_t ' + guid_BtForkTransition_Stop
    R2aryConclusion_whereToGo += '; GUID_ASSIGN_VALUE(' + guid_BtForkTransition_Stop + ', in_r2aWdone->wdone[0]);'
    OA.addCustomText(G, nc, R2aryConclusion_whereToGo)

    nc += 1;  taskName="R2aryJoinIF"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_ledgerR2ary2); dbk.flight = 'flLANDING'; dbk.localname = 'in_ledgerR2ary2'; OA.addDataBlocks(G, nc, dbk)  # From R2arySetup
    dbk = copy.deepcopy(dbk_ledgerR2ary);  dbk.flight = 'flLANDING'; dbk.localname = 'in_ledgerR2ary'; OA.addDataBlocks(G, nc, dbk)  # From another R2arySetup
    dbk = copy.deepcopy(dbk_ledgerR2ary); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_ledgerR2ary'; OA.addDataBlocks(G, nc, dbk)  # To self
    dbk = copy.deepcopy(dbk_whenDoneR2ary);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_r2aWdone'; OA.addDataBlocks(G, nc, dbk) # To R2aryForkIF
    dbk = copy.deepcopy(dbk_nullGuid);     dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)  # To R2aryJoinELSE
    OA.addIFconditionText(G, nc, 'conditionR2aryJoinIF(OA_edtTypeNb, OA_DBG_thisEDT, in_ledgerR2ary, in_ledgerR2ary2)')
    guid_R2aryFORKif_text = OA.makeGuidEdtname('R2aryForkIF')  # This is where one goes next in the EDT bracketing.
    guid1_R2aryJoinIF_text = OA.makeGuidEdtname('R2aryJoinIF')
    text1_R2ary_nextEDT = 'ocrGuid_t ' + guid1_R2aryJoinIF_text + ';'; OA.addCustomText(G, nc, text1_R2ary_nextEDT)
    R2aryJoinIF_slot_text = 'unsigned int R2aryJoinIF_slot;';          OA.addCustomText(G, nc, R2aryJoinIF_slot_text)
    R2aryJoinIF_text = 'err = R2aryJoinIF_fcn(OA_DEBUG_OUTVARS, &'
    R2aryJoinIF_text += guid_R2aryFORKif_text + ', in_ledgerR2ary, in_ledgerR2ary2, '
    R2aryJoinIF_text += 'o_ledgerR2ary, o_r2aWdone, R2aryJoinIF, '
    R2aryJoinIF_text += '&' + guid1_R2aryJoinIF_text + ', &R2aryJoinIF_slot); IFEB;'
    OA.addCustomText(G, nc, R2aryJoinIF_text)

    nc += 1;  taskName="R2aryJoinELSE"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From R2aryJoinIF
    dbk = copy.deepcopy(dbk_ledgerR2ary2); dbk.flight = 'flLANDING'; dbk.localname = 'in_ledgerR2ary2'; OA.addDataBlocks(G, nc, dbk)# From R2arySetup
    dbk = copy.deepcopy(dbk_ledgerR2ary);  dbk.flight = 'flLANDING'; dbk.localname = 'in_ledgerR2ary'; OA.addDataBlocks(G, nc, dbk)# From another R2arySetup
    dbk = copy.deepcopy(dbk_whenDoneR2ary);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_r2aWdone'; OA.addDataBlocks(G, nc, dbk) # To R2aryForkIF
    dbk = copy.deepcopy(dbk_valueR2ary); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_valueR2ary'; OA.addDataBlocks(G, nc, dbk)  # To R2aryForkIF
    R2aryJoinELSE_text = 'err = R2aryJoinELSE_fcn(OA_edtTypeNb, OA_DBG_thisEDT, '
    R2aryJoinELSE_text += 'in_ledgerR2ary, in_ledgerR2ary2, o_valueR2ary, o_r2aWdone); IFEB;'
    OA.addCustomText(G, nc, R2aryJoinELSE_text)

    nc += 1;  taskName="R2aryForkIF"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_whenDoneR2ary); dbk.flight = 'flTAGO'; dbk.localname = 'io_r2aWdone'; OA.addDataBlocks(G, nc, dbk)# To R2aryForkIF_FOR
    dbk = copy.deepcopy(dbk_valueR2ary);    dbk.flight = 'flTAGO'; dbk.localname = 'io_valueR2ary'; OA.addDataBlocks(G, nc, dbk)  # To R2aryForkIF_FOR
    dbk = copy.deepcopy(dbk_nullGuid);      dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)  # To R2aryForkELSE
    OA.addIFconditionText(G, nc, 'conditionR2aryForkIF(OA_edtTypeNb, OA_DBG_thisEDT, io_r2aWdone)')

    nc += 1;  taskName="R2aryForkIF_FOR"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_whenDoneR2ary); dbk.flight = 'flLANDING'; dbk.localname = 'in_r2aWdone'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_valueR2ary);    dbk.flight = 'flLANDING'; dbk.localname = 'in_valueR2ary'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_valueR2ary); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_valueR2ary'; OA.addDataBlocks(G, nc, dbk)  # To R2aryForkIF
    R2aryForkIF_FOR_fortext = 'int r2aryX; for(r2aryX=0; r2aryX < reductNaryFoliation; ++r2aryX)'
    OA.addFORconditionText(G, nc, R2aryForkIF_FOR_fortext)
    guid_R2aryForkIF_FOR_2go = OA.makeGuidEdtname('R2aryForkIF')
    OA.addCustomText(G, nc, 'ocrGuid_t ' + guid_R2aryForkIF_FOR_2go + ';')
    R2aryForkIF_FOR_bodytext = 'err = R2aryForkIF_FOR_body(OA_edtTypeNb, OA_DBG_thisEDT, r2aryX, in_r2aWdone, '
    R2aryForkIF_FOR_bodytext += 'in_valueR2ary, o_valueR2ary, &' + guid_R2aryForkIF_FOR_2go + '); IFEB;'
    OA.addCustomText(G, nc, R2aryForkIF_FOR_bodytext)

    nc += 1;  taskName="R2aryForkELSE"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From R2aryForkIF
    dbk = copy.deepcopy(dbk_whenDoneR2ary); dbk.flight = 'flTAGO'; dbk.localname = 'io_r2aWdone'; OA.addDataBlocks(G, nc, dbk)# To R2aryForkELSE_FOR
    dbk = copy.deepcopy(dbk_valueR2ary);    dbk.flight = 'flTAGO'; dbk.localname = 'io_valueR2ary'; OA.addDataBlocks(G, nc, dbk)  # To R2aryForkELSE_FOR

    nc += 1;  taskName="R2aryForkELSE_FOR"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_whenDoneR2ary); dbk.flight = 'flLANDING'; dbk.localname = 'in_r2aWdone'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_valueR2ary);    dbk.flight = 'flLANDING'; dbk.localname = 'in_valueR2ary'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_valueR2ary); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_valueR2ary'; OA.addDataBlocks(G, nc, dbk)  # To R2aryConclusion
    R2aryForkELSE_FOR_fortext = 'int r2aryX; for(r2aryX=0; r2aryX < reductNaryFoliation; ++r2aryX)'
    OA.addFORconditionText(G, nc, R2aryForkELSE_FOR_fortext)
    guid_R2aryForkELSE_FOR_2go = OA.makeGuidEdtname('R2aryConclusion')
    OA.addCustomText(G, nc, 'ocrGuid_t ' + guid_R2aryForkELSE_FOR_2go + ';')
    R2aryForkELSE_FOR_bodytext = 'err = R2aryForkELSE_FOR_body(OA_edtTypeNb, OA_DBG_thisEDT, r2aryX, in_r2aWdone, '
    R2aryForkELSE_FOR_bodytext += 'in_valueR2ary, o_valueR2ary, &' + guid_R2aryForkELSE_FOR_2go + '); IFEB;'
    OA.addCustomText(G, nc, R2aryForkELSE_FOR_bodytext)

    # =========================================================================
    # =========================================================================
    # =========================================================================
    # =========================================================================
    # =========================================================================
    # ----- EDGES - Fork-Join Tree
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "Globals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Globals')
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'

    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "RefWork")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('RefWork')
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "GlobalsRef")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalsRef')

    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "BtForkIF", "Globals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Globals')
    btForkIF_globals_input_edge=ledg[0]  # This takes only the edge out of the labelled edge.
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
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkFOR", "Globals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Globals')
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    btJoin_gDone_main_input_edge = ledg[0]

    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    OA.sharedConx(G, ledg[0]).append(btForkIF_main_input_edge)
    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "Globals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';  OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    OA.sharedConx(G, ledg[0]).append(btForkIF_globals_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    OA.sharedConx(G, ledg[0]).append(btJoin_gDone_main_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "Globals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Globals')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "BtForkTransition_Stop", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "BtForkTransition_Stop", "Globals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Globals')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Stop", "BtJoinIFTHEN", "Work2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work2')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "BtForkTransition_Stop", "BtJoinIFTHEN", "Work")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work')
    OA.getEvent(G, ledg).fertile = False
    btFork_transition_to_btJoinIF_main_edge1 = ledg[0]

    ledg = OA.graphAddEdge(G, "BtJoinIFTHEN", "BtJoinELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True
    ledg = OA.graphAddEdge(G, "BtJoinIFTHEN", "BtJoinIFTHEN", "Work")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work')
    OA.getEvent(G, ledg).fertile = False
    OA.getEvent(G, ledg).user_slotnb_text = 'slot_for_work'
    OA.sharedConx(G, ledg[0]).append(btFork_transition_to_btJoinIF_main_edge1)

    ledg = OA.graphAddEdge(G, "BtJoinELSE", "ConcludeBtForkJoin", "Work")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work')
    OA.getEvent(G, ledg).fertile = False

    # ----- EDGES - Reduction2ary
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "R2arySetup", "LedgerR2ary")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('LedgerR2ary')

    ledg = OA.graphAddEdge(G, "R2arySetup", "R2aryConclusion", "R2aryWhenDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryWhenDone')
    ledg = OA.graphAddEdge(G, "R2arySetup", "R2aryJoinIF", "LedgerR2ary2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('LedgerR2ary2')
    OA.getEvent(G, ledg).user_slotnb_text = 'ra2ary_ledgerSlot2use'
    OA.getEvent(G, ledg).fertile = False  # Sterile because it will be created through the labeled EDT map.

    ledg = OA.graphAddEdge(G, "R2aryConclusion", "BtForkTransition_Stop", "R2aryValue")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryValue')
    OA.getEvent(G, ledg).fertile = False

    #2016Sept14:
    #Notice the setting of edges outgoing from R2aryJoinELSE first, instead of the usual traitment of R2aryJoinIF first.
    #It is because of the way shared edges currently work:  If a node has 1 shared edge, all its edges are considered
    #shared, which is not really a good idea.
    #To temporily fix this, this edt inversion causes all the shared edges to NOT be on R2aryJoinELSE, allowing the
    #slot attribution algorithm to work correctly.
    ledg = OA.graphAddEdge(G, "R2aryJoinELSE", "R2aryForkIF", "R2aryWhenDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryWhenDone')
    R2aryJoinELSE_main_wDone_edge = ledg[0];
    ledg = OA.graphAddEdge(G, "R2aryJoinELSE", "R2aryForkIF", "R2aryValue")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryValue')
    R2aryForkIF_main_value_edge = ledg[0]

    ledg = OA.graphAddEdge(G, "R2aryJoinIF", "R2aryJoinELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True  # Automatically sterile
    ledg = OA.graphAddEdge(G, "R2aryJoinIF", "R2aryJoinIF", "LedgerR2ary")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('LedgerR2ary')
    OA.getEvent(G, ledg).user_slotnb_text = 'R2aryJoinIF_slot'
    OA.getEvent(G, ledg).fertile = False  # Sterile because it will be created through the labeled EDT map.
    ledg = OA.graphAddEdge(G, "R2aryJoinIF", "R2aryForkIF", "R2aryWhenDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryWhenDone')
    OA.sharedConx(G, ledg[0]).append(R2aryJoinELSE_main_wDone_edge)

    ledg = OA.graphAddEdge(G, "R2aryForkIF", "R2aryForkIF_FOR", "R2aryWhenDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryWhenDone')
    ledg = OA.graphAddEdge(G, "R2aryForkIF", "R2aryForkIF_FOR", "R2aryValue")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryValue')
    ledg = OA.graphAddEdge(G, "R2aryForkIF", "R2aryForkELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True # Automatically sterile

    ledg = OA.graphAddEdge(G, "R2aryForkIF_FOR", "R2aryForkIF", "R2aryValue")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryValue')
    OA.sharedConx(G, ledg[0]).append(R2aryForkIF_main_value_edge)
    OA.getEvent(G, ledg).fertile = False  # Already created by the Join operations

    ledg = OA.graphAddEdge(G, "R2aryForkELSE", "R2aryForkELSE_FOR", "R2aryWhenDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryWhenDone')
    ledg = OA.graphAddEdge(G, "R2aryForkELSE", "R2aryForkELSE_FOR", "R2aryValue")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryValue')

    ledg = OA.graphAddEdge(G, "R2aryForkELSE_FOR", "R2aryConclusion", "R2aryValue")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('R2aryValue')
    OA.getEvent(G, ledg).fertile = False # Already created by the Setup operations

    # =========================================================================
    # =========================================================================
    # =========================================================================
    # =========================================================================
    # =========================================================================
    erri = 0
    while not erri:
        if False: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if False: OA.printGraph(G)
        if True :
            erri = OA.outputDot(G, "z_reductionBy2ary.dot")
            if erri: break
        if True:
            erri = OA.outputOCR(G, "z_reductionBy2ary.c")
            if erri: break

        if True: OA.printGraph(G)

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

