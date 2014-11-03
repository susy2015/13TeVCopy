//--------------------------------------------------------------------------------------------------
//
//   Class to manage a plot. Member methods are provided to facilitate properties of the plot
//   (e.g. styles, colors, combining histograms or graphs, etc.)
//
//--------------------------------------------------------------------------------------------------

#ifndef PLOT_HH
#define PLOT_HH

#include <TROOT.h>
#include <TSystem.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TMath.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TPaveStats.h>
#include <TPaveText.h>
#include <TF1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TProfile.h>
#include <TLine.h>
#include <THStack.h>
#include <TBox.h>
#include <TLatex.h>
#include <vector>
#include <assert.h>

using namespace std;

template <class T>
class PlotElement
{

  public :
    PlotElement(T* inmember, TString inopt) : member(inmember), opt(inopt) {}

    ~PlotElement()		{ delete member;		}

    const char* classname()	{ return member->ClassName();	}

    T* member;

    TString opt;

};

typedef PlotElement<TH1F> h1D;
typedef PlotElement<TH2F> h2D;
typedef PlotElement<TGraph> graph;
typedef PlotElement<TProfile> profile;

class Plot {
  public:
    Plot();
    Plot(TString name, TString title, TString xtitle, TString ytitle);
    ~Plot();

    void clear();

    static TString outputdir;  // output directory
  
    // Draw ratio of 2 histograms
    void drawRatio(TCanvas *c, TH1F *h1, TH1F *h2, bool doSave=false, TString format="png");

    // Draw ratio of data/MC, with MC constituents stacked
    void drawRatioStack(TCanvas *c, TH1F *hData, TH1F *hMC, bool doSave=false, TString format="png");

    // Draw the plot to a given canvas
    void draw(TCanvas *c, bool doSave=false, TString format="png");
  
    // Adding a histogram/graph to the plot
    template<class T>
    void add(T* item, TString drawopt="", int color=0, int fillstyle=0, int linecolor=1, int linestyle=1);

    template<class T>
    void add(T* item, TString label, TString drawopt="", int color=0, int fillstyle=0, int linecolor=1, int linestyle=1);

    template<class T>
    void add(TFile *f, TString itemname, TString drawopt="", int color=0, int fillstyle=0, int linecolor=1, int linestyle=1);

    template<class T>
    void add(TFile *f, TString label, TString itemname, TString drawopt="", int color=0, int fillstyle=0, int linecolor=1, int linestyle=1);

    // Adding a 1D histogram to a histogram stack
    void addToStack(TH1F *h, int color);
    void addToStack(TH1F *h, TString label, int color, int fillstyle=1001, int linecolor=1, int linestyle=1, int linewidth=0);
    void addToStack(TFile *f, TString histname, int color);
    void addToStack(TFile *f, TString histname, TString label, int color, int fillstyle=1001, int linecolor=1, int linestyle=1, int linewidth=0);
  
    // Adding a text box to the plot
    void addTextBox(TString text, double x1, double y1, double x2, double y2, int bordersize=1, int textcolor=0, int fillcolor=0);
  
    // Add a line between two points (x1,y1) and (x2,y2)
    void addLine(double x1, double y1, double x2, double y2, int color=1, int style=1);
    void addLine(double x1, double y1, double x2, double y2, int color, int style, TString label);
  
    // Add a box with coordinates: bottom left (x1,y1), top right (x2,y2)
    void addBox(double x1, double y1, double x2, double y2, int linecolor=1, int linesty=1, int fillcolor=0);
    void addBox(double x1, double y1, double x2, double y2, int linecolor, int linesty, int fillcolor, TString label);
  
    // Add a 1D function
    void addFcn(TF1* fcn, int color=1, int linesty=1);
    void addFcn(TF1* fcn, TString label, int color=1, int linesty=1); 
    
    // Set legend position
    void setLegend(double x1, double y1, double x2, double y2) {
      assert(fLeg);
      fLeg->SetX1(x1); fLeg->SetY1(y1); fLeg->SetX2(x2); fLeg->SetY2(y2);
    }

    // Translate legend box
    void moveLegend(double dx, double dy) {
      assert(fLeg);
      fLeg->SetX1(fLeg->GetX1()+dx); fLeg->SetY1(fLeg->GetY1()+dy); 
      fLeg->SetX2(fLeg->GetX2()+dx); fLeg->SetY2(fLeg->GetY2()+dy);
    } 
  
    // Set stats box position
    void setStats(double x, double y) { fStatsX = x; fStatsY = y; }
  
    // Translate stats box
    void transStats(double dx, double dy) { fStatsX += dx; fStatsY += dy; }  

    // Add logo, lumi, etc.
    void header(const char* lumitext, const char* channel, double lowX=0.4, double lowY=0.74);
    
    // Set general properties of the plot
    void setName(TString str)                { fName = str; }                // plot name (for output)
    void setTitle(TString str)               { fTitle = str; }               // plot title
    void setXTitle(TString str)              { fXTitle = str; }              // x-axis title
    void setYTitle(TString str)              { fYTitle = str; }              // y-axis title
    void setXRange(double xmin, double xmax) { fXmin = xmin; fXmax = xmax; } // x-axis range
    void setYRange(double ymin, double ymax) { fYmin = ymin; fYmax = ymax; } // y-axis range
    void setLogx(int value=1)                { fLogx = value; }              // toggle logscale x-axis
    void setLogy(int value=1)                { fLogy = value; }              // toggle logscale y-axis
    void setGridx(bool value=1)              { fGridx = value; }             // toggle grid lines from x-axis ticks
    void setGridy(bool value=1)              { fGridy = value; }             // toggle grid lines from y-axis ticks
    void rebin(int ngroup)                   { fRebin = ngroup; }            // 1D histogram re-bin
    void showStats(int show=111)             { fShowStats = show; }          // display statistics

    // Accessors
    TLegend* getLegend() { return fLeg; }
    THStack* getStack()  { return fStack; }
  
  protected:
    vector<h1D*> fHists1D;                // list of 1D hists to be plotted
    vector<h2D*> fHists2D;                // list of 2D hists to be plotted
    vector<graph*> fGraphs;               // list of graphs to be plotted
    vector<profile*> fProfiles;           // list of profile hists to be plotted
    vector<TPaveText*> fTextBoxes;        // list of text boxes
    vector<TLine*> fLines;                // list of lines
    vector<TBox*> fBoxes;                 // list of boxes
    vector<TF1*> fFcns;                   // list of 1D functions
    THStack *fStack;                      // histogram stack
    TString fName;                        // plot name
    TString fTitle;                       // plot title
    TString fXTitle;                      // x-axis title
    TString fYTitle;                      // y-axis title
    double fXmin, fXmax;                  // x-axis range
    double fYmin, fYmax;                  // y-axis range
    int fLogx, fLogy;                     // logscale axes
    int fGridx, fGridy;                   // grid lines
    int fRebin;                           // grouping for histogram re-bin
    TLegend *fLeg;                        // legend object
    double fLegX1, fLegY1, fLegX2, fLegY2;// x,y coordinates of legend box
    int fShowStats;                       // whether to display statistics
    double fStatsX, fStatsY;              // x,y coordinates of top left corner of stats box
  
    vector<TLegendEntry*> fStackEntries;  // pointer to legend entry objects for histograms in a stack
  
    static int counter;                   // number of Plot instances
};

int Plot::counter = 0;
TString Plot::outputdir = ".";

template<> void Plot::add<TH1F>(TH1F* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle);
template<> void Plot::add<TH2F>(TH2F* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle);
template<> void Plot::add<TGraph>(TGraph* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle);
template<> void Plot::add<TProfile>(TProfile* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle);

template<> void Plot::add<TH1F>(TH1F* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle)
{

  if(!item)
    return;

  if(linecolor==0)
    item->SetLineColor(color);
  else
    item->SetLineColor(linecolor);

  item->SetLineStyle(linestyle);
  item->SetFillColor(color);
  item->SetFillStyle(fillstyle);

  if(drawopt.CompareTo("E",TString::kIgnoreCase)==0) {
    item->SetMarkerSize(1.3);
    item->SetMarkerStyle(20);
    item->SetLineWidth(3);
  } else {
    item->SetLineWidth(3);
  }

  fHists1D.push_back(new h1D(item, drawopt));

}

template<> void Plot::add<TH2F>(TH2F* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle)
{

  if(!item)
    return;

  if(linecolor==0)
    item->SetLineColor(color);
  else
    item->SetLineColor(linecolor);

  item->SetLineStyle(linestyle);
  item->SetFillColor(color);
  item->SetFillStyle(fillstyle);

  if(drawopt.CompareTo("E",TString::kIgnoreCase)==0) {
    item->SetMarkerSize(1.3);
    item->SetMarkerStyle(20);
    item->SetLineWidth(3);
  } else {
    item->SetLineWidth(3);
  }

  fHists2D.push_back(new h2D(item, drawopt));

}

// implementation of drawing options to be updated
template<> void Plot::add<TGraph>(TGraph* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle)
{

  if(!item)
    return;

  if(linecolor==0)
    item->SetLineColor(color);
  else
    item->SetLineColor(linecolor);

  item->SetLineStyle(linestyle);
  item->SetFillColor(color);
  item->SetFillStyle(fillstyle);

  if(drawopt.CompareTo("E",TString::kIgnoreCase)==0) {
    item->SetMarkerSize(1.3);
    item->SetMarkerStyle(20);
    item->SetLineWidth(3);
  } else {
    item->SetLineWidth(3);
  }

  fGraphs.push_back(new graph(item, drawopt));

}

template<> void Plot::add<TProfile>(TProfile* item, TString drawopt, int color, int fillstyle, int linecolor, int linestyle)
{

  if(!item)
    return;

  if(linecolor==0)
    item->SetLineColor(color);
  else
    item->SetLineColor(linecolor);

  item->SetLineStyle(linestyle);
  item->SetFillColor(color);
  item->SetFillStyle(fillstyle);

  if(drawopt.CompareTo("E",TString::kIgnoreCase)==0) {
    item->SetMarkerSize(1.3);
    item->SetMarkerStyle(20);
    item->SetLineWidth(3);
  } else {
    item->SetLineWidth(3);
  }

  fProfiles.push_back(new profile(item, drawopt));

}

template<class T>
void Plot::add(T* item, TString label, TString drawopt, int color, int fillstyle, int linecolor, int linestyle)
{

  if(!item)
    return;

  if(!fLeg)
    fLeg = new TLegend(fLegX1, fLegY1, fLegX2, fLegY2);
  else
    fLeg->SetY1(fLeg->GetY1()-0.06);

  if(drawopt.CompareTo("E",TString::kIgnoreCase)==0) {
    fLeg->AddEntry(item,label,"PL");
  } else {
    if(fillstyle > 0) fLeg->AddEntry(item,label,"F");
    else              fLeg->AddEntry(item,label,"L");
  }

  add(item, drawopt, color, fillstyle, linecolor, linestyle);

}

template<class T>
void Plot::add(TFile *f, TString itemname, TString drawopt, int color, int fillstyle, int linecolor, int linestyle)
{

  if(!f)
    return;

  T* item = (T*)f->FindObjectAny(itemname);

  add(item, drawopt, color, fillstyle, linecolor, linestyle);

}

template<class T>
void Plot::add(TFile *f, TString itemname, TString label, TString drawopt, int color, int fillstyle, int linecolor, int linestyle)
{

  if(!f)
    return;

  T* item = (T*)f->FindObjectAny(itemname);

  add(item, label, drawopt, color, fillstyle, linecolor, linestyle);

}

#endif