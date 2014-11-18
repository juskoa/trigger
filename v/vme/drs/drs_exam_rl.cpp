#include "tools.cpp"
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
