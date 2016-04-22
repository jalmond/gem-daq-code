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

//using namespace std;

#endif


void printDQMCanvases()
{
  gROOT->SetBatch(kTRUE);
  printIntegrityCanvas();
  printOccupancyCanvas();
  printClusterSizeCanvas();
  printClusterMultCanvas();
}
void printIntegrityCanvas()
{
  TGaxis::SetMaxDigits(3);
  gStyle->SetOptStat(0000);
  TString integrity_plots[7] = {"Control Bit 1010", "Control Bit 1100", "Control Bit 1110", "Flag", "VFAT Slot Number", "N_{VFAT} Per Event", "CRC difference"};
  TCanvas *integrity = newCanvas("Integrity plots", 4, 2, 2400,1200);
  THStack *stack;

  integrity->cd(1);
  stack = stackH1(integrity_plots[0], hi1010[0], hi1010[2], 4, 2);
  stack->Draw();
  //gPad->SetLogy();

  integrity->cd(2);
  stack = stackH1(integrity_plots[1], hi1100[0], hi1100[2], 4, 2);
  stack->Draw();
  //gPad->SetLogy();

  integrity->cd(3);
  stack = stackH1(integrity_plots[2], hi1110[0], hi1110[2], 4, 2);
  stack->Draw();
  //gPad->SetLogy();

  integrity->cd(4);
  stack = stackH1(integrity_plots[3], hiFlag[0], hiFlag[2], 4, 2);
  stack->Draw();
  //gPad->SetLogy();

  integrity->cd(5);
  stack = stackH1(integrity_plots[4], hiVFATsn[0], hiVFATsn[2], 4, 2, true);
  stack->Draw();

  integrity->cd(6);
  stack = stackH1(integrity_plots[5], hiVFAT[0], hiVFAT[2], 4, 2);
  stack->Draw();
  //gPad->SetLogy();

  integrity->cd(7);
  stack = stackH1(integrity_plots[6], hiDiffCRC[0], hiDiffCRC[2], 4, 2);
  stack->Draw();
  //gPad->SetLogy();

  integrity->cd(8);
  hi2DCRC[1]->SetMarkerColor(4);
  hi2DCRC[2]->SetMarkerColor(2);
  hi2DCRC[1]->Draw();
  integrity->cd(8);
  hi2DCRC[2]->Draw("same");

  //integrity->Print("integrity.png","png");
  //integrity->Print("integrity.pdf","pdf");

  ofstream jsonfile;
  jsonfile.open("/tmp/dqm_hists/canvases/integrity.json");
  TString json = TBufferJSON::ConvertToJSON(integrity);
  jsonfile << json;
  jsonfile.close();

  //integrity->Draw();
  //integrity->Update();
  //integrity->WaitPrimitive();
}
THStack *stackH1(TString title, TH1 * h1, TH1 * h2, int cl1, int cl2, bool scale=false)
{
  THStack *t_stack = new THStack(title,title);
  h1->SetLineColor(cl1);
  h2->SetLineColor(cl1);
  if (scale) {
    h1->Scale(1/hiVFAT[0]->GetEntries());
    h2->Scale(1/hiVFAT[0]->GetEntries());
    TString tmp_label = h1->GetYaxis()->GetTitle();
    tmp_label += " per event";
    h1->GetYaxis()->SetTitle(tmp_label);
  }
  t_stack->Add(h1);
  t_stack->Add(h2);
  t_stack->Draw();
  t_stack->GetXaxis()->SetTitle(h1->GetXaxis()->GetTitle());
  t_stack->GetYaxis()->SetTitle(h1->GetYaxis()->GetTitle());
  t_stack->GetYaxis()->SetTitleOffset(1.5);
  return t_stack;
}
void printOccupancyCanvas()
{
  TCanvas *occupancy = newCanvas("Occupancy plots", 3, 3, 1800,1800);
  occupancy->cd(1);
  std::stringstream ss;
  for (int nb = 1; nb < 9; nb ++)
  {
    std::string name = "eta_";
    ss.str(std::string());
    ss << 9-nb;
    name+=ss.str();
    hiBeamProfile[0]->GetXaxis()->SetBinLabel(nb, name.c_str());
  }
  hiBeamProfile[0]->GetYaxis()->SetTitle("Strips");
  hiBeamProfile[0]->GetXaxis()->SetTitle("Pseudorapidity partitions");
  hiBeamProfile[0]->Draw("colz");

  //TPad *grid = new TPad("grid","",0,0,1,1);
  //grid->Draw();
  //grid->cd();
  //grid->SetGridx();
  //grid->SetFillStyle(4000);
  //grid->SetFrameFillStyle(0);

  //TH2 *hgrid = new TH2C("hgrid","",8,0.,8.,384,0.,384.);   
  //hgrid->Draw("same");
  //hgrid->GetXaxis()->SetNdivisions(8);
  //hgrid->GetYaxis()->SetLabelOffset(999.);
  //hgrid->GetXaxis()->SetLabelOffset(999.); 
  
  //TLine *l1 = new TLine(0, 128, 8, 128);
  //TLine *l2 = new TLine(0, 256, 8, 256);
  //l1->Draw("same");
  //l2->Draw("same");

  TH1D* p_temp;
  for (int p_i = 1; p_i < 9; p_i++)
  {
    occupancy->cd(p_i+1);
    std::string title = "eta_";
    ss.str(std::string());
    ss << 9-p_i;
    title+=ss.str();
    p_temp = hiBeamProfile[0]->ProjectionY(title.c_str(),p_i,p_i);
    p_temp->SetTitle(title.c_str());
    p_temp->GetYaxis()->SetTitle("Number of events");
    p_temp->Draw();
    gPad->Update();
    //TLine *l3 = new TLine(128, 0, 128, gPad->GetUymax());
    //TLine *l4 = new TLine(256, 0, 256, gPad->GetUymax());
    //l3->Draw("same");
    //l4->Draw("same");
  }
  //occupancy->Print("occupancy.png","png");
  //occupancy->Print("occupancy.pdf","pdf");

  ofstream jsonfile;
  jsonfile.open("/tmp/dqm_hists/canvases/occupancy.json");
  TString json = TBufferJSON::ConvertToJSON(occupancy);
  jsonfile << json;
  jsonfile.close();

  //occupancy->Draw();
  //occupancy->Update();
  //occupancy->WaitPrimitive();
}

void printClusterSizeCanvas()
{
  gStyle->SetOptStat("emr");
  TCanvas *clusterSize = newCanvas("Cluster size plots", 3, 3, 1800,1800);
  std::stringstream ss;
  clusterSize->cd(1);
  hiClusterSize[0]->GetYaxis()->SetTitle("Number of entries");
  hiClusterSize[0]->GetXaxis()->SetTitle("Cluster size");
  hiClusterSize[0]->SetTitle("Integrated over pseudorapidity");
  hiClusterSize[0]->Draw();
  gPad->SetLogy();
  for (int p_i = 1; p_i < NETA+1; p_i++)
  {
    clusterSize->cd(p_i+1);
    std::string title = "eta_";
    ss.str(std::string());
    ss << 9-p_i;
    title+=ss.str();
    hiClusterSizeEta[0][NETA-p_i]->GetYaxis()->SetTitle("Number of entries");
    hiClusterSizeEta[0][NETA-p_i]->GetXaxis()->SetTitle("Cluster size");
    hiClusterSizeEta[0][NETA-p_i]->SetTitle(title.c_str());
    hiClusterSizeEta[0][NETA-p_i]->Draw();
    gPad->SetLogy();
  }
  //clusterSize->Print("clusterSize.png","png");
  //clusterSize->Print("clusterSize.pdf","pdf");

  ofstream jsonfile;
  jsonfile.open("/tmp/dqm_hists/canvases/clusterSize.json");
  TString json = TBufferJSON::ConvertToJSON(clusterSize);
  jsonfile << json;
  jsonfile.close();

  //clusterSize->Draw();
  //clusterSize->Update();
  //clusterSize->WaitPrimitive();
  gStyle->SetOptStat(0000);
}

void printClusterMultCanvas()
{
  gStyle->SetOptStat("emr");
  TCanvas *clusterMult = newCanvas("Cluster multiplicity plots", 3, 3, 1800,1800);
  std::stringstream ss;
  clusterMult->cd(1);
  hiClusterMult[0]->GetYaxis()->SetTitle("Number of entries");
  hiClusterMult[0]->GetXaxis()->SetTitle("Cluster multiplicity");
  hiClusterMult[0]->SetTitle("Integrated over pseudorapidity");
  hiClusterMult[0]->Draw();
  gPad->SetLogy();
  for (int p_i = 1; p_i < NETA+1; p_i++)
  {
    clusterMult->cd(p_i+1);
    std::string title = "eta_";
    ss.str(std::string());
    ss << 9-p_i;
    title+=ss.str();
    hiClusterMultEta[0][NETA-p_i]->GetYaxis()->SetTitle("Number of entries");
    hiClusterMultEta[0][NETA-p_i]->GetXaxis()->SetTitle("Cluster multiplicity");
    hiClusterMultEta[0][NETA-p_i]->SetTitle(title.c_str());
    hiClusterMultEta[0][NETA-p_i]->Draw();
    gPad->SetLogy();
  }
  //clusterMult->Print("clusterMult.png","png");
  //clusterMult->Print("clusterMult.pdf","pdf");

  ofstream jsonfile;
  jsonfile.open("/tmp/dqm_hists/canvases/clusterMult.json");
  TString json = TBufferJSON::ConvertToJSON(clusterMult);
  jsonfile << json;
  jsonfile.close();

  //clusterMult->Draw();
  //clusterMult->Update();
  //clusterMult->WaitPrimitive();
  gStyle->SetOptStat(0000);
}
