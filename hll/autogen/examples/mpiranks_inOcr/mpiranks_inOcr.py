import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    OA.GBL.insert_debug_code = 0

    G=OA.GraphTool.DiGraph()

    # ==============================================================================
    # ----- GLOBALS
    OA.setupGraph(G,"mpiRanks_inOcr")

    OA.addHeader(G, '#include "treeForkJoin.h" ')
    OA.addHeader(G, '#include "global_data.h" ')
    OA.addHeader(G, '#include "tailrecursion.h" ')
    OA.addHeader(G, '#define Nfoliation 2')  # If set to two that makes it a binary tree.

    # ==============================================================================
    # ==============================================================================
    # Fork-Join tree for the distribution of MPI ranks
    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'
    dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'gDone';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_gDone = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'TFJiterate';   dbk.count=1;     dbk.type='TFJiterate_t'
    dbk_TFJiterate = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'TCsum';   dbk.count=1;     dbk.type='TChecksum_work_t'
    dbk_work = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'TCsum2';   dbk.count=1;     dbk.type='TChecksum_work_t'
    dbk_work2 = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'RefTCsum';   dbk.count=1;     dbk.type='TChecksum_work_t'
    dbk_refwork = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'GlobalD';   dbk.count=1;     dbk.type='GlobalData_t'
    dbk_globald = dbk

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
    dbk = copy.deepcopy(dbk_globald);  dbk.flight = 'flTAKEOFF'; dbk.localname='o_globald'; OA.addDataBlocks(G, nc, dbk)   # To SetupBtForkJoin
    toFinalt = 'GUID_ASSIGN_VALUE(*o_gDone, ' + OA.makeGuidEdtname("finalEDT") + ');'
    OA.addCustomText(G, nc, toFinalt)
    init_globald = 'init_globalData(o_globald);'
    OA.addCustomText(G, nc, init_globald)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From mainEdt

    nc += 1;  taskName="SetupBtForkJoin"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)  # Toward ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname='io_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_refwork); dbk.flight = 'flTAKEOFF'; dbk.localname='o_refTCsum'; OA.addDataBlocks(G, nc, dbk) # To ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname='o_TFJiterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkIF
    setupt = 'err = setupBtForkJoin(OA_edtTypeNb, OA_DBG_thisEDT, o_TFJiterate, '
    setupt += OA.makeGuidEdtname("ConcludeBtForkJoin") + ', io_globald, &o_refTCsum->result);'
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
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate'; OA.addDataBlocks(G, nc, dbk) # From SetupBtForkJoin or BtForkFOR
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_TFJiterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkFOR or BtForkELSE
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flLANDING'; dbk.localname = 'in_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)    # To BtJoinIFTHEN
    #dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF';  <-- This is the NULL_GUID used to link to the ELSE clause
    OA.addIFconditionText(G, nc, 'conditionBtFork(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate)')
    iterText = 'err = btForkThen(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_TFJiterate, o_gDone);'
    OA.addCustomText(G, nc, iterText)
    copy_globals = 'copy_globalData(in_globald, o_globald);'
    OA.addCustomText(G, nc, copy_globals)

    nc += 1;  taskName="BtForkFOR"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)  # From SetupBtForkJoin or self
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flLANDING'; dbk.localname = 'in_globald'; dbk.delayReleaseDestroy = True

    OA.addDataBlocks(G, nc, dbk)
    fortext = 'int btFoliationIndex; for(btFoliationIndex=0; btFoliationIndex < Nfoliation; ++btFoliationIndex)'
    OA.addFORconditionText(G, nc, fortext)

    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_TFJiterate'; OA.addDataBlocks(G, nc, dbk)  # To self
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_globald'; OA.addDataBlocks(G, nc, dbk)
    forkFORt = 'err = btForkFOR(OA_edtTypeNb, OA_DBG_thisEDT, btFoliationIndex, in_TFJiterate, o_TFJiterate);'
    OA.addCustomText(G, nc, forkFORt)
    copy_globals = 'copy_globalData(in_globald, o_globald);'
    OA.addCustomText(G, nc, copy_globals)

    nc += 1;  taskName="BtForkELSE"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "btFork".
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate'; OA.addDataBlocks(G,nc,dbk)  # From self BtForkIF
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_TFJiterate';  OA.addDataBlocks(G, nc, dbk)  # To BtForkTransition
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flLANDING'; dbk.localname = 'in_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    btForkElseText = 'err = btForkElse(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_TFJiterate, o_gDone);'
    OA.addCustomText(G, nc, btForkElseText)
    copy_globals = 'copy_globalData(in_globald, o_globald);'
    OA.addCustomText(G, nc, copy_globals)

    nc += 1;  taskName="BtForkTransition_Start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_TFJiterate);  dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname = 'io_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)
    transitStart_text = '*o_gDone = ' + OA.makeGuidEdtname("BtForkTransition_Stop") + ';'
    OA.addCustomText(G, nc, transitStart_text)

    nc += 1;  taskName="BtForkTransition_Stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_TFJiterate);  dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate'; OA.addDataBlocks(G,nc,dbk)  # From BtForkELSE
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flLANDING'; dbk.localname = 'in_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work2); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work2'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    guid_BtJoinIFTHEN = OA.makeGuidEdtname('BtJoinIFTHEN');
    transit_whereToText = 'ocrGuid_t ' + guid_BtJoinIFTHEN + '; GUID_ASSIGN_VALUE(' + guid_BtJoinIFTHEN + ', in_TFJiterate->whereToGoWhenFalse);'
    OA.addCustomText(G, nc, transit_whereToText)
    transitText = 'err = transitionBTFork(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, o_work, o_work2);'
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

    # ==============================================================================
    # ==============================================================================
    # Tail recursion used to provide an execution path for each rank
    # It is inserted between BtForkTransition_Start and BtForkTransition_Stop
    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'
    dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'TailRecurIterate';   dbk.count=1;     dbk.type='TailRecurIterate_t'
    dbk_tailRecurIterate = dbk

    # ----- NODES
    nc += 1;  taskName="setupTailRecursion"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname = 'io_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_tailRecurIterate);  dbk.flight = 'flTAKEOFF';  dbk.localname='o_tailRecurIterate'; OA.addDataBlocks(G,nc,dbk)
    tailRecurSetupt = 'err = tailRecurInitialize(o_tailRecurIterate, '
    tailRecurSetupt += OA.makeGuidEdtname("concludeTailRecursion") + ', io_globald); IFEB;'
    OA.addCustomText(G, nc, tailRecurSetupt)

    nc += 1;  taskName="tailRecursionIFThen"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname = 'io_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; dbk.localname = 'io_tailRecurIterate'
    dbk.addLocalText('ocrGuid_t ' + OA.makeGuidEdtname("concludeTailRecursion") + ' = io_tailRecurIterate->whereToGoWhenDone;')
    OA.addDataBlocks(G,nc,dbk)  # This DBK guid will be needed by the tailRecursionELSE part of this EDT.
    tailRecurCondition_text = 'tailRecurCondition(io_tailRecurIterate)'; OA.addIFconditionText(G,nc,tailRecurCondition_text)
    tailRecurTHEN_text = 'err = tailRecurIfThenClause(io_tailRecurIterate); IFEB;'
    OA.addCustomText(G, nc, tailRecurTHEN_text)

    nc += 1;  taskName="tailRecursionELSE"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "tailRecursionIFThen".
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname = 'io_globald'; OA.addDataBlocks(G, nc, dbk)
    tailRecurELSE_text = 'err = tailRecurElseClause(io_tailRecurIterate); IFEB;'
    OA.addCustomText(G, nc, tailRecurELSE_text)

    nc += 1;  taskName="tailRecurTransitBEGIN"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname = 'io_globald'; OA.addDataBlocks(G, nc, dbk)

    nc += 1;  taskName="tailRecurTransitEND"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname = 'io_globald'; OA.addDataBlocks(G, nc, dbk)

    nc += 1;  taskName="concludeTailRecursion"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_globald); dbk.flight = 'flTAGO'; dbk.localname = 'io_globald'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone'; OA.addDataBlocks(G, nc, dbk)
    OA.addCustomText(G, nc, 'err = tailRecurConclude(); IFEB;')
    dbk.addLocalText('ocrGuid_t ' + OA.makeGuidEdtname("BtForkTransition_Stop") + ' = *in_gDone;')

    # ==============================================================================
    # ==============================================================================
    # Fork-Join tree for the distribution of MPI ranks
    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'

    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "RefTCsum")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('RefTCsum')

    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "BtForkIF", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    btForkIF_global_input_edge=ledg[0]  # This takes only the edge out of the labelled edge.
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
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkFOR", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    btJoin_gDone_main_input_edge = ledg[0]

    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';  OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    OA.sharedConx(G, ledg[0]).append(btForkIF_main_input_edge)
    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';  OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    OA.sharedConx(G, ledg[0]).append(btForkIF_global_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    OA.sharedConx(G, ledg[0]).append(btJoin_gDone_main_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "BtForkTransition_Stop", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')

#    Old code
#    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "BtForkTransition_Stop", "GlobalD")
#    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    #This is where we stitch in the tail recursion.
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "setupTailRecursion", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "setupTailRecursion", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Stop", "BtJoinIFTHEN", "TCsum2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TCsum2')
    OA.getEvent(G, ledg).fertile = False
    btFork_transition_to_btJoinIF_main_edge2 = ledg[0]
    ledg = OA.graphAddEdge(G, "BtForkTransition_Stop", "BtJoinIFTHEN", "TCsum")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TCsum')
    OA.getEvent(G, ledg).fertile = False
    btFork_transition_to_btJoinIF_main_edge1 = ledg[0]

    ledg = OA.graphAddEdge(G, "BtJoinIFTHEN", "BtJoinELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True
    ledg = OA.graphAddEdge(G, "BtJoinIFTHEN", "BtJoinIFTHEN", "TCsum")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TCsum')
    OA.getEvent(G, ledg).fertile = False
    OA.getEvent(G, ledg).user_slotnb_text = 'slot_for_work'
    OA.sharedConx(G, ledg[0]).append(btFork_transition_to_btJoinIF_main_edge1)

    ledg = OA.graphAddEdge(G, "BtJoinELSE", "ConcludeBtForkJoin", "TCsum")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TCsum')
    OA.getEvent(G, ledg).fertile = False

    # ==============================================================================
    # ==============================================================================
    # Tail recursion used to provide an execution path for each rank
    # It is inserted between BtForkTransition_Start and BtForkTransition_Stop
    # ----- EDGES
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "setupTailRecursion", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')

    ledg = OA.graphAddEdge(G, "setupTailRecursion", "concludeTailRecursion", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone'), 'DBK')

    ledg=OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TailRecurIterate')
    tailRecur_main_iteration_edge=ledg[0]  # This takes only the edge out of the labelled edge.

    ledg=OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('GlobalD')
    tailRecur_main_globald_edge=ledg[0]  # This takes only the edge out of the labelled edge.

    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('TailRecurIterate'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "tailRecurTransitEND", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('TailRecurIterate'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('TailRecurIterate'), 'DBK')
    OA.sharedConx(G, ledg[0]).append(tailRecur_main_iteration_edge)

    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('GlobalD'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "tailRecurTransitEND", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('GlobalD'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('GlobalD'), 'DBK')
    OA.sharedConx(G, ledg[0]).append(tailRecur_main_globald_edge)

    ledg=OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecursionELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True

    ledg = OA.graphAddEdge(G, "tailRecursionELSE", "concludeTailRecursion", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('GlobalD'), 'DBK')
    OA.getEvent(G, ledg).fertile = False

    ledg=OA.graphAddEdge(G, "concludeTailRecursion", "BtForkTransition_Stop", "GlobalD")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('GlobalD'), 'DBK')
    OA.getEvent(G, ledg).fertile = False

    # ==============================================================================
    # ==============================================================================
    # ==============================================================================
    # ==============================================================================
    # ==============================================================================
    # ----- WORK
    erri = 0
    while not erri:
        if 1 == 2: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if 1 == 2: OA.printGraph(G)
        if 1 == 2:
            erri = OA.outputDot(G, "z_mpiranks_inOcr.dot")
            if erri: break
        if 1 == 1:
            erri = OA.outputOCR(G, "z_mpiranks_inOcr.c")
            if erri: break

        if 1 == 2: OA.printGraph(G)

        break  # while not erri
    OA.errmsg(erri)
    return erri
# ==============================================================================
# ==============================================================================
# ==============================================================================
# ==============================================================================
# ==============================================================================
if __name__ == "__main__":
    errm = theMain()
    if errm:
        exit(1)
    else:
        exit(0)

