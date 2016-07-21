import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    use_a_work_block = True

    G=OA.GraphTool.DiGraph()

    OA.setupGraph(G,"TailRecursion")

    OA.addHeader(G, '#include "tailrecursion.h" ')  # This will be put at the beginning of the file

    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'
    dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'Iterate';   dbk.count=1;     dbk.type='Iterate_t'
    dbk_iterate = dbk

    if use_a_work_block:
        dbk = OA.ocrDataBlock();  dbk.name = 'Work';   dbk.count=1000+1;     dbk.type='int'
                                                        #Offset 0 is used to store the size.
        dbk_work = dbk

    # ----- NODES
    # Always root the starting/root/main EDT at ID=0 = OA.GBL.MAINNODE
    # Always cap all ending EDT by the FinalEDT at ID=1 = OA.GBL.FINALNODE
    nc = OA.GBL.MAINNODE;    taskName = "mainEdt"; OA.graphAddNode(G,nc,taskName)   # Got to have one 'mainEdt'
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G,nc,dbk)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)

    nc += 1;  taskName="SetupIteration"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_iterate);  dbk.flight = 'flTAKEOFF';  dbk.localname='o_iterate'; OA.addDataBlocks(G,nc,dbk)
    setupt = 'err = initializeIterate(o_iterate, ' + OA.makeGuidEdtname("ConcludeIteration") + '); IFEB;'
    OA.addCustomText(G, nc, setupt)
    if use_a_work_block:
        dbk = copy.deepcopy(dbk_work); dbk.flight = 'flTAKEOFF'; dbk.localname = 'o_work'; OA.addDataBlocks(G, nc, dbk)
        setupWork = 'err = initializeWork(' + dbk_work.text_for_sizeName() + ', o_work); IFEB;'
        OA.addCustomText(G, nc, setupWork)

    nc += 1;  taskName="Iteration"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flTAGO'; dbk.localname = 'io_iterate';
    dbk.addLocalText('ocrGuid_t ' + OA.makeGuidEdtname("ConcludeIteration") + ' = io_iterate->whereToGoWhenFalse;')
    OA.addDataBlocks(G,nc,dbk)
    iterConditionText = 'condition(io_iterate)'
    OA.addIFconditionText(G,nc,iterConditionText)
    if use_a_work_block:
        dbk = copy.deepcopy(dbk_work); dbk.flight = 'flTAGO'; dbk.localname = 'io_work'; OA.addDataBlocks(G,nc,dbk)
        text_for_THEN_clause = 'err = trueClauseWithWork(io_iterate, io_work); IFEB;'
    else:
        text_for_THEN_clause = 'err = trueClause(io_iterate); IFEB;'
    OA.addCustomText(G, nc, text_for_THEN_clause)

    nc += 1;  taskName="IterationElse"; OA.graphAddNode(G,nc,taskName)
    # DBK local names are taken care off by the IF-THEN edt called "Iteration".
    dbk = copy.deepcopy(dbk_iterate); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)
    if use_a_work_block:
        dbk = copy.deepcopy(dbk_work); dbk.flight = 'flTAGO'; OA.addDataBlocks(G,nc,dbk)
        text_for_ELSE_clause = 'err = falseClauseWithWork(io_iterate, io_work); IFEB;'
    else:
        dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)
        text_for_ELSE_clause = 'err = falseClause(io_iterate); IFEB;'
    OA.addCustomText(G, nc, text_for_ELSE_clause)

    nc += 1;  taskName="ConcludeIteration"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G, nc, dbk)
    if use_a_work_block:
        dbk = copy.deepcopy(dbk_work); dbk.flight = 'flLANDING'; dbk.localname = 'io_work'; OA.addDataBlocks(G, nc, dbk)
        OA.addCustomText(G, nc, 'err = concludeIteration(io_work); IFEB;')
    else:
        OA.addCustomText(G, nc, 'err = concludeIteration(NULL); IFEB;')

    # ----- EDGES
    ledg = OA.graphAddEdge(G, "mainEdt", "SetupIteration", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'
    OA.getEvent(G, ledg).satisfy = 'NULL_GUID'

    ledg = OA.graphAddEdge(G, "SetupIteration", "ConcludeIteration", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'
    OA.getEvent(G, ledg).satisfy = 'NULL_GUID'

    ledg=OA.graphAddEdge(G, "SetupIteration", "Iteration", "Iterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'
    OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Iterate')
    main_iteration_input_edge=ledg[0]  # This takes only the edge out of the labelled edge.

    main_iteration_work_edge=()
    if use_a_work_block:
        ledg=OA.graphAddEdge(G, "SetupIteration", "Iteration", "Work")
        OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
        OA.getEvent(G, ledg).satisfy = OA.makeGuidDataBlockname('Work')
        main_iteration_work_edge=ledg[0]  # This takes only the edge out of the labelled edge.

    ledg=OA.graphAddEdge(G, "Iteration", "Iteration", "Iterate")
    OA.getEvent(G, ledg).accessMode = 'DB_MODE_RW'
    OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('Iterate'), 'DBK')
    OA.sharedConx(G,ledg[0]).append(main_iteration_input_edge)

    if use_a_work_block:
        ledg = OA.graphAddEdge(G, "Iteration", "Iteration", "Work")
        OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
        OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('Work'), 'DBK')
        OA.sharedConx(G, ledg[0]).append(main_iteration_work_edge)

    ledg=OA.graphAddEdge(G, "Iteration", "IterationElse", "NULL_GUID")
    OA.getEdge(G, ledg[0])["leads_to_ElseClause"] = True

    if use_a_work_block:
        ledg = OA.graphAddEdge(G, "IterationElse", "ConcludeIteration", "Work")
        OA.getEvent(G, ledg).accessMode = 'DB_MODE_RO'
        OA.getEvent(G, ledg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName + '->', OA.makeGuidDataBlockname('Work'), 'DBK')
        OA.getEvent(G, ledg).fertile = False
    else:
        ledg=OA.graphAddEdge(G, "IterationElse", "ConcludeIteration", "NULL_GUID")
        OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'
        OA.getEvent(G, ledg).satisfy = 'NULL_GUID'
        OA.getEvent(G, ledg).fertile = False

    ledg=OA.graphAddEdge(G, "ConcludeIteration", "finalEDT", "NULL_GUID")
    OA.getEvent(G, ledg).eflag = 'EVT_PROP_NONE'
    OA.getEvent(G, ledg).satisfy = 'NULL_GUID'

    # ----- WORK
    erri = 0
    while not erri:
        if 1 == 2: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if 1 == 2: OA.printGraph(G)
        if 1 == 2:
            erri = OA.outputDot(G, "z_tailRecursion.dot")
            if erri: break
        if 1 == 1:
            erri = OA.outputOCR(G, "z_tailRecursion.c")
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

