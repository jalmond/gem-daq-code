/**
 * A collection of simple ROOT macros.
 *
 * N. Amapane, G. Cerminara, M. Dalchenko
 */
#define DEBUG 1

#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#if !defined(__CINT__) || defined(__MAKECINT__)
#include "TProfile.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TVirtualPad.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TPostScript.h"
#include "THStack.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TAxis.h"
#include "TGaxis.h"
#include "TMath.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TObject.h"
#include "TH1.h"
#include "TH1F.h"
#include <TFile.h>
#include "TPaveStats.h"
#include <math.h>
#include "TBufferJSON.h"
#include <iostream>

using namespace std;

#endif



TStyle * getStyle(TString name) 
{
    TStyle *theStyle;
    
    if ( name == "ZZ" ) {
        // Style from http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/pandolf/CommonTools/DrawBase.C?revision=1.25&view=markup
        theStyle = new TStyle("ZZStyle", "ZZ Style");
        theStyle->SetCanvasColor(0);
        theStyle->SetPadColor(0);
        theStyle->SetFrameFillColor(0);
        theStyle->SetStatColor(0);
        theStyle->SetOptStat(1111);
        theStyle->SetTitleFillColor(0);
        theStyle->SetCanvasBorderMode(0);
        theStyle->SetPadBorderMode(0);
        theStyle->SetFrameBorderMode(0);
        theStyle->SetPadBottomMargin(0.12);
        theStyle->SetPadLeftMargin(0.12);
        theStyle->cd();
        
        // For the canvas:
        theStyle->SetCanvasBorderMode(0);
        theStyle->SetCanvasColor(kWhite);
        theStyle->SetCanvasDefH(600); //Height of canvas
        theStyle->SetCanvasDefW(600); //Width of canvas
        theStyle->SetCanvasDefX(0);   //POsition on screen
        theStyle->SetCanvasDefY(0);
        
        // For the Pad:
        theStyle->SetPadBorderMode(0);
        theStyle->SetPadColor(kWhite);
        theStyle->SetPadGridX(false);
        theStyle->SetPadGridY(false);
        theStyle->SetGridColor(0);
        theStyle->SetGridStyle(3);
        theStyle->SetGridWidth(1);
        
        // For the frame:
        theStyle->SetFrameBorderMode(0);
        theStyle->SetFrameBorderSize(1);
        theStyle->SetFrameFillColor(0);
        theStyle->SetFrameFillStyle(0);
        theStyle->SetFrameLineColor(1);
        theStyle->SetFrameLineStyle(1);
        theStyle->SetFrameLineWidth(1);
        
        // Margins:
        theStyle->SetPadTopMargin(0.05);
        theStyle->SetPadBottomMargin(0.15);//0.13);
        theStyle->SetPadLeftMargin(0.2);//0.16);
        theStyle->SetPadRightMargin(0.15);//0.02);
        
        // For the Global title:
        theStyle->SetOptTitle(0);
        theStyle->SetTitleFont(42);
        theStyle->SetTitleColor(1);
        theStyle->SetTitleTextColor(1);
        theStyle->SetTitleFillColor(10);
        theStyle->SetTitleFontSize(0.05);
        
        // For the axis titles:
        theStyle->SetTitleColor(1, "XYZ");
        theStyle->SetTitleFont(42, "XYZ");
        theStyle->SetTitleSize(0.05, "XYZ");
        theStyle->SetTitleXOffset(1.25);//0.9);
        theStyle->SetTitleYOffset(1.65); // => 1.15 if exponents
        
        // For the axis labels:
        theStyle->SetLabelColor(1, "XYZ");
        theStyle->SetLabelFont(42, "XYZ");
        theStyle->SetLabelOffset(0.007, "XYZ");
        theStyle->SetLabelSize(0.045, "XYZ");
        
        // For the axis:
        theStyle->SetAxisColor(1, "XYZ");
        theStyle->SetStripDecimals(kTRUE);
        theStyle->SetTickLength(0.03, "XYZ");
        theStyle->SetNdivisions(10, "XYZ");
        theStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
        theStyle->SetPadTickY(1);
        TGaxis::SetMaxDigits(3);
    }else if ( name == "myStyle" ) {
        theStyle = new TStyle("myStyle", "myStyle");
        //    theStyle->SetOptStat(0);
        theStyle->SetPadBorderMode(0);
        theStyle->SetCanvasBorderMode(0);
        theStyle->SetPadColor(0);
        theStyle->SetCanvasColor(0);
        theStyle->SetMarkerStyle(8);
        theStyle->SetMarkerSize(0.7);
        theStyle->SetStatH(0.3);
        theStyle->SetStatW(0.15);
        //   theStyle->SetTextFont(132);
        //   theStyle->SetTitleFont(132);
        theStyle->SetTitleBorderSize(1);
        theStyle->SetPalette(1);
        
    }else if( name == "tdr" ) {
        theStyle = new TStyle("tdrStyle","Style for P-TDR");
        
        // For the canvas:
        theStyle->SetCanvasBorderMode(0);
        theStyle->SetCanvasColor(kWhite);
        theStyle->SetCanvasDefH(600); //Height of canvas
        theStyle->SetCanvasDefW(600); //Width of canvas
        theStyle->SetCanvasDefX(0);   //POsition on screen
        theStyle->SetCanvasDefY(0);
        
        // For the Pad:
        theStyle->SetPadBorderMode(0);
        // theStyle->SetPadBorderSize(Width_t size = 1);
        theStyle->SetPadColor(kWhite);
        theStyle->SetPadGridX(true);
        theStyle->SetPadGridY(true);
        theStyle->SetGridColor(0);
        theStyle->SetGridStyle(3);
        theStyle->SetGridWidth(1);
        
        // For the frame:
        theStyle->SetFrameBorderMode(0);
        theStyle->SetFrameBorderSize(1);
        theStyle->SetFrameFillColor(0);
        theStyle->SetFrameFillStyle(0);
        theStyle->SetFrameLineColor(1);
        theStyle->SetFrameLineStyle(1);
        theStyle->SetFrameLineWidth(1);
        
        // For the histo:
        theStyle->SetHistLineColor(1);
        theStyle->SetHistLineStyle(0);
        theStyle->SetHistLineWidth(1);
        
        
        theStyle->SetEndErrorSize(2);
        
        theStyle->SetMarkerStyle(20);
        theStyle->SetMarkerSize(0.5);
        
        
        //For the fit/function:
        theStyle->SetOptFit(1);
        theStyle->SetFitFormat("5.4g");
        theStyle->SetFuncColor(2);
        theStyle->SetFuncStyle(1);
        theStyle->SetFuncWidth(1);
        
        //For the date:
        theStyle->SetOptDate(0);
        
        // For the statistics box:
        theStyle->SetOptFile(0);
        
        theStyle->SetOptStat("e");
        theStyle->SetStatColor(kWhite);
        theStyle->SetStatTextColor(1);
        theStyle->SetStatFormat("6.4g");
        theStyle->SetStatBorderSize(1);
        
        // Margins:
         theStyle->SetPadTopMargin(0.1);
         theStyle->SetPadBottomMargin(0.1);
         theStyle->SetPadLeftMargin(0.1);
         theStyle->SetPadRightMargin(0.05);
        
        // For the Global title:
        
        // Uncomment to remove title
        //    theStyle->SetOptTitle(0); 
        theStyle->SetTitleFont(42);
        theStyle->SetTitleColor(1);
        theStyle->SetTitleTextColor(1);
        theStyle->SetTitleFillColor(10);
        theStyle->SetTitleFontSize(0.05);
        // theStyle->SetTitleH(0); // Set the height of the title box
        // theStyle->SetTitleW(0); // Set the width of the title box
        // theStyle->SetTitleX(0); // Set the position of the title box
        theStyle->SetTitleY(1); // Set the position of the title box
        theStyle->SetTitleStyle(1001);
        // theStyle->SetTitleBorderSize(2);
        
        // For the axis titles:
        
        theStyle->SetTitleColor(1, "XYZ");
        theStyle->SetTitleFont(42, "XYZ");
        theStyle->SetTitleSize(0.05, "XYZ");
        theStyle->SetTitleXOffset(0.9);
        theStyle->SetTitleYOffset(1.25);
        
        // For the axis labels:
        
        theStyle->SetLabelColor(1, "XYZ");
        
        theStyle->SetLabelFont(42, "XYZ");
        
        theStyle->SetLabelOffset(0.007, "XYZ");
        
        theStyle->SetLabelSize(0.045, "XYZ");
        
        // For the axis:
        
        theStyle->SetAxisColor(1, "XYZ");
        theStyle->SetStripDecimals(kTRUE);
        theStyle->SetTickLength(0.03, "XYZ");
        theStyle->SetNdivisions(510, "XYZ");
        theStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
        theStyle->SetPadTickY(1);
        
        // Change for log plots:
        theStyle->SetOptLogx(0);
        theStyle->SetOptLogy(0);
        theStyle->SetOptLogz(0);
        
        // Postscript options:
        theStyle->SetPaperSize(20.,20.);
        // theStyle->SetLineScalePS(Float_t scale = 3);
        // theStyle->SetLineStyleString(Int_t i, const char* text);
        // theStyle->SetHeaderPS(const char* header);
        // theStyle->SetTitlePS(const char* pstitle);
        
        // theStyle->SetBarOffset(Float_t baroff = 0.5);
        // theStyle->SetBarWidth(Float_t barwidth = 0.5);
        // theStyle->SetPaintTextFormat(const char* format = "g");
        // theStyle->SetPalette(Int_t ncolors = 0, Int_t* colors = 0);
        // theStyle->SetTimeOffset(Double_t toffset);
        // theStyle->SetHistMinimumZero(kTRUE);
        theStyle->SetTextSize(0.045);
        theStyle->SetTextFont(42);
        
        //   style->SetOptFit(101);
        //   style->SetOptStat(1111111); 
        
    }else {
        // Avoid modifying the default style!
        theStyle = gStyle;
    }
    return theStyle;
}

// Make a new canvas setting name, title, x/y divisions, size.
TCanvas * newCanvas(TString title="", Int_t xdiv=1, Int_t ydiv =1,
            Int_t w=600, Int_t h=600)
{
    TCanvas* c= new TCanvas;
    c->SetWindowSize(w,h);
    if (title != "") c->SetTitle(title);
    if (xdiv*ydiv>1) {
        c->Divide(xdiv,ydiv);
        c->cd(1);
    }
    return c;
}

// Print all open canvases in separate files
// Type = "png", "eps", etc.
void printCanvases(TString type)
{
    TIter iter(gROOT->GetListOfCanvases());
    TCanvas *c;
    while( (c = (TCanvas *)iter()) ) {
        TString name =  c->GetTitle();
        c->Print(name+"."+type,type);
    }
}


void printCanvases(TString type, TString prefix="")
{
    
    TIter next(gROOT->GetListOfCanvases());
    TCanvas *c;
    while ((c = (TCanvas *)next()) )
    {
	TString name =  c->GetTitle();
        if (prefix!="") gROOT->ProcessLine(".!mkdir -p ./"+prefix);
        c->Print(prefix+name+"."+type,type);
    }
}




// Set histogram titles
void setTitles(TH1 *h, TString xtitle, TString ytitle)
{
    h->GetXaxis()->SetTitle(xtitle);
    h->GetYaxis()->SetTitle(ytitle);
    h->GetYaxis()->SetTitleOffset(1.5);
}

void setTitles(TH1 *h, TString xtitle, TString ytitle, TString ztitle)
{
    h->GetXaxis()->SetTitle(xtitle);
    h->GetYaxis()->SetTitle(ytitle);
    h->GetZaxis()->SetTitle(ztitle);
}

// Print all histograms in separate files
// Type = "png", "eps", etc.
void printHistograms(TDirectory* dir, TString type, TString prefix="", bool createJSON=false)
{
  dir->cd();
  TIter next(gDirectory->GetListOfKeys());
  TKey *key;
  while ((key = (TKey*)next())) 
  {
    TClass *cl = gROOT->GetClass(key->GetClassName());
    if (!cl->InheritsFrom("TH1")) continue;
    TH1 *h = (TH1*)key->ReadObj();
    TCanvas *c = newCanvas();
    h->Draw("colz");
    TString name =  h->GetTitle();
    //if (prefix!="") gROOT->ProcessLine(".!mkdir -p "+prefix); // don't print images for the moment
    c->Print(prefix+name+"."+type,type);
    delete c;
    if (createJSON) {
      ofstream jsonfile;
      //jsonfile.open("/tmp/"+name+".json");
      jsonfile.open(prefix+name+".json");
      TString json = TBufferJSON::ConvertToJSON(h);
      jsonfile << json;
      jsonfile.close();
    }
  }
}
void retrieveHistograms(TDirectory* dir, vector<TH1*>& v)
{
  dir->cd();
  TIter next(gDirectory->GetListOfKeys());
  TKey *key;
  while ((key = (TKey*)next())) 
  {
      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (!cl->InheritsFrom("TH1")) continue;
      TH1 *h = (TH1*)key->ReadObj();
      h->AddDirectory(kFALSE);
      v.push_back(h);
  }
  //cout << "[retrieveHistograms]: Size of v : " << v.size() << endl;
}

void drawStack(TDirectory* dir1, TDirectory* dir2, int cl1, int cl2, TString type, TString prefix="")
{
  vector<TH1*> v1;
  vector<TH1*> v2;
  retrieveHistograms(dir1, v1);
  //cout << "Size of v1 : " << v1.size() << endl;
  retrieveHistograms(dir2, v2);
  //cout << "Size of v2 : " << v2.size() << endl;
  for (int i = 0; i < v1.size(); i++){
    TH1* h_1 = v1.at(i);
    h_1->SetLineColor(cl1);
    h_1->SetFillColor(cl1);
    TH1* h_2 = v2.at(i);
    h_2->SetLineColor(cl2);
    h_2->SetFillColor(cl2);
    TString name = h_1->GetTitle();
    THStack* hs = new THStack("hs", name);
    hs->Add(h_1);
    hs->Add(h_2);
    if (prefix!="") gROOT->ProcessLine(".!mkdir -p ./"+prefix);
    TCanvas *c = newCanvas();
    hs->Draw();
    c->Print(prefix+name+"."+type,type);
    delete c;
    delete hs;
  }
}

//Prints each individual histogram to picture formats
void printPictures(TH1 *h, TString iname, TString opath, TCanvas *c, vector<TString> types)
{							
    c->cd(1);
    int max=h->GetBinContent(h->GetMaximumBin());
    h->SetMaximum(max);
    h->SetMinimum(0);

    //Focus on Valid ChipIDs
    if (strcmp(h->GetName(),"ChipID")==0)
	h->GetXaxis()->SetRangeUser(3700,3800);

    //Set color/style
    h->SetMarkerColor(kBlue+3);
    h->SetMarkerStyle(1);
    h->SetFillStyle(1001);
    h->SetFillColor(kBlue+3);
    h->SetLineColor(kBlue+3);

    gStyle->SetOptStat(111111);

    h->Draw();

    //Add Legend
    TLegend* leg = new TLegend(0.78,0.4,0.98,0.45);
    leg->AddEntry(h,iname,"l");
    leg->Draw();

    for (int t=0;t<types.size(); t++)
    {
	gROOT->ProcessLine(".!mkdir -p "+opath+types[t]+"/");
	c->Print(opath+types[t]+"/"+iname+"--"+h->GetName()+"."+types[t],types[t]);
    }
}

//Layers each histograms in h on single canvas/pad in different colors/styles
void layerHistogram(vector<TH1*> h, vector<TString> inames, TCanvas *c, int pad=1)
{
    int numH = h.size();
    vector<int> maxs;
    int max=h[0]->GetBinContent(h[0]->GetMaximumBin());
    // vector<int> mins;
    // int min=h[0]->GetBinContent(h[0]->GetMinimumBin());
    h[0]->SetMaximum(max);
    h[0]->SetMinimum(0);

    //Focus on Valid ChipIDs
    if (strcmp(h[0]->GetName(),"ChipID")==0)
    {
	for(int f=0;f<numH;f++)
	{
	    h[f]->GetXaxis()->SetRangeUser(3700,3800);
	}
    }
	
    c->cd(pad);
    int n = 0;
    for (int i=0;i<numH;i++)
    { 
	//Set color/style

	if (i==4) //avoid yellow
	    n++; 
	
	if (numH==1)
	{
	    h[i]->SetMarkerColor(kBlue+3);
	    h[i]->SetMarkerStyle(1);
	    h[i]->SetFillStyle(1001);
	    h[i]->SetFillColor(kBlue+3);
	    h[i]->SetLineColor(kBlue+3);
	}
	else if (numH>9 and i>9) //Should never have more than 9 histograms, but it works
	{
	    h[i]->SetMarkerColor(i+30+n);
	    h[i]->SetMarkerStyle(1);
	    h[i]->SetFillStyle(i+3004);
	    h[i]->SetFillColor(i+30+n);
	    h[i]->SetLineColor(i+30+n);
	}
	else if (numH==2)
	{

	    h[i]->SetMarkerColor(pow(2,i));
	    h[i]->SetMarkerStyle(1);
	    h[i]->SetFillStyle(3001+2*i);
	    h[i]->SetFillColor(pow(2,i));
	    h[i]->SetLineColor(pow(2,i));
	}
	else
	{
	    h[i]->SetMarkerColor(i+1+n);
	    h[i]->SetMarkerStyle(1);
	    h[i]->SetFillStyle(i+3004);
	    h[i]->SetFillColor(i+1+n);
	    h[i]->SetLineColor(i+1+n);

	}

	//Resize as necessary
	maxs.push_back(h[i]->GetBinContent(h[i]->GetMaximumBin()));
	if (maxs[i]>max)
	    h[0]->SetMaximum(maxs[i]);

    }

    gStyle->SetOptStat(111111);
    h[0]->Draw();
    for (int i=0;i<numH;i++)
	    h[i]->Draw("SAMES");


   //Add Legend
    TLegend* leg = new TLegend(0.78,0.4,0.98,0.4+0.05*numH);
    for (int l=0;l<numH;l++)
	leg->AddEntry(h[l],inames[l],"l");
    leg->Draw();
    

    c->Update();
    int no_yellow=0;
    for (int f=0;f<numH;f++)
    {	
    	//Retrieve stat box
    	TPaveStats *st = (TPaveStats*)h[f]->FindObject("stats");
   
    	//Color code
	if (f==4) 
	    no_yellow++; //avoid yellow
	if (numH==2)
	    st->SetTextColor(pow(2,f));
	else
	    st->SetTextColor(f+1+no_yellow);

    	//Reposition
    	st->SetX1NDC(.78-0.22*f);  //new x start position
    	st->SetX2NDC(.98-0.22*f);  //new x end position
    }
}


//Layers and plots only histograms with names listed in hnames
void layerSpecific(vector<vector<TH1*>> hs, vector<const char*> hnames,vector<TString> inames, TCanvas *can)
{
    int numF = hs.size();
    int numH = hs[0].size();
    vector<TH1*> plot_hists;
    int complete=0;
    for (int i=0;i<numH;i++)
    {
    	const char* histname = hs[0][i]->GetName();
	
	for (int n=0;n<hnames.size();n++)
	{
	    plot_hists.clear();
	    if ( strcmp(histname,hnames[n])==0 )
	    { 
		complete++;
		cout << "Layering Histogram: "<< hnames[n] <<endl;
		for (int j=0;j<numF;j++)
		    plot_hists.push_back( hs[j][i] );
		layerHistogram(plot_hists,inames,can,n+1);
	    }
	}
    }
    if (complete==hnames.size())
    {
    	can->Write();
    }
    else
    	cout << "Unable to locate all desired Histograms." << endl;
}

//Plots single-layered NxN canvases for every dimension of hs, with each dimension named by inames
void plotEvery(vector<vector<TH1*>> hs, vector<TString> inames, TString opath, vector<TString> types)
{
    cout << "plotEvery: Plotting Every Histograms." << endl;
    int numF = hs.size();
    int numH = hs[0].size();
    int c_side = ceil(sqrt(numH));
    for (int i=0;i<numF;i++)
    {
	TString name = "All_Histograms-";
	name+=inames[i];
	TCanvas* allc = newCanvas(name);
	allc->SetName(name);
	allc->SetTitle(name);
	double ipart;
	if(modf(sqrt(numH),&ipart) < 0.4)
	    allc->Divide(c_side,c_side-1);
	else
	    allc->Divide(c_side,c_side);
	for (int j=0;j<numH;j++)
	{
	    allc->cd(j+1);
	    hs[i][j]->SetMarkerColor(kBlue+3);
	    hs[i][j]->SetMarkerStyle(1);
	    hs[i][j]->SetFillStyle(1001);
	    hs[i][j]->SetFillColor(kBlue+3);
	    hs[i][j]->SetLineColor(kBlue+3);
	    gStyle->SetOptStat(111111);
 	    hs[i][j]->DrawCopy();
	}
	allc->Write();
	
	for (int t=0;t<types.size(); t++)
	{
	    	gROOT->ProcessLine(".!mkdir -p "+opath+types[t]+"/");
		allc->Print(opath+types[t]+"/"+"all--"+inames[i]+"."+types[t],types[t]);
	}
    }
}

//Creates NxN canvas of every histogram layered
void layerAll(vector<vector<TH1*>> hs, vector<TString> inames)
{
    cout << "Layering all histograms." << endl;
    int numF = hs.size();
    int numH = hs[0].size();
    vector<const char*> names;
    for (int i=0;i<numH;i++)
	names.push_back(hs[0][i]->GetName());
    TCanvas* layc = newCanvas("All_Histograms_Layered");
    int c_side = ceil(sqrt(numH));
    double ipart;
    if(modf(sqrt(numH),&ipart) < 0.4)
	layc->Divide(c_side,c_side-1);
    else
	layc->Divide(c_side,c_side);
    layc->SetName("AllHistogramsLayered");
    layerSpecific(hs, names, inames, layc);
}


void gtprint(TH1 *h, TString name, TString opath)
{
  
  TCanvas *cv = newCanvas(name);			
  cv->cd(1);
  int max=h->GetBinContent(h->GetMaximumBin());
  h->SetMaximum(max);
  h->SetMinimum(0);

  //Focus on Valid ChipIDs
  if (strcmp(h->GetName(),"ChipID")==0)
    h->GetXaxis()->SetRangeUser(3700,3800);

  //Set color/style
  h->SetMarkerColor(kBlue+3);
  h->SetMarkerStyle(1);
  h->SetFillStyle(1001);
  h->SetFillColor(kBlue+3);
  h->SetLineColor(kBlue+3);

  gStyle->SetOptStat(111111);

  h->Draw();

  //Add Legend
  TLegend* leg = new TLegend(0.78,0.4,0.98,0.45);
  leg->AddEntry(h,name,"l");
  leg->Draw();

  gROOT->ProcessLine(".!mkdir -p "+opath+"/");

  cv->Print(opath+name+".jpg","jpg");
  cv->Print(opath+name+".png","png");
  //cv->Print(opath+name+".pdf","pdf");

  //Create JSON file
  ofstream jsonfile;
  jsonfile.open(opath+name+".json");
  TString json = TBufferJSON::ConvertToJSON(h);
  jsonfile << json;
  jsonfile.close();

}

void gemTreePrint(TDirectory *source, TString outPath, bool first)
{

  //Create equivalent output directory via newPath (ignore initial .root directory)
  TString newPath;
  if(!first){
    newPath = outPath + source->GetName() + "/";      
    if(DEBUG) std::cout<<"[gemTreePrint]"<< "newPath: " << newPath << std::endl;
    gROOT->ProcessLine(".!mkdir -p "+newPath);
  }
  else  newPath = outPath;

  //Retrieve histograms from current directory
  vector<TH1*> hs;
  retrieveHistograms(source, hs);
  int numH = hs.size();
  if(DEBUG) std::cout<<"[gemTreePrint]"<< "In directory: " << source->GetName() << std::endl;
  if(DEBUG) std::cout<<"[gemTreePrint]"<< "Number of histrograms retrieved: " << numH << std::endl;

  //Check for directories, print histograms
  TList* keylist = new TList;
  keylist = source->GetListOfKeys();
  TIter nextkey(keylist);
  TKey *key = new TKey;
  int key_c = 1; //counter
  while (key = (TKey*)nextkey())
    {
      if(DEBUG) std::cout<< std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key: " << key_c << std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key Name: " << key->GetName() << std::endl;
      if(DEBUG) std::cout<<"[gemTreePrint]"<< "Key Class: " << key->GetClassName() << std::endl;
      key_c++;
      TClass *cl = gROOT->GetClass(key->GetClassName());
      
      //Recursively loop through directories
      if (cl->InheritsFrom(TDirectory::Class())) {
         source->cd(key->GetName());
         TDirectory *subdir = gDirectory;
	 gemTreePrint(subdir, newPath, false);
      }
      //Print if key is a histogram
      if (cl->InheritsFrom("TH1")) {
	if(DEBUG) std::cout<<"[gemTreePrint]"<< "Printing... " << std::endl;
	TH1 *h = (TH1*)key->ReadObj();
	gtprint(h,key->GetName(),newPath);
      }
    }
  return;
}
