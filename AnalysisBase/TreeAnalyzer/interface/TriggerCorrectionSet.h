/*
 * TriggerCorrectionSet.h
 *
 */

#ifndef ANALYSISBASE_TREEANALYZER_INTERFACE_TRIGGERCORRECTIONSET_H_
#define ANALYSISBASE_TREEANALYZER_INTERFACE_TRIGGERCORRECTIONSET_H_

#include "AnalysisBase/TreeAnalyzer/interface/CorrectionSet.h"

namespace ucsbsusy {

class PhotonTriggerCorrection : public Correction {
public :
  PhotonTriggerCorrection(TString corrName, TFile* file) :
    Correction(corrName), corrEE("HLT_Photon165_HE10_EE", file), corrEB("HLT_Photon165_HE10_EB", file) {} // needs a better way to set the graph names
  ~PhotonTriggerCorrection() {}

  const double ETA_EB = 1.479;

  double get(CORRTYPE corrType, double pt, double eta);

private:
  GraphAsymmErrorsCorrectionHelper corrEE;
  GraphAsymmErrorsCorrectionHelper corrEB;
};

class ElectronTriggerCorrection : public Correction {
public :
  ElectronTriggerCorrection(TString corrName, TFile* file) :
    Correction(corrName), eta_range{0, 0.8, 1.479, 10} {
    corrs.emplace_back("efftrig_eta0p000to0p800_data_el", file);
    corrs.emplace_back("efftrig_eta0p800to1p479_data_el", file);
    corrs.emplace_back("efftrig_eta1p479to2p100_data_el", file);
  }
  ~ElectronTriggerCorrection() {}

  double get(CORRTYPE corrType, double pt, double eta);

private:
  std::vector<double> eta_range;
  std::vector<GraphAsymmErrorsCorrectionHelper> corrs;
};

class MuonTriggerCorrection : public Correction {
public :
  MuonTriggerCorrection(TString corrName, TFile* file) :
    Correction(corrName), eta_range{0, 0.8, 1.5, 2.1, 10} {
      corrs.emplace_back("efftrig_eta0p000to0p800_data_mu", file);
      corrs.emplace_back("efftrig_eta0p800to1p500_data_mu", file);
      corrs.emplace_back("efftrig_eta1p500to2p100_data_mu", file);
      corrs.emplace_back("efftrig_eta2p100to2p400_data_mu", file);
    }
  ~MuonTriggerCorrection() {}

  double get(CORRTYPE corrType, double pt, double eta);

private:
  std::vector<double> eta_range;
  std::vector<GraphAsymmErrorsCorrectionHelper> corrs;
};

// ----------------------------------------------------

class TriggerCorrectionSet : public CorrectionSet {
public:
  enum  CorrectionOptions {
                            NULLOPT          = 0
                          , PHOTON           = (1 <<  0)
                          , ELECTRON         = (1 <<  1)
                          , MUON             = (1 <<  1)

  };
  TriggerCorrectionSet(): trigPhoCorr(0), trigEleCorr(0), trigMuCorr(0), trigPhoWeight(1), trigEleWeight(1), trigMuWeight(1) {}

  virtual ~TriggerCorrectionSet() {};
  virtual void load(TString filename, int correctionOptions = NULLOPT);
  virtual void processCorrection(const BaseTreeAnalyzer * ana);

  //individual accessors
  float getTrigPhoWeight() const {return trigPhoWeight;}
  float getTrigEleWeight() const {return trigEleWeight;}
  float getTrigMuWeight()  const {return trigMuWeight;}

private:
  //Correction list
  PhotonTriggerCorrection *trigPhoCorr;
  ElectronTriggerCorrection *trigEleCorr;
  MuonTriggerCorrection *trigMuCorr;

  //output values
  float trigPhoWeight;
  float trigEleWeight;
  float trigMuWeight;


};


} /* namespace ucsbsusy */

#endif /* ANALYSISBASE_TREEANALYZER_INTERFACE_TRIGGERCORRECTIONSET_H_ */
