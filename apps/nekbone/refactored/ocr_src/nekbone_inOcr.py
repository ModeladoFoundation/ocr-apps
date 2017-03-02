import sys
sys.path.append('../../../../hll/autogen/tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    OA.GBL.insert_debug_code = 0

    OA.GBL.app_ocr_util_filename = 'app_ocr_util.h'

    G=OA.GraphTool.DiGraph()

    # = The following are slots count and position overrides used for reduction
    # = If they are changed also change their values in nekbone_inOCR.py
    # = The number is set to +1 the number of slots needed by the receiving EDT
    #   (e.g. nekCGstep0_stop).  The +1 accommodates for the reduction sum the
    #   receiving EDT will get.
    REDUC_SLOT_4CGstep0 =  7 +1
    REDUC_SLOT_4Beta    = 18 +1
    REDUC_SLOT_4Alpha   = 18 +1
    REDUC_SLOT_4Rtr     = 18 +1

    # ==============================================================================
    # ----- GLOBALS
    OA.setupGraph(G,"nekbone_inOcr")

    OA.addHeader(G, '#include "treeForkJoin.h" ')
    OA.addHeader(G, '#include "spmd_global_data.h" ')
    OA.addHeader(G, '#include "tailrecursion.h" ')
    OA.addHeader(G, '#include "neko_globals.h" ')
    OA.addHeader(G, '#include "nekos_tools.h" ')
    OA.addHeader(G, '#include "nekbone_setup.h" ')
    OA.addHeader(G, '#include "nekbone_cg.h" ')
    OA.addHeader(G, '#include "reduction.h" ')
    OA.addHeader(G, '#include "neko_reduction.h" ')
    OA.addHeader(G, '#include "neko_halo.h" ')
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

    dbk = OA.ocrDataBlock(); dbk.name = 'nekLGLEL'; dbk.count='(sz_nekLGLEL+1)*sizeof(unsigned int)'; dbk.type='unsigned int'
    dbk_nekLGLEL = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nekGLO_NUM'; dbk.count='(sz_nekGLO_NUM+1)*sizeof(unsigned long)'; dbk.type='unsigned long'
    dbk_nekGLO_NUM = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nekC'; dbk.count='(sz_C)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekC = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wA'; dbk.count='(pdof2D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_wA = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wC'; dbk.count='(pdof2D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_wC = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wD'; dbk.count='(pdof2D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_wD = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wB'; dbk.count='(pdof+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_wB = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wZ'; dbk.count='(pdof+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_wZ = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wW'; dbk.count='(2*pdof+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_wW = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wZd'; dbk.count='(pdof+1)*sizeof(double)'; dbk.type='double'
    dbk_wZd = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_wWd'; dbk.count='(2*pdof+1)*sizeof(double)'; dbk.type='double'
    dbk_wWd = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nek_G1'; dbk.count='(io_NEKOstatics->pDOFmax3D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_G1 = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_G4'; dbk.count='(io_NEKOstatics->pDOFmax3D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_G4 = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_G6'; dbk.count='(io_NEKOstatics->pDOFmax3D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_G6 = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nek_dxm1'; dbk.count='(io_NEKOstatics->pDOFmax2D+1)*sizeof(BLAS_REAL_TYPE)'; dbk.type='BLAS_REAL_TYPE'
    dbk_dxm1 = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nek_dxTm1'; dbk.count='(io_NEKOstatics->pDOFmax2D+1)*sizeof(BLAS_REAL_TYPE)'; dbk.type='BLAS_REAL_TYPE'
    dbk_dxTm1 = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nekF'; dbk.count='(sz_F)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekF = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nekX'; dbk.count='(io_NEKOglobals->pDOF3DperR+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekX = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nekR'; dbk.count='(io_NEKOglobals->pDOF3DperR+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekR = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nekW'; dbk.count='(io_NEKOglobals->pDOF3DperR+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekW = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nekP'; dbk.count='(io_NEKOglobals->pDOF3DperR+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekP = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nekZ'; dbk.count='(io_NEKOglobals->pDOF3DperR+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekZ = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nekCGscalars'; dbk.count=1; dbk.type='NEKO_CGscalars_t'
    dbk_nekCGscalars = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'nekUR'; dbk.count='(io_NEKOstatics->pDOFmax3D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekUR = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nekUS'; dbk.count='(io_NEKOstatics->pDOFmax3D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekUS = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'nekUT'; dbk.count='(io_NEKOstatics->pDOFmax3D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_nekUT = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'AItemp'; dbk.count='(io_NEKOstatics->pDOFmax3D+1)*sizeof(NBN_REAL)'; dbk.type='NBN_REAL'
    dbk_AItemp = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'CGtimes'; dbk.count=1; dbk.type='NEKO_CGtimings_t'
    dbk_CGtimes = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'reducSharedRef'; dbk.count=1; dbk.type='Reduct_shared_t'
    dbk_reducSharedRef = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'reducShare'; dbk.count=1; dbk.type='Reduct_shared_t'
    dbk_reducShare = dbk
    dbk = OA.ocrDataBlock(); dbk.name = 'reducPrivate'; dbk.count=1; dbk.type='reductionPrivate_t'
    dbk_reducPrivate = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'riValues4multi'; dbk.count='length_riValues4multi * sizeof(rankIndexedValue_t)'; dbk.type='rankIndexedValue_t'
    dbk_riValues4multi = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'riValues4setF'; dbk.count='length_riValues4setF * sizeof(rankIndexedValue_t)'; dbk.type='rankIndexedValue_t'
    dbk_riValues4setF = dbk

    dbk = OA.ocrDataBlock(); dbk.name = 'riValues4axi'; dbk.count='length_riValues4axi * sizeof(rankIndexedValue_t)'; dbk.type='rankIndexedValue_t'
    dbk_riValues4axi = dbk

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
    dbk = copy.deepcopy(dbk_reducSharedRef); dbk.flight = 'flTAKEOFF'; dbk.localname='o_reductSharedRef'; OA.addDataBlocks(G, nc, dbk)   # To FinalEDT
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flTAKEOFF'; dbk.localname='o_reductShare'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);    dbk.flight = 'flTAKEOFF'; dbk.localname='o_gDone'; OA.addDataBlocks(G, nc, dbk)   # To SetupBtForkJoin
    dbk = copy.deepcopy(dbk_spmd_globals);  dbk.flight = 'flTAKEOFF'; dbk.localname='o_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)   # To SetupBtForkJoin
    toFinalt = 'GUID_ASSIGN_VALUE(*o_gDone, ' + OA.makeGuidEdtname("finalEDT") + ');'
    OA.addCustomText(G, nc, toFinalt)
    init_globald = 'init_SPMDglobals(o_SPMDglobals);'; OA.addCustomText(G, nc, init_globald)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAKEOFF'; dbk.localname='o_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    OA.addCustomText(G, nc, 'err = init_NEKOstatics(o_NEKOstatics, depv[0].ptr); IFEB;')
    OA.addCustomText(G, nc, 'err = setup_SPMD_using_NEKOstatics(o_NEKOstatics, o_SPMDglobals); IFEB;')
    mainEdt_reductText = 'err = NEKO_mainEdt_reduction(o_NEKOstatics->Rtotal, 1, o_reductSharedRef, o_reductShare); IFEB;'
    OA.addCustomText(G, nc, mainEdt_reductText)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)  # From ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_reducSharedRef); dbk.flight = 'flLANDING'; dbk.localname='in_reducShare'; OA.addDataBlocks(G, nc, dbk)
    OA.addCustomText(G, nc, 'err = NEKO_finalEdt_reduction(in_reducShare); IFEB;')
    finalEDTt = 'err = nekbone_finalEDTt(); IFEB;'
    OA.addCustomText(G, nc, finalEDTt)

    nc += 1;  taskName="SetupBtForkJoin"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)  # Toward ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; dbk.localname='io_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_refwork); dbk.flight = 'flTAKEOFF'; dbk.localname='o_refTCsum'; OA.addDataBlocks(G, nc, dbk) # To ConcludeBtForkJoin
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.localname='o_TFJiterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkIF
    setupt = 'err = setupBtForkJoin(OA_edtTypeNb, OA_DBG_thisEDT, o_TFJiterate, '
    setupt += OA.makeGuidEdtname("ConcludeBtForkJoin") + ', io_SPMDglobals, &o_refTCsum->result);'
    OA.addCustomText(G, nc, setupt)
    dbk = copy.deepcopy(dbk_nekoStatics);  dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)

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
    dbk.addLocalText('unsigned int rankID = in_TFJiterate->low - 1;')
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_TFJiterate'; OA.addDataBlocks(G, nc, dbk) # To BtForkFOR or BtForkELSE
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)    # To BtJoinIFTHEN
    #dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF';  <-- This is the NULL_GUID used to link to the ELSE clause
    OA.addIFconditionText(G, nc, 'conditionBtFork(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate)')
    iterText = 'err = btForkThen(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_TFJiterate, o_gDone);'
    OA.addCustomText(G, nc, iterText)
    copy_SPMDglobals = 'copy_SPMDglobals(in_SPMDglobals, o_SPMDglobals);'; OA.addCustomText(G, nc, copy_SPMDglobals)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOstatics';
    dbk.addLocalText('unsigned long pdID = calcPDid_S(in_NEKOstatics->OCR_affinityCount, rankID);')
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0, hintDBK, *pHintDBK=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(pdID, &hintEDT, &pHintEDT); IFEB;')
    dbk.addLocalText('err = ocrXgetDbkHint(pdID, &hintDBK, &pHintDBK); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    copy_NEKOstatics = 'copy_NEKOstatics(in_NEKOstatics, o_NEKOstatics);'; OA.addCustomText(G, nc, copy_NEKOstatics)
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flLANDING'; dbk.localname = 'in_reducShare'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_reducShare'; OA.addDataBlocks(G, nc, dbk)
    copy_reducShare = 'copy_Reduct_shared(in_reducShare, o_reducShare);'; OA.addCustomText(G, nc, copy_reducShare)

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
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flLANDING'; dbk.localname = 'in_reducShare'; dbk.delayReleaseDestroy = True
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_reducShare'; OA.addDataBlocks(G, nc, dbk)
    copy_reducShare = 'copy_Reduct_shared(in_reducShare, o_reducShare);'; OA.addCustomText(G, nc, copy_reducShare)

    nc += 1;  taskName="BtForkELSE"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "btFork".
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flLANDING'; dbk.localname = 'in_TFJiterate';
    dbk.addLocalText('unsigned int rankID = in_TFJiterate->low - 1;')  # This does nothing because BtForkIF takes over.  Here for completion.
    OA.addDataBlocks(G,nc,dbk)  # From self BtForkIF
    dbk = copy.deepcopy(dbk_TFJiterate); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_TFJiterate';  OA.addDataBlocks(G, nc, dbk)  # To BtForkTransition
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flLANDING'; dbk.localname = 'in_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)  # To BtJoinIFTHEN
    btForkElseText = 'err = btForkElse(OA_edtTypeNb, OA_DBG_thisEDT, in_TFJiterate, ' + OA.makeGuidEdtname("BtJoinIFTHEN") +', o_TFJiterate, o_gDone);'
    OA.addCustomText(G, nc, btForkElseText)
    copy_SPMDglobals = 'copy_SPMDglobals(in_SPMDglobals, o_SPMDglobals);'
    OA.addCustomText(G, nc, copy_SPMDglobals)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    copy_NEKOstatics = 'copy_NEKOstatics(in_NEKOstatics, o_NEKOstatics);'; OA.addCustomText(G, nc, copy_NEKOstatics)
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flLANDING'; dbk.localname = 'in_reducShare'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flTAKEOFF'; dbk.hint='pHintDBK'; dbk.localname = 'o_reducShare'; OA.addDataBlocks(G, nc, dbk)
    copy_NEKOstatics = 'copy_Reduct_shared(in_reducShare, o_reducShare);'; OA.addCustomText(G, nc, copy_NEKOstatics)

    nc += 1;  taskName="BtForkTransition_Start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_TFJiterate);  dbk.flight = 'flTAGO'; dbk.localname = 'io_TFJiterate'
    dbk.addLocalText('    unsigned int rankID = io_TFJiterate->low - 1;')
    OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; dbk.localname = 'io_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone);   dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone'; OA.addDataBlocks(G, nc, dbk)
    transitStart_text = '*o_gDone = ' + OA.makeGuidEdtname("BtForkTransition_Stop") + ';'
    OA.addCustomText(G, nc, transitStart_text)

    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO';    dbk.localname = 'io_NEKOstatics';
    dbk.addLocalText('    unsigned int sz_nekLGLEL = io_NEKOstatics->Etotal;')
    dbk.addLocalText('    unsigned int sz_nekGLO_NUM = io_NEKOstatics->pDOF3DperRmax;')
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOglobals';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekLGLEL); dbk.flight = 'flHOP'; dbk.localname = 'o_lglel'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekGLO_NUM); dbk.flight = 'flHOP'; dbk.localname = 'o_glo_num'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducShare); dbk.flight = 'flLANDING'; dbk.localname = 'in_reducShare';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_reducPrivate';  OA.addDataBlocks(G, nc, dbk)
    nekoGlobals_init = 'err = init_NEKOglobals(io_NEKOstatics, rankID, o_NEKOglobals); IFEB;'
    OA.addCustomText(G, nc, nekoGlobals_init)
    nekbone_setup_txt= 'err = nekbone_setup(io_NEKOstatics, o_NEKOglobals, o_lglel, o_glo_num); IFEB;'
    OA.addCustomText(G, nc, nekbone_setup_txt)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone2'; OA.addDataBlocks(G, nc, dbk)
    toget2nekMulti_text = '*o_gDone2 = ' + OA.makeGuidEdtname("setupTailRecursion") + ';'
    OA.addCustomText(G, nc, toget2nekMulti_text)
    forkTransit_reduc_Text = 'err = NEKO_ForkTransit_reduction(rankID, in_reducShare, o_reducPrivate); IFEB;'
    OA.addCustomText(G, nc, forkTransit_reduc_Text)

    nc += 1;  taskName="channelExchange_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics';
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOglobals';
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    nekoTools_init = 'err = init_NEKOtools(o_NEKOtools, *io_NEKOstatics, in_NEKOglobals->rankID, in_NEKOglobals->pDOF); IFEB;'
    OA.addCustomText(G, nc, nekoTools_init)
    guid_channelExchange_stop = OA.makeGuidEdtname('channelExchange_stop')
    haloex_start_text = 'err = start_channelExchange(OA_DEBUG_OUTVARS, o_NEKOtools, '
    haloex_start_text += '&io_NEKOstatics->haloLabeledGuids[0], &' + guid_channelExchange_stop + ', '
    haloex_start_text += 'in_NEKOglobals, o_NEKOglobals); IFEB;'
    OA.addCustomText(G, nc, haloex_start_text)

    nc += 1;  taskName="channelExchange_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = 'SLOTCNT_total_channelExchange'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics';
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flLANDING'; dbk.localname = 'in_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_NEKOglobals';
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    haloex_stop_text = 'err = stop_channelExchange(OA_DEBUG_OUTVARS, io_NEKOtools, depv, '
    haloex_stop_text +='in_NEKOglobals, o_NEKOglobals); IFEB;'
    OA.addCustomText(G, nc, haloex_stop_text)

    nc += 1;  taskName="nekMultiplicity_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics';
    dbk.addLocalText('    Idz sz_C = io_NEKOstatics->pDOF3DperRmax+1;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    dbk.addLocalText('    Triplet Rlattice = {io_NEKOstatics->Rx, io_NEKOstatics->Ry, io_NEKOstatics->Rz};')
    dbk.addLocalText('    Triplet Elattice = {io_NEKOstatics->Ex, io_NEKOstatics->Ey, io_NEKOstatics->Ez};')
    dbk.addLocalText('    const Idz length_riValues4multi = calculate_length_rankIndexedValue(io_NEKOglobals->pDOF, Elattice);')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_C'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_riValues4multi); dbk.flight = 'flHOP'; dbk.localname = 'riValues4multi'; OA.addDataBlocks(G,nc,dbk)
    nekbone_set_multiplicity_txt1= 'err = nekbone_set_multiplicity_start(io_NEKOglobals, o_C); IFEB;'
    OA.addCustomText(G, nc, nekbone_set_multiplicity_txt1)
    guid_nekMultiplicity_stop = OA.makeGuidEdtname('nekMultiplicity_stop')
    halomulti_txt = 'err = start_halo_multiplicity(OA_DEBUG_OUTVARS, io_NEKOtools, '
    halomulti_txt += 'Rlattice,Elattice, io_NEKOglobals, sz_C, o_C, riValues4multi, &'
    halomulti_txt += guid_nekMultiplicity_stop +'); IFEB;'
    OA.addCustomText(G, nc, halomulti_txt);

    nc += 1;  taskName="nekMultiplicity_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = 'SLOTCNT_total_channels4multiplicity'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics';
    dbk.addLocalText('    unsigned int pdof = io_NEKOstatics->pDOF_max;')
    dbk.addLocalText('    unsigned int pdof2D = io_NEKOstatics->pDOF_max * io_NEKOstatics->pDOF_max;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'io_gDone2'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; dbk.localname = 'io_C'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    halomulti_stop = 'err = stop_halo_multiplicity(OA_DEBUG_OUTVARS, io_NEKOtools, depv, io_C); IFEB;'
    OA.addCustomText(G, nc, halomulti_stop)
    nekbone_set_multiplicity_txt2 = 'err = nekbone_set_multiplicity_stop(io_NEKOglobals, io_C); IFEB;'
    OA.addCustomText(G, nc, nekbone_set_multiplicity_txt2)

    dbk = copy.deepcopy(dbk_wA); dbk.flight = 'flHOP'; dbk.localname = 'in_wA'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wC); dbk.flight = 'flHOP'; dbk.localname = 'in_wC'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wD); dbk.flight = 'flHOP'; dbk.localname = 'in_wD'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wB); dbk.flight = 'flHOP'; dbk.localname = 'in_wB'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wZ); dbk.flight = 'flHOP'; dbk.localname = 'in_wZ'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wW); dbk.flight = 'flHOP'; dbk.localname = 'in_wW'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wZd);dbk.flight = 'flHOP'; dbk.localname = 'in_wZd'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_wWd);dbk.flight = 'flHOP'; dbk.localname = 'in_wWd'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_G1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_G4'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_G6'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_dxm1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_dxTm1'; OA.addDataBlocks(G,nc,dbk)

    nekbone_proxy_setup = 'err = nekbone_proxy_setup(io_NEKOstatics, io_NEKOglobals,'
    nekbone_proxy_setup += 'in_wA,in_wC,in_wD, in_wB,in_wZ,in_wW, in_wZd,in_wWd,'
    nekbone_proxy_setup += 'o_G1,o_G4,o_G6, o_dxm1,o_dxTm1'
    nekbone_proxy_setup += '); IFEB;'
    OA.addCustomText(G, nc, nekbone_proxy_setup)

    nekMulti_stop_go2cg1_text = 'ocrGuid_t ' + OA.makeGuidEdtname("setupTailRecursion") + ' = *io_gDone2;'
    OA.addCustomText(G, nc, nekMulti_stop_go2cg1_text)

    nc += 1;  taskName="nekSetF_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    dbk.addLocalText('    Triplet Rlattice = {io_NEKOstatics->Rx, io_NEKOstatics->Ry, io_NEKOstatics->Rz};')
    dbk.addLocalText('    Triplet Elattice = {io_NEKOstatics->Ex, io_NEKOstatics->Ey, io_NEKOstatics->Ez};')
    dbk.addLocalText('    const Idz length_riValues4setF = calculate_length_rankIndexedValue(io_NEKOglobals->pDOF, Elattice);')
    dbk.addLocalText('    Idz sz_F = io_NEKOglobals->pDOF3DperR+1;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekF); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekF'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_riValues4setF); dbk.flight = 'flHOP'; dbk.localname = 'riValues4setF'; OA.addDataBlocks(G,nc,dbk)
    guid_nekSetF_stop = OA.makeGuidEdtname('nekSetF_stop')
    nekSetF_start_txt = 'err = nekbone_set_f_start(io_NEKOstatics, io_NEKOglobals, o_nekF);IFEB;'
    OA.addCustomText(G, nc, nekSetF_start_txt)
    haloSetF_txt = 'err = start_halo_setf(OA_DEBUG_OUTVARS, io_NEKOtools, '
    haloSetF_txt += 'Rlattice,Elattice, io_NEKOglobals, sz_F, o_nekF, riValues4setF, &'
    haloSetF_txt += guid_nekSetF_stop +'); IFEB;'
    OA.addCustomText(G, nc, haloSetF_txt);

    nc += 1;  taskName="nekSetF_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = 'SLOTCNT_total_channels4setf'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; dbk.localname = 'io_C'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekF); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekF'; OA.addDataBlocks(G, nc, dbk)
    haloSetF_stop = 'err = stop_halo_setf(OA_DEBUG_OUTVARS, io_NEKOtools, depv, io_nekF); IFEB;'
    OA.addCustomText(G, nc, haloSetF_stop)
    nekSetF_stop_txt2 = 'err = nekbone_set_f_stop(io_NEKOglobals, io_C, io_nekF); IFEB;'
    OA.addCustomText(G, nc, nekSetF_stop_txt2)

    nc += 1;  taskName="nekCGstep0_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; dbk.localname = 'io_C'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekF); dbk.flight = 'flLANDING'; dbk.localname = 'in_nekF'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekR'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate';
    dbk.user2destroyORrelease=True; dbk.localnameGuid='guid_reducPrivate';
    dbk.addLocalText('    //DBK reducPrivate will be released by reductionLaunch() in nekbone_CGstep0_start().')
    OA.addDataBlocks(G,nc,dbk)
    guid_nekCGstep0_stop = OA.makeGuidEdtname('nekCGstep0_stop')
    nekCGstep0_start_txt = 'err = nekbone_CGstep0_start(io_NEKOstatics, io_NEKOglobals, in_nekF, io_C, o_nekR, '
    nekCGstep0_start_txt += 'guid_reducPrivate, io_reducPrivate, REDUC_SLOT_4CGstep0-1, ' + guid_nekCGstep0_stop + '); IFEB;'
    OA.addCustomText(G, nc, nekCGstep0_start_txt)

    nc += 1;  taskName="nekCGstep0_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = REDUC_SLOT_4CGstep0
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone2); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone2'; OA.addDataBlocks(G,nc,dbk)
    CGstep0_2go_text = 'ocrGuid_t ' + OA.makeGuidEdtname("setupTailRecursion") + ' = *in_gDone2;'
    OA.addCustomText(G, nc, CGstep0_2go_text)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekX'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekW'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekZ'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_sum = depv[REDUC_SLOT_4CGstep0-1];  //From nekbone_CGstep0_start().')
    dbk.addLocalText('    ocrGuid_t in_sum_guid = IN_BY_USER_depv_sum.guid;')
    dbk.addLocalText('    ReducSum_t * in_sum = IN_BY_USER_depv_sum.ptr;')
    OA.addDataBlocks(G,nc,dbk)
    CGstep0_stop_txt = 'err = nekbone_CGstep0_stop(io_NEKOstatics, io_NEKOglobals, o_nekX, o_nekW, o_nekP, o_nekZ, o_nekCGscalars, o_CGtimes, '
    CGstep0_stop_txt += 'in_sum_guid, in_sum); IFEB;'
    OA.addCustomText(G, nc, CGstep0_stop_txt)

    nc += 1;  taskName="BtForkTransition_Stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
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
    dbk = copy.deepcopy(dbk_reducPrivate);dbk.flight = 'flLANDING'; dbk.localname = 'in_reducPrivate';OA.addDataBlocks(G, nc, dbk)
    OA.addCustomText(G, nc, '//2016Nov15:No known destructor for in_reducPrivate')
    OA.addCustomText(G, nc, '//          in_reducPrivate Guid map will be uniquely destroyed in the finalEdt.')
    BtForkTransit_Stopt = 'err = nekbone_BtForkTransition_Stop(in_NEKOglobals); IFEB;'
    OA.addCustomText(G, nc, BtForkTransit_Stopt)

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
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; dbk.localname = 'io_SPMDglobals'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_tailRecurIterate);  dbk.flight = 'flTAKEOFF';  dbk.localname='o_tailRecurIterate'; OA.addDataBlocks(G,nc,dbk)
    tailRecurSetupt = 'err = tailRecurInitialize(o_tailRecurIterate, '
    tailRecurSetupt += OA.makeGuidEdtname("concludeTailRecursion") + ', io_SPMDglobals); IFEB;'
    OA.addCustomText(G, nc, tailRecurSetupt)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars); dbk.flight = 'flLANDING'; dbk.localname = 'in_nekCGscalars'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    setupTailRecursion ='err = nekbone_setupTailRecusion(io_NEKOglobals, in_nekCGscalars, o_nekCGscalars);IFEB;'
    OA.addCustomText(G, nc, setupTailRecursion)

    nc += 1;  taskName="tailRecursionIFThen"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; dbk.localname = 'io_tailRecurIterate'
    dbk.addLocalText('ocrGuid_t ' + OA.makeGuidEdtname("concludeTailRecursion") + ' = io_tailRecurIterate->whereToGoWhenDone;')
    OA.addDataBlocks(G,nc,dbk)  # This DBK guid will be needed by the tailRecursionELSE part of this EDT.
    tailRecurCondition_text = 'tailRecurCondition(io_tailRecurIterate)'; OA.addIFconditionText(G,nc,tailRecurCondition_text)
    tailRecurTHEN_text = 'err = tailRecurIfThenClause(io_tailRecurIterate); IFEB;'
    OA.addCustomText(G, nc, tailRecurTHEN_text)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAGO'; dbk.localname = 'io_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)

    nc += 1;  taskName="tailRecursionELSE"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    # DBK local names are taken care off by the IF-THEN edt called "tailRecursionIFThen".
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    tailRecurELSE_text = 'err = tailRecurElseClause(io_tailRecurIterate); IFEB;'
    OA.addCustomText(G, nc, tailRecurELSE_text)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes);dbk.flight = 'flLANDING'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducPrivate);dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    tailRecursionELSEt = 'err = nekbone_tailRecursionELSE(io_CGtimes); IFEB;'  # Check tailRecursionIFThen for io_CGtimes
    OA.addCustomText(G, nc, tailRecursionELSEt)

    nc += 1;  taskName="tailRecurTransitBEGIN"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; dbk.localname = 'io_tailRecurIterate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_gDone3'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flLANDING'; dbk.localname = 'in_nekCGscalars'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    toget2tailTransitEnd_text = '*o_gDone3 = ' + OA.makeGuidEdtname("tailRecurTransitEND") + ';'
    OA.addCustomText(G, nc, toget2tailTransitEnd_text)
    nekCG_tailTransitBegin_text = 'err = nekbone_tailTransitBegin(io_tailRecurIterate->current, in_nekCGscalars, o_nekCGscalars, in_CGtimes, o_CGtimes); IFEB;'
    OA.addCustomText(G, nc, nekCG_tailTransitBegin_text)

    nc += 1;  taskName="nekCG_solveMi"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAGO'; dbk.localname = 'io_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; dbk.localname = 'io_G1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; dbk.localname = 'io_G4'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; dbk.localname = 'io_G6'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; dbk.localname = 'io_dxm1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; dbk.localname = 'io_dxTm1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekC'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekW'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekX'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flLANDING'; dbk.localname = 'in_nekZ'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekZ'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    nekCG_solveMi_text = 'err = nekbone_solveMi(io_NEKOstatics, io_NEKOglobals, io_nekR, o_nekZ, in_CGtimes, o_CGtimes); IFEB;'
    OA.addCustomText(G, nc, nekCG_solveMi_text)

    nc += 1;  taskName="nekCG_beta_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flLANDING'; dbk.localname = 'in_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekC'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekZ'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate';
    dbk.user2destroyORrelease=True; dbk.localnameGuid='guid_reducPrivate';
    dbk.addLocalText('    //DBK reducPrivate will be released by reductionLaunch() in nekbone_beta_start().')
    OA.addDataBlocks(G,nc,dbk)
    guid_nekCG_beta_stop = OA.makeGuidEdtname('nekCG_beta_stop')
    nekCG_beta_start_text = 'err = nekbone_beta_start(io_NEKOstatics, io_NEKOglobals, in_nekCGscalars, o_nekCGscalars, '
    nekCG_beta_start_text += 'io_nekR, io_nekC, io_nekZ, in_CGtimes,o_CGtimes, '
    nekCG_beta_start_text += 'guid_reducPrivate, io_reducPrivate, REDUC_SLOT_4Beta-1, ' + guid_nekCG_beta_stop + '); IFEB;'
    OA.addCustomText(G, nc, nekCG_beta_start_text)

    nc += 1;  taskName="nekCG_beta_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = REDUC_SLOT_4Beta
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flLANDING'; dbk.localname = 'in_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flLANDING'; dbk.localname = 'in_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekZ'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_sum = depv[REDUC_SLOT_4Beta-1];  //From nekCG_beta_start().')
    dbk.addLocalText('    ocrGuid_t in_sum_guid = IN_BY_USER_depv_sum.guid;')
    dbk.addLocalText('    ReducSum_t * in_sum = IN_BY_USER_depv_sum.ptr;')
    OA.addDataBlocks(G,nc,dbk)
    nekCG_beta_stop_text = 'err = nekbone_beta_stop(io_NEKOstatics, io_NEKOglobals, in_nekCGscalars, o_nekCGscalars, '
    nekCG_beta_stop_text += 'in_nekP, io_nekZ, o_nekP, in_CGtimes,o_CGtimes, in_sum_guid, in_sum); IFEB;'
    OA.addCustomText(G, nc, nekCG_beta_stop_text)

    nc += 1;  taskName="nekCG_axi_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    dbk.addLocalText('    Triplet Rlattice = {io_NEKOstatics->Rx, io_NEKOstatics->Ry, io_NEKOstatics->Rz};')
    dbk.addLocalText('    Triplet Elattice = {io_NEKOstatics->Ex, io_NEKOstatics->Ey, io_NEKOstatics->Ez};')
    dbk.addLocalText('    const Idz length_riValues4axi = calculate_length_rankIndexedValue(io_NEKOglobals->pDOF, Elattice);')
    dbk.addLocalText('    Idz sz_nekW = io_NEKOglobals->pDOF3DperR+1;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; dbk.localname = 'io_G1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; dbk.localname = 'io_G4'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; dbk.localname = 'io_G6'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; dbk.localname = 'io_dxm1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; dbk.localname = 'io_dxTm1'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekW'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekUR); dbk.flight = 'flHOP'; dbk.localname = 'nekUR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekUS); dbk.flight = 'flHOP'; dbk.localname = 'nekUS'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekUT); dbk.flight = 'flHOP'; dbk.localname = 'nekUT'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_AItemp); dbk.flight = 'flHOP'; dbk.localname = 'AItemp'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_riValues4axi); dbk.flight = 'flHOP'; dbk.localname = 'riValues4axi'; OA.addDataBlocks(G,nc,dbk)
    nekCG_axi_start = 'err = nekbone_ai_start(io_NEKOstatics, io_NEKOglobals, io_nekW,io_nekP, '
    nekCG_axi_start += 'nekUR,nekUS,nekUT, io_G1,io_G4,io_G6,io_dxm1,io_dxTm1, AItemp, '
    nekCG_axi_start += 'in_CGtimes,o_CGtimes); IFEB;'
    OA.addCustomText(G, nc, nekCG_axi_start)
    guid_nekCG_axi_stop = OA.makeGuidEdtname('nekCG_axi_stop')
    haloAxi_txt = 'err = start_halo_ai(OA_DEBUG_OUTVARS, io_NEKOtools, '
    haloAxi_txt += 'Rlattice,Elattice, io_NEKOglobals, sz_nekW, io_nekW, riValues4axi, &'
    haloAxi_txt += guid_nekCG_axi_stop +'); IFEB;'
    OA.addCustomText(G, nc, haloAxi_txt);

    nc += 1;  taskName="nekCG_axi_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = 'SLOTCNT_total_channels4ax'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOtools'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekW'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate'; OA.addDataBlocks(G,nc,dbk)
    haloAxi_stop = 'err = stop_halo_ai(OA_DEBUG_OUTVARS, io_NEKOtools, depv, io_nekW); IFEB;'
    OA.addCustomText(G, nc, haloAxi_stop)
    nekCG_axi_stop = 'err = nekbone_ai_stop(io_NEKOstatics, io_NEKOglobals, io_nekP, io_nekW, in_CGtimes,o_CGtimes); IFEB;'
    OA.addCustomText(G, nc, nekCG_axi_stop)

    nc += 1;  taskName="nekCG_alpha_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAGO'; dbk.localname = 'io_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekC'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekW'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate';
    dbk.user2destroyORrelease=True; dbk.localnameGuid='guid_reducPrivate';
    dbk.addLocalText('    //DBK reducPrivate will be released by reductionLaunch() in nekbone_alpha_start().')
    OA.addDataBlocks(G,nc,dbk)
    guid_nekCG_alpha_stop = OA.makeGuidEdtname('nekCG_alpha_stop')
    nekCG_alpha_start = 'err = nekbone_alpha_start(io_NEKOstatics, io_NEKOglobals, io_nekW, '
    nekCG_alpha_start += 'io_nekC, io_nekP, in_CGtimes,o_CGtimes, '
    nekCG_alpha_start += 'guid_reducPrivate, io_reducPrivate, REDUC_SLOT_4Alpha-1,' + guid_nekCG_alpha_stop + '); IFEB;'
    OA.addCustomText(G, nc, nekCG_alpha_start )

    nc += 1;  taskName="nekCG_alpha_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = REDUC_SLOT_4Alpha
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flLANDING'; dbk.localname = 'in_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekP'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flLANDING'; dbk.localname = 'in_nekR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekW'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flLANDING'; dbk.localname = 'in_nekX'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekX'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_sum = depv[REDUC_SLOT_4Alpha-1];  //From nekCG_alpha_start().')
    dbk.addLocalText('    ocrGuid_t in_sum_guid = IN_BY_USER_depv_sum.guid;')
    dbk.addLocalText('    ReducSum_t * in_sum = IN_BY_USER_depv_sum.ptr;')
    OA.addDataBlocks(G,nc,dbk)
    nekCG_alpha_stop = 'err = nekbone_alpha_stop(io_NEKOstatics, io_NEKOglobals, in_nekCGscalars, o_nekCGscalars, '
    nekCG_alpha_stop += 'in_nekX,io_nekP,o_nekX, in_nekR,io_nekW,o_nekR, in_CGtimes,o_CGtimes, in_sum_guid, in_sum); IFEB;'
    OA.addCustomText(G, nc, nekCG_alpha_stop)

    nc += 1;  taskName="nekCG_rtr_start"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('    ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('    err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flTAGO';OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekC'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; dbk.localname = 'io_nekR'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; dbk.localname = 'io_reducPrivate';
    dbk.user2destroyORrelease=True; dbk.localnameGuid='guid_reducPrivate';
    dbk.addLocalText('    //DBK reducPrivate will be released by reductionLaunch() in nekbone_rtr_start().')
    OA.addDataBlocks(G,nc,dbk)
    guid_nekCG_rtr_stop = OA.makeGuidEdtname('nekCG_rtr_stop')
    nekCG_rtr_start = 'err = nekbone_rtr_start(io_NEKOstatics, io_NEKOglobals, '
    nekCG_rtr_start += 'io_nekR, io_nekC, in_CGtimes,o_CGtimes, '
    nekCG_rtr_start += 'guid_reducPrivate, io_reducPrivate, REDUC_SLOT_4Rtr-1, ' + guid_nekCG_rtr_stop + '); IFEB;'
    OA.addCustomText(G, nc, nekCG_rtr_start )

    nc += 1;  taskName="nekCG_rtr_stop"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    OA.getMyTask(G,nc).depc = REDUC_SLOT_4Rtr
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone3); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone3'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flLANDING'; dbk.localname = 'in_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars';   OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flLANDING'; dbk.localname = 'in_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO';
    dbk.addLocalText('    ocrEdtDep_t IN_BY_USER_depv_sum = depv[REDUC_SLOT_4Rtr-1];  //From nekbone_rtr_start().')
    dbk.addLocalText('    ocrGuid_t in_sum_guid = IN_BY_USER_depv_sum.guid;')
    dbk.addLocalText('    ReducSum_t * in_sum = IN_BY_USER_depv_sum.ptr;')
    OA.addDataBlocks(G,nc,dbk)
    nekCG_rtr_stop = 'err = nekbone_rtr_stop(io_NEKOstatics, io_NEKOglobals, '
    nekCG_rtr_stop += 'in_nekCGscalars, o_nekCGscalars, in_CGtimes,o_CGtimes, in_sum_guid, in_sum); IFEB;'
    OA.addCustomText(G, nc, nekCG_rtr_stop)
    tailTransitEdn2go_text = 'ocrGuid_t ' + OA.makeGuidEdtname("tailRecurTransitEND") + ' = *in_gDone3;'
    OA.addCustomText(G, nc, tailTransitEdn2go_text)

    nc += 1;  taskName="tailRecurTransitEND"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_tailRecurIterate); dbk.flight = 'flTAGO'; dbk.localname = 'io_tailRecurIter'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOglobals';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_G1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G4); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_G6); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_dxTm1); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekC); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekP); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekR); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekW); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekX); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekZ); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flLANDING'; dbk.localname = 'in_nekCGscalars'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekCGscalars);dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_nekCGscalars'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_CGtimes); dbk.flight = 'flTAGO'; dbk.localname = 'io_CGtimes'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_reducPrivate); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
    tailRecurTransitENDt = 'err = nekbone_tailRecurTransitEND(io_tailRecurIter->current, '
    tailRecurTransitENDt += 'io_NEKOglobals->rankID, in_nekCGscalars, o_nekCGscalars); IFEB;'
    OA.addCustomText(G, nc, tailRecurTransitENDt)

    nc += 1;  taskName="concludeTailRecursion"; OA.graphAddNode(G,nc,taskName)
    OA.getMyTask(G,nc).hint = 'pHintEDT'
    dbk = copy.deepcopy(dbk_spmd_globals); dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_gDone); dbk.flight = 'flLANDING'; dbk.localname = 'in_gDone'; OA.addDataBlocks(G, nc, dbk)
    OA.addCustomText(G, nc, 'err = tailRecurConclude(); IFEB;')
    dbk.addLocalText('ocrGuid_t ' + OA.makeGuidEdtname("BtForkTransition_Stop") + ' = *in_gDone;')
    dbk = copy.deepcopy(dbk_nekoStatics); dbk.flight = 'flTAGO'; dbk.localname = 'io_NEKOstatics'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoGlobals); dbk.flight = 'flTAGO';
    dbk.addLocalText('ocrHint_t hintEDT, *pHintEDT=0;')
    dbk.addLocalText('err = ocrXgetEdtHint(NEK_OCR_USE_CURRENT_PD, &hintEDT, &pHintEDT); IFEB;')
    OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_nekoTools);   dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_reducPrivate);dbk.flight = 'flTAGO'; OA.addDataBlocks(G, nc, dbk)

    # ==============================================================================
    # ==============================================================================
    # Fork-Join tree for the distribution of MPI ranks
    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    ledg = OA.graphAddEdge(G, "mainEdt", "finalEDT", "reducSharedRef")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducSharedRef')
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", 'NEKOstatics')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupBtForkJoin", 'reducShare')
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducShare')

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
    ledg = OA.graphAddEdge(G, "SetupBtForkJoin", "BtForkIF", "reducShare")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducShare')
    btForkIF_reducShare = ledg[0]

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
    ledg = OA.graphAddEdge(G, "BtForkIF", "BtForkFOR", "reducShare")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducShare')

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
    ledg = OA.graphAddEdge(G, "BtForkFOR", "BtForkIF", "reducShare")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO';  OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducShare')
    OA.sharedConx(G, ledg[0]).append(btForkIF_reducShare)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtJoinIFTHEN", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    OA.sharedConx(G, ledg[0]).append(btJoin_gDone_main_input_edge)

    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')
    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')
    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "BtForkELSE", "BtForkTransition_Start", "reducShare")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducShare')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "BtForkTransition_Stop", "TFJiterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('TFJiterate')

    #This is where we stitch in the tail recursion.
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "setupTailRecursion", "gDone")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "setupTailRecursion", "SPMDGlobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('SPMDGlobals')

    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "channelExchange_start", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "channelExchange_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "channelExchange_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "BtForkTransition_Start", "channelExchange_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

    ledg = OA.graphAddEdge(G, "channelExchange_start", "channelExchange_stop", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "channelExchange_start", "channelExchange_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "channelExchange_start", "channelExchange_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "channelExchange_start", "channelExchange_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    ledg = OA.graphAddEdge(G, "channelExchange_start", "channelExchange_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')

    ledg = OA.graphAddEdge(G, "channelExchange_stop", "nekMultiplicity_start", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "channelExchange_stop", "nekMultiplicity_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "channelExchange_stop", "nekMultiplicity_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "channelExchange_stop", "nekMultiplicity_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    ledg = OA.graphAddEdge(G, "channelExchange_stop", "nekMultiplicity_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')

    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_start", "nekMultiplicity_stop", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')

    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "nekSetF_start", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')

    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "setupTailRecursion", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "setupTailRecursion", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "setupTailRecursion", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "setupTailRecursion", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekMultiplicity_stop", "setupTailRecursion", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    OA.getEvent(G, ledg).fertile = False

    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekSetF_start", "nekSetF_stop", "nekF")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekF')

    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekSetF_stop", "nekCGstep0_start", "nekF")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekF')

    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "gDone2")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('gDone2')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCGstep0_start", "nekCGstep0_stop", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')

    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "NEKOstatics")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOstatics')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "NEKOglobals")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOglobals')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "NEKOtools")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('NEKOtools')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCGstep0_stop", "setupTailRecursion", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
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
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    tailRecur_nekCGscalars_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    tailRecur_CGtimes_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    tailRecur_nek_G1_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    tailRecur_nek_G4_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    tailRecur_nek_G6_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    tailRecur_nek_dxm1_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    tailRecur_nek_dxTm1_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    tailRecur_nekC_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    tailRecur_nekP_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    tailRecur_nekR_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    tailRecur_nekW_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    tailRecur_nekX_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    tailRecur_nekZ_edge=ledg[0]  # This takes only the edge out of the labeled edge.
    ledg = OA.graphAddEdge(G, "setupTailRecursion", "tailRecursionIFThen", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    tailRecur_reducPrivate_edge=ledg[0]  # This takes only the edge out of the labeled edge.

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
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "tailRecursionIFThen", "tailRecurTransitBEGIN", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "tailRecurTransitBEGIN", "nekCG_solveMi", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_solveMi", "nekCG_beta_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G,"nekCG_beta_start", "nekCG_beta_stop", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G,"nekCG_beta_start", "nekCG_beta_stop", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G,"nekCG_beta_start", "nekCG_beta_stop", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G,"nekCG_beta_start", "nekCG_beta_stop", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G,"nekCG_beta_start", "nekCG_beta_stop", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_beta_start", "nekCG_beta_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_beta_stop", "nekCG_axi_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_axi_start", "nekCG_axi_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_axi_stop", "nekCG_alpha_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_start", "nekCG_alpha_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_alpha_stop", "nekCG_rtr_start", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    ledg = OA.graphAddEdge(G, "nekCG_rtr_start", "nekCG_rtr_stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')

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
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    OA.getEvent(G, ledg).fertile = False
    ledg = OA.graphAddEdge(G, "nekCG_rtr_stop", "tailRecurTransitEND", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
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
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nekCGscalars")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekCGscalars')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nekCGscalars_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "CGtimes")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('CGtimes')
    OA.sharedConx(G, ledg[0]).append(tailRecur_CGtimes_edge)

    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nek_G1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G1')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nek_G1_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nek_G4")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G4')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nek_G4_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nek_G6")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_G6')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nek_G6_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nek_dxm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxm1')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nek_dxm1_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nek_dxTm1")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nek_dxTm1')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nek_dxTm1_edge)

    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nekC")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekC')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nekC_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nekP")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekP')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nekP_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nekR")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekR')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nekR_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nekW")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekW')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nekW_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nekX")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekX')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nekX_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "nekZ")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('nekZ')
    OA.sharedConx(G, ledg[0]).append(tailRecur_nekZ_edge)
    ledg = OA.graphAddEdge(G, "tailRecurTransitEND", "tailRecursionIFThen", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'; OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('reducPrivate')
    OA.sharedConx(G, ledg[0]).append(tailRecur_reducPrivate_edge)

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
    ledg = OA.graphAddEdge(G, "tailRecursionELSE", "concludeTailRecursion", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('reducPrivate'), 'DBK')
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
    ledg=OA.graphAddEdge(G, "concludeTailRecursion", "BtForkTransition_Stop", "reducPrivate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('reducPrivate'), 'DBK')
    OA.getEvent(G, ledg).fertile = False

    # ==============================================================================
    # ==============================================================================
    # ==============================================================================
    # ==============================================================================
    # ==============================================================================
    # ----- WORK
    erri = 0
    while not erri:
        if True: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if False: OA.printGraph(G)
        if True:
            erri = OA.outputDot(G, "z_nekbone_inOcr.dot")
            if erri: break
        if True:
            erri = OA.outputOCR(G, "z_nekbone_inOcr.c")
            if erri: break

        if False: OA.printGraph(G)

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

