import sys
sys.path.append('../../tools')
import ocr_autogen as OA
import copy

# ------------------------------------------------------------------------------
def theMain():

    G = OA.GraphTool.DiGraph()

    OA.setupGraph(G, "RecursiveBinaryTree")

    # ----- Data Blocks
    # NOTE: The counts
    #           123456     for the original Set
    #           10123456   for Set1
    #           20123456   for Set2
    #       are purely arbitrary and serves as placeholders that the customer
    #       can track easily in the generated code.

    dbk = OA.ocrDataBlock();  dbk.name = 'dSet';   dbk.count=123456;     dbk.type='int'
    dbk_dSet = dbk
    # These 2 "dSet" are sub-arrays split from "dSet"
    dbk = OA.ocrDataBlock();  dbk.name = 'dSet1';  dbk.count=10123456;     dbk.type='int'
    dbk_dSet1 = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'dSet2';  dbk.count=20123456;     dbk.type='int'
    dbk_dSet2 = dbk

    # These 2 "boundary" sets contain the offsets in the original dSet
    dbk = OA.ocrDataBlock();  dbk.name = 'boundary1';   dbk.count=2;     dbk.type='int'
    dbk_boundary1 = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'boundary2';   dbk.count=2;     dbk.type='int'
    dbk_boundary2 = dbk

    # These contains the scattering of the gathering EDT
    dbk = OA.ocrDataBlock();  dbk.name = 'scatter1';  dbk.count=1;    dbk.type='ocrGuid_t'
    dbk_scatter1 = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'scatter2';  dbk.count=1;    dbk.type='ocrGuid_t'
    dbk_scatter2 = dbk

    # This contains the propagation of the gather element
    dbk = OA.ocrDataBlock();  dbk.name = 'gather';    dbk.count=1;    dbk.type='ocrGuid_t'
    dbk_gather = dbk

    # These contains the results from the gathering operations
    dbk = OA.ocrDataBlock();  dbk.name = 'result1';   dbk.count=1;    dbk.type='int'
    dbk_result1 = dbk
    dbk = OA.ocrDataBlock();  dbk.name = 'result2';   dbk.count=1;    dbk.type='int'
    dbk_result2 = dbk

    # ----- NODES
    # A
    nc = OA.GBL.MAINNODE;   taskName = "mainEDT";     OA.graphAddNode(G,nc,taskName)
    OA.getEdts(G, nc).append('a')
    dbk = copy.deepcopy(dbk_dSet);      dbk.flight = 'HOP';      OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_dSet1);     dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary1); dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter1);  dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_dSet2);     dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary2); dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter2);  dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # a
    nc = OA.GBL.FINALNODE;   taskName = "a";    OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_result1);   dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result2);   dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)

    # B
    nc += 1;  taskName = "B";    OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_dSet1);     dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary1); dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter1);  dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_dSet1);     dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter1);  dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary1); dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_dSet2);     dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter2);  dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary2); dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_gather);    dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # D
    nc += 1;  taskName = "D";        OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_dSet1);     dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter1);  dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary1); dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result1);   dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # E
    nc += 1;  taskName = "E";        OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_dSet2);     dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter2);  dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary2); dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result2);   dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # b
    nc += 1;  taskName = "b";        OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gather);    dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result1);   dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result2);   dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_result1);   dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # C
    nc += 1;  taskName = "C";    OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_dSet2);     dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary2); dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter2);  dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_dSet1);     dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter1);  dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary1); dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_dSet2);     dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter2);  dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary2); dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_gather);    dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    #F
    nc += 1;  taskName = "F";        OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_dSet1);     dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter1);  dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary1); dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result1);   dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # G
    nc += 1;  taskName = "G";        OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_dSet2);     dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_scatter2);  dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_boundary2); dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result2);   dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # c
    nc += 1;  taskName = "c";        OA.graphAddNode(G,nc,taskName)
    dbk = copy.deepcopy(dbk_gather);    dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result1);   dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)
    dbk = copy.deepcopy(dbk_result2);   dbk.flight = 'LANDING';  OA.addDataBlocks(G, nc, dbk)

    dbk = copy.deepcopy(dbk_result2);   dbk.flight = 'TAKEOFF';  OA.addDataBlocks(G, nc, dbk)

    # ----- EDGES
    # TODO: Put in the correct " getEvent(G, edg).accessMode" as everything is now DB_MODE_RW

    # From A
    edg = OA.graphAddEdge(G,     "mainEDT", "B", "dSet1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('dSet1')
    edg = OA.graphAddEdge(G,     "mainEDT", "B", "boundary1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('boundary1')
    edg = OA.graphAddEdge(G,     "mainEDT", "B", "scatter1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('scatter1')

    edg = OA.graphAddEdge(G,     "mainEDT", "C", "dSet2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('dSet2')
    edg = OA.graphAddEdge(G,     "mainEDT", "C", "boundary2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('boundary2')
    edg = OA.graphAddEdge(G,     "mainEDT", "C", "scatter2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('scatter2')

    # From B
    edg = OA.graphAddEdge(G,     "B", "D", "dSet1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('dSet1')
    edg = OA.graphAddEdge(G,     "B", "D", "boundary1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('boundary1')
    edg = OA.graphAddEdge(G,     "B", "D", "scatter1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('scatter1')
    edg = OA.graphAddEdge(G,     "B", "E", "dSet2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('dSet2')
    edg = OA.graphAddEdge(G,     "B", "E", "boundary2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('boundary2')
    edg = OA.graphAddEdge(G,     "B", "E", "scatter2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('scatter2')

    edg = OA.graphAddEdge(G,     "B", "b", "gather")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('gather')

    # From C
    edg = OA.graphAddEdge(G,     "C", "F", "dSet1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('dSet1')
    edg = OA.graphAddEdge(G,     "C", "F", "boundary1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('boundary1')
    edg = OA.graphAddEdge(G,     "C", "F", "scatter1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('scatter1')
    edg = OA.graphAddEdge(G,     "C", "G", "dSet2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('dSet2')
    edg = OA.graphAddEdge(G,     "C", "G", "boundary2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('boundary2')
    edg = OA.graphAddEdge(G,     "C", "G", "scatter2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('scatter2')

    edg = OA.graphAddEdge(G,     "C", "c", "gather")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('gather')

    # From D
    edg = OA.graphAddEdge(G,     "D", "b", "result1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('result1')
    OA.getEvent(G, edg).fertile = False

    # From E
    edg = OA.graphAddEdge(G,     "E", "b", "result2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('result2')
    OA.getEvent(G, edg).fertile = False

    # From F
    edg = OA.graphAddEdge(G,     "F", "c", "result1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('result1')
    OA.getEvent(G, edg).fertile = False

    # From G
    edg = OA.graphAddEdge(G,     "G", "c", "result2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('result2')
    OA.getEvent(G, edg).fertile = False

    # From b
    edg = OA.graphAddEdge(G,     "b", "a", "result1")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('result1')
    OA.getEvent(G, edg).fertile = False

    # From c
    edg = OA.graphAddEdge(G,     "c", "a", "result2")
    OA.getEvent(G, edg).satisfy = OA.makeGuidDataBlockname('result2')
    OA.getEvent(G, edg).fertile = False

    # ----- WORK
    erri = 0
    while not erri:
        if 1 == 2: OA.printGraph(G)

        erri = OA.OCRanalysis(G)
        if erri: break

        if 1 == 2: OA.printGraph(G)
        if 1 == 1:
            erri = OA.outputDot(G, "z_recursiveBinaryTree_full.dot")
            if erri: break
        if 1 == 1:
            erri = OA.outputOCR(G, "z_recursiveBinaryTree_full.c")
            if erri: break

        if 1 == 1: OA.printGraph(G)

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

