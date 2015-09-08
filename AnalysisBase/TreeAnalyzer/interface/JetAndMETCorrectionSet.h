/*
 * CorrectionSet.h
 *
 *  Created on: Aug 4, 2015
 *      Author: nmccoll
 */

#ifndef ANALYSISBASE_TREEANALYZER_INTERFACE_JETANDMETCORRECTIONSET_H_
#define ANALYSISBASE_TREEANALYZER_INTERFACE_JETANDMETCORRECTIONSET_H_

#include "AnalysisBase/TreeAnalyzer/interface/CorrectionSet.h"
#include "AnalysisTools/Utilities/interface/Types.h"
#include "AnalysisTools/DataFormats/interface/Momentum.h"


namespace ucsbsusy {

class METScaleCorr : public Correction {
public:
  METScaleCorr() : Correction("METScale") {}
  float get() const {return 1.0;}
  float scaleCorr(float trueMETPT) const;
  CylLorentzVectorF getCorrectedMET(const CylLorentzVectorF& trueBosons, const CylLorentzVectorF& met) const;
};

class METResCorr : public Correction {
public:
  METResCorr();
  float get() const {return 1.0;}
  CylLorentzVectorF getCorrectedMET(const CylLorentzVectorF& trueBosons,const CylLorentzVectorF& trueMET, CylLorentzVectorF met) const;
  float metParScale;
  float metPerpScale;
};

class JetAndMETCorrectionSet : public CorrectionSet {
public:
  enum  CorrectionOptions {
                            NULLOPT               = 0
                          , METSCALE              = (1 <<  0)   ///< Correct MET Scale
                          , METRESOLUTION         = (1 <<  1)   ///< Correct MET Resolution
  };
  JetAndMETCorrectionSet(): metScale(0),metResolution(0) {}
  virtual ~JetAndMETCorrectionSet() {};
  virtual void load(int correctionOptions = NULLOPT);
  virtual void processMET(const BaseTreeAnalyzer * ana);
  virtual void processCorrection(const BaseTreeAnalyzer * ana) {}; //does not apply to this guy, must process corrections when he wants

  //individual accessors
  MomentumF getCorrectedMET() const {return correctedMET;}
  MomentumF getOriginalMET() const {return originalMET;}

private:
  //Correction list
  METScaleCorr * metScale;
  METResCorr * metResolution;

  MomentumF originalMET;
  MomentumF correctedMET;

  CylLorentzVectorF trueBosons; //direction of the correction
  CylLorentzVectorF trueMET; //neutrinos from EWK bosons or LSPs
};


} /* namespace ucsbsusy */

#endif /* ANALYSISBASE_TREEANALYZER_INTERFACE_JETANDMETCORRECTIONSET_H_ */
