#pragma once
#ifndef NIFTY_GRAPH_RAG_GRID_RAG_FEATURES_HXX
#define NIFTY_GRAPH_RAG_GRID_RAG_FEATURES_HXX


#include "nifty/graph/rag/grid_rag.hxx"
#include "nifty/marray/marray.hxx"

#include "nifty/tools/for_each_coordinate.hxx"

#include "vigra/accumulator.hxx"

namespace nifty{
namespace graph{

    template<class T, unsigned int NBINS>
    class DefaultAcc{
        
    public:
        DefaultAcc()
        :   minVal_(std::numeric_limits<T>::infinity()),
            maxVal_(-1.0*std::numeric_limits<T>::infinity()),
            sumVal_(0.0),
            countVal_(0.0)
        {

        }
        void accumulate(const T & val){
            minVal_ = std::min(val, minVal_);
            maxVal_ = std::max(val, maxVal_);
            sumVal_ += val;
            ++countVal_;
        }
        void merge(const DefaultAcc & other){
            minVal_ = std::min(other.minVal_, minVal_);
            maxVal_ = std::max(other.maxVal_, maxVal_);
            sumVal_ += other.sumVal_;
            countVal_ +=other.countVal_;
        }
        void getFeatures(T * featuresOut){
            featuresOut[0] = minVal_;
            featuresOut[1] = maxVal_;
            featuresOut[2] = sumVal_;
            featuresOut[3] = sumVal_/static_cast<T>(countVal_);
            featuresOut[4] = static_cast<T>(countVal_);
        }
    private:
        T minVal_,maxVal_,sumVal_;
        uint64_t countVal_;
    };


    // FIXME min and max val are never passed to the DefaultAcc, afaik
    template<class GRAPH, class T, unsigned int NBINS, template<class> class ITEM_MAP >
    class DefaultAccMapBase 
    {
    public:
        typedef GRAPH Graph;
        DefaultAccMapBase(const Graph & graph, const T globalMinVal = std::numeric_limits<T>::infinity(), const T globalMaxVal=-1.0*std::numeric_limits<T>::infinity())
        :   graph_(graph),
            currentPass_(0),
            globalMinVal_(globalMinVal),
            globalMaxVal_(globalMaxVal),
            accs_(graph){

        }
        // needed for the getFeatureMatrix gluecode
        size_t numberOfEdges() {
            return graph_.numberOfEdges();
        }
        size_t numberOfNodes() {
            return graph_.numberOfNodes();
        }
        void accumulate(const uint64_t item, const T & val){
            if(currentPass_ == 0){
                accs_[item].accumulate(val);
            }
        }
        void startPass(const size_t passIndex){
            currentPass_ = passIndex;
        }
        size_t numberOfPasses()const{
            return 1;
        }
        void merge(const uint64_t alive,  const uint64_t dead){
            accs_[alive].merge(accs_[dead]);
        }
        uint64_t numberOfFeatures() const {
            return 5;
        }
        void getFeatures(const uint64_t item, T * featuresOut){
            accs_[item].getFeatures(featuresOut);
        }   
        void resetFrom(const DefaultAccMapBase & other){
            std::copy(other.accs_.begin(), other.accs_.end(), accs_.begin());
            globalMinVal_ = other.globalMinVal_;
            globalMinVal_ = other.globalMaxVal_;
            currentPass_ = other.currentPass_;
        }
    private:
        const GRAPH & graph_;
        size_t currentPass_;
        T globalMinVal_;
        T globalMaxVal_;
        ITEM_MAP<DefaultAcc<T, NBINS> > accs_;
    };



    template<class GRAPH, class T, unsigned int NBINS=40>
    class DefaultAccEdgeMap : 
        public  DefaultAccMapBase<GRAPH, T, NBINS, GRAPH:: template EdgeMap >
    {
    public:
        typedef GRAPH Graph;
        //typedef typename Graph:: template EdgeMap<DefaultAcc<T, NBINS> > BasesBaseType;
        typedef DefaultAccMapBase<Graph, T, NBINS, Graph:: template EdgeMap >  BaseType;
        using BaseType::BaseType;
    };

    template<class GRAPH, class T, unsigned int NBINS=40>
    class DefaultAccNodeMap : 
        public  DefaultAccMapBase<GRAPH, T, NBINS, GRAPH:: template NodeMap >
    {
    public:
        typedef GRAPH Graph;
        //typedef typename Graph:: template NodeMap<DefaultAcc<T, NBINS> > BasesBaseType;
        typedef DefaultAccMapBase<Graph, T, NBINS, Graph:: template NodeMap >  BaseType;
        using BaseType::BaseType;
    };


    
    template<class GRAPH, class T, unsigned int NBINS, template<class> class ITEM_MAP >
    class VigraAccMapBase {
    public:

        typedef GRAPH Graph;
        
        // for now, we hardcode the features here, should be easy to expose them though
        // Maybe we should get rid of some (Kurtosis / min and max could be replaced by the corresponding quantle 0 resp. 6)
        // TODO should we expose histogram options ?
        typedef vigra::acc::StandardQuantiles<vigra::acc::AutoRangeHistogram<0>> Quantiles;
        typedef vigra::acc::Select<vigra::acc::Mean, vigra::acc::Sum, vigra::acc::Minimum, vigra::acc::Maximum, vigra::acc::Variance, vigra::acc::Skewness, vigra::acc::Kurtosis, Quantiles> features; 
        typedef vigra::acc::AccumulatorChain<T,features> AccumulatorType;
        
        VigraAccMapBase(const Graph & graph )
        :   graph_(graph),
            currentPass_(0),
            accs_(graph) {

            // TODO we could use Auto Histogram instead, but then we would need one more pass
            vigra::HistogramOptions histo_opts;
            histo_opts.setBinCount(NBINS);

            for(size_t edge = 0; edge < graph_.numberOfEdges; ++edge)
                accs_[edge].setHistogramOptions(histo_opts);
        
        }
        
        void accumulate(const uint64_t item, const T & val){
            accs_[item].updatePassN(val, currentPass_);
        }
        
        void startPass(const size_t passIndex){
            currentPass_ = passIndex;
        }
        
        size_t numberOfPasses() const{
            return accs_[0].passesRequired();
        }
        
        
        uint64_t numberOfFeatures() const {
            return 11;
        }
        
        void getFeatures(const uint64_t item, T * featuresOut){

            using namespace vigra::acc;

            const auto & a = accs_[item];
            featuresOut[0] = get<Mean>(a);
            featuresOut[1] = get<Minimum>(a);
            featuresOut[2] = get<Maximum>(a);
            featuresOut[3] = get<Variance>(a);
            featuresOut[4] = get<Skewness>(a);
            featuresOut[5] = get<Kurtosis>(a);
            vigra::TinyVector<double, 7> quants = get<Quantiles>(a);
            featuresOut[6] = quants[1]; 
            featuresOut[7] = quants[2]; 
            featuresOut[8] = quants[3]; 
            featuresOut[9] = quants[4]; 
            featuresOut[10] = quants[5]; 
        }   
        
        //void resetFrom(const DefaultAccMapBase & other){
        //    std::copy(other.accs_.begin(), other.accs_.end(), accs_.begin());
        //    globalMinVal_ = other.globalMinVal_;
        //    globalMinVal_ = other.globalMaxVal_;
        //    currentPass_ = other.currentPass_;
        //}

    private:
        const GRAPH & graph_;
        size_t currentPass_;
        //T globalMinVal_;
        //T globalMaxVal_;
        ITEM_MAP<AccumulatorType> accs_;
    };
    
    
    template<class GRAPH, class T, unsigned int NBINS=40>
    class VigraAccEdgeMap : 
        public  VigraAccMapBase<GRAPH, T, NBINS, GRAPH:: template EdgeMap >
    {
    public:
        typedef GRAPH Graph;
        //typedef typename Graph:: template EdgeMap<DefaultAcc<T, NBINS> > BasesBaseType;
        typedef VigraAccMapBase<Graph, T, NBINS, Graph:: template EdgeMap >  BaseType;
        using BaseType::BaseType;
    };

    template<class GRAPH, class T, unsigned int NBINS=40>
    class VigraAccNodeMap : 
        public  VigraAccMapBase<GRAPH, T, NBINS, GRAPH:: template NodeMap >
    {
    public:
        typedef GRAPH Graph;
        //typedef typename Graph:: template NodeMap<DefaultAcc<T, NBINS> > BasesBaseType;
        typedef VigraAccMapBase<Graph, T, NBINS, Graph:: template NodeMap >  BaseType;
        using BaseType::BaseType;
    };



    

    template< size_t DIM, class LABELS_TYPE, class T, class EDGE_MAP, class NODE_MAP>
    void gridRagAccumulateFeatures(
        const ExplicitLabelsGridRag<DIM, LABELS_TYPE> & graph,
        nifty::marray::View<T> data,
        EDGE_MAP & edgeMap,
        NODE_MAP &  nodeMap
    ){
        typedef std::array<int64_t, DIM> Coord;

        const auto labelsProxy = graph.labelsProxy();
        const auto & shape = labelsProxy.shape();
        const auto & labels = labelsProxy.labels(); 
        
        const auto numberOfPasses =  std::max(edgeMap.numberOfPasses(),nodeMap.numberOfPasses());
        for(size_t p=0; p<numberOfPasses; ++p){
            // start path p
            edgeMap.startPass(p);
            nodeMap.startPass(p);

            nifty::tools::forEachCoordinate(shape,[&](const Coord & coord){
                const auto lU = labels(coord);
                const auto dU = data(coord);
                nodeMap.accumulate(lU, dU);
                for(size_t axis=0; axis<DIM; ++axis){
                    Coord coord2 = coord;
                    coord2[axis] += 1;
                    if(coord2[axis] < shape[axis]){
                        const auto lV = labels(coord2);
                        if(lU != lV){
                            const auto e = graph.findEdge(lU, lV);
                            const auto dV = data(coord2);
                            edgeMap.accumulate(e, dU);
                            edgeMap.accumulate(e, dV);
                        }
                    }
                }
            });
        }
    }
    
    
    
    
    template<size_t DIM, class LABELS_TYPE, class LABELS, class NODE_MAP>
    void gridRagAccumulateLabels(
        const ExplicitLabelsGridRag<DIM, LABELS_TYPE> & graph,
        nifty::marray::View<LABELS> data,
        NODE_MAP &  nodeMap
    ){
        typedef std::array<int64_t, DIM> Coord;

        const auto labelsProxy = graph.labelsProxy();
        const auto & shape = labelsProxy.shape();
        const auto labels = labelsProxy.labels(); 

        std::vector<  std::unordered_map<uint64_t, uint64_t> > overlaps(graph.numberOfNodes());
        

        nifty::tools::forEachCoordinate(shape,[&](const Coord & coord){
            const auto x = coord[0];
            const auto y = coord[1];
            const auto node = labels(x,y);            
            const auto l  = data(x,y);
            overlaps[node][l] += 1;
        });

        for(const auto node : graph.nodes()){
            const auto & ol = overlaps[node];
            // find max ol 
            uint64_t maxOl = 0 ;
            uint64_t maxOlLabel = 0;
            for(auto kv : ol){
                if(kv.second > maxOl){
                    maxOl = kv.second;
                    maxOlLabel = kv.first;
                }
            }
            nodeMap[node] = maxOlLabel;
        }
    }
    
    

} // end namespace graph
} // end namespace nifty


#endif /* NIFTY_GRAPH_RAG_GRID_RAG_FEATURES_HXX */
