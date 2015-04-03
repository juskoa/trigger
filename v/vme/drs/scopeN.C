#define NDIM 1024
//-----------------------------------------------------------------------
void splitstring(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
TCanvas *c1 = new TCanvas("c1","Waves",200,10,700,500);
//===================================================================================================
int DrawWaves(Float_t *t1,Float_t* t2,Float_t* w1,Float_t* w2)
{
 Int_t Npoints=NDIM;
 //Int_t Npoints=nlines;
 //for(Int_t i=0;i<10;i++)printf("%f %f \n",t1[i],w1[i]);
 //
 // First wave
 //
 c1->cd();
 TGraph* gw1 = new TGraph(Npoints,t1,w1);
 gw1->SetMarkerColor(1);
 gw1->SetMarkerStyle(21);
 gw1->SetMarkerSize(0.5);
 gw1->GetXaxis()->SetTitle("time [ns]");
 //gw1->Draw("AP");
 gw1->Draw("APC");
 //
 // Second wave
 //
 TGraph* gw2 = new TGraph(Npoints,t2,w2);
 gw2->SetMarkerColor(2);
 gw2->SetMarkerStyle(21);
 gw2->SetMarkerSize(0.5);
 gw2->Draw("CP");
}
int readFile(string& filename)
{
  float stime[8][1024];
  float wave[8][1024];
  int nsample=0;
  string dir("data/");
  //filename=dir+filename;
  printf("Reading file:%s \n ",filename.c_str());
  ifstream file;
  file.open(filename.c_str());
  if(!file){
    printf("%s : cannot be opened.\n",filename.c_str());
    return 1;
  }else{
    printf("%s : opened successfully.\n",filename.c_str());
  }
  //anal.SetWaveTime(0,stime[0],wave[0]);
  //anal.SetWaveTime(1,stime[1],wave[1]);
  //anal.SetWaveTime((float**) stime,(float**) wave);
  string line;
  bool dflag=0,eflag=0;
  int nlines=0,ndat=0;
  int nmaxsample=NDIM;
  if(nsample>0) nmaxsample=nsample;
  nsample=0;
  while (getline(file,line)) {
       //if(nlines<10)printf("%s \n", line.c_str()); 
       //printf("%s \n", line.c_str()); 
       if(line.find("Event") != string::npos){
          printf("%s \n", line.c_str()); 
          eflag=1;
          continue;
       }
       if(line.find("t1[ns]") != string::npos){
          if(!eflag){
            printf("Error: unexpected line sequence.\n");
            return 1;
          }
          eflag=0;
          if(dflag){
            printf("close sample %i \n",ndat);
            if(ndat != NDIM){
             printf("ErrorL ndat= %i \n",ndat);
             return 1;
            }
    	    anal.Compare2Waves(0,1);
            ndat=0;
            nsample++;
            if(nsample>=nmaxsample){
             printf("Warning:  nsample=%i \n",nsample);
             return 0;
            }
          }
          printf("open sample \n");
          dflag=1;
          continue;
       }
       // take data
       vector<string> items;
       splitstring(line,items," ");
       int nitems=items.size();
       if((nitems != 4) && (nitems != 8)){
         printf("Expexted number of items in line != 4 : %i \n",nitems);
         return 1;
       }
       double t1,t2;
       //cout << atof(items[0].c_str()) << endl;
       t1=atof(items[0].c_str());
       wave[0][ndat]=atof(items[1].c_str());
       t2=atof(items[2].c_str());
       wave[1][ndat]=atof(items[3].c_str());
       stime[0][ndat]=t1;
       stime[1][ndat]=t2;
       //if(t1 != t2){
       //  printf("Warning: t1 != t2 %f %f \n",t1,t2);
       //}
       if(ndat >= NDIM){
         printf("Error: unexpected size of data %i \n",ndat);
         return 1;
       }
       nlines++;
       ndat++;
  }
  if(dflag){
    printf("close sample %i \n",ndat);
    if(ndat != NDIM){
       printf("ErrorL ndat= %i \n",ndat);
       return 1;
    }
    //anal.Compare2Waves(0,1);
    nsample++;
  }
  Float_t* tt1=stime[0];
  Float_t* tt2=stime[1];
  Float_t* w1=wave[0];
  Float_t* w2=wave[1];
  DrawWaves(tt1,tt2,w1,w2);
  printf("All samples read nsample: %i \n",nsample);
  return 0;

}
int scopeN()
{
 string file("/home/alice/trigger/rl/drs/dataspike5.dat");
 readFile(file);
}
