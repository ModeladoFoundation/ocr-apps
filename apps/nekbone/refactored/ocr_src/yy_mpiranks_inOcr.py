import sys
sys.path.append('../../../../hll/autogen/tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    OA.GBL.insert_debug_code = 1

    G=OA.GraphTool.DiGraph()

    # ==============================================================================
    # ----- GLOBALS
    OA.setupGraph(G,"nekbone_inOcr")

    OA.addHeader(G, '#include "treeForkJoin.h" ')
    OA.addHeader(G, '#include "spmd_global_data.h" ')
    OA.addHeader(G, '#include "tailrecursion.h" ')
    OA.addHeader(G, '#include "neko_globals.h" ')
    OA.addHeader(G, '#include "nekos_tools.h" ')
    OA.addHeader(G, ' ')
    OA.addHeader(G, '#define Nfoliation 2')  # If set to two that makes it a binary tree.

    # ==============================================================================
    # ==============================================================================
    # Fork-Join tree for the distribution of MPI ranks
    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'
    dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'gDone';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_gDone = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'gDone2';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_gDone2 = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'gDone3';   dbk.count=1;     dbk.type='ocrGuid_t'
    dbk_gDone3 = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'TFJiterate';   dbk.count=1;     dbk.type='TFJiterate_t'
    dbk_TFJiterate = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'TCsum';   dbk.count=1;     dbk.type='TChecksum_work_t'
    dbk_work = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'TCsum2';   dbk.count=1;     dbk.type='TChecksum_work_t'
    dbk_work2 = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'RefTCsum';   dbk.count=1;     dbk.type='TChecksum_work_t'
    dbk_refwork = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'SPMDGlobals';   dbk.count=1;     dbk.type='SPMD_GlobalData_t'
    dbk_spmd_globals = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'NEKOstatics';   dbk.count=1;     dbk.type='NEKOstatics_t'
    dbk_nekoStatics = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'NEKOglobals';   dbk.count=1;     dbk.type='NEKOglobals_t'
    dbk_nekoGlobals = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'NEKOtools';   dbk.count=1;     dbk.type='NEKOtools_t'
    dbk_nekoTools = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'nekoC';   dbk.count='sz_nekoC';     dbk.type='double'
    dbk_nekoC = dbk

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
    dbk = copy.deepcopy(dbk_spmd_globals);  dbk.flight = 'flTAKEOFF'; dbk.localname='o_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)   # To SetupBtForkJoin
    toFinalt = 'GUID_ASSIGN_VALUE(*o_gDone, ' + OA.makeGuidEdtname("finalEDT") + ');'
    OA.addCustomText(G, nc, toFinalt)
    init_globald = 'init_SPMDglobals(o_SPMDglobals);'; OA.addCustomText(G, nc, init_globald)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAKEOFF'; dbk.localname='o_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    OA.addCustomText(G, nc, 'err = init_NEKOstatics(o_NEKOstatics); IFEB;')
    OA.addCustomText(G, nc, 'err = setup_SPMD_using_NEKOstatics(o_NEKOstatics, o_SPMDglobals); IFEB;')

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From mainEdt

    nc += 1;  taskName="SetupBtForkJoin"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)  # Toward ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; dbk.localname='io_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_refwork); dbk.flight = 'flTAKEOFF'; dbk.localname='o_refTCsum'; OA.addDataBlocks(G, nc, dbk) # To ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname='o_TFJiterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkIF
    setupt = 'err = setupBtForkJoin(OA_edtTypeNb, OA_DBG_thisEDT, o_TFJiterate, '
    setupt += OA.makeGuidEdtname("ConcludeBtForkJoin") + ', io_SPMDglobals, &o_refTCsum->result);'
    OA.addCustomText(G, nc, setupt)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)

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
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)    # To BtJoinIFTHEN
    #dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF';  <-- This is the NULL_GUID used to link to the ELSE clause
    OA.addIFconditionText(G, nc, 'conditionBtFork(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate)')
    iterText = 'err = btForkThen(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_TFJiterate, o_gDone);'
    OA.addCustomText(G, nc, iterText)
    copy_SPMDglobals = 'copy_SPMDglobals(in_SPMDglobals, o_SPMDglobals);'; OA.addCustomText(G, nc, copy_SPMDglobals)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    copy_NEKOstatics = 'copy_NEKOstatics(in_NEKOstatics, o_NEKOstatics);'; OA.addCustomText(G, nc, copy_NEKOstatics)

    nc += 1;  taskName="BtForkFOR"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)  # From SetupBtForkJoin or self
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_SPMDglobals'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    fortext = 'int btFoliationIndex; for(btFoliationIndex=0; btFoliationIndex < Nfoliation; ++btFoliationIndex)'
    OA.addFORconditionText(G, nc, fortext)
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_TFJiterate'; OA.addDataBlocks(G, nc, dbk)  # To self
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    forkFORt = 'err = btForkFOR(OA_edtTypeNb, OA_DBG_thisEDT, btFoliationIndex, in_TFJiterate, o_TFJiterate);'
    OA.addCustomText(G, nc, forkFORt)
    copy_SPMDglobals = 'copy_SPMDglobals(in_SPMDglobals, o_SPMDglobals);'
    OA.addCustomText(G, nc, copy_SPMDglobals)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOstatics'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    copy_NEKOstatics = 'copy_NEKOstatics(in_NEKOstatics, o_NEKOstatics);'; OA.addCustomText(G, nc, copy_NEKOstatics)

    nc += 1;  taskName="BtForkELSE"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "btFork".
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate'; OA.addDataBlocks(G,nc,dbk)  # From self BtForkIF
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_TFJiterate';  OA.addDataBlocks(G, nc, dbk)  # To BtForkTransition
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    btForkElseText = 'err = btForkElse(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_TFJiterate, o_gDone);'
    OA.addCustomText(G, nc, btForkElseText)
    copy_SPMDglobals = 'copy_SPMDglobals(in_SPMDglobals, o_SPMDglobals);'
    OA.addCustomText(G, nc, copy_SPMDglobals)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    copy_NEKOstatics = 'copy_NEKOstatics(in_NEKOstatics, o_NEKOstatics);'; OA.addCustomText(G, nc, copy_NEKOstatics)

    nc += 1;  taskName="BtForkTransition_Start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_TFJiterate);  dbk.flight = 'flTAGO'; dbk.localname = 'io_TFJiterate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; dbk.localname = 'io_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)
    transitStart_text = '*o_gDone = ' + OA.makeGuidEdtname("BtForkTransition_Stop") + ';'
    OA.addCustomText(G, nc, transitStart_text)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    nekoGlobals_init = 'err = init_NEKOglobals(io_NEKOstatics, io_TFJiterate->low-1, o_NEKOglobals); IFEB;'
    OA.addCustomText(G, nc, nekoGlobals_init)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone2'; OA.addDataBlocks(G, nc, dbk)
    toget2nekMulti_text = '*o_gDone2 = ' + OA.makeGuidEdtname("setupTailRecursion") + ';'
    OA.addCustomText(G, nc, toget2nekMulti_text)

    nc += 1;  taskName="nekMultiplicity_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    nekoTools_init = 'err = init_NEKOtools(o_NEKOtools, *io_NEKOstatics, *io_NEKOglobals); IFEB;'
    OA.addCustomText(G, nc, nekoTools_init)
    OA.addCustomText(G, nc, '//SKE-NOTE:  HALO exchange of c')

    nc += 1;  taskName="nekMultiplicity_stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'o_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)

    nc += 1;  taskName="nekSetF_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'o_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  HALO exchange of f')

    nc += 1;  taskName="nekSetF_stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'o_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)

    nc += 1;  taskName="nekCGstep0_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'o_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  ALLREDUCTION due to rnorminit')

    nc += 1;  taskName="nekCGstep0_stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'o_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    nekSetF_2go_text = 'ocrGuid_t ' + OA.makeGuidEdtname("setupTailRecursion") + ' = *in_gDone2;'
    OA.addCustomText(G, nc, nekSetF_2go_text)

    nc += 1;  taskName="BtForkTransition_Stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_TFJiterate);  dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate'; OA.addDataBlocks(G,nc,dbk)  # From BtForkELSE
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_work2); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work2'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    dbk = copy.deepcopy(dbk_work);  dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    guid_BtJoinIFTHEN = OA.makeGuidEdtname('BtJoinIFTHEN');
    transit_whereToText = 'ocrGuid_t ' + guid_BtJoinIFTHEN + '; GUID_ASSIGN_VALUE(' + guid_BtJoinIFTHEN + ', in_TFJiterate->whereToGoWhenFalse);'
    OA.addCustomText(G, nc, transit_whereToText)
    transitText = 'err = transitionBTFork(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, o_work, o_work2);'
    OA.addCustomText(G, nc, transitText)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOtools';   OA.addDataBlocks(G, nc, dbk)

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
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; dbk.localname = 'io_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_tailRecurIterate);  dbk.flight = 'flTAKEOFF';  dbk.localname='o_tailRecurIterate'; OA.addDataBlocks(G,nc,dbk)
    tailRecurSetupt = 'err = tailRecurInitialize(o_tailRecurIterate, '
    tailRecurSetupt += OA.makeGuidEdtname("concludeTailRecursion") + ', io_SPMDglobals); IFEB;'
    OA.addCustomText(G, nc, tailRecurSetupt)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)

    nc += 1;  taskName="tailRecursionIFThen"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; dbk.localname = 'io_tailRecurIterate'
    dbk.addLocalText('ocrGuid_t ' + OA.makeGuidEdtname("concludeTailRecursion") + ' = io_tailRecurIterate->whereToGoWhenDone;')
    OA.addDataBlocks(G,nc,dbk)  # This DBK guid will be needed by the tailRecursionELSE part of this EDT.
    tailRecurCondition_text = 'tailRecurCondition(io_tailRecurIterate)'; OA.addIFconditionText(G,nc,tailRecurCondition_text)
    tailRecurTHEN_text = 'err = tailRecurIfThenClause(io_tailRecurIterate); IFEB;'
    OA.addCustomText(G, nc, tailRecurTHEN_text)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)

    nc += 1;  taskName="tailRecursionELSE"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "tailRecursionIFThen".
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    tailRecurELSE_text = 'err = tailRecurElseClause(io_tailRecurIterate); IFEB;'
    OA.addCustomText(G, nc, tailRecurELSE_text)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)

    nc += 1;  taskName="tailRecurTransitBEGIN"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone3'; OA.addDataBlocks(G, nc, dbk)
    toget2tailTransitEnd_text = '*o_gDone3 = ' + OA.makeGuidEdtname("tailRecurTransitEND") + ';'
    OA.addCustomText(G, nc, toget2tailTransitEnd_text)

    nc += 1;  taskName="nekCG_solveMi"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)

    nc += 1;  taskName="nekCG_beta_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  ALLREDUCTION due to rtz1')

    nc += 1;  taskName="nekCG_beta_stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  This involves add2s1i(p,z,beta,...')
    OA.addCustomText(G, nc, '//SKE-NOTE:  This involves the loop over all call ax_e')

    nc += 1;  taskName="nekCG_axi_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  HALO exchange on w ... gs_op(gsh,w,...)')

    nc += 1;  taskName="nekCG_axi_stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  This involves call add2s2i(w,u,.1,n,...)')
    OA.addCustomText(G, nc, '//SKE-NOTE:  This involves call mask(w)')

    nc += 1;  taskName="nekCG_alpha_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  ALLREDUCTION due to call glsc3i(pap, w,c,p,n,...)')

    nc += 1;  taskName="nekCG_alpha_stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  ALLREDUCTION due to call glsc3i(pap, w,c,p,n,...)')
    OA.addCustomText(G, nc, '//SKE-NOTE:  This involves alpha=rtz1/pap  alphm=-alpha')
    OA.addCustomText(G, nc, '//SKE-NOTE:  This involves call add2s2i(x,p,alpha,n,find,lind)')
    OA.addCustomText(G, nc, '//SKE-NOTE:  This involves call add2s2i(r,w,alphm,n,find,lind)')

    nc += 1;  taskName="nekCG_rtr_start"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, '//SKE-NOTE:  ALLREDUCTION due to call glsc3i(rtr, r,c,r,n,')

    nc += 1;  taskName="nekCG_rtr_stop"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    tailTransitEdn2go_text = 'ocrGuid_t ' + OA.makeGuidEdtname("tailRecurTransitEND") + ' = *in_gDone3;'
    OA.addCustomText(G, nc, tailTransitEdn2go_text)

    nc += 1;  taskName="tailRecurTransitEND"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)

    nc += 1;  taskName="concludeTailRecursion"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone'; OA.addDataBlocks(G, nc, dbk)
    OA.addCustomText(G, nc, 'err = tailRecurConclude(); IFEB;')
    dbk.addLocalText('ocrGuid_t ' + OA.makeGuidEdtname("BtForkTransition_Stop") + ' = *in_gDone;')
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)

    # ==============================================================================
    # ==============================================================================
    # Fork-Join tree for the distribution of MPI ranks
    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", 'NEKOstatics')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')

    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "ConcludeBtForkJoin", "RefTCsum")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('RefTCsum')

    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "BtForkIF", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    btForkIF_global_input_edge=ledg[0]  # This takes only the edge out of the labelled edge.
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "BtForkIF", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    btForkIF_main_input_edge=ledg[0]  # This takes only the edge out of the labelled edge.
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "BtForkIF", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    btForkIF_NEKOstatics = ledg[0]

    ledg = OA.graphAddEdge(G, "ConcludeBtForkJoin", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'; OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
    OA.getEvent(G, ledg).fertile = False

    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True

    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkFOR", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkFOR", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkFOR", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')

    ledg = OA.graphAddEdge(G, "BtForkIF", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    btJoin_gDone_main_input_edge = ledg[0]

    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';  OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    OA.sharedConx(G, ledg[0]).append(btForkIF_main_input_edge)
    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';  OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    OA.sharedConx(G, ledg[0]).append(btForkIF_global_input_edge)
    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';  OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    OA.sharedConx(G, ledg[0]).append(btForkIF_NEKOstatics)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    OA.sharedConx(G, ledg[0]).append(btJoin_gDone_main_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "BtForkTransition_Stop", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')

    #This is where we stitch in the tail recursion.
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "setupTailRecursion", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "setupTailRecursion", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "nekMultiplicity_start", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "nekMultiplicity_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "nekMultiplicity_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')

    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')

    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')

    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')

    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')

    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')


    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    OA.getEvent(G, ledg).fertile = False


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
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "concludeTailRecursion", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone'), 'DBK')

    ledg=OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TailRecurIterate')
    tailRecur_main_iteration_edge=ledg[0]  # This takes only the edge out of the labelled edge.
    ledg=OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    tailRecur_main_globald_edge=ledg[0]  # This takes only the edge out of the labelled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    tailRecur_NEKOstatics_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    tailRecur_NEKOglobals_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    tailRecur_NEKOtools_edge=ledg[0]  # This takes only the edge out of the labeled edge.

    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('TailRecurIterate'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('TailRecurIterate'), 'DBK')
    OA.sharedConx(G, ledg[0]).append(tailRecur_main_iteration_edge)

    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('SPMDGlobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "tailRecurTransitEND", "TailRecurIterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('TailRecurIterate'), 'DBK')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "tailRecurTransitEND", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('SPMDGlobals'), 'DBK')

    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "gDone3")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('gDone3'), 'DBK')

    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    OA.getEvent(G, ledg).fertile = False

    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('SPMDGlobals'), 'DBK')
    OA.sharedConx(G, ledg[0]).append(tailRecur_main_globald_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    OA.sharedConx(G, ledg[0]).append(tailRecur_NEKOstatics_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    OA.sharedConx(G, ledg[0]).append(tailRecur_NEKOglobals_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    OA.sharedConx(G, ledg[0]).append(tailRecur_NEKOtools_edge)

    ledg=OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecursionELSE", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True

    ledg = OA.graphAddEdge(G, "tailRecursionELSE", "concludeTailRecursion", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('SPMDGlobals'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "tailRecursionELSE", "concludeTailRecursion", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "tailRecursionELSE", "concludeTailRecursion", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "tailRecursionELSE", "concludeTailRecursion", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
    OA.getEvent(G, ledg).fertile = False

    ledg=OA.graphAddEdge(G, "concludeTailRecursion", "BtForkTransition_Stop", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('SPMDGlobals'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg=OA.graphAddEdge(G, "concludeTailRecursion", "BtForkTransition_Stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOstatics'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg=OA.graphAddEdge(G, "concludeTailRecursion", "BtForkTransition_Stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOglobals'), 'DBK')
    OA.getEvent(G, ledg).fertile = False
    ledg=OA.graphAddEdge(G, "concludeTailRecursion", "BtForkTransition_Stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('NEKOtools'), 'DBK')
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
        if True:
            erri = OA.outputDot(G, "z_mpiranks_inOcr.dot")
            if erri: break
        if True:
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

