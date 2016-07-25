from __future__ import print_function
import nifty
import numpy



G = nifty.graph.UndirectedGraph
CG = G.EdgeContractionGraph

GMCO = G.MulticutObjective
CGMCO = CG.MulticutObjective



def testUndirectedGraph():

    g =  nifty.graph.UndirectedGraph(4)
    edges =  numpy.array([[0,1],[0,2],[0,3]],dtype='uint64')
    g.insertEdges(edges)

    edgeList = [e for e in g.edges()]
    assert edgeList == [0,1,2]

    nodeList = [e for e in g.nodes()]
    assert nodeList == [0,1,2,3]

    assert g.u(0) == 0
    assert g.v(0) == 1 
    assert g.u(1) == 0 
    assert g.v(1) == 2 
    assert g.u(2) == 0 
    assert g.v(2) == 3 


def testEdgeContractionGraph():

    g =  nifty.graph.UndirectedGraph(4)
    edges =  numpy.array([[0,1],[0,2],[0,3]],dtype='uint64')
    g.insertEdges(edges)





    class MyCb(nifty.graph.EdgeContractionGraphCallback):
        def __init__(self):
            super(MyCb, self).__init__()

            self.nCallsContractEdge = 0
            self.nCallsMergeEdges = 0
            self.nCallsMergeNodes = 0
            self.nCallscontractEdgeDone = 0

        def contractEdge(self, edge):
            self.nCallsContractEdge += 1

        def mergeEdges(self, alive, dead):
            self.nCallsMergeEdges += 1

        def mergeNodes(self, alive, dead):
            self.nCallsMergeNodes += 1

        def contractEdgeDone(self, edge):
            self.nCallscontractEdgeDone += 1



    cb = MyCb()
    ecg = nifty.graph.edgeContractionGraph(g,cb)
    ecg.contractEdge(0)

    assert cb.nCallsContractEdge == 1
    assert cb.nCallsMergeNodes == 1
    assert cb.nCallscontractEdgeDone == 1


def make2x2Rag():

    labels = numpy.zeros(shape=[2,2],dtype='uint32')

    labels[0,0] = 0 
    labels[1,0] = 1 
    labels[0,1] = 0 
    labels[1,1] = 2 

    g =  nifty.graph.rag.explicitLabelsGridRag2D(labels)

    return g

def testGridRag():

    labels = numpy.zeros(shape=[2,2],dtype='uint32')

    labels[0,0] = 0 
    labels[1,0] = 1 
    labels[0,1] = 0 
    labels[1,1] = 2 

    g =  nifty.graph.rag.explicitLabelsGridRag2D(labels)
    weights = numpy.ones(g.numberOfEdges)*1
    obj = nifty.graph.multicut.multicutObjective(g, weights)


    greedy=obj.greedyAdditiveFactory().create(obj)
    visitor = obj.multicutVerboseVisitor()
    ret = greedy.optimize()
    #print("greedy",obj.evalNodeLabels(ret))




    assert g.numberOfNodes == 3
    assert g.numberOfEdges == 3

    insertWorked = True
    try:
        g.insertEdge(0,1)
    except:
        insertWorked = False
    assert insertWorked == False
