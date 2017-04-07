#include <math.h>
#include <TH1F.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TError.h>
#include <iostream.h>
#include <stdio.h>
#include <iostream.h>
using namespace std;

Double_t busyplot()
{
   
 
 gSystem->Exec("sh $VMECFDIR/toobusy/read5.sh");
   
ifstream myfile("/home/alice/trigger/v/vme/WORK/busysweep");
if (myfile.is_open()) {
  Int_t i=0;
  while (! myfile.eof() ) {
    Int_t val;
    myfile>>val;
    Int_t numbers[1000];
    numbers[i]=val;
    i++;
  }; myfile.close();
} else {
  cout<< "Cannot open busysweep input file" <<endl;
  return;
};
Int_t  rangemax = numbers[0];
  Int_t rangemin = numbers[1];

Int_t  numberofsteps = numbers[2];


TFile *f = new TFile("toobusysweep.root", "RECREATE");


 TH1F *anticumulative = new TH1F("anticumulative", "Anti-cumulative distribution of BUSYlong counts", numberofsteps, rangemin, rangemax);

anticumulative->SetOption("E0");
 anticumulative->GetXaxis()->SetTitle("MINIMAX_limit in microseconds");
  anticumulative->GetYaxis()->SetTitle("Nlongbusy");
  anticumulative->SetMarkerStyle(kFullCircle);

   TH1F *differential = new TH1F("differential", "Differential distribution of BUSYlong counts", numberofsteps,rangemin, rangemax);

differential->SetOption("E0");
 differential->GetXaxis()->SetTitle("MINIMAX_limit in microseconds");
  differential->GetYaxis()->SetTitle("Nlongbusy(i) - Nlongbusy(i+1)");
  differential->SetMarkerStyle(kFullCircle);

  Int_t firstnumber;
      Int_t secondnumber;
      //   Double_t firsterror;
      // Double_t seconderror;
for(Int_t i=3; i<(numberofsteps+4); i++){
  firstnumber = numbers[i];
  Int_t numberprev = numbers[i-1];
  if (i==3){
    secondnumber = 0;
  } else {
    secondnumber = numberprev - firstnumber;
  };
  //anticumulative->SetBinContent(i-2, firstnumber);
  //differential->SetBinContent(i-3, secondnumber);
  anticumulative->SetBinContent(i-3, firstnumber);   // original (-3/-4)
  differential->SetBinContent(i-4, secondnumber);
  //anticumulative->SetBinError(i-3, firsterror);
};
gROOT->SetStyle("Plain");   //white background (default is gray)
TCanvas *c1 = new TCanvas("c1", "Anticumulative plot", 500, 400);
gPad->SetLogy();
anticumulative->DrawCopy(); //c1->Update();
TCanvas *c2 = new TCanvas("c2", "Differential plot", 500, 400);
differential->DrawCopy(); //c2->Update();
gPad->SetLogy();
anticumulative->Write();
differential->Write();
f->Close();
return 0;
}
