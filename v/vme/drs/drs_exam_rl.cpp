/********************************************************************\

  Name:         drs_exam.cpp
  Created by:   Stefan Ritt

  Contents:     Simple example application to read out a DRS4
                evaluation board

  $Id: drs_exam.cpp 21308 2014-04-11 14:50:16Z ritt $

\********************************************************************/

#include <math.h>

#ifdef _MSC_VER

#include <windows.h>

#elif defined(OS_LINUX)

#define O_BINARY 0

#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DIR_SEPARATOR '/'

#endif

#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <dis.hxx>
#include <stdlib.h>

#include "strlcpy.h"
#include <csignal>
#include <ctime>
#include "DRS.h"
//============================================================================================
#define NDIM 1024
using namespace std;
//-----------------------------------------------------------------------
// Utility for parsing text file
//
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
//==============================================================================================
//  Used in dim service publishing
//
struct ScopeDimData{
 double time;
 double val1,val2,val3;
};
//===============================================================================================
// Analysis of wave forms
//
class AnalyseWaves {
 private:
	bool first[8];
	float *stime[8],*wave[8];
	float edges[8][NDIM];
	float mins[8],maxs[8];
	float fmean[8],fsigma[8],fmax[8];
 public:
	AnalyseWaves();
	void SetWaveTime(int i,float *t,float *w){stime[i]=t;wave[i]=w;}
	void SetWaveTime(float *t[8],float *w[8]);
	int FindMinMax(int iw);
	int FindMinMax();
	int FindEdge(float thres,float* t,float * w, float* edge,int& nedges);
	int FindEdge(float thres,int iw,int& nedges);
	int Compare2Waves(int iw1,int iw2);
	int Compare4Channels(ScopeDimData* dd);
        int SaveWaves();
};
AnalyseWaves::AnalyseWaves()
{
 for(int i=0;i<8;i++){
    first[i]=1;
    stime[i]=0; wave[i]=0;
    mins[i]=0;maxs[i]=0;
 }
}
int AnalyseWaves::SaveWaves()
{
 int i;
 FILE *f;
 f = fopen("data.dat", "w");
 float *t0=stime[0]; float *t1=stime[1];float *t2=stime[2];float *t3=stime[3];
 float *w0=wave[0];float *w1=wave[1]; float *w2=wave[2]; float *w3=wave[3];
 for (i=0 ; i<1024 ; i++){
   fprintf(f, "%10.5f %8.2f %10.5f %8.2f %10.5f  %8.2f  %10.5f  %8.2f\n", t0[i],w0[i],t1[i],w1[i],t2[i], w2[i], t3[i], w3[i]);
   }
 fclose(f);
 return 0;
}
int AnalyseWaves::FindMinMax()
{
 for(int i=0;i<8;i++){
  if(wave[i])FindMinMax(i);
 }
 return 0;
}
//----------------------------------------------
// Find MIn Max only at first call
//---------------------------------------------
int AnalyseWaves::FindMinMax(int iw)
{
 if(first[iw]==0) return 0;
 first[iw]=0;
 if(wave[iw]==0){
  printf("Trying  FinfMinMax with unitialised wave %i \n",iw);
  return 1;
 }
 float fmin=0; float fmax=0;
 for(int i=0;i<NDIM;i++){
   if(fmin>wave[iw][i])fmin=wave[iw][i];
   if(fmax<wave[iw][i])fmax=wave[iw][i];
 }
 mins[iw]=fmin;maxs[iw]=fmax;
 printf("Wave %i: min= %f max=%f \n",iw,fmin,fmax);
 return 0;
}
void AnalyseWaves::SetWaveTime(float *t[8],float *w[8])
{
 for(int i=0;i < 8; i++){
    stime[i]=t[i];
    wave[i]=w[i];	
    printf("SetWaveTime: %p %p \n",t[i],wave[i]);
 }
}
//-------------------------------------------------------
// wave and time are parameters
//----------------------------------------------------------
int AnalyseWaves::FindEdge(float thres,float* t,float * w, float* edge,int& nedges)
{
 // some broken data at the begining of sample ?
 // brutal force =  start from 5
 nedges=0;
 for(int i=5;i<NDIM;i++){
    if((w[i-1]<=thres) && (w[i]>thres)){
      float a=(w[i]-w[i-1])/(t[i]-t[i-1]);
      float b=w[i]-a*t[i];
      edge[nedges]=(thres-b)/a;
      //printf("a, b %f %f \n",a,b);
      //printf("i=%i time %f w0 %f w2 %f  edge %f \n",i,t[i],w[i-1],w[i],edge[nedges]);
      nedges++;
    }
 }
 return 0;
}
//----------------------------------------------------------
// wave and time are members of the class
//----------------------------------------------------------
int AnalyseWaves::FindEdge(float thres,int iw,int& nedges)
{
 // some broken data at the begining of sample ?
 // brutal force =  start from 5
 float *w=wave[iw];
 float *t=stime[iw];
 float *edge=edges[iw];
 nedges=0;
 for(int i=5;i<NDIM;i++){
    if((w[i-1]<=thres) && (w[i]>thres)){
      float a=(w[i]-w[i-1])/(t[i]-t[i-1]);
      float b=w[i]-a*t[i];
      edge[nedges]=(thres-b)/a;
      //printf("a, b %f %f \n",a,b);
      //printf("i=%i time %f w0 %f w2 %f  edge %f \n",i,t[i],w[i-1],w[i],edge[nedges]);
      nedges++;
    }
 }
 return 0;
}
int AnalyseWaves::Compare2Waves(int iw1,int iw2)
{
 int nedges1,nedges2;
 if(FindMinMax(iw1)) return 1;
 //FindEdge((mins[iw1]+maxs[iw1])/3.,iw1,nedges1);
 FindEdge(0,iw1,nedges1);
 //FindEdge((mins[iw2]+maxs[iw2])/3.,iw2,nedges2);
 FindEdge(0,iw2,nedges2);
 // define jitter as: rising edge1-rising edge2, so values 0-25 always positive
 // now make sure that sequence start with edge1
 int istart1=0,istart2=0;
 //if(abs(edges[iw1][0]-edges[iw2][0]) > 12.5){
 //  if(edges[iw1][0]<edges[iw2][0])istart1=1;
 //     else istart2=1;
 //}
 //if(edges[iw1][0] > edges[iw2][0]){
 //  printf("edge1 > edge2 %f %f \n",edges[iw1][0],edges[iw2][0]);
 //  return 1;
 //}
 int nn=nedges1;
 if(nedges1>nedges2 )nn=nedges2;
 if((istart1+istart2)>0)nn = nn-1;
 //printf("istart1 %i istart2 %i nn %i \n",istart1,istart2,nn);
 float dmax=0;
 float sumx=0,sumx2=0;
 int ndelta=0;
 for(int j=0;j<nn;j++){
    float delta=edges[iw1][j+istart1]-edges[iw2][j+istart2];
    //printf("edge1= %f  edge2= %f delta=%f \n",edges[iw1][j+istart1],edges[iw2][j+istart2],delta);      
    sumx += delta;
    sumx2 += delta*delta;
    ndelta++;
    if(dmax<delta)dmax=delta;
 }
 float dmean=sumx/ndelta;
 float dmean2=sumx2/ndelta;
 float dsigma=sqrt(dmean2-dmean*dmean);
 //printf("Waves %i %i Tot mean= %f Tot sigma=%f max= %f \n\n",iw1,iw2,dmean,dsigma,dmax);
 fmean[iw2]=dmean;
 fsigma[iw2]=dsigma;
 fmax[iw2]=dmax;

 return 0;
}
int AnalyseWaves::Compare4Channels(ScopeDimData *dd)
{
 if(Compare2Waves(0,1)) return 1;
 //Compare2Waves(0,2);
 //Compare2Waves(0,3);
 time_t now;
 now=time(0);
 //printf("------------------------------------------\n");
 //printf("measured means: %ld %f %f %f\n",now,fmean[1],fmean[2],fmean[3]); 
 //printf("measured sigmas: %ld %f %f %f\n",now,fsigma[1],fsigma[2],fsigma[3]); 
 //printf("measured maxs: %ld %f %f %f\n",now,fmax[1],fmax[2],fmax[3]); 
 dd->time=now;
 dd->val1=fsigma[1];
 dd->val2=fsigma[2];
 dd->val3=fsigma[3];
 if(fsigma[1] > 1.) SaveWaves();
 return 0;
}

//****************************************************************************
// Read Data from file. Used for debugging
//****************************************************************************
int mainfromfile(string& filename)
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
  AnalyseWaves anal;
  anal.SetWaveTime(0,stime[0],wave[0]);
  anal.SetWaveTime(1,stime[1],wave[1]);
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
       if(nitems != 4){
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
    anal.Compare2Waves(0,1);
    nsample++;
  }
  printf("All samples read nsample: %i \n",nsample);
  return 0;
}
//**********************************************************************
// Signal interrupted
sig_atomic_t signaled = 0;
int numsigs=0;
void sighandler( int signum )
{
    printf("Interrupt signal %i received.\n",signum);
    if(numsigs>10) exit(signum); 
    // cleanup and close up stuff here  
    // terminate program  
    printf("Stoping properly \n");
    signaled=1;
    //exit(signum);
    numsigs++;
    return;  
}
//**************************************************************************
int main()
{
   // Debug from file
   //
   //string file("data.dat");
   //mainfromfile(file);
   //return 0;
   //
   // Signal handler
   //
   signal(SIGABRT, sighandler);
   signal(SIGTERM, sighandler);
   signal(SIGINT, sighandler);
   signal(SIGKILL, sighandler);
   //
   // Signal debug
   //j=0;
   //while(signaled==0 )printf("signaled=%i %i\n",signaled,j++);
   //printf("Finished \n");
   //fflush(stdout);
   //return 1;
   //
   //  DIM
   char format[4];
   strcpy(format,"D:4");
   printf("format= %s \n",format);
   ScopeDimData* a = new ScopeDimData;
   int asize=  sizeof(*a);
   printf("size of scope data: %i \n",sizeof(*a));
   DimService scope("ScopeServer/SIGMAS",format,(void*)a,asize);
   DimServer::start("ScopeServer");
   //
   int i, j, nBoards;
   DRS *drs;
   DRSBoard *b;
   float time_array[8][1024];
   float wave_array[8][1024];
   //FILE  *f;

   // Prepare analysis
   AnalyseWaves anal;
   anal.SetWaveTime(0,time_array[0],wave_array[0]);
   anal.SetWaveTime(1,time_array[1],wave_array[1]);
   anal.SetWaveTime(2,time_array[2],wave_array[2]);
   anal.SetWaveTime(3,time_array[3],wave_array[3]);

   /* do initial scan */
   drs = new DRS();

   /* show any found board(s) */
   for (i=0 ; i<drs->GetNumberOfBoards() ; i++) {
      b = drs->GetBoard(i);
      printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n", 
         b->GetBoardSerialNumber(), b->GetFirmwareVersion());
   }

   /* exit if no board found */
   nBoards = drs->GetNumberOfBoards();
   if (nBoards == 0) {
      printf("No DRS4 evaluation board found\n");
      return 0;
   }

   /* continue working with first board only */
   b = drs->GetBoard(0);

   /* initialize board */
   b->Init();

   /* set sampling frequency */
   b->SetFrequency(5, true);

   /* enable transparent mode needed for analog trigger */
   b->SetTranspMode(1);

   /* set input range to -0.5V ... +0.5V */
   b->SetInputRange(0);

   /* use following line to set range to 0..1V */
   //b->SetInputRange(0.5);
   
   /* use following line to turn on the internal 100 MHz clock connected to all channels  */
   //b->EnableTcal(1);

   /* use following lines to enable hardware trigger on CH1 at 50 mV positive edge */
   if (b->GetBoardType() >= 8) {        // Evaluaiton Board V4&5
      b->EnableTrigger(1, 0);           // enable hardware trigger
      b->SetTriggerSource(1<<0);        // set CH1 as source
   } else if (b->GetBoardType() == 7) { // Evaluation Board V3
      b->EnableTrigger(0, 1);           // lemo off, analog trigger on
      b->SetTriggerSource(0);           // use CH1 as source
   }
   b->SetTriggerLevel(0.01);            // 0.05 V
   b->SetTriggerPolarity(false);        // positive edge
   
   /* use following lines to set individual trigger elvels */
   //b->SetIndividualTriggerLevel(1, 0.1);
   //b->SetIndividualTriggerLevel(2, 0.2);
   //b->SetIndividualTriggerLevel(3, 0.3);
   //b->SetIndividualTriggerLevel(4, 0.4);
   //b->SetTriggerSource(15);
   
   b->SetTriggerDelayNs(0);             // zero ns trigger delay
   
   /* use following lines to enable the external trigger */
   //if (b->GetBoardType() == 8) {     // Evaluaiton Board V4
   //   b->EnableTrigger(1, 0);           // enable hardware trigger
   //   b->SetTriggerSource(1<<4);        // set external trigger as source
   //} else {                          // Evaluation Board V3
   //   b->EnableTrigger(1, 0);           // lemo on, analog trigger off
   // }

   /* open file to save results */
   // Analysing files directly
   //f = fopen("results.dat", "w");
   //if (f == NULL) {
   //   perror("ERROR: Cannot open file \"results.dat\"");
   //   return 1;
   //}
   /* repeat X times */
   //for (j=0 ; j<300000 ; j++) {
   j=0;
   //while((j++<1)) {
   while((signaled==0)) {
      //printf("Taking sample # %i  signaled=%i \n",j,signaled);

      /* start board (activate domino wave) */
      b->StartDomino();

      /* wait for trigger */
      //printf("Waiting for trigger... \n");
      
      fflush(stdout);
      int iw=0;
      while (b->IsBusy() && (iw<10000))iw++;
      if(iw==10000){
	printf("Trigger not arrived ... \n");
        delete drs;
	return 1;
      }	
      /* read all waveforms */
      b->TransferWaves(0, 8);

      /* read time (X) array of first channel in ns */
      /* decode waveform (Y) array of first channel in mV */
      b->GetTime(0, 0, b->GetTriggerCell(0), time_array[0]);
      b->GetWave(0, 0, wave_array[0]);

      /* read time (X) array of second channel in ns
       Note: On the evaluation board input #1 is connected to channel 0 and 1 of
       the DRS chip, input #2 is connected to channel 2 and 3 and so on. So to
       get the input #2 we have to read DRS channel #2, not #1. */
      /* decode waveform (Y) array of second channel in mV */
      b->GetTime(0, 2, b->GetTriggerCell(0), time_array[1]);
      b->GetWave(0, 2, wave_array[1]);
      //
      b->GetTime(0, 4, b->GetTriggerCell(0), time_array[2]);
      b->GetWave(0, 4, wave_array[2]);
      //
      b->GetTime(0, 6, b->GetTriggerCell(0), time_array[3]);
      b->GetWave(0, 6, wave_array[3]);

      /* Save waveform: X=time_array[i], Yn=wave_array[n][i] */
      //fprintf(f, "Event #%d ----------------------\nt1[ns]\tu1[mV]\tt2[ns]\tu2[mV]\n",j);
      //for (i=0 ; i<1024 ; i++)
      //   fprintf(f, "%10.5f\t%8.2f\t%10.5f\t%8.2f\n", time_array[0][i], wave_array[0][i], time_array[1][i], wave_array[1][i]);
      //
      //for (i=0 ; i<1024 ; i++)
      //printf("%10.5f\t%8.2f\t%10.5f\t%8.2f\n", time_array[2][i], wave_array[2][i], time_array[3][i], wave_array[3][i]);

      if(anal.Compare4Channels(a)==0){
        scope.updateService();
        usleep(5000000);
      }
      /* print some progress indication */
      if((j%100) == 0)printf("\rEvent #%d read successfully\n", j);
      j++;
   }

   //fclose(f);
   
   /* delete DRS object -> close USB connection */
   delete drs;
}
