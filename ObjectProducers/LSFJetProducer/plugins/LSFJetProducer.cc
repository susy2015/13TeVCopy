// -*- C++ -*-
//
// Package:    ObjectProducers/LSFJetProducer
// Class:      LSFJetProducer
// 
/**\class LSFJetProducer LSFJetProducer.cc ObjectProducers/LSFJetProducer/plugins/LSFJetProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Loukas Gouskos
//         Created:  Sun, 28 Dec 2014 14:21:07 GMT
//
//

//#ifndef OBJECTPRODUCERS_LSFJETPRODUCERS_LSFJETPRODUCER_H
//#define OBJECTPRODUCERS_LSFJETPRODUCERS_LSFJETPRODUCER_H

#include <functional>


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

#include "AnalysisBase/Analyzer/interface/BaseFiller.h"
#include "AnalysisBase/Analyzer/interface/EventInfoFiller.h"

#include "ObjectProducers/JetProducers/interface/FastJetClusterer.h"
#include "ObjectProducers/PickyJetUtilities/interface/Nsubjettiness.hh"
#include "ObjectProducers/PickyJetUtilities/interface/Njettiness.hh"
#include "ObjectProducers/PickyJetUtilities/interface/NjettinessPlugin.hh"
#include "ObjectProducers/PickyJetUtilities/interface/FastFunctions.h"

#include <fastjet/JetDefinition.hh>
#include <fastjet/PseudoJet.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/ClusterSequenceArea.hh>
#include "fastjet/tools/Filter.hh"

#include "DataFormats/Math/interface/LorentzVector.h"

#include <sstream>

//namespace ucsbsusy {

//
// class declaration
//
using namespace fastjet;
//using namespace fastjet::contrib;


class LSFJetProducer : public edm::EDProducer {
   public:
      explicit LSFJetProducer(const edm::ParameterSet&);
      ~LSFJetProducer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void produce(edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      
      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
  typedef math::PtEtaPhiMLorentzVectorF LorentzVector;
  typedef std::vector<LorentzVector> LorentzVectorCollection;
};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
LSFJetProducer::LSFJetProducer(const edm::ParameterSet& iConfig)
{
   //register your products
/* Examples
   produces<ExampleData2>();

   //if do put with a label
   produces<ExampleData2>("label");
 
   //if you want to put into the Run
   produces<ExampleData2,InRun>();
*/

  produces<LorentzVectorCollection>("LSFJets2");
  produces<LorentzVectorCollection>("LSFJets3");
  produces<LorentzVectorCollection>("LSFJets4");


   //now do what ever other initialization is needed
  
}


LSFJetProducer::~LSFJetProducer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
LSFJetProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
/* This is an event example
   //Read 'ExampleData' from the Event
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);

   //Use the ExampleData to create an ExampleData2 which 
   // is put into the Event
   std::unique_ptr<ExampleData2> pOut(new ExampleData2(*pIn));
   iEvent.put(std::move(pOut));
*/

/* this is an EventSetup example
   //Read SetupData from the SetupRecord in the EventSetup
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
*/




   edm::Handle<reco::PFJetCollection> fatJets;
   FileUtilities::enforceGet(iEvent, "redCA8",fatJets,true);


   double R = 1.5708;
   fastjet::JetDefinition jet_def(fastjet::kt_algorithm,R);
   int nSJ2 = 2;
   int nSJ3 = 3;
   int nSJ4 = 4;

   Nsubjettiness tau1(1, Njettiness::AxesMode::onepass_kt_axes, 1.0, 0.8);
   Nsubjettiness tau2(2, Njettiness::AxesMode::onepass_kt_axes, 1.0, 0.8);
   Nsubjettiness tau3(3, Njettiness::AxesMode::onepass_kt_axes, 1.0, 0.8);
   Nsubjettiness tau4(4, Njettiness::AxesMode::onepass_kt_axes, 1.0, 0.8);

   std::vector<fastjet::PseudoJet> subJets2FJ_;
   std::vector<fastjet::PseudoJet> subJets3FJ_;
   std::vector<fastjet::PseudoJet> subJets4FJ_;
   for (reco::PFJetCollection::const_iterator fjt = fatJets->begin(); fjt != fatJets->end(); fjt++) {

     std::vector<fastjet::PseudoJet> FJparticles;
     for(UInt_t iD = 0; iD < fjt->numberOfDaughters(); ++iD){
       
       const reco::Candidate *this_constituent = fjt->daughter(iD);
       FJparticles.push_back( fastjet::PseudoJet( this_constituent->px(), this_constituent->py(), this_constituent->pz(), this_constituent->energy() ) );

     } // end of looping over the constituents of each fat jet
     fastjet::PseudoJet combJet = fastjet::join(FJparticles);
     //cout << tau1.result(combJet) << " " << tau2.result(combJet) << " " << tau3.result(combJet) << " " << tau4.result(combJet) << "\n";

     fastjet::ClusterSequence clust_seq_exkt(FJparticles, jet_def);

     std::vector<fastjet::PseudoJet> sj2_exkt_ = sorted_by_pt(clust_seq_exkt.exclusive_jets_up_to(nSJ2));
     std::vector<fastjet::PseudoJet> sj3_exkt_ = sorted_by_pt(clust_seq_exkt.exclusive_jets_up_to(nSJ3));
     std::vector<fastjet::PseudoJet> sj4_exkt_ = sorted_by_pt(clust_seq_exkt.exclusive_jets_up_to(nSJ4));

     // loop over the subjets and store them in the vector

     for (UInt_t isj = 0; isj < sj2_exkt_.size(); ++isj) {

       subJets2FJ_.push_back( fastjet::PseudoJet( sj2_exkt_[isj].px(), sj2_exkt_[isj].py(), sj2_exkt_[isj].pz(), sj2_exkt_[isj].m() ) );

     }

     for (UInt_t isj = 0; isj < sj3_exkt_.size(); ++isj) {

       subJets3FJ_.push_back( fastjet::PseudoJet( sj3_exkt_[isj].px(), sj3_exkt_[isj].py(), sj3_exkt_[isj].pz(), sj3_exkt_[isj].m() ) );

     }

     for (UInt_t isj = 0; isj < sj4_exkt_.size(); ++isj) {

       subJets4FJ_.push_back( fastjet::PseudoJet( sj4_exkt_[isj].px(), sj4_exkt_[isj].py(), sj4_exkt_[isj].pz(), sj4_exkt_[isj].m() ) );

     }

   } // end of looping over the fat jets

   std::vector<fastjet::PseudoJet> subJets2FJ = sorted_by_pt(subJets2FJ_);
   std::vector<fastjet::PseudoJet> subJets3FJ = sorted_by_pt(subJets3FJ_);
   std::vector<fastjet::PseudoJet> subJets4FJ = sorted_by_pt(subJets4FJ_);


   auto_ptr<LorentzVectorCollection> LSFJets2 ( new LorentzVectorCollection );
   auto_ptr<LorentzVectorCollection> LSFJets3 ( new LorentzVectorCollection );
   auto_ptr<LorentzVectorCollection> LSFJets4 ( new LorentzVectorCollection );

   const int size2 = subJets2FJ.size();
   const int size3 = subJets3FJ.size();
   const int size4 = subJets4FJ.size();

   LSFJets2->reserve(size2);
   LSFJets3->reserve(size3);
   LSFJets4->reserve(size4);

   // make the TLorentzVectors
   for (UInt_t isj = 0; isj<subJets2FJ.size(); ++isj) {

     LorentzVector TLV_;
     TLV_.SetPxPyPzE(subJets2FJ[isj].px(),subJets2FJ[isj].py(),subJets2FJ[isj].pz(),subJets2FJ[isj].e());
     LSFJets2->push_back(TLV_);

   }
   
   for (UInt_t isj = 0; isj<subJets3FJ.size(); ++isj) {

     LorentzVector TLV_;
     TLV_.SetPxPyPzE(subJets3FJ[isj].px(),subJets3FJ[isj].py(),subJets3FJ[isj].pz(),subJets3FJ[isj].e());
     LSFJets3->push_back(TLV_);

   }

   for (UInt_t isj = 0; isj<subJets4FJ.size(); ++isj) {

     LorentzVector TLV_;
     TLV_.SetPxPyPzE(subJets4FJ[isj].px(),subJets4FJ[isj].py(),subJets4FJ[isj].pz(),subJets4FJ[isj].e());
     LSFJets4->push_back(TLV_);

   }
   
   iEvent.put(LSFJets2,"LSFJets2");
   iEvent.put(LSFJets3,"LSFJets3");
   iEvent.put(LSFJets4,"LSFJets4");
   
}

// ------------ method called once each job just before starting event loop  ------------
void 
LSFJetProducer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
LSFJetProducer::endJob() {
}

// ------------ method called when starting to processes a run  ------------
/*
void
LSFJetProducer::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
LSFJetProducer::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
LSFJetProducer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
LSFJetProducer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------

void
LSFJetProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
 
//define this as a plug-in
DEFINE_FWK_MODULE(LSFJetProducer);
