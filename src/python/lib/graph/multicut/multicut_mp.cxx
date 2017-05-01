#ifdef WITH_LP_MP

#include <pybind11/pybind11.h>

#include "nifty/graph/optimization/multicut/multicut_mp.hxx"

#include "nifty/python/graph/optimization/multicut/multicut_objective.hxx"
#include "nifty/python/converter.hxx"
#include "nifty/python/graph/optimization/multicut/export_multicut_solver.hxx"
#include "nifty/python/graph/undirected_list_graph.hxx"
#include "nifty/python/graph/edge_contraction_graph.hxx"

namespace py = pybind11;

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

namespace nifty{
namespace graph{

    // TODO exports with different primal solvers once implemented
    
    void exportMpSettings(py::module & multicutModule){
        
        py::class_<MpSettings>(multicutModule, "MpSettings")
            .def(py::init<>())
        ;

    }

    
    template<class OBJECTIVE>
    void exportMulticutMpT(py::module & multicutModule){
        
        typedef OBJECTIVE ObjectiveType;
        typedef MulticutMp<ObjectiveType> Solver;
        typedef typename Solver::Settings Settings;
        
        // FIXME nIter and verbose have no effect yet
        const auto solverName = std::string("MulticutMp");
        exportMulticutSolver<Solver>(multicutModule, solverName.c_str())
            .def(py::init<>())
            .def_readwrite("verbose",&Settings::verbose)
            .def_readwrite("numberOfIterations",&Settings::numberOfIterations)
            .def_readwrite("primalComputationInterval",&Settings::primalComputationInterval)
            .def_readwrite("standardReparametrization",&Settings::standardReparametrization)
            .def_readwrite("roundingReparametrization",&Settings::roundingReparametrization)
            .def_readwrite("tightenReparametrization",&Settings::tightenReparametrization)
            .def_readwrite("tighten",&Settings::tighten)
            .def_readwrite("tightenInterval",&Settings::tightenInterval)
            .def_readwrite("tightenIteration",&Settings::tightenIteration)
            .def_readwrite("tightenSlope",&Settings::tightenSlope)
            .def_readwrite("tightenConstraintsPercentage",&Settings::tightenConstraintsPercentage)
            .def_readwrite("numberOfIterations",&Settings::numberOfIterations)
            .def_readwrite("minDualImprovement",&Settings::minDualImprovement)
            .def_readwrite("minDualImprovementInterval",&Settings::minDualImprovementInterval)
            .def_readwrite("timeout",&Settings::timeout)
        ; 

    }

    
    void exportMulticutMp(py::module & multicutModule){
        
        exportMpSettings(multicutModule);
        {
            typedef PyUndirectedGraph GraphType;
            typedef MulticutObjective<GraphType, double> ObjectiveType;
            exportMulticutMpT<ObjectiveType>(multicutModule);
        }
        {
            typedef PyContractionGraph<PyUndirectedGraph> GraphType;
            typedef MulticutObjective<GraphType, double> ObjectiveType;
            exportMulticutMpT<ObjectiveType>(multicutModule);
        }     

    }

} // namespace graph
} // namespace nifty
#endif
