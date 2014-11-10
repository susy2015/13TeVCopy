//--------------------------------------------------------------------------------------------------
// 
// JetFiller
// 
// Class to fill some basic jet information for testing. To be developed as needed.
// 
// JetFiller.cc created on Thu Aug 14 11:10:55 CEST 2014 
// 
//--------------------------------------------------------------------------------------------------

#include "AnalysisBase/Analyzer/interface/JetFiller.h"
#include "AnalysisTools/Utilities/interface/JetFlavorMatching.h"
#include "AnalysisTools/Utilities/interface/Types.h"

#include "AnalysisBase/Analyzer/interface/EventInfoFiller.h"
#include "AnalysisTools/JetShapeVariables/interface/QuarkGluonTagInterface.h"
#include "AnalysisTools/JetShapeVariables/interface/QuarkGluonTaggingVariables.h"

using namespace ucsbsusy;

#define TAGGABLE_TYPE_HACK
#define REDEFINED_GENJET_HACK

//--------------------------------------------------------------------------------------------------
JetFiller::JetFiller(const edm::ParameterSet &cfg, const bool isMC, const EventInfoFiller * evtInfoFiller) :
  fillGenInfo_     (isMC && cfg.getUntrackedParameter<bool>("fillJetGenInfo",false)),
  fillJetShapeInfo_(cfg.getUntrackedParameter<bool>("fillJetShapeInfo",false)),
  jetTag_          (cfg.getParameter<edm::InputTag>("jets")),
  reGenJetTag_     (fillGenInfo_      ? cfg.getParameter<edm::InputTag>("reGenJets")  : edm::InputTag()),
  stdGenJetTag_    (fillGenInfo_      ? cfg.getParameter<edm::InputTag>("stdGenJets")  : edm::InputTag()),
  genParticleTag_  (fillGenInfo_      ? cfg.getParameter<edm::InputTag>("genParticles") : edm::InputTag()),
  jptMin_          (cfg.getUntrackedParameter<double>("minJetPt")),
  jetsName_        (cfg.getUntrackedParameter<string>("jetsType")),
  evtInfofiller_   (evtInfoFiller),
  qglInterface_    (fillJetShapeInfo_ ? new QuarkGluonTagInterface : 0),
  qgTaggingVar_    (fillJetShapeInfo_ ? new QuarkGluonTaggingVariables : 0),
  jets_            (0),
  reGenJets_       (0),
  stdGenJets_      (0)
{
  jetpt_     = data.addMulti<float>(jetsName_,"jet_pt"    ,0);
  jeteta_    = data.addMulti<float>(jetsName_,"jet_eta"   ,0);
  jetphi_    = data.addMulti<float>(jetsName_,"jet_phi"   ,0);
  jetmass_   = data.addMulti<float>(jetsName_,"jet_mass"  ,0);
  jetptraw_  = data.addMulti<float>(jetsName_,"jet_ptraw" ,0);
  jetpuId_   = data.addMulti<float>(jetsName_,"jet_puId"  ,0);
  jetcsv_    = data.addMulti<float>(jetsName_,"jet_csv"   ,0);
  jetflavor_ = data.addMulti<int  >(jetsName_,"jet_flavor",0);

  if(fillGenInfo_) {
    genjetpt_     = data.addMulti<float>(jetsName_,"matchedgenjet_pt"    ,0);
    genjeteta_    = data.addMulti<float>(jetsName_,"matchedgenjet_eta"   ,0);
    genjetphi_    = data.addMulti<float>(jetsName_,"matchedgenjet_phi"   ,0);
    genjetmass_   = data.addMulti<float>(jetsName_,"matchedgenjet_mass"  ,0);
    genjetflavor_ = data.addMulti<int  >(jetsName_,"matchedgenjet_flavor",0);
  }

  if(fillJetShapeInfo_) {
    jetbetaStar_=data.addMulti<float>(jetsName_,"jet_betaStar",0);
    jetqgl_     =data.addMulti<float>(jetsName_,"jet_qgl"     ,0);
    jetptD_     =data.addMulti<float>(jetsName_,"jet_ptD"     ,0);
    jetaxis1_   =data.addMulti<float>(jetsName_,"jet_axis1"   ,0);
    jetaxis2_   =data.addMulti<float>(jetsName_,"jet_axis2"   ,0);
    jetMult_    =data.addMulti<int  >(jetsName_,"jet_jetMult" ,0);
    if(fillGenInfo_) {
      genjetptD_  = data.addMulti<float>(jetsName_,"matchedgenjet_ptD"    ,0);
      genjetaxis1_= data.addMulti<float>(jetsName_,"matchedgenjet_axis1"  ,0);
      genjetaxis2_= data.addMulti<float>(jetsName_,"matchedgenjet_axis2"  ,0);
      genjetMult_ = data.addMulti<int  >(jetsName_,"matchedgenjet_jetMult",0);
    }
  }
}

//--------------------------------------------------------------------------------------------------
reco::GenJetRef JetFiller::getReGenJet(const pat::Jet& jet) const
{
  if (!reGenJets_.isValid())
    throw cms::Exception("JetFiller.getReGenJet()", "genJets have not been loaded.");

  const reco::CandidatePtr&   genPtr  = jet.userCand(REGENJET);
  if (genPtr.id() != reGenJets_.id())
    throw cms::Exception("JetFiller.getReGenJet()", "Inconsistent reGenJets collection with pat::Jet::userCand(REGENJET).");

  return reco::GenJetRef(reGenJets_, genPtr.key());
}

//--------------------------------------------------------------------------------------------------
reco::GenJetRef JetFiller::getStdGenJet(const pat::Jet& jet) const
{
  return jet.genJetFwdRef().backRef();
}

//--------------------------------------------------------------------------------------------------
void JetFiller::load(edm::Event& iEvent)
{
  reset();
  FileUtilities::enforceGet(iEvent,jetTag_,jets_,true);

  if(fillGenInfo_) {
    FileUtilities::enforceGet(iEvent,reGenJetTag_,reGenJets_,true);
    FileUtilities::enforceGet(iEvent,stdGenJetTag_,stdGenJets_,true);
//    edm::Handle<reco::GenParticleCollection> genParticles_;
//    FileUtilities::enforceGet(iEvent,genParticleTag_,genParticles_,true);

#ifndef TAGGABLE_TYPE_HACK
    std::vector<HadronDecay> bHadrons = JetFlavorMatching::getBHadronDecays(genParticles_);
    JetFlavorMatching::storeBHadronInfo(*jets_,*reGenJets_,bHadrons);
#endif
  }

  isLoaded_ = true;

}

//--------------------------------------------------------------------------------------------------
void JetFiller::fill(TreeWriter& tW, const int& numAnalyzed)
{
  int index = -1;
  for (const pat::Jet &j : *jets_) {

#ifdef REDEFINED_GENJET_HACK
    const reco::GenJetRef gJ = getStdGenJet(j);
    if(j.pt() < jptMin_ && (fillGenInfo_ ? (gJ.isNonnull() ? gJ->pt() : 0) < jptMin_ : true)) continue;
#else
    const reco::GenJet * gJ = fillGenInfo_ ? &(*getReGenJet(j)) : 0;
    if(j.pt() < jptMin_ && (fillGenInfo_ ? gJ->pt() : 0) < jptMin_ : true) continue;
#endif
    index++;

    data.fillMulti<float>(jetpt_   ,j.pt());
    data.fillMulti<float>(jeteta_  ,j.eta());
    data.fillMulti<float>(jetphi_  ,j.phi());
    data.fillMulti<float>(jetmass_ ,j.mass());
    data.fillMulti<float>(jetptraw_,j.pt()*j.jecFactor("Uncorrected"));
    data.fillMulti<float>(jetpuId_ ,j.userFloat("pileupJetId:fullDiscriminant"));
    data.fillMulti<float>(jetcsv_  ,j.bDiscriminator("combinedInclusiveSecondaryVertexBJetTags"));


    if(fillGenInfo_) {
      data.fillMulti<int>(jetflavor_,j.partonFlavour());
#ifdef REDEFINED_GENJET_HACK
      if(gJ.isNonnull()) {
//#ifdef TAGGABLE_TYPE_HACK
//	jetflavor_.push_back(JetFlavorMatching::getPATTaggableType(j));
//#else
//	jetflavor_.push_back(JetFlavorMatching::getTaggableType(j));
//#endif
	data.fillMulti<float>(genjetpt_    ,gJ->pt());
	data.fillMulti<float>(genjeteta_   ,gJ->eta());
	data.fillMulti<float>(genjetphi_   ,gJ->phi());
	data.fillMulti<float>(genjetmass_  ,gJ->mass());
	data.fillMulti<int  >(genjetflavor_,j.partonFlavour());
      } else {
//	jetflavor_.push_back(numTaggableTypes);
        data.fillMulti<float>(genjetpt_    );
        data.fillMulti<float>(genjeteta_   );
        data.fillMulti<float>(genjetphi_   );
        data.fillMulti<float>(genjetmass_  );
        data.fillMulti<int  >(genjetflavor_);
      }
#else
//#ifdef TAGGABLE_TYPE_HACK
//      jetflavor_.push_back(JetFlavorMatching::getPATTaggableType(j));
//#else
//      jetflavor_.push_back(JetFlavorMatching::getTaggableType(j));
//#endif
      genjetpt_.push_back(gJ->pt());
      genjeteta_.push_back(gJ->eta());
      genjetphi_.push_back(gJ->phi());
      genjetmass_.push_back(gJ->mass());
      genjetflavor_.push_back(jetflavor_[index]);
#endif
    }
    if(fillJetShapeInfo_){
      assert(evtInfofiller_->isLoaded());
      data.fillMulti<float>(jetqgl_,qglInterface_->getDiscriminator(j,*evtInfofiller_->rhoHandle_));
      qgTaggingVar_->compute(&j);
      data.fillMulti<float>(jetbetaStar_,qgTaggingVar_->getBetaStar(&j,*evtInfofiller_->vertices_,evtInfofiller_->primaryVertexIndex_));
      data.fillMulti<float>(jetptD_     ,qgTaggingVar_->getPtD());
      data.fillMulti<float>(jetaxis1_   ,qgTaggingVar_->getAxis1());
      data.fillMulti<float>(jetaxis2_   ,qgTaggingVar_->getAxis2());
      data.fillMulti<int  >(jetMult_    ,qgTaggingVar_->getTotalMult());

      if(fillGenInfo_){
        if(gJ.isNonnull()) {
          qgTaggingVar_->compute(gJ.get());
          data.fillMulti<float>(genjetptD_     ,qgTaggingVar_->getPtD());
          data.fillMulti<float>(genjetaxis1_   ,qgTaggingVar_->getAxis1());
          data.fillMulti<float>(genjetaxis2_   ,qgTaggingVar_->getAxis2());
          data.fillMulti<float>(genjetMult_    ,qgTaggingVar_->getTotalMult());
        } else {
          data.fillMulti<float>(genjetptD_  );
          data.fillMulti<float>(genjetaxis1_);
          data.fillMulti<float>(genjetaxis2_);
          data.fillMulti<int  >(genjetMult_ );
        }
      }
    }
  }
  isFilled_ = true;
}

//--------------------------------------------------------------------------------------------------
const std::string   JetFiller::REGENJET           = "GenPtr";
