/********************************************************************\

  Name:         drs_exam.cpp
  Created by:   Stefan Ritt

  Contents:     Simple example application to read out a DRS4
                evaluation board

  $Id: drs_exam.cpp 21308 2014-04-11 14:50:16Z ritt $

\********************************************************************/

#include <math.h>

#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>

#include <csignal>
#include <ctime>
#define DEBUG 0 
#define DEBUG2 0 
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
      if(DEBUG){
        printf("a, b %f %f \n",a,b);
        printf("i=%i time %f w0 %f w2 %f  edge %f \n",i,t[i],w[i-1],w[i],edge[nedges]);
      }
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
 int ret=0;
 float *w=wave[iw];
 float *t=stime[iw];
 float *edge=edges[iw];
 nedges=0;
 for(int i=10;i<(NDIM-2);i++){
    if((w[i-1]<=thres) && (w[i]>thres)){
      float a=(w[i]-w[i-1])/(t[i]-t[i-1]);  // 1st derivative
      float b=w[i]-a*t[i];
      float c=(w[i+2]-w[i])/(t[i+2]-t[i]);  // 1st derivative up
      //if(c<20.) continue;  // the curve bemd down
      edge[nedges]=(thres-b)/a;
      //check period
      if(nedges>0){
        float prev,deltaT;
        prev=edge[nedges-1];
        deltaT=abs(edge[nedges]-prev-25.);
        if(deltaT>1.){
         printf("Warning: deltaT= %f \n",deltaT);
	 ret=1;
        }
      }
      if(DEBUG){
        float prev=0;
        if(nedges>0) prev=edge[nedges-1];
        printf("%i---------------------------------------\n",iw);
        printf("i= %i a, b, c %f  %f %f \n",i,a,b,c);
        printf("time %f w0 %f w2 %f  edge %f  %f\n",t[i],w[i-1],w[i],edge[nedges],edge[nedges]-prev-25.);
      }
      nedges++;
    }
 }
 return ret;
}
int AnalyseWaves::Compare2Waves(int iw1,int iw2)
{
 int ret=0;
 int nedges1,nedges2;
 if(FindMinMax(iw1)) return 1;
 //FindEdge((mins[iw1]+maxs[iw1])/3.,iw1,nedges1);
 ret += FindEdge(-0.0,iw1,nedges1);
 //FindEdge((mins[iw2]+maxs[iw2])/3.,iw2,nedges2);
 ret += FindEdge(-0.0,iw2,nedges2);
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
    if(DEBUG){
      printf("edge1= %f  edge2= %f delta=%f \n",edges[iw1][j+istart1],edges[iw2][j+istart2],delta);      
    }
    sumx += delta;
    sumx2 += delta*delta;
    ndelta++;
    if(fabs(dmax)<fabs(delta))dmax=delta;
 }
 float dmean=sumx/ndelta;
 float dmean2=sumx2/ndelta;
 float dsigma=sqrt(dmean2-dmean*dmean);
 //printf("Waves %i %i Tot mean= %f Tot sigma=%f max= %f \n\n",iw1,iw2,dmean,dsigma,dmax);
 fmean[iw2]=dmean;
 fsigma[iw2]=dsigma;
 fmax[iw2]=dmax;

 return ret;
}
int AnalyseWaves::Compare4Channels(ScopeDimData *dd)
{
 int ret = 0;
 ret += Compare2Waves(0,1);
 // Uncomment when other 2 waves are connected
 Compare2Waves(0,2);
 //Compare2Waves(0,3);
 time_t now;
 now=time(0);
 if(DEBUG2){
   printf("------------------------------------------\n");
   printf("measured means: %ld %f %f %f\n",now,fmean[1],fmean[2],fmean[3]); 
   printf("measured sigmas: %ld %f %f %f\n",now,fsigma[1],fsigma[2],fsigma[3]); 
   printf("measured maxs: %ld %f %f %f\n",now,fmax[1],fmax[2],fmax[3]); 
 }
 dd->time=now;
 dd->val1=fsigma[1];
 dd->val2=fsigma[2];
 dd->val3=fsigma[3];
 if(fsigma[1] > 1.) SaveWaves();
 if(ret) SaveWaves();
 return 0;
}
