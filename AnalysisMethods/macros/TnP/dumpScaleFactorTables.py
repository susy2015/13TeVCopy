import ROOT
from optparse import OptionParser

def makeTable(h, tablefilename):
    nX = h.GetNbinsX()
    nY = h.GetNbinsY()
  
    f = open(tablefilename, "w+")
  
    for i in xrange(1, nX+1):
        pT0 = h.GetXaxis().GetBinLowEdge(i)
        pT1 = h.GetXaxis().GetBinLowEdge(i+1)
    
        for j in xrange(1, nY+1):
            x = h.GetBinContent(i,j)
            dx = h.GetBinError(i,j)
            eta0 = h.GetYaxis().GetBinLowEdge(j)
            eta1 = h.GetYaxis().GetBinLowEdge(j+1)
            
            f.write("%4.1f  %4.1f   %+6.4f   %+6.4f  %6.4f   %6.4f \n"%(pT0, pT1, eta0, eta1, x, dx))
            
    f.close()

def main(options):
    fData = ROOT.TFile(options.data)
    fMC   = ROOT.TFile(options.mc)
    hData = ""
    hMC   = ""

    temp = "%s/%s/fit_eff_plots/" % (options.directory, options.subdir)
    if("ToHLT" in temp):
        temp = "%s/%s/cnt_eff_plots/" %(options.directory, options.subdir)
    fData.cd(temp)

    keyList = [key.GetName() for key in ROOT.gDirectory.GetListOfKeys()]
    for k in  keyList:
        obj = ROOT.gDirectory.GetKey(k).ReadObj();
        innername = obj.GetName()
        if (obj.ClassName() == "TCanvas"):
            for p in obj.GetListOfPrimitives():
                if (p.ClassName() == "TH2F"):
                    hData = p

    fMC.cd(temp)
    keyList = [key.GetName() for key in ROOT.gDirectory.GetListOfKeys()]
    for k in  keyList:
        obj = ROOT.gDirectory.GetKey(k).ReadObj();
        innername = obj.GetName()
        if (obj.ClassName() == "TCanvas"):
            for p in obj.GetListOfPrimitives():
                if (p.ClassName() == "TH2F"):
                    hMC = p

    temp = "ScaleFactor_%s_%s.txt"%(options.directory, options.subdir)
    hData.Divide(hMC)
    makeTable(hData, temp)
    
    fData.Close()
    fMC.Close()

if (__name__ == "__main__"):
    parser = OptionParser()
    parser.add_option("", "--mc", default="fitResults_mc.root", help="Input filename for MC")
    parser.add_option("", "--data", default="fitResults_data.root", help="Input filename for data")
    parser.add_option("-d", "--directory", default="fits", help="Directory with workspace")
    parser.add_option("-s", "--subdir", default="pt_eta_eff", help="Subdirectory with results")
    parser.add_option("-b", dest="batch", action="store_true", help="ROOT batch mode", default=False)
    
    (options, arg) = parser.parse_args()

    if (options.batch):
        ROOT.gROOT.SetBatch(True)

    main(options)
