//--------------------------------------------------------------------------------------------------
// 
// PhysicsAnalyzer
// 
// Implementation of BaseAnalyzer with a few additions:
// It provides an interface to event objects, providing functions to load the event content
// It stores the event content classes for easy processing
// It includes a few more advanced header files that are useful in analyzers
// It stores what kind of event it is in addition to the event weight
// 
//--------------------------------------------------------------------------------------------------

#ifndef PHYSICSANALYZER_H
#define PHYSICSANALYZER_H

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "AnalysisBase/Analyzer/interface/BaseAnalyzer.h"
//#include "AnalysisTools/Utilities/interface/PhysicsUtilities.h"

// Data products
#include "AnalysisBase/Analyzer/interface/EventInfoFiller.h"
#include "AnalysisBase/Analyzer/interface/PatJetFiller.h"
#include "AnalysisBase/Analyzer/interface/MuonFiller.h"
#include "AnalysisBase/Analyzer/interface/ElectronFiller.h"
#include "AnalysisBase/Analyzer/interface/TauFiller.h"
#include "AnalysisBase/Analyzer/interface/GenParticleFiller.h"

namespace ucsbsusy {

  class PhysicsAnalyzer : public BaseAnalyzer {
  public:


    //--------------------------------------------------------------------------------------------------
    // Functions for base-level running
    //--------------------------------------------------------------------------------------------------
    public :
      PhysicsAnalyzer(const edm::ParameterSet& iConfig);
      virtual ~PhysicsAnalyzer();

      virtual void beginJob() override;
      virtual void book();
      virtual bool load(const edm::Event& iEvent, const edm::EventSetup& iSetup);
      virtual bool filter(edm::Event& iEvent, const edm::EventSetup& iSetup) override;
      virtual void fill();

      //--------------------------------------------------------------------------------------------------
      // Functions for running the default variable types
      //--------------------------------------------------------------------------------------------------
    public:
      enum VarType {EVTINFO, AK4JETS,ELECTRONS, MUONS, TAUS, GENPARTICLES};
      virtual void initialize(const edm::ParameterSet& cfg, VarType type, int options = -1, std::string branchName = "" );
      virtual void initialize(BaseFiller * filler);

      //--------------------------------------------------------------------------------------------------
      // Functions to manipulate and get stored data members
      //--------------------------------------------------------------------------------------------------
    public:
      bool isData() const;
      bool isMC  () const;

      //--------------------------------------------------------------------------------------------------
      // Data members owned by this class
      //--------------------------------------------------------------------------------------------------
    public :
      const int     isRealData;            // Whether or not processing real data; deduced from input file name and verified once first event is loaded
      const TString globalTag;             // Global tag name

      //--------------------------------------------------------------------------------------------------
      // "Filler" classes to store event information
      //--------------------------------------------------------------------------------------------------
    public:
      EventInfoFiller * eventInfo;
      PatJetFiller    * ak4Jets;
      MuonFiller      * muons;
      ElectronFiller  * electrons;
      TauFiller       * taus;
      GenParticleFiller * genparticles;
    protected:
      //vector of initialized fillers for automatic processing
      std::vector<BaseFiller*> initializedFillers;
  };

}

#endif
