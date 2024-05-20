import networkx as nx
import sys

def readAdjGraph(inputFilename):
    graph = nx.DiGraph()
    f = open(inputFilename, mode="rb")
    header = f.read(80) # Read to skip
    version = int.from_bytes( f.read(4), 'little' )
    numNodes = int.from_bytes( f.read(4), 'little' )
    print("AAG binary format V%s" %version)

    # Create FAG without attributes
    nodeCounter = 0
    while nodeCounter < numNodes:
        nodeCounter += 1
        fid = int.from_bytes( f.read(4), 'little' ) # Next face ID
        numAdjacent = int.from_bytes( f.read(4), 'little' ) # Num. of adjacent faces

        # Read adjacent
        adjCounter = 0
        while adjCounter < numAdjacent:
            adjCounter += 1
            nid = int.from_bytes( f.read(4), 'little' ) # Next face ID
            graph.add_edge(fid, nid)
    
    f.close()

    print('We have %d nodes in the FAG.' % graph.number_of_nodes())
    print('We have %d edges in the FAG.' % graph.number_of_edges())
    return graph

G = readAdjGraph(sys.argv[1]) # E.g. C:/Users/serge/Desktop/aag.bin
