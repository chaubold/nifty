import opengm
import vigra
import nifty
import numpy


f = "/home/tbeier/Desktop/mc_models/knott-3d-150/gm_knott_3d_039.h5"
#f = "/home/tbeier/Desktop/mc_models/knot-3d-550/gm_knott_3d_119.h5"
#f = "/home/tbeier/Desktop/mc_models/knott-3d-450/gm_knott_3d_103.h5"
#f = "/home/tbeier/Downloads/gm_large_3.gm"
f = "/home/tbeier/Downloads/gm_small_1.gm"
#f = "/home/tbeier/Desktop/mc_models/knott-3d-300/gm_knott_3d_072.h5"
gm = opengm.loadGm(f)





def opengmToNumpy(gm):
    nNodes = gm.numberOfVariables
    nEdges = gm.numberOfFactors


    factorSubset=opengm.FactorSubset(gm)
    weights = factorSubset.evaluate([0,1])
    weightsB = factorSubset.evaluate([0,0])
    print "wb", weightsB.sum()
    vis =  factorSubset.variableIndices()
    assert len(numpy.unique(vis.reshape(-1))) == nNodes
    #print vis.shape,weights.shape
    assert vis.max()+1 == nNodes
    return nNodes,weights,vis

nNodes,weights, uvs = opengmToNumpy(gm)
nFac = weights.shape[0]


print weights.min(),weights.max()


g =  nifty.graph.UndirectedGraph(int(nNodes))
g.insertEdges(uvs)
assert g.numberOfEdges == weights.shape[0]
assert g.numberOfEdges == uvs.shape[0]
obj = nifty.graph.multicut.multicutObjective(g, weights)


with vigra.Timer("time that shit"):
    greedy=nifty.greedyAdditiveFactory().create(obj)
    visitor = nifty.multicutVerboseVisitor(1000)
    ret = greedy.optimizeWithVisitor(visitor=visitor)

    