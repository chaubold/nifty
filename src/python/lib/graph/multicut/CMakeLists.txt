
addPythonModule(
    NESTED_NAME 
        nifty/graph/multicut  
    SOURCES
        multicut.cxx
        multicut_objective.cxx
        multicut_visitor_base.cxx
        multicut_base.cxx
        multicut_factory.cxx
        multicut_ilp.cxx
        multicut_greedy_additive.cxx
        fusion_move_based.cxx
        perturb_and_map.cxx
    LIBRRARIES
        ${NIFTY_LP_SOLVER_LIBS}
)