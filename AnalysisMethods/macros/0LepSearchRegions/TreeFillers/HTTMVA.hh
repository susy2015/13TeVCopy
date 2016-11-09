/*
  HTT MVA Tagger

  Struct:
    filler HTTMVAFiller
  Methods:
    book / fillBasicInfo for basics (# loose, # tight, pt, eta, sd mass, ropt mass, mva)
    bookTrain / fillTrainInfo for everything
  Filling scheme:
    fillMulti is used to fill every variable for every preselected fatjet in the event (pt 200, eta 2.4, softdrop mass 50, min subjet pt 20)
    thus if there are 5 preselected fatjets in the event every variable is filled exactly 5 times
    defaults are -9 -9. or false
    all csv of <=0 ( from failure of csv algo, few percent ) are set to 0 to save the MVA
    all eta are NOT abs(eta)
    a matching hadr. gen top quark is looked for only if process is ttbar(0l,1l) or signal. if one is found (see Matching Scheme) htt_matches_gen = true and gen variables are non-default (>=0)
    preselected fatjets are sorted by descending MVA score, thus so are all variables. use [0] to flatten all variables to best MVA scoring fatjet
  Matching scheme:
    for each preselected fatjet if the process is ttbar(0l,1l) or signal we look for a top quark satisfying
      "matching" : gen_maxdrqfj < ropt+0.1, meaning this top quark's b/w1/w2 quarks are contained within the fatjet ropt+0.1 cone
        --> gen_maxdrqfj is defined as MAX dR( the b/w1/w2 quarks from this hadr. top quark decay, the preselected fatjet to which this hadr. top quark is matched )
      "exclusion": gen_mindrtq > ropt, meaning no b/w1/w2 quarks from other hadr. top quark decays in the ropt cone
        --> gen_mindrtq is defined as min dR( this hadr. top quark , the b/w1/w2 quarks from other hadr. top quark decays )
    if both are satisfied, htt_matches_gen is filled with true. if not, false.
  Variables:
    variables and the fatjet preselections are calculated in ONE place, AnalysisTools/ObjectSelection/src/HTTMVA.hh
    for gen properties see *gen* and combine with htt_matches_gen (or else you'll get the default value), eg Scan("htt_gen_top_pt", "met>200 && htt_matches_gen")
    for preselected fat jet properties *fj*, their subjet properties *bydef* (by default method) and *bycsv* (by csv method), nick variables *nick*
    if wonky quickly check that vector variables are all filling correctly in parallel:
      tree->Scan("event:fj_pt:htt_matches_gen:gen_toppt")
                   1 : 100 : 0 : -9
                   1 : 150 : 1 : 160
                   2 : ...
      using two sets of vector variables, say if leptonpt were fillMulti and you did "fj_pt>400 && leptonpt>20", can cause shit to hit the fan
*/

#ifndef HTTMVA_HH
#define HTTMVA_HH

#include "AnalysisBase/TreeAnalyzer/interface/TreeCopier.h"
#include "AnalysisTools/DataFormats/interface/CMSTop.h"
#include "AnalysisTools/Utilities/interface/JetFlavorInfo.h"
#include "AnalysisTools/Utilities/interface/PhysicsUtilities.h"
#include "AnalysisTools/KinematicVariables/interface/JetKinematics.h"
#include "AnalysisBase/TreeAnalyzer/interface/DefaultProcessing.h"
#include "AnalysisTools/Utilities/interface/ParticleInfo.h"
#include <vector>
#include "Math/LorentzVector.h"
#include "Math/VectorUtil.h"
#include "AnalysisTools/Utilities/interface/PartonMatching.h"
#include "AnalysisTools/ObjectSelection/src/HTTMVA.cc"
#include "MVACommon.hh"

using namespace ucsbsusy;

struct HTTMVAFiller {

  HTTMVAFiller() {}

  std::vector<const HTTFatJetF*> preselectedFatjets; // those HTTTops passing the preselection defined in preparePreselectedFatJets
  //std::vector<pair<const PartonMatching::TopDecay*, const HTTFatJetF*> > matchPairs; // exactly one pair for every preselectedFatjet. first entry is the gen top if match found.
  std::vector<const PartonMatching::TopDecay*> gentops; // hadronic gen tops
  PartonMatching::PartonEvent * partonEvent;

  //// basic variables
  size i_nLooseHTTMVA;
  size i_nMediumHTTMVA;
  size i_nTightHTTMVA;
  size i_basic_htt_matches_gen;
  size i_basic_htt_pt;
  size i_basic_htt_eta;
  size i_basic_htt_sdmass;
  size i_basic_htt_roptmass;
  size i_basic_htt_mva;

  //// training variables
  // of gen top quark, if match found
  size i_htt_gen_top_pt;
  size i_htt_gen_w_pt;
  size i_htt_gen_b_pt;
  size i_htt_gen_mindrtt;      // min dR( this hadr. top quark , other hadr. top quarks ) 
  size i_htt_gen_mindrtq;      // min dR( this hadr. top quark , the b/w1/w2 quarks from other hadr. top quark decays )
  size i_htt_gen_mindrqq;      // min dR( the b/w1/w2 quarks from this hadr. top quark decay, the b/w1/w2 quarks from other hadr. top quark decays )
  size i_htt_gen_maxdrqfj;     // MAX dR( the b/w1/w2 quarks from this hadr. top quark decay, the preselected fatjet to which this hadr. top quark is matched )
  size i_htt_gen_quarkalarmpt; // highest pT b/w1/w2 quark from other hadr. top quark decays ("contaminant" quark) for which dR( this hadr. top quark, contaminant ) < 1.5
  size i_htt_gen_quarkalarmdr; // see previous. dR( this hadr. top quark, contaminant )

  // is preselected fatjet matched to gen top quark?
  size i_htt_matches_gen;

  // of preselected fatjet
  size i_htt_nsubjets;
  size i_htt_minsubjetpt;
  size i_htt_pt;
  size i_htt_eta;
  size i_htt_rawmass;
  size i_htt_sdmass;
  size i_htt_tau1;
  size i_htt_tau2;
  size i_htt_tau3;
  size i_htt_tau21;
  size i_htt_tau32;
  size i_htt_tau31;
  size i_htt_mva;

  // of ropt fatjet
  size i_htt_ropt_pt;
  size i_htt_ropt_mass;
  size i_htt_ropt_tau1;
  size i_htt_ropt_tau2;
  size i_htt_ropt_tau3;
  size i_htt_ropt_tau21;
  size i_htt_ropt_tau32;
  size i_htt_ropt_tau31;
  size i_htt_ropt;
  size i_htt_ropt_ptDR;
  size i_htt_ropt_calc;
  size i_htt_ropt_deltaRopt;
  size i_htt_ropt_deltaRopt_pt;
  size i_htt_ropt_frec;

  // of subjets: by default method
  size i_htt_bydef_b_pt;
  size i_htt_bydef_b_mass;
  size i_htt_bydef_b_csv;
  size i_htt_bydef_b_ptD;
  size i_htt_bydef_b_axis1;
  size i_htt_bydef_b_axis2;
  size i_htt_bydef_b_squaredaxis;
  size i_htt_bydef_b_mult;

  size i_htt_bydef_w1_mass;
  size i_htt_bydef_w1_pt;
  size i_htt_bydef_w1_csv;
  size i_htt_bydef_w1_ptD;
  size i_htt_bydef_w1_axis1;
  size i_htt_bydef_w1_axis2;
  size i_htt_bydef_w1_squaredaxis;
  size i_htt_bydef_w1_mult;

  size i_htt_bydef_w2_mass;
  size i_htt_bydef_w2_pt;
  size i_htt_bydef_w2_csv;
  size i_htt_bydef_w2_ptD;
  size i_htt_bydef_w2_axis1;
  size i_htt_bydef_w2_axis2;
  size i_htt_bydef_w2_squaredaxis;
  size i_htt_bydef_w2_mult;

  size i_htt_bydef_w_pt;
  size i_htt_bydef_w_mass;
  size i_htt_bydef_w_deltaR;
  size i_htt_bydef_w_ptDR;

  size i_htt_bydef_t_deltaR;
  size i_htt_bydef_t_ptDR;

  size i_htt_bydef_b_w1_mass;
  size i_htt_bydef_b_w2_mass;

  // of subjets, bycsv method: high csv is b, with pt2>pt3 for W subjets
  size i_htt_bycsv_b_pt;
  size i_htt_bycsv_b_mass;
  size i_htt_bycsv_b_csv;
  size i_htt_bycsv_b_ptD;
  size i_htt_bycsv_b_axis1;
  size i_htt_bycsv_b_axis2;
  size i_htt_bycsv_b_squaredaxis;
  size i_htt_bycsv_b_mult;

  size i_htt_bycsv_w1_pt;
  size i_htt_bycsv_w1_mass;
  size i_htt_bycsv_w1_csv;
  size i_htt_bycsv_w1_ptD;
  size i_htt_bycsv_w1_axis1;
  size i_htt_bycsv_w1_axis2;
  size i_htt_bycsv_w1_squaredaxis;
  size i_htt_bycsv_w1_mult;

  size i_htt_bycsv_w2_pt;
  size i_htt_bycsv_w2_mass;
  size i_htt_bycsv_w2_csv;
  size i_htt_bycsv_w2_ptD;
  size i_htt_bycsv_w2_axis1;
  size i_htt_bycsv_w2_axis2;
  size i_htt_bycsv_w2_squaredaxis;
  size i_htt_bycsv_w2_mult;

  size i_htt_bycsv_w_pt;
  size i_htt_bycsv_w_mass;
  size i_htt_bycsv_w_deltaR;
  size i_htt_bycsv_w_ptDR;

  size i_htt_bycsv_t_deltaR;
  size i_htt_bycsv_t_ptDR;

  size i_htt_bycsv_b_w1_mass;
  size i_htt_bycsv_b_w2_mass;


  /// nick MVA
  size i_htt_nick_mva;
  size i_htt_nick_matches_gen;

  // of ropt
  size i_htt_nick_ropt;
  size i_htt_nick_frec;
  size i_htt_nick_roptcalc;
  size i_htt_nick_ptforropt;
  size i_htt_nick_tau1;
  size i_htt_nick_tau2;
  size i_htt_nick_tau3;
  size i_htt_nick_ropt_pt;
  size i_htt_nick_ropt_mass;

  // pt and mass of sum of subjets' p4
  size i_htt_nick_subjet_pt;
  size i_htt_nick_subjet_mass;

  // of subjets, bycsv method
  size i_htt_nick_subjet_w_bycsv_pt;
  size i_htt_nick_subjet_w_bycsv_mass;
  size i_htt_nick_subjet_b_bycsv_mass;
  size i_htt_nick_subjet_b_bycsv_pt;
  size i_htt_nick_subjet_b_bycsv_csv;

  // of subjets, by default method
  size i_htt_nick_subjet_w_pt;
  size i_htt_nick_subjet_w_mass;
  size i_htt_nick_subjet_min_pt;
  size i_htt_nick_w1_pt;
  size i_htt_nick_w1_mass;
  size i_htt_nick_w1_csv;
  size i_htt_nick_w2_pt;
  size i_htt_nick_w2_mass;
  size i_htt_nick_w2_csv;
  size i_htt_nick_b_pt;
  size i_htt_nick_b_mass;
  size i_htt_nick_b_csv;

  void book(TreeWriterData* data) {
    i_nLooseHTTMVA          = data->add<int  >("","nLooseHTTMVA","I",-9);
    i_nMediumHTTMVA         = data->add<int  >("","nMediumHTTMVA","I",-9);
    i_nTightHTTMVA          = data->add<int  >("","nTightHTTMVA","I",-9);
    i_basic_htt_matches_gen = data->addMulti<bool >("","basic_htt_matches_gen",false);
    i_basic_htt_pt          = data->addMulti<float>("","basic_htt_pt",-9);
    i_basic_htt_eta         = data->addMulti<float>("","basic_htt_eta",-9);
    i_basic_htt_sdmass      = data->addMulti<float>("","basic_htt_sdmass",-9);
    i_basic_htt_roptmass    = data->addMulti<float>("","basic_htt_roptmass",-9);
    i_basic_htt_mva         = data->addMulti<float>("","basic_htt_mva",-9);
  }

  void bookTrain(TreeWriterData* data) {

    // of gen top quark, if match found
    i_htt_gen_top_pt           = data->addMulti<float>("","htt_gen_top_pt",-9);
    i_htt_gen_w_pt             = data->addMulti<float>("","htt_gen_w_pt"  ,-9);
    i_htt_gen_b_pt             = data->addMulti<float>("","htt_gen_b_pt"  ,-9);
    i_htt_gen_mindrtt          = data->addMulti<float>("","htt_gen_mindrtt",-9);
    i_htt_gen_mindrtq          = data->addMulti<float>("","htt_gen_mindrtq",-9);
    i_htt_gen_mindrqq          = data->addMulti<float>("","htt_gen_mindrqq",-9);
    i_htt_gen_maxdrqfj         = data->addMulti<float>("","htt_gen_maxdrqfj",-9);
    i_htt_gen_quarkalarmpt     = data->addMulti<float>("","htt_gen_quarkalarmpt",-9);
    i_htt_gen_quarkalarmdr     = data->addMulti<float>("","htt_gen_quarkalarmdr",-9);

    // is preselected fatjet matched to gen top quark?
    i_htt_matches_gen          = data->addMulti<bool >("","htt_matches_gen",false);

    // of preselected fatjet
    i_htt_nsubjets          = data->addMulti<int  >("","htt_nsubjets",-9);
    i_htt_minsubjetpt       = data->addMulti<float>("","htt_minsubjetpt",-9);
    i_htt_pt                = data->addMulti<float>("","htt_pt",-9);
    i_htt_eta               = data->addMulti<float>("","htt_eta",-9);
    i_htt_rawmass           = data->addMulti<float>("","htt_rawmass",-9);
    i_htt_sdmass            = data->addMulti<float>("","htt_sdmass",-9);
    i_htt_tau1              = data->addMulti<float>("","htt_tau1",-9);
    i_htt_tau2              = data->addMulti<float>("","htt_tau2",-9);
    i_htt_tau3              = data->addMulti<float>("","htt_tau3",-9);
    i_htt_tau21             = data->addMulti<float>("","htt_tau21",-9);
    i_htt_tau32             = data->addMulti<float>("","htt_tau32",-9);
    i_htt_tau31             = data->addMulti<float>("","htt_tau31",-9);
    i_htt_mva               = data->addMulti<float>("","htt_mva",-9);

    // of ropt fatjet
    i_htt_ropt_pt              = data->addMulti<float>("","htt_ropt_pt",-9);
    i_htt_ropt_mass            = data->addMulti<float>("","htt_ropt_mass",-9);
    i_htt_ropt_tau1            = data->addMulti<float>("","htt_ropt_tau1",-9);
    i_htt_ropt_tau2            = data->addMulti<float>("","htt_ropt_tau2",-9);
    i_htt_ropt_tau3            = data->addMulti<float>("","htt_ropt_tau3",-9);
    i_htt_ropt_tau21           = data->addMulti<float>("","htt_ropt_tau21",-9);
    i_htt_ropt_tau32           = data->addMulti<float>("","htt_ropt_tau32",-9);
    i_htt_ropt_tau31           = data->addMulti<float>("","htt_ropt_tau31",-9);
    i_htt_ropt                 = data->addMulti<float>("","htt_ropt",-9);
    i_htt_ropt_ptDR            = data->addMulti<float>("","htt_ropt_ptDR",-9);
    i_htt_ropt_calc            = data->addMulti<float>("","htt_ropt_calc",-9);
    i_htt_ropt_deltaRopt       = data->addMulti<float>("","htt_ropt_deltaRopt",-9);
    i_htt_ropt_deltaRopt_pt    = data->addMulti<float>("","htt_ropt_deltaRopt_pt",-9);
    i_htt_ropt_frec            = data->addMulti<float>("","htt_ropt_frec",-9);

    // of subjets: by default method
    i_htt_bydef_b_pt           = data->addMulti<float>("","htt_bydef_b_pt",-9);
    i_htt_bydef_b_mass         = data->addMulti<float>("","htt_bydef_b_mass",-9);
    i_htt_bydef_b_csv          = data->addMulti<float>("","htt_bydef_b_csv",-9);
    i_htt_bydef_b_ptD          = data->addMulti<float>("","htt_bydef_b_ptD",-9);
    i_htt_bydef_b_axis1        = data->addMulti<float>("","htt_bydef_b_axis1",-9);
    i_htt_bydef_b_axis2        = data->addMulti<float>("","htt_bydef_b_axis2",-9);
    i_htt_bydef_b_squaredaxis  = data->addMulti<float>("","htt_bydef_b_squaredaxis",-9);
    i_htt_bydef_b_mult         = data->addMulti<int  >("","htt_bydef_b_mult",-9);

    i_htt_bydef_w1_mass        = data->addMulti<float>("","htt_bydef_w1_mass",-9);
    i_htt_bydef_w1_pt          = data->addMulti<float>("","htt_bydef_w1_pt",-9);
    i_htt_bydef_w1_csv         = data->addMulti<float>("","htt_bydef_w1_csv",-9);
    i_htt_bydef_w1_ptD         = data->addMulti<float>("","htt_bydef_w1_ptD",-9);
    i_htt_bydef_w1_axis1       = data->addMulti<float>("","htt_bydef_w1_axis1",-9);
    i_htt_bydef_w1_axis2       = data->addMulti<float>("","htt_bydef_w1_axis2",-9);
    i_htt_bydef_w1_squaredaxis = data->addMulti<float>("","htt_bydef_w1_squaredaxis",-9);
    i_htt_bydef_w1_mult        = data->addMulti<int  >("","htt_bydef_w1_mult",-9);

    i_htt_bydef_w2_mass        = data->addMulti<float>("","htt_bydef_w2_mass",-9);
    i_htt_bydef_w2_pt          = data->addMulti<float>("","htt_bydef_w2_pt",-9);
    i_htt_bydef_w2_csv         = data->addMulti<float>("","htt_bydef_w2_csv",-9);
    i_htt_bydef_w2_ptD         = data->addMulti<float>("","htt_bydef_w2_ptD",-9);
    i_htt_bydef_w2_axis1       = data->addMulti<float>("","htt_bydef_w2_axis1",-9);
    i_htt_bydef_w2_axis2       = data->addMulti<float>("","htt_bydef_w2_axis2",-9);
    i_htt_bydef_w2_squaredaxis = data->addMulti<float>("","htt_bydef_w2_squaredaxis",-9);
    i_htt_bydef_w2_mult        = data->addMulti<int  >("","htt_bydef_w2_mult",-9);

    i_htt_bydef_w_pt           = data->addMulti<float>("","htt_bydef_w_pt",-9);
    i_htt_bydef_w_mass         = data->addMulti<float>("","htt_bydef_w_mass",-9);
    i_htt_bydef_w_deltaR       = data->addMulti<float>("","htt_bydef_w_deltaR",-9);
    i_htt_bydef_w_ptDR         = data->addMulti<float>("","htt_bydef_w_ptDR",-9);

    i_htt_bydef_t_deltaR       = data->addMulti<float>("","htt_bydef_t_deltaR",-9);
    i_htt_bydef_t_ptDR         = data->addMulti<float>("","htt_bydef_t_ptDR",-9);

    i_htt_bydef_b_w1_mass      = data->addMulti<float>("","htt_bydef_b_w1_mass",-9);
    i_htt_bydef_b_w2_mass      = data->addMulti<float>("","htt_bydef_b_w2_mass",-9);

    // of subjets, bycsv method: high csv is b, with pt2>pt3 for W subjets
    i_htt_bycsv_b_pt           = data->addMulti<float>("","htt_bycsv_b_pt",-9);
    i_htt_bycsv_b_mass         = data->addMulti<float>("","htt_bycsv_b_mass",-9);
    i_htt_bycsv_b_csv          = data->addMulti<float>("","htt_bycsv_b_csv",-9);
    i_htt_bycsv_b_ptD          = data->addMulti<float>("","htt_bycsv_b_ptD",-9);
    i_htt_bycsv_b_axis1        = data->addMulti<float>("","htt_bycsv_b_axis1",-9);
    i_htt_bycsv_b_axis2        = data->addMulti<float>("","htt_bycsv_b_axis2",-9);
    i_htt_bycsv_b_squaredaxis = data->addMulti<float>("","htt_bycsv_b_squaredaxis",-9);
    i_htt_bycsv_b_mult         = data->addMulti<int  >("","htt_bycsv_b_mult",-9);

    i_htt_bycsv_w1_mass        = data->addMulti<float>("","htt_bycsv_w1_mass",-9);
    i_htt_bycsv_w1_pt          = data->addMulti<float>("","htt_bycsv_w1_pt",-9);
    i_htt_bycsv_w1_csv         = data->addMulti<float>("","htt_bycsv_w1_csv",-9);
    i_htt_bycsv_w1_ptD         = data->addMulti<float>("","htt_bycsv_w1_ptD",-9);
    i_htt_bycsv_w1_axis1       = data->addMulti<float>("","htt_bycsv_w1_axis1",-9);
    i_htt_bycsv_w1_axis2       = data->addMulti<float>("","htt_bycsv_w1_axis2",-9);
    i_htt_bycsv_w1_squaredaxis = data->addMulti<float>("","htt_bycsv_w1_squaredaxis",-9);
    i_htt_bycsv_w1_mult        = data->addMulti<int  >("","htt_bycsv_w1_mult",-9);

    i_htt_bycsv_w2_mass        = data->addMulti<float>("","htt_bycsv_w2_mass",-9);
    i_htt_bycsv_w2_pt          = data->addMulti<float>("","htt_bycsv_w2_pt",-9);
    i_htt_bycsv_w2_csv         = data->addMulti<float>("","htt_bycsv_w2_csv",-9);
    i_htt_bycsv_w2_ptD         = data->addMulti<float>("","htt_bycsv_w2_ptD",-9);
    i_htt_bycsv_w2_axis1       = data->addMulti<float>("","htt_bycsv_w2_axis1",-9);
    i_htt_bycsv_w2_axis2       = data->addMulti<float>("","htt_bycsv_w2_axis2",-9);
    i_htt_bycsv_w2_squaredaxis = data->addMulti<float>("","htt_bycsv_w2_squaredaxis",-9);
    i_htt_bycsv_w2_mult        = data->addMulti<int  >("","htt_bycsv_w2_mult",-9);

    i_htt_bycsv_w_pt           = data->addMulti<float>("","htt_bycsv_w_pt",-9);
    i_htt_bycsv_w_mass         = data->addMulti<float>("","htt_bycsv_w_mass",-9);
    i_htt_bycsv_w_deltaR       = data->addMulti<float>("","htt_bycsv_w_deltaR",-9);
    i_htt_bycsv_w_ptDR         = data->addMulti<float>("","htt_bycsv_w_ptDR",-9);

    i_htt_bycsv_t_deltaR       = data->addMulti<float>("","htt_bycsv_t_deltaR",-9);
    i_htt_bycsv_t_ptDR         = data->addMulti<float>("","htt_bycsv_t_ptDR",-9);

    i_htt_bycsv_b_w1_mass      = data->addMulti<float>("","htt_bycsv_b_w1_mass",-9);
    i_htt_bycsv_b_w2_mass      = data->addMulti<float>("","htt_bycsv_b_w2_mass",-9);

    /// nick MVA
    i_htt_nick_matches_gen           = data->addMulti<bool >("","htt_nick_matches_gen",false);
    i_htt_nick_mva                   = data->addMulti<float>("","htt_nick_mva",-9);

    // of ropt
    i_htt_nick_ropt                  = data->addMulti<float>("","htt_nick_ropt",-9);
    i_htt_nick_frec                  = data->addMulti<float>("","htt_nick_frec",-9);
    i_htt_nick_roptcalc              = data->addMulti<float>("","htt_nick_roptcalc",-9);
    i_htt_nick_ptforropt             = data->addMulti<float>("","htt_nick_ptforropt",-9);
    i_htt_nick_tau1                  = data->addMulti<float>("","htt_nick_tau1",-9);
    i_htt_nick_tau2                  = data->addMulti<float>("","htt_nick_tau2",-9);
    i_htt_nick_tau3                  = data->addMulti<float>("","htt_nick_tau3",-9);
    i_htt_nick_ropt_pt               = data->addMulti<float>("","htt_nick_ropt_pt",-9);
    i_htt_nick_ropt_mass             = data->addMulti<float>("","htt_nick_ropt_mass",-9);

    // pt and mass of sum of subjets' p4
    i_htt_nick_subjet_pt             = data->addMulti<float>("","htt_nick_subjet_pt",-9);
    i_htt_nick_subjet_mass           = data->addMulti<float>("","htt_nick_subjet_mass",-9);

    // of subjets, bycsv method
    i_htt_nick_subjet_w_bycsv_pt     = data->addMulti<float>("","htt_nick_subjet_w_bycsv_pt",-9);
    i_htt_nick_subjet_w_bycsv_mass   = data->addMulti<float>("","htt_nick_subjet_w_bycsv_mass",-9);
    i_htt_nick_subjet_b_bycsv_mass   = data->addMulti<float>("","htt_nick_subjet_b_bycsv_mass",-9);
    i_htt_nick_subjet_b_bycsv_pt     = data->addMulti<float>("","htt_nick_subjet_b_bycsv_pt",-9);
    i_htt_nick_subjet_b_bycsv_csv    = data->addMulti<float>("","htt_nick_subjet_b_bycsv_csv",-9);

    // of subjets, by default method
    i_htt_nick_subjet_w_pt           = data->addMulti<float>("","htt_nick_subjet_w_pt",-9);
    i_htt_nick_subjet_w_mass         = data->addMulti<float>("","htt_nick_subjet_w_mass",-9);
    i_htt_nick_subjet_min_pt         = data->addMulti<float>("","htt_nick_subjet_min_pt",-9);
    i_htt_nick_w1_pt                 = data->addMulti<float>("","htt_nick_w1_pt",-9);
    i_htt_nick_w1_mass               = data->addMulti<float>("","htt_nick_w1_mass",-9);
    i_htt_nick_w1_csv                = data->addMulti<float>("","htt_nick_w1_csv",-9);
    i_htt_nick_w2_pt                 = data->addMulti<float>("","htt_nick_w2_pt",-9);
    i_htt_nick_w2_mass               = data->addMulti<float>("","htt_nick_w2_mass",-9);
    i_htt_nick_w2_csv                = data->addMulti<float>("","htt_nick_w2_csv",-9);
    i_htt_nick_b_pt                  = data->addMulti<float>("","htt_nick_b_pt",-9);
    i_htt_nick_b_mass                = data->addMulti<float>("","htt_nick_b_mass",-9);
    i_htt_nick_b_csv                 = data->addMulti<float>("","htt_nick_b_csv",-9);
  }

  // Nick variables
  // note: new construction (start with preselected fatjets) guarantees nSubjets>2
  void fillNickTrainInfo(TreeWriterData* data, const HTTFatJetF * fatjet, bool matchesGen){
    bool dbg = false;
    if(dbg) std::cout << "[HTTMVA]     fillNickTrainInfo " << std::endl;
    assert(fatjet->nSubjets()>=3); //  done in fatjet preselection. otherwise MUST fill below vector vars with default values or else vector vars will be of different lengths

    data->fillMulti<bool> (i_htt_nick_matches_gen, matchesGen);
    data->fillMulti<float>(i_htt_nick_mva,         fatjet->mva());

    // all other vars use ONE set of code located in AnalysisTools/ObjectSelection/src/HTTMVA.cc 
    auto varMap = HTTMVA::calcHTTNickMVAVars(fatjet, true);

    data->fillMulti<float>(i_htt_nick_ropt             , varMap["htt_nick_ropt"]);
    data->fillMulti<float>(i_htt_nick_frec             , varMap["htt_nick_frec"]);
    data->fillMulti<float>(i_htt_nick_roptcalc         , varMap["htt_nick_roptcalc"]);
    data->fillMulti<float>(i_htt_nick_ptforropt        , varMap["htt_nick_ptforropt"]);
    data->fillMulti<float>(i_htt_nick_tau1             , varMap["htt_nick_tau1"]);
    data->fillMulti<float>(i_htt_nick_tau2             , varMap["htt_nick_tau2"]);
    data->fillMulti<float>(i_htt_nick_tau3             , varMap["htt_nick_tau3"]);
    data->fillMulti<float>(i_htt_nick_ropt_pt          , varMap["htt_nick_ropt_pt"]);
    data->fillMulti<float>(i_htt_nick_ropt_mass        , varMap["htt_nick_ropt_mass"]);

    data->fillMulti<float>(i_htt_nick_subjet_pt                 , varMap["htt_nick_subjet_pt"]);
    data->fillMulti<float>(i_htt_nick_subjet_mass               , varMap["htt_nick_subjet_mass"]);
    data->fillMulti<float>(i_htt_nick_subjet_w_pt               , varMap["htt_nick_subjet_w_pt"]);
    data->fillMulti<float>(i_htt_nick_subjet_w_mass             , varMap["htt_nick_subjet_w_mass"]);
    data->fillMulti<float>(i_htt_nick_subjet_w_bycsv_pt         , varMap["htt_nick_subjet_w_bycsv_pt"]);
    data->fillMulti<float>(i_htt_nick_subjet_w_bycsv_mass       , varMap["htt_nick_subjet_w_bycsv_mass"]);
    data->fillMulti<float>(i_htt_nick_subjet_b_bycsv_mass       , varMap["htt_nick_subjet_b_bycsv_mass"]);
    data->fillMulti<float>(i_htt_nick_subjet_b_bycsv_pt         , varMap["htt_nick_subjet_b_bycsv_pt"]);
    data->fillMulti<float>(i_htt_nick_subjet_b_bycsv_csv        , varMap["htt_nick_subjet_b_bycsv_csv"]);
    data->fillMulti<float>(i_htt_nick_subjet_min_pt             , varMap["htt_nick_subjet_min_pt"]);
    data->fillMulti<float>(i_htt_nick_w1_pt                     , varMap["htt_nick_w1_pt"]);
    data->fillMulti<float>(i_htt_nick_w1_mass                   , varMap["htt_nick_w1_mass"]);
    data->fillMulti<float>(i_htt_nick_w1_csv                    , varMap["htt_nick_w1_csv"]);
    data->fillMulti<float>(i_htt_nick_w2_pt                     , varMap["htt_nick_w2_pt"]);
    data->fillMulti<float>(i_htt_nick_w2_mass                   , varMap["htt_nick_w2_mass"]);
    data->fillMulti<float>(i_htt_nick_w2_csv                    , varMap["htt_nick_w2_csv"]);
    data->fillMulti<float>(i_htt_nick_b_pt                      , varMap["htt_nick_b_pt"]);
    data->fillMulti<float>(i_htt_nick_b_mass                    , varMap["htt_nick_b_mass"]);
    data->fillMulti<float>(i_htt_nick_b_csv                     , varMap["htt_nick_b_csv"]);

  }

  void fillFatJetTrainInfo(TreeWriterData* data, const HTTFatJetF *fatjet, bool matchesGen){
    bool dbg = false;
    if(dbg) std::cout << "[HTTMVA]     fillFatJetTrainInfo " << std::endl;
    assert(fatjet->nSubjets()>=3); //  done in fatjet preselection. otherwise MUST fill below vector vars with default values or else vector vars will be of different lengths

    data->fillMulti<bool> (i_htt_matches_gen, matchesGen);
    data->fillMulti<float>(i_htt_mva,         fatjet->mva());

    // all other vars use ONE set of code located in AnalysisTools/ObjectSelection/src/HTTMVA.cc 
    auto varMap = HTTMVA::calcHTTMVAVars(fatjet, true);
    data->fillMulti<int  >(i_htt_nsubjets,               int(varMap["htt_nsubjets"]));
    data->fillMulti<float>(i_htt_minsubjetpt,            varMap["htt_minsubjetpt"]);
    data->fillMulti<float>(i_htt_pt,                     varMap["htt_pt"]);
    data->fillMulti<float>(i_htt_eta,                    varMap["htt_eta"]);
    data->fillMulti<float>(i_htt_rawmass,                varMap["htt_rawmass"]);
    data->fillMulti<float>(i_htt_sdmass,                 varMap["htt_sdmass"]);
    data->fillMulti<float>(i_htt_tau1,                   varMap["htt_tau1"]);
    data->fillMulti<float>(i_htt_tau2,                   varMap["htt_tau2"]);
    data->fillMulti<float>(i_htt_tau3,                   varMap["htt_tau3"]);
    data->fillMulti<float>(i_htt_tau21,                  varMap["htt_tau21"]);
    data->fillMulti<float>(i_htt_tau32,                  varMap["htt_tau32"]);
    data->fillMulti<float>(i_htt_tau31,                  varMap["htt_tau31"]);

    data->fillMulti<float>(i_htt_ropt_pt,                varMap["htt_ropt_pt"]);
    data->fillMulti<float>(i_htt_ropt_mass,              varMap["htt_ropt_mass"]);
    data->fillMulti<float>(i_htt_ropt_tau1,              varMap["htt_ropt_tau1"]);
    data->fillMulti<float>(i_htt_ropt_tau2,              varMap["htt_ropt_tau2"]);
    data->fillMulti<float>(i_htt_ropt_tau3,              varMap["htt_ropt_tau3"]);
    data->fillMulti<float>(i_htt_ropt_tau21,             varMap["htt_ropt_tau21"]);
    data->fillMulti<float>(i_htt_ropt_tau32,             varMap["htt_ropt_tau32"]);
    data->fillMulti<float>(i_htt_ropt_tau31,             varMap["htt_ropt_tau31"]);
    data->fillMulti<float>(i_htt_ropt,                   varMap["htt_ropt"]);
    data->fillMulti<float>(i_htt_ropt_ptDR,              varMap["htt_ropt_ptDR"]);
    data->fillMulti<float>(i_htt_ropt_calc,              varMap["htt_ropt_calc"]);
    data->fillMulti<float>(i_htt_ropt_deltaRopt,         varMap["htt_ropt_deltaRopt"]);
    data->fillMulti<float>(i_htt_ropt_deltaRopt_pt,      varMap["htt_ropt_deltaRopt_pt"]);
    data->fillMulti<float>(i_htt_ropt_frec,              varMap["htt_ropt_frec"]);

    data->fillMulti<float>(i_htt_bydef_b_pt,             varMap["htt_bydef_b_pt"]);
    data->fillMulti<float>(i_htt_bydef_b_mass,           varMap["htt_bydef_b_mass"]);
    data->fillMulti<float>(i_htt_bydef_b_csv,            varMap["htt_bydef_b_csv"]);
    data->fillMulti<float>(i_htt_bydef_b_ptD,            varMap["htt_bydef_b_ptD"]);
    data->fillMulti<float>(i_htt_bydef_b_axis1,          varMap["htt_bydef_b_axis1"]);
    data->fillMulti<float>(i_htt_bydef_b_axis2,          varMap["htt_bydef_b_axis2"]);
    data->fillMulti<float>(i_htt_bydef_b_squaredaxis,    varMap["htt_bydef_b_squaredaxis"]);
    data->fillMulti<int>  (i_htt_bydef_b_mult,           int(varMap["htt_bydef_b_mult"]));

    data->fillMulti<float>(i_htt_bydef_w1_mass,          varMap["htt_bydef_w1_mass"]);
    data->fillMulti<float>(i_htt_bydef_w1_pt,            varMap["htt_bydef_w1_pt"]);
    data->fillMulti<float>(i_htt_bydef_w1_csv,           varMap["htt_bydef_w1_csv"]);
    data->fillMulti<float>(i_htt_bydef_w1_ptD,           varMap["htt_bydef_w1_ptD"]);
    data->fillMulti<float>(i_htt_bydef_w1_axis1,         varMap["htt_bydef_w1_axis1"]);
    data->fillMulti<float>(i_htt_bydef_w1_axis2,         varMap["htt_bydef_w1_axis2"]);
    data->fillMulti<float>(i_htt_bydef_w1_squaredaxis,   varMap["htt_bydef_w1_squaredaxis"]);
    data->fillMulti<int>  (i_htt_bydef_w1_mult,          int(varMap["htt_bydef_w1_mult"]));

    data->fillMulti<float>(i_htt_bydef_w2_mass,          varMap["htt_bydef_w2_mass"]);
    data->fillMulti<float>(i_htt_bydef_w2_pt,            varMap["htt_bydef_w2_pt"]);
    data->fillMulti<float>(i_htt_bydef_w2_csv,           varMap["htt_bydef_w2_csv"]);
    data->fillMulti<float>(i_htt_bydef_w2_ptD,           varMap["htt_bydef_w2_ptD"]);
    data->fillMulti<float>(i_htt_bydef_w2_axis1,         varMap["htt_bydef_w2_axis1"]);
    data->fillMulti<float>(i_htt_bydef_w2_axis2,         varMap["htt_bydef_w2_axis2"]);
    data->fillMulti<float>(i_htt_bydef_w2_squaredaxis,   varMap["htt_bydef_w2_squaredaxis"]);
    data->fillMulti<int>  (i_htt_bydef_w2_mult,          int(varMap["htt_bydef_w2_mult"]));

    data->fillMulti<float>(i_htt_bydef_w_pt,             varMap["htt_bydef_w_pt"]);
    data->fillMulti<float>(i_htt_bydef_w_mass,           varMap["htt_bydef_w_mass"]);
    data->fillMulti<float>(i_htt_bydef_w_deltaR,         varMap["htt_bydef_w_deltaR"]);
    data->fillMulti<float>(i_htt_bydef_w_ptDR,           varMap["htt_bydef_w_ptDR"]);

    data->fillMulti<float>(i_htt_bydef_t_deltaR,         varMap["htt_bydef_t_deltaR"]);
    data->fillMulti<float>(i_htt_bydef_t_ptDR,           varMap["htt_bydef_t_ptDR"]);

    data->fillMulti<float>(i_htt_bydef_b_w1_mass,        varMap["htt_bydef_b_w1_mass"]);
    data->fillMulti<float>(i_htt_bydef_b_w2_mass,        varMap["htt_bydef_b_w2_mass"]);

    data->fillMulti<float>(i_htt_bycsv_b_pt,             varMap["htt_bycsv_b_pt"]);
    data->fillMulti<float>(i_htt_bycsv_b_mass,           varMap["htt_bycsv_b_mass"]);
    data->fillMulti<float>(i_htt_bycsv_b_csv,            varMap["htt_bycsv_b_csv"]);
    data->fillMulti<float>(i_htt_bycsv_b_ptD,            varMap["htt_bycsv_b_ptD"]);
    data->fillMulti<float>(i_htt_bycsv_b_axis1,          varMap["htt_bycsv_b_axis1"]);
    data->fillMulti<float>(i_htt_bycsv_b_axis2,          varMap["htt_bycsv_b_axis2"]);
    data->fillMulti<float>(i_htt_bycsv_b_squaredaxis,    varMap["htt_bycsv_b_squaredaxis"]);
    data->fillMulti<int>  (i_htt_bycsv_b_mult,           int(varMap["htt_bycsv_b_mult"]));

    data->fillMulti<float>(i_htt_bycsv_w1_mass,          varMap["htt_bycsv_w1_mass"]);
    data->fillMulti<float>(i_htt_bycsv_w1_pt,            varMap["htt_bycsv_w1_pt"]);
    data->fillMulti<float>(i_htt_bycsv_w1_csv,           varMap["htt_bycsv_w1_csv"]);
    data->fillMulti<float>(i_htt_bycsv_w1_ptD,           varMap["htt_bycsv_w1_ptD"]);
    data->fillMulti<float>(i_htt_bycsv_w1_axis1,         varMap["htt_bycsv_w1_axis1"]);
    data->fillMulti<float>(i_htt_bycsv_w1_axis2,         varMap["htt_bycsv_w1_axis2"]);
    data->fillMulti<float>(i_htt_bycsv_w1_squaredaxis,   varMap["htt_bycsv_w1_squaredaxis"]);
    data->fillMulti<int>  (i_htt_bycsv_w1_mult,          int(varMap["htt_bycsv_w1_mult"]));

    data->fillMulti<float>(i_htt_bycsv_w2_mass,          varMap["htt_bycsv_w2_mass"]);
    data->fillMulti<float>(i_htt_bycsv_w2_pt,            varMap["htt_bycsv_w2_pt"]);
    data->fillMulti<float>(i_htt_bycsv_w2_csv,           varMap["htt_bycsv_w2_csv"]);
    data->fillMulti<float>(i_htt_bycsv_w2_ptD,           varMap["htt_bycsv_w2_ptD"]);
    data->fillMulti<float>(i_htt_bycsv_w2_axis1,         varMap["htt_bycsv_w2_axis1"]);
    data->fillMulti<float>(i_htt_bycsv_w2_axis2,         varMap["htt_bycsv_w2_axis2"]);
    data->fillMulti<float>(i_htt_bycsv_w2_squaredaxis,   varMap["htt_bycsv_w2_squaredaxis"]);
    data->fillMulti<int>  (i_htt_bycsv_w2_mult,          int(varMap["htt_bycsv_w2_mult"]));

    data->fillMulti<float>(i_htt_bycsv_w_pt,             varMap["htt_bycsv_w_pt"]);
    data->fillMulti<float>(i_htt_bycsv_w_mass,           varMap["htt_bycsv_w_mass"]);
    data->fillMulti<float>(i_htt_bycsv_w_deltaR,         varMap["htt_bycsv_w_deltaR"]);
    data->fillMulti<float>(i_htt_bycsv_w_ptDR,           varMap["htt_bycsv_w_ptDR"]);

    data->fillMulti<float>(i_htt_bycsv_t_deltaR,         varMap["htt_bycsv_t_deltaR"]);
    data->fillMulti<float>(i_htt_bycsv_t_ptDR,           varMap["htt_bycsv_t_ptDR"]);

    data->fillMulti<float>(i_htt_bycsv_b_w1_mass,        varMap["htt_bycsv_b_w1_mass"]);
    data->fillMulti<float>(i_htt_bycsv_b_w2_mass,        varMap["htt_bycsv_b_w2_mass"]);

  }

  template<class FatJet>
  void fillGenInfo(TreeWriterData* data, const PartonMatching::TopDecay *top, const vector<const PartonMatching::TopDecay*> tops, const FatJet& fatjet){
    bool dbg = false;
    if(dbg) std::cout << "[HTTMVA]     fillGenInfo, is gen top? " << (top ? top->top->pt() : 0) << std::endl;
    data->fillMulti<float>(i_htt_gen_top_pt,  (top ? top->top->pt() : -9));
    data->fillMulti<float>(i_htt_gen_w_pt,    (top ? top->W->pt() : -9));
    data->fillMulti<float>(i_htt_gen_b_pt,    (top ? top->b->parton->pt() : -9));

    float mindrtq = 20., mindrqq = 20., mindrtt = 20., quarkalarmdr = 20., quarkalarmpt = -9.;
    if(top) MVACommon::calcGenInfo(top, tops, mindrtq, mindrqq, mindrtt, quarkalarmdr, quarkalarmpt, 1.5);
    data->fillMulti<float>(i_htt_gen_mindrtt, (top ? mindrtt : -9));
    data->fillMulti<float>(i_htt_gen_mindrtq, (top ? mindrtq : -9));
    data->fillMulti<float>(i_htt_gen_mindrqq, (top ? mindrqq : -9));
    data->fillMulti<float>(i_htt_gen_maxdrqfj,(top ? MVACommon::getmaxdrqfj(top, fatjet) : -9));
    data->fillMulti<float>(i_htt_gen_quarkalarmdr, (top ? quarkalarmdr : -9));
    data->fillMulti<float>(i_htt_gen_quarkalarmpt, (top ? quarkalarmpt : -9));
  }

  void preparePreselectedFatjets(BaseTreeAnalyzer* ana) {
    bool dbg = false;
    if(dbg) std::cout << "[HTTMVA] Preparing preselected fatjets:" << std::endl;
    preselectedFatjets.clear();
    for (const auto *fatjet : ana->httTops){
      if(dbg) std::cout << "[HTTMVA]     httTop has pt, sd, ropt, raw masses: " << fatjet->pt() << " " << fatjet->softDropMass() << " " << fatjet->ropt_mom().mass() << " " <<  fatjet->p4().mass() << std::endl;
      if(dbg) std::cout << "[HTTMVA]               MVA score, isPreselected? " << fatjet->mva() << " " << HTTMVA::isPreselected(fatjet) << std::endl;
      if(HTTMVA::isPreselected(fatjet)) {
        preselectedFatjets.push_back(fatjet);   
      } 
    }
    if(dbg) std::cout << "[HTTMVA]    sorting by MVA" << std::endl;

    // sort by descending MVA score
    std::sort(preselectedFatjets.begin(), preselectedFatjets.end(), [&](const HTTFatJetF *a, const HTTFatJetF *b){ return a->mva() > b->mva(); });
  }

  void prepareGenTops(BaseTreeAnalyzer* ana){
    bool dbg = false;
    if(dbg) std::cout << "[HTTMVA] Preparing hadr. gen tops" << std::endl;

    gentops.clear();
    std::vector<GenJetF*> filteredGenJets;
    for(auto * j : ana->jets){ if(j->genJet()) filteredGenJets.push_back(j->genJet()); }
    partonEvent = new PartonMatching::PartonEvent(ana->genParticleReader,*ana->defaultJets,filteredGenJets);
    for(unsigned int i = 0 ; i < partonEvent->topDecays.size() ; i++){
      PartonMatching::TopDecay* top = &partonEvent->topDecays[i];
      if(top->isLeptonic) continue;
      gentops.push_back(top); // Note: don't condense this loop or else bugs from &X->Y[i] resolving as &X ?
    }
    if(dbg) std::cout << "[HTTMVA]   Found # gen tops: " << gentops.size() << std::endl;
  }

  void fillBasicInfo(TreeWriterData* data, BaseTreeAnalyzer* ana) {
    bool dbg = false;
    //matchPairs.clear();
    gentops.clear(); preselectedFatjets.clear();

    if(dbg) std::cout << "[HTTMVA] Filling basic info" << std::endl;
    bool topProcess =  ana->process==defaults::TTBAR || ana->process==defaults::SIGNAL;
    preparePreselectedFatjets(ana);
    prepareGenTops(ana);

    data->fill<int  >(i_nLooseHTTMVA,  std::count_if( preselectedFatjets.begin(), preselectedFatjets.end(), [](const HTTFatJetF* fatjet){ return fatjet->mva() > HTTMVA::WP_LOOSE; }));
    data->fill<int  >(i_nMediumHTTMVA, std::count_if( preselectedFatjets.begin(), preselectedFatjets.end(), [](const HTTFatJetF* fatjet){ return fatjet->mva() > HTTMVA::WP_MEDIUM; }));
    data->fill<int  >(i_nTightHTTMVA,  std::count_if( preselectedFatjets.begin(), preselectedFatjets.end(), [](const HTTFatJetF* fatjet){ return fatjet->mva() > HTTMVA::WP_TIGHT; }));
    for(const auto &fatjet : preselectedFatjets) {
      const PartonMatching::TopDecay * top = (topProcess ? MVACommon::getTopCand(fatjet, gentops, TMath::Min(fatjet->ropt()+0.1,1.5), fatjet->ropt()) : 0); // if not top process don't try to match
      data->fillMulti<bool >(i_basic_htt_matches_gen, (top ? true : false));
      data->fillMulti<float>(i_basic_htt_pt, fatjet->pt());
      data->fillMulti<float>(i_basic_htt_eta, fatjet->eta());
      data->fillMulti<float>(i_basic_htt_sdmass, fatjet->softDropMass());
      data->fillMulti<float>(i_basic_htt_roptmass, fatjet->ropt_mom().mass());
      data->fillMulti<float>(i_basic_htt_mva, fatjet->mva());
    }
    delete partonEvent;
  }//fillBasicInfo

  void fillTrainInfo(TreeWriterData* data, BaseTreeAnalyzer* ana) {
    bool dbg = false;
    //matchPairs.clear();
    gentops.clear(); preselectedFatjets.clear();

    if(dbg) std::cout << "[HTTMVA] Filling training info" << std::endl;
    bool topProcess =  ana->process==defaults::TTBAR || ana->process==defaults::SIGNAL;
    preparePreselectedFatjets(ana);
    prepareGenTops(ana);

    //int ngenhadronictops = 0;
    //for(auto top : gentops) { if( !top->isLeptonic) ngenhadronictops++; }
    //data->fill<int>(i_ngenhadronictops, ngenhadronictops);

    // fill for EVERY preselected fatjet. first entry 0 if unmatched --> match_gen variable.
    for(const auto &fatjet : preselectedFatjets) {
      if(dbg) std::cout << "[HTTMVA]   Matching presel fj with pt, ropt: " << fatjet->pt() << " " << fatjet->ropt() << std::endl;
      const PartonMatching::TopDecay * top = (topProcess ? MVACommon::getTopCand(fatjet, gentops, TMath::Min(fatjet->ropt()+0.1,1.5), fatjet->ropt()) : 0); // if not top process don't try to match
      if(dbg) std::cout << "[HTTMVA]     return of getTopCand gen pt, process: " << (top ? top->top->pt() : 0) << " " << topProcess << " " << std::endl;
      //matchPairs.emplace_back(top,fatjet);
      fillGenInfo(data,top, gentops, fatjet);
      fillFatJetTrainInfo(data,fatjet, (top ? true : false));
      fillNickTrainInfo(data,fatjet, (top ? true : false));
    }
    delete partonEvent;
  }//fillTrainInfo

};//HTTMVAFiller

#endif