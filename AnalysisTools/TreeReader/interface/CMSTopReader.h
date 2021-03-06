//--------------------------------------------------------------------------------------------------
// 
// JetReader
// 
// Class for reading electron object information from TTree.
// 
// JetReader.h created on Wed Oct 22 01:39:44 CEST 2014
// 
//--------------------------------------------------------------------------------------------------

#ifndef ANALYSISTOOLS_TREEREADER_CMSTOPREADER_H
#define ANALYSISTOOLS_TREEREADER_CMSTOPREADER_H


#include "AnalysisTools/TreeReader/interface/BaseReader.h"
#include "AnalysisTools/DataFormats/interface/CMSTop.h"
#include <TRandom3.h>

namespace ucsbsusy {

  class CMSTopReader : public BaseReader {
  public :
    enum  Options {
                      NULLOPT     = 0
                    , LOADRECO    = (1 << 0)
                    , LOADEXTRECO = (1 <<  1)   ///< Load extra info
                    , FILLOBJ     = (1 << 2)   ///< Fill objects (as opposed to just pointers
    };
    static const int defaultOptions;
    
    CMSTopReader();
    ~CMSTopReader() {}
    
    void load(TreeReader *treeReader, int options, std::string branchName);
    void refresh();
    
    void pushToTree(); //push changes made to the momentum back to the tree
  public:
    // Members to hold info to be filled in the tree (for now; this implementation is to be updated)
    std::vector<float>* fjmass_;
    std::vector<float>* fjtrimmedmass_;
    std::vector<float>* fjprunedmass_;
    std::vector<float>* fjsoftdropmass_;
    std::vector<float>* fjfilteredmass_;
    std::vector<float>* fjmdfilteredmass_;
    std::vector<float>* fjtau1_;
    std::vector<float>* fjtau2_;
    std::vector<float>* fjtau3_;
    std::vector<float>* toppt_;
    std::vector<float>* topeta_;
    std::vector<float>* topphi_;
    std::vector<float>* topmass_;
    std::vector<float>* wmass_;
    std::vector<float>* minmass_;
    std::vector<int>*   nsubjets_;

    //the actual jet collection
    CMSTopFCollection cmsTops;

};

}

#endif
