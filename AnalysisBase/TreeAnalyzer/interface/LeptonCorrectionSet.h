/*
 * CorrectionSet.h
 *
 *  Created on: Aug 4, 2015
 *      Author: nmccoll
 */

#ifndef ANALYSISBASE_TREEANALYZER_INTERFACE_LEPTONCORRECTIONSET_H_
#define ANALYSISBASE_TREEANALYZER_INTERFACE_LEPTONCORRECTIONSET_H_

#include "AnalysisBase/TreeAnalyzer/interface/CorrectionSet.h"

namespace ucsbsusy {

  class LepCorr : public HistogramCorrection {

    public:
      LepCorr(TFile * file)  : HistogramCorrection("LEP",file) {}

      static const unsigned int defaultBin       = 1;
      static const unsigned int muCorrBinLowPt   = 2;
      static const unsigned int muCorrBinHighPt  = 3;
      static const unsigned int eleCorrBinLowPt  = 4;
      static const unsigned int eleCorrBinHighPt = 5;
      static const unsigned int tauCorrBinLowPt  = 6;
      static const unsigned int tauCorrBinHighPt = 7;
      static const unsigned int hpsTauCorrBin    = 8;
      static const unsigned int fakeBin          = 9;

  };

  class LeptonCorrectionSet : public CorrectionSet {

    public :
      enum CorrectionOptions {
        NULLOPT       = 0,        // no correction
        LEP           = (1 << 0), // apply lepton corrections
        LEP_VARY_UP   = (1 << 1), // systematic variation up
        LEP_VARY_DOWN = (1 << 2), // systematic variation down
        USE_MVATAUS   = (1 << 3)
      }; 

      LeptonCorrectionSet() : lepCorr(0), vetoLepWeight(1), selLepWeight(1), useHPS(true) {}
      virtual ~LeptonCorrectionSet() {}
      virtual void load(TString fileName, int correctionOptions = NULLOPT);
      virtual void processCorrection(const BaseTreeAnalyzer * ana);

      float getVetoLepWeight()     const { return vetoLepWeight; }
      float getSelLepWeight()      const { return selLepWeight;  }
      void  setUseHPSTaus(bool setHPS)   { useHPS = setHPS;      }

    private :
      LepCorr* lepCorr;

      float vetoLepWeight;
      float selLepWeight;
      bool  useHPS;
  };

} /* namespace ucsbsusy */

#endif /* ANALYSISBASE_TREEANALYZER_INTERFACE_LEPTONCORRECTIONSET_H_ */
