/*
 * CorrectionSet.cc
 *
 *  Created on: Aug 4, 2015
 *      Author: nmccoll
 */
#include <assert.h>
#include "TFile.h"

#include "AnalysisBase/TreeAnalyzer/interface/CorrectionSet.h"

namespace ucsbsusy {

Correction::Correction(TString corrName, TFile * file) : corr(0), name(corrName) {
  assert(file);
  std::clog << "Loading correction: "<< name <<std::endl;
  corr = (const QuickRefold::Refold*)(file->Get(name) );
  assert(corr);
}

CorrectionSet::CorrectionSet() : file(0), options_(0)  {}

void CorrectionSet::loadFile(TString correctionSetName, TString fileName, int correctionOptions) {
  options_ =correctionOptions;
  std::clog << "Loading file: "<< fileName <<" and correctionSet: " << correctionSetName <<std::endl;
  file = TFile::Open(fileName,"read");
  assert(file);
}

CorrectionSet::~CorrectionSet() {
  if(file)
    file->Close();
  delete file;

  for(auto correction : corrections)
    delete correction;
}


float CorrectionSet::getTotalCorrection() const  {
  float corr = 1;
  for(auto correction : corrections)
    corr *= correction->get();
  return corr;
}

} /* namespace ucsbsusy */
