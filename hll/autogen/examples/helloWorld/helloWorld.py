import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    OA.GBL.insert_debug_code = 0

    G=OA.GraphTool.DiGraph()

    OA.setupGraph(G,"HelloWorld")

    OA.addHeader(G, '#include "helloworld.h" ')  # This will be put at the beginning of the file

    # ----- Data Blocks
    dbk = OA.ocrDataBlock();  dbk.name = 'NULL_GUID'
    dbk_nullGuid = dbk

    dbk = OA.ocrDataBlock();  dbk.name = 'Parabola';   dbk.count=3;     dbk.type='int'
    dbk_parabola = dbk

    dbk=OA.ocrDataBlock();  dbk.name = 'Roots';   dbk.count=2;     dbk.type='double'
    dbk_roots = dbk

    # ----- NODES
    # Always root the starting/root/main EDT at ID=0 = OA.GBL.MAINNODE
    # Always cap all ending EDT by the FinalEDT at ID=1 = OA.GBL.FINALNODE
    nc = OA.GBL.MAINNODE;    taskName = "mainEdt"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flTAKEOFF'; OA.addDataBlocks(G,nc,dbk)

    nc = OA.GBL.FINALNODE;   taskName="finalEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_nullGuid); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)

    nc += 1;  taskName="beginAlgoEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_parabola); dbk.flight = 'flTAKEOFF';  dbk.localname='o_parabola'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, 'err = setupTheParabola(o_parabola); IFEB;')

    nc += 1;  taskName="SolveEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_parabola); dbk.flight = 'flLANDING'; dbk.localname='in_parabola'; OA.addDataBlocks(G,nc,dbk)
    dbk = copy.deepcopy(dbk_roots); dbk.flight = 'flTAKEOFF'; dbk.localname='o_roots'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, 'err = solveForRoots(in_parabola, o_roots); IFEB;')

    nc += 1;  taskName="CountEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_roots); dbk.flight = 'flTAGO'; dbk.localname='io_roots'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, 'err = deduplicateRoots(io_roots); IFEB;')

    nc += 1;  taskName="PrintEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_roots); dbk.flight = 'flTAGO'; dbk.localname='io_roots'; OA.addDataBlocks(G,nc,dbk)
    OA.addCustomText(G, nc, 'err = printRoots(io_roots); IFEB;')

    nc += 1;  taskName="endAlgoEDT"; OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_roots); dbk.flight = 'flLANDING'; OA.addDataBlocks(G,nc,dbk)

    # ----- EDGES
    edg = OA.graphAddEdge(G,     "mainEdt", "beginAlgoEDT", "NULL_GUID")
    OA.getEvent(G, edg).eflag = 'EVT_PROP_NONE'
    OA.getEvent(G, edg).satisfy = 'NULL_GUID'

    edg=OA.graphAddEdge(G, "beginAlgoEDT", "SolveEDT", "Parabola")
    OA.getEvent(G, edg).accessMode = 'DB_MODE_EW'
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('Parabola')

    edg=OA.graphAddEdge(G,    "SolveEDT",     "CountEDT", "Roots")
    OA.getEvent(G, edg).accessmode = 'DB_MODE_EW'
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('Roots')

    edg=OA.graphAddEdge(G,    "CountEDT",     "PrintEDT", "Roots")
    OA.getEvent(G, edg).accessMode = 'DB_MODE_CONST'
    OA.getEvent(G, edg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName +'->', OA.makeGuidDataBlockname('Roots'), 'DBK')

    edg=OA.graphAddEdge(G,    "PrintEDT",   "endAlgoEDT", "Roots")
    OA.getEvent(G, edg).accessMode = 'DB_MODE_CONST'
    OA.getEvent(G, edg).satisfy = OA.contextedGuid(OA.GBL.DBK_structName +'->', OA.makeGuidDataBlockname('Roots'), 'DBK')

    edg=OA.graphAddEdge(G,  "endAlgoEDT",     "finalEDT", "NULL_GUID")
    OA.getEvent(G, edg).eflag = 'EVT_PROP_NONE'
    OA.getEvent(G, edg).satisfy = 'NULL_GUID'

    # ----- WORK
    erri = 0
    while not erri:
        if 1 == 2: OA.printGraph(G)   # Just to check if the inputs are ok.

        erri = OA.OCRanalysis(G)
        if erri: break

        if 1 == 2: OA.printGraph(G)
        if 1 == 2:
            erri = OA.outputDot(G, "z_helloWorld.dot")
            if erri: break
        if 1 == 1:
            erri = OA.outputOCR(G, "z_helloWorld.c")
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

