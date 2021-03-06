#include "PhysicsTools/SelectorUtils/interface/CutApplicatorBase.h"
// #include "PhysicsTools/SelectorUtils/interface/CutApplicatorWithEventContentBase.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"

class GsfEleFull5x5SigmaIEtaIEtaCut72X : public CutApplicatorBase {
public:
  GsfEleFull5x5SigmaIEtaIEtaCut72X(const edm::ParameterSet& c);
  
  result_type operator()(const reco::GsfElectronPtr&) const override final;

  // void setConsumes(edm::ConsumesCollector&) override final;
  // void getEventContent(const edm::EventBase&) override final;

  CandidateType candidateType() const override final { 
    return ELECTRON; 
  }

private:
  float _full5x5SigmaIEtaIEtaCutValueEB;
  float _full5x5SigmaIEtaIEtaCutValueEE;
  float _barrelCutOff;
  // edm::Handle<edm::ValueMap<float> > _full5x5SigmaIEtaIEtaMap;

  // constexpr static char full5x5SigmaIEtaIEta_[] = "full5x5SigmaIEtaIEta";
};

// constexpr char GsfEleFull5x5SigmaIEtaIEtaCut72X::full5x5SigmaIEtaIEta_[];

DEFINE_EDM_PLUGIN(CutApplicatorFactory,
		  GsfEleFull5x5SigmaIEtaIEtaCut72X,
		  "GsfEleFull5x5SigmaIEtaIEtaCut72X");

GsfEleFull5x5SigmaIEtaIEtaCut72X::GsfEleFull5x5SigmaIEtaIEtaCut72X(const edm::ParameterSet& c) :
  CutApplicatorBase(c),
  _full5x5SigmaIEtaIEtaCutValueEB(c.getParameter<double>("full5x5SigmaIEtaIEtaCutValueEB")),
  _full5x5SigmaIEtaIEtaCutValueEE(c.getParameter<double>("full5x5SigmaIEtaIEtaCutValueEE")),
  _barrelCutOff(c.getParameter<double>("barrelCutOff")) {
  
  // edm::InputTag maptag = c.getParameter<edm::InputTag>("full5x5SigmaIEtaIEtaMap");
  // contentTags_.emplace(full5x5SigmaIEtaIEta_,maptag);
}

// void GsfEleFull5x5SigmaIEtaIEtaCut72X::setConsumes(edm::ConsumesCollector& cc) {
//   auto full5x5SigmaIEtaIEta = 
//     cc.consumes<edm::ValueMap<float> >(contentTags_[full5x5SigmaIEtaIEta_]);
//   contentTokens_.emplace(full5x5SigmaIEtaIEta_,full5x5SigmaIEtaIEta);
// }

// void GsfEleFull5x5SigmaIEtaIEtaCut72X::getEventContent(const edm::EventBase& ev) {  
//   ev.getByLabel(contentTags_[full5x5SigmaIEtaIEta_],_full5x5SigmaIEtaIEtaMap);
// }

CutApplicatorBase::result_type 
GsfEleFull5x5SigmaIEtaIEtaCut72X::
operator()(const reco::GsfElectronPtr& cand) const{  

  // Figure out the cut value
  const float full5x5SigmaIEtaIEtaCutValue = 
    ( std::abs(cand->superCluster()->position().eta()) < _barrelCutOff ? 
      _full5x5SigmaIEtaIEtaCutValueEB : _full5x5SigmaIEtaIEtaCutValueEE );
  
  // Retrieve the variable value for this particle
  // const float full5x5SigmaIEtaIEta = (*_full5x5SigmaIEtaIEtaMap)[cand];
  const float full5x5SigmaIEtaIEta = cand->full5x5_sigmaEtaEta();
  
  // Apply the cut and return the result
  return full5x5SigmaIEtaIEta < full5x5SigmaIEtaIEtaCutValue;
}
