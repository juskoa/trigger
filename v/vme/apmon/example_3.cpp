/*********************************************************************ApMon: EXAMPLE_3.CPP*****************************************************************************************************/                                                                             
/**\file example_3.cpp                                                          
 * This ApMon file is responsible for sending CTP inputs rates and DET busy values
 * to MonALISA. The user selects what kind of data he wants to send. If busy values,
 * he writes a "B" in the beginning of command line. If CTP inputs, "I".
 *
 * For busy values, 4 values are sent:
 * -> Run number where the detector is
 * -> Name of the detector                                                      
 * -> Busy values in microseconds                                               
 * -> Busy limit: also in microseconds, depends on the detector                 
 *                                    
 * For the CTP inputs, 2 values are sent:
 * -> CTP input name
 * -> CTP input rate (in Hz)
 *
 * The user has to enter a command such as the following:                       
 * './example_3
 * /**************BUSY VALUES**********                                                                 
 * B epochtime Det_Number1 run_number1 Busy_value1 Det_Number2 run_number2 Busy_value2 .....                                                                     
 * /**************CTP INPUTS**********
 * I epochtime INP1 INP2 ... INP48
 *
 * DETECTORS BUSY VALUES:
 * There is a maximum of 6 different run numbers that can be entered in the command line.                                                                      
 * For each of this run, this program selects the corresponding detectors and busy value/limit                                                                 
 * and then sends it to MonALISA.                                               
 * The format expected in MonALISA is: for a given run number, there is a list of detectors,                                                                   
 * like DET(SPD) or DET(TPC) for example. For each of these detector, it is possible to get                                                                   
 * the busy value and the busy limit.                                                                       
 * The detector numbers are the followings                                      
 * Attention: Numbers between 0 and 24 -> For now, det 20, 22, 23 are not implemented.                                                                         
 * ACO:16 AD0:21 CPV:8 EMC:18 FMD:12 HMP:6 MTR:10 MCH:11 PHS:7 PMD:9 SDD:1      
 * SPD:0 SSD:2 T00:13 TOF:5 TPC:3 TRD:4 TRI:17 TST:19 V00:14 ZDC:15'.           
 * For a non-defined detector, just send 0 as run number or nothing at all for this det
 * and nothing will be sent for this detector. Det number 20, 22 and 23 are not attributed
 * for now.                                                                                                       
 * The busy value is supposed to be expressed in microsecond and to be under 1 second,                                                                         
 * otherwise the detector is regarded ad permanently busy and the value 10 000 is sent                                                                        
 * to MonALISA (to manage to see the other curves).                             
 *                                                       
 * CTP INPUTS:
 * This is sent even where there is no run. 48 values are always sent to MonALISA.
 * There is a table where the inputs names are entered. The user has just to enter the ept
 * and 48 float numbers in the write order (the first for inp1, ...). The ApMon program
 * associates these values to the corresponding name and sends this to MonALISA.
 *                       
 * The sent data can also be seen in the MonALISA service test - daqTest_pcaldvmh05 - 40.                                                                      
 * The file "apmonConfig.conf" contains the addresses of the hosts to which     
 * we send the parameters, and also the corresponding ports.                    
 *                                                                              
 * To try this program you can launch it via ./example_3 and then:              
 * B 1234567890 0 1000 100 1 1001 101 2 1002 102 3 1003 103 4 1004 104 5 1005 105 6 1000 106 7 1001 107 8 1002 108 9 1003 109 10 1004 110 11 1005 111 12 1000 112 13 1001 113 14 1002 114 15 1003 115 16 1004 116 17 1005 117 18 1000 118 19 1001 119 21 1002 121
 * I 1234567890 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47                                        
 */
/***************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ApMon.h"
#include <math.h>
#include <string.h>

/*********************************************NEW TYPES**********************************************/
/*New type for data linked to detectors*/
typedef struct det det;
struct det {
  char name[10];
  int run_number;
  int busy_value;
  int busy_limit;
  int det_number;
};

/*New type for CTP INPUTS rate*/
typedef struct ctp_inp ctp_inp;
struct ctp_inp {
  char name[15];
  int input_number;
  float input_rate;
};

/**********************************************FUNCTIONS**********************************************/
/**Function freeBuffer()                                                        
 * Free memory stored in the buffer while using fgets                           
 */
void freeBuffer(){
  int c = 0;
  while((c != '\n') && (c != EOF)){
    c = getchar();
  }
}

/**Function read ---> Better than scanf!                                        
 * Read what the user enter in the program                                      
 * return 1 if the reading worked well and 0 otherwise                          
 */
int read(char *text, int size){
  char *entrance = NULL;
  if(fgets(text, size, stdin) != NULL){
    entrance = strchr(text, '\n');
    if (entrance != NULL){
      *entrance = '\0';
    }
    else {
      freeBuffer();
    }
    return 1;
  }
  else {
    freeBuffer();
    return 0;
  }
}

int main() {
  bool condition = true;
  int nCommand = 0;
  char *vmesite, *apmondir; char filename[40]="";
  ApMon *apm;
  vmesite= getenv("VMESITE");
  apmondir= getenv("APMON");
  if(strcmp(vmesite,"ALICE")==0) {
    sprintf(filename, "%s/examples/apmonConfig_alice.conf", apmondir);
  } else if(strcmp(vmesite,"SERVER")==0) {
    sprintf(filename, "%s/examples/apmonConfig_lab.conf", apmondir);
  } else {
    strcpy(filename, "");
  };
  if(filename[0] == '\0') {
    apm= NULL;
    return 8;
  } else {
    //ApMon *apm = new ApMon(filename);
    apm = new ApMon(filename);
  };
  while(condition){                                           
    int p = 0;       //Indicates the position in the read of command line
    int q = 0;       //Fulfills the arrays of detectors or ctp_inputs
    int error = 0;   //Correct detectors/inputs numbers
    long timestamp;
    int nParameters; //Parameters: busy time and busy limit (microseconds) 
    int etime;       //Stores the epochtime
    int timeSent;    //ept sends
    char user_text[1000]; //Brut text entered by the user
    char *textfin = NULL; //Stores the text between two spaces " ". Usefull to fulfill the arrays.
    //Sending busy information
    char **paramNames, **paramValues;
    int *valueTypes;
    //Sending epoch time                                                        
    char **paramNames2, **paramValues2;
    int *valueTypes2;

    /* Table containing the busy limit values ---> Det 20, 22 and 23 are not defined yet                                   
       busy_limit[0] = 0;    //SPD                                              
       busy_limit[1] = 1100; //SDD                                              
       busy_limit[2] = 300;  //SSD                                              
       busy_limit[3] = 600;  //TPC                                              
       busy_limit[4] = 300;  //TRD                                              
       busy_limit[5] = 0;    //TOF                                              
       busy_limit[6] = 300;  //HMP                                              
       busy_limit[7] = 200;  //PHS                                              
       busy_limit[8] = 300;  //CPV                                              
       busy_limit[9] = 600;  //PMD                                              
       busy_limit[10] = 400; //MTR = Muon_TRK                                   
       busy_limit[11] = 200; //MCH = Muon_TRG                                   
       busy_limit[12] = 250; //FMD                                              
       busy_limit[13] = 0;   //T00 = T0                                         
       busy_limit[14] = 0;   //V00 = V0                                         
       busy_limit[15] = 200; //ZDC                                              
       busy_limit[16] = 200; //ACO = ACORDE                                     
       busy_limit[17] = 200; //TRI = TRIGGER                                    
       busy_limit[18] = 300; //EMCAL                                            
       busy_limit[19] = 200;//TST = Test_DAQ                                    
       busy_limit[21] = 0;  //AD0 = AD     
       /***************************************Data coming from the user**********************************************/


    nCommand++;
    printf("command number: %d\n", nCommand);
                                                    
    read(user_text, 1000);
    //Separation of the blanc spaces and storage in an array                    
    textfin = strtok(user_text, " ");
    
    //If the line begins by a "B", busy values sent. If by an "I", CTP rate sent.
    char type_busy[2]="B";
    char type_ctp[2]="I";
    
    /*********SENDING BUSY VALUES***/
    if(strcmp(textfin,type_busy)==0){
      //printf("\n******************************BUSY VALUE SELECTED***/
      int vallimit;      //Busy limit sent to MonALISA
      int valbusy;       //Busy value sent 
      det detectors[24]; //Stores all the data linked to detectors (Busy limit, busy value, name and number)
      textfin  = strtok(NULL, " ");   
      nParameters = 2;  //2 paramaters sents (busy limit and value)
      while(textfin!=NULL){ //Browses all the words in the user text
	if(p==0){
	  etime = atoi(textfin); //First: ept
	}
	else if((p-1)%3 == 0){ //Detector number: To add one, modify the switch loop
	  switch(atoi(textfin)){
	  case 0:
	    sprintf(detectors[q].name, "DET(SPD)");
	    detectors[q].busy_limit = 0;
	    break;
	  case 1:
	    sprintf(detectors[q].name, "DET(SDD)");
	    detectors[q].busy_limit = 1100;
	    break;
	  case 2:
	    sprintf(detectors[q].name, "DET(SSD)");
	    detectors[q].busy_limit = 300;
	    break;
	  case 3:
	    sprintf(detectors[q].name, "DET(TPC)");
	    detectors[q].busy_limit = 600;
	    break;
	  case 4:
	    sprintf(detectors[q].name, "DET(TRD)");
	    detectors[q].busy_limit = 300;
	    break;
	  case 5:
	    sprintf(detectors[q].name, "DET(TOF)");
	    detectors[q].busy_limit = 0;
	    break;
	  case 6:
	    sprintf(detectors[q].name, "DET(HMP)");
	    detectors[q].busy_limit = 300;
	    break;
	  case 7:
	    sprintf(detectors[q].name, "DET(PHS)");
	    detectors[q].busy_limit = 200;
	    break;
	  case 8:
	    sprintf(detectors[q].name, "DET(CPV)");
	    detectors[q].busy_limit = 300;
	    break;
	  case 9:
	    sprintf(detectors[q].name, "DET(PMD)");
	    detectors[q].busy_limit = 600;
	    break;
	  case 10:
	    sprintf(detectors[q].name, "DET(MTR)");
	    detectors[q].busy_limit = 400;
	    break;
	  case 11:
	    sprintf(detectors[q].name, "DET(MCH)");
	    detectors[q].busy_limit = 200;
	    break;
	  case 12:
	    sprintf(detectors[q].name, "DET(FMD)");
	    detectors[q].busy_limit = 250;
	    break;
	  case 13:
	    sprintf(detectors[q].name, "DET(T00)");
	    detectors[q].busy_limit = 0;
	    break;
	  case 14:
	    sprintf(detectors[q].name, "DET(V00)");
	    detectors[q].busy_limit = 0;
	    break;
	  case 15:
	    sprintf(detectors[q].name, "DET(ZDC)");
	    detectors[q].busy_limit = 200;
	    break;
	  case 16:
	    sprintf(detectors[q].name, "DET(ACO)");
	    detectors[q].busy_limit = 200;
	    break;
	  case 17:
	    sprintf(detectors[q].name, "DET(TRI)");
	    detectors[q].busy_limit = 200;
	    break;
	  case 18:
	    sprintf(detectors[q].name, "DET(EMC)");
	    detectors[q].busy_limit = 300;
	    break;
	  case 19:
	    sprintf(detectors[q].name, "DET(TST)");
	    detectors[q].busy_limit = 200;
	    break;
	  case 20:
	    sprintf(detectors[q].name, "UNDEF");
	    detectors[q].busy_limit = 0;
	    break;
	  case 21:
	    sprintf(detectors[q].name, "DET(AD0)");
	    detectors[q].busy_limit = 0;
	    break;
	  case 22:
	    sprintf(detectors[q].name, "UNDEF");
	    detectors[q].busy_limit = 0;
	    break;
	  case 23:
	    sprintf(detectors[q].name, "UNDEF");
	    detectors[q].busy_limit = 0;
	    break;
	  default:
	    printf("Impossible detector number: %d\n", atoi(textfin));
	    printf("Nothing sent for this det (run number, busy limit, busy value)\n\n");
	    error++;
	  }
	  detectors[q].det_number = atoi(textfin);
	}
	else if((p-1)%3 == 1){ //Run number
	  if(error==0){
	    detectors[q].run_number = atoi(textfin);
	  }
	}
	else{ //Busy value
	  if(error==0){
	    detectors[q].busy_value = atoi(textfin);
	    q++;
	  }
	  error=0;
	}
	p++;
	textfin  = strtok(NULL, " ");
      }
      /************************************RUN NUMBERS*****************************************************************************************/

      int nDet = q; //q is the number of detectors
      int global_runs[6];//Stores the different run numbers for sending TRI busy values                                                                                                      
      int r = 1;//Number of different runs                                                                                                                                                          
      for(int h=0;h<6;h++){
        global_runs[h]=0;
      }
      
      global_runs[0] = detectors[0].run_number;
      for(int k=1;k<nDet;k++){
        if((detectors[k].run_number!=global_runs[0])&&(detectors[k].run_number!=global_runs[1])&&(detectors[k].run_number!=global_runs[2])&&(detectors[k].run_number!=global_runs[3])&&(detectors[k]
.run_number!=global_runs[4])&&(detectors[k].run_number!=global_runs[5])&&(detectors[k].run_number!=0)){
          if(r>=6){
            printf("More than 6 runs!\n");
          }
          global_runs[r]=detectors[k].run_number;
          printf("global runs: %d ", global_runs[r]);
          printf("r=%d\n",r);
          r++;
        }
      }
      /*************************************************CHECKING POINT***********************************************************/
      //printf("ok\n");
      
      /****************************************PREPARING THE FORMAT OF THE DATA SENT TO MONALISA*****************************************************/
      /* allocate memory for the arrays */
      paramNames = (char **)malloc(nParameters * sizeof(char *));
      paramValues = (char **)malloc(nParameters * sizeof(char *));
      valueTypes = (int *)malloc(nParameters * sizeof(int));
      /*Sending epoch time*/
      paramNames2 = (char **)malloc(1 * sizeof(char *));
      paramValues2 = (char **)malloc(1 * sizeof(char *));
      valueTypes2 = (int *)malloc(1 * sizeof(int));
      /* initialize the parameter names and types */
      paramNames[0] = (char *)"busyLimit";  //0 if busy and 1 if not
      paramNames[1] = (char *)"busyTime";   //busy value in microseconds
      paramNames2[0] = (char *)"epochtime"; //Epochtime
      valueTypes[0] = XDR_INT32;            //Type of the values
      valueTypes[1] = XDR_INT32;
      valueTypes2[0] = XDR_INT32;           //epochtime
      /*initializes the pointers to the values*/
      paramValues[0] = (char *)&vallimit;
      paramValues[1] = (char *)&valbusy;
      paramValues2[0] = (char *)&timeSent;
      //printf("ok1\n");
      
      /*******************************************SENDING THE DATA TO MONALISA**************************************************************/
      try {
	if((nDet==1) && (detectors[0].det_number==17)){ //If there is no global run: just B EPT 17 0 BusyValue -> TRI sent
          printf("There is no global run \n");
          valbusy = detectors[0].busy_value;
          vallimit = detectors[0].busy_limit;
          printf("Busy time sent for %s: %d limit:%d\n", detectors[0].name, 
            detectors[0].busy_value, detectors[0].busy_limit);
          try {
            timestamp = time(NULL);
            apm -> sendTimedParameters((char *)"0", detectors[0].name, nParameters, paramNames, valueTypes, paramValues, timestamp);
          } catch(runtime_error &e) {
            fprintf(stderr, "Send operation failed: %s\n", e.what());                         
          }
        } else {
	  for (int l=0; l<nDet;l++){
	    /**If the run number is null, nothing is sent.
	     * The detector is in 0 run or is not defined yet.
	     */
	    if((detectors[l].run_number==0) && (detectors[l].det_number!=17)){ 
	      printf("\nDet number %d is in 0 run: nothing sent \n\n", detectors[l].det_number);
	    }
	    else{
	      printf("Detector number: %d\n", detectors[l].det_number);
	      char run_sent[10];
	      sprintf(run_sent, "%d", detectors[l].run_number);
	      
	      /** If busy values are too high, the detector is permanently busy, so we send 10 ms
	       * every time. Allow to see the other curves and to make clear that it is permanently                                                       
	       * busy.*/
	      valbusy = detectors[l].busy_value;
	      if ( valbusy >= 10000){
		printf("Detector %s is permanently busy. %d changed to 10000\n", detectors[l].name, valbusy);
		valbusy = 10000;
	      }
	      else {
		printf("Busy time sent for %s: %d linit:%d\n",
                  detectors[l].name, valbusy, detectors[l].busy_limit);
	      }
	      /**We also send the busy limit values.
	       * They are needed for plotting busy time.
	       * The comparaison busy value and busy limit is done in javascript
	       */
	      vallimit = detectors[l].busy_limit;
	      printf("run value for %s: %s\n", detectors[l].name, run_sent);
	      /**sends the datagram****/

	      /**********FOR TRI*********/
	      if(detectors[l].det_number==17){//For TRI detector, busy value is sent for each run number                                  
		for(int k=0; k<r; k++){
		  /**sends the datagram for TRI****/                                           
		  try {
		    timestamp = time(NULL);
		    char *runs_tri=NULL; //Dynamic allocation because required type (char *)  
		    runs_tri = (char *)malloc(sizeof(char));
		    sprintf(runs_tri,"%d", global_runs[k]);//Convert to the right type 
		    printf("TRI sent for run %s\n",runs_tri);
		    apm -> sendTimedParameters((char *)runs_tri, detectors[l].name, nParameters, paramNames, valueTypes, paramValues, timestamp);
		    free(runs_tri);
		  } catch(runtime_error &e) {
		    fprintf(stderr, "Send operation failed: %s\n", e.what());
		  }
		}
	      }
	      else{ /*************OTHER DETECTORS************/
		try {
		  timestamp = time(NULL);
		  // 42: run number, my_name: detname
		  apm -> sendTimedParameters((char *)run_sent, detectors[l].name, nParameters, paramNames, valueTypes, paramValues, timestamp);
		} catch(runtime_error &e) {
		  fprintf(stderr, "Send operation failed: %s\n", e.what());
		  //exit(-1); 
		}
	      }
	    }
	  }
	}      
	  /*************************************************epochtime*************************************************/
	  printf("etime: %d \n ", etime);
	  timeSent = etime;
	  try {
	    timestamp = time(NULL);
	    apm -> sendTimedParameters((char *)"EPT", (char *)"epochtime", 1, paramNames2, valueTypes2, paramValues2, timestamp);
	  } catch(runtime_error &e) {
	    fprintf(stderr, "send operation failed: %s\n", e.what());
	  }
      } catch(runtime_error &e) {
	fprintf(stderr, "%s\n", e.what());
      }
    /****************SENDING CTP INPUTS RATE***************************************************************************/
    }
    else if(strcmp(textfin,type_ctp)==0){
      //printf("\n**************CTP INPUTS SELECTED**************************************\n\n");
      float valfloat;           //CTP Input rate sent to MonALISA
      ctp_inp ctp_inputs[48];   //Storage of CTP inputs data (name, input rate)
      textfin  = strtok(NULL, " ");
      nParameters = 1; //CTP rate
      while(textfin!=NULL){
	if(p==0){
	  etime = atoi(textfin);
	}
	else {
	  switch(p){ //To add or change a CTP Input name, just modify the switch loop
	  case 1:
	    sprintf(ctp_inputs[q].name, "INPUT(0T0C)");
	    break;
	  case 2:
	    sprintf(ctp_inputs[q].name, "INPUT(0T0A)");
	    break;
	  case 3:
	    sprintf(ctp_inputs[q].name, "INPUT(0TVX)");
	    break;
	  case 4:
	    sprintf(ctp_inputs[q].name, "INPUT(0TSC)");
	    break;
	  case 5:
	    sprintf(ctp_inputs[q].name, "INPUT(0TCE)");
	    break;
	  case 6:
	    sprintf(ctp_inputs[q].name, "INPUT(0VBA)");
	    break;
	  case 7:
	    sprintf(ctp_inputs[q].name, "INPUT(0VBC)");
	    break;
	  case 8:
	    sprintf(ctp_inputs[q].name, "INPUT(0VHM)");
	    break;
	  case 9:
	    sprintf(ctp_inputs[q].name, "INPUT(0VIR)");
	    break;
	  case 10:
	    sprintf(ctp_inputs[q].name, "INPUT(0V0M)");
	    break;
	  case 11:
	    sprintf(ctp_inputs[q].name, "INPUT(0HCO)");
	    break;
	  case 12:
	    sprintf(ctp_inputs[q].name, "INPUT(UNKN)");
	    break;
	  case 13:
	    sprintf(ctp_inputs[q].name, "INPUT(UNKN)");
	    break;
	  case 14:
	    sprintf(ctp_inputs[q].name, "INPUT(0EMC)");
	    break;
	  case 15:
	    sprintf(ctp_inputs[q].name, "INPUT(0DMC)");
	    break;
	  case 16:
	    sprintf(ctp_inputs[q].name, "INPUT(0MUH)");
	    break;
	  case 17:
	    sprintf(ctp_inputs[q].name, "INPUT(0MUL)");
	    break;
	  case 18:
	    sprintf(ctp_inputs[q].name, "INPUT(0MSH)");
	    break;
	  case 19:
	    sprintf(ctp_inputs[q].name, "INPUT(0MLL)");
	    break;
	  case 20:
	    sprintf(ctp_inputs[q].name, "INPUT(0MSL)");
	    break;
	  case 21:
	    sprintf(ctp_inputs[q].name, "INPUT(0SMH)");
	    break;
	  case 22:
	    sprintf(ctp_inputs[q].name, "INPUT(0SH1)");
	    break;
	  case 23:
	    sprintf(ctp_inputs[q].name, "INPUT(0SH2)");
	    break;
	  case 24:
	    sprintf(ctp_inputs[q].name, "INPUT(0SH3)");
	    break;
	  case 25:
	    sprintf(ctp_inputs[q].name, "INPUT(0SH4)");
	    break;
	  case 26:
	    sprintf(ctp_inputs[q].name, "INPUT(0STP)");
	    break;
	  case 27:
	    sprintf(ctp_inputs[q].name, "INPUT(0SLT)");
	    break;
	  case 28:
	    sprintf(ctp_inputs[q].name, "INPUT(0SX2)");
	    break;
	  case 29:
	    sprintf(ctp_inputs[q].name, "INPUT(0SMB)");
	    break;
	  case 30:
	    sprintf(ctp_inputs[q].name, "INPUT(0SC0)");
	    break;
	  case 31:
	    sprintf(ctp_inputs[q].name, "INPUT(0OM2)");
	    break;
	  case 32:
	    sprintf(ctp_inputs[q].name, "INPUT(UNKN)");
	    break;
	  case 33:
	    sprintf(ctp_inputs[q].name, "INPUT(0OMU)");
	    break;
	  case 34:
	    sprintf(ctp_inputs[q].name, "INPUT(0OB3)");
	    break;
	  case 35:
	    sprintf(ctp_inputs[q].name, "INPUT(0O1X)");
	    break;
	  case 36:
	    sprintf(ctp_inputs[q].name, "INPUT(0OB0)");
	    break;
	  case 37:
	    sprintf(ctp_inputs[q].name, "INPUT(0BPA)");
	    break;
	  case 38:
	    sprintf(ctp_inputs[q].name, "INPUT(0BPC)");
	    break;
	  case 39:
	    sprintf(ctp_inputs[q].name, "INPUT(0LSR)");
	    break;
	  case 40:
	    sprintf(ctp_inputs[q].name, "INPUT(0UBA)");
	    break;
	  case 41:
	    sprintf(ctp_inputs[q].name, "INPUT(0UBC)");
	    break;
	  case 42:
	    sprintf(ctp_inputs[q].name, "INPUT(0UGA)");
	    break;
	  case 43:
	    sprintf(ctp_inputs[q].name, "INPUT(0UGC)");
	    break;
	  case 44:
	    sprintf(ctp_inputs[q].name, "INPUT(0USP)");
	    break;
	  case 45:
	    sprintf(ctp_inputs[q].name, "INPUT(0ASC)");
	    break;
	  case 46:
	    sprintf(ctp_inputs[q].name, "INPUT(0ASL)");
	    break;
	  case 47:
	    sprintf(ctp_inputs[q].name, "INPUT(0AMU)");
	    break;
	  case 48:
	    sprintf(ctp_inputs[q].name, "INPUT(0PH0)");
	    break;
	  }
	  ctp_inputs[q].input_rate = atof(textfin); //Stores each number in CTP input_rate
	  q++;
	}
	p++;
	textfin = strtok(NULL, " ");
      }
      int nInp = q; //total number of inputs
      //printf("Fulfill the arrays: ok\n");
      if(q==48){
	printf("Number of inputs: 48 -> OK\n");
	/*Allocates memory for the arrays*/
	//CTP rate
	paramNames = (char **)malloc(nParameters*sizeof(char *));
	paramValues =(char **)malloc(nParameters*sizeof(char *));
	valueTypes = (int *)malloc(nParameters*sizeof(int));
	//epochtime
	paramNames2 = (char **)malloc(nParameters*sizeof(char *));
	paramValues2 =(char **)malloc(nParameters*sizeof(char *));
	valueTypes2 = (int *)malloc(nParameters*sizeof(int));
	
	/*Initializes the parameters names and types*/
	paramNames[0] = (char *)"input_rate";
	valueTypes[0] = XDR_REAL32;
	paramValues[0] = (char *)&valfloat;
	//CTP rate
	paramNames2[0] = (char *)"epochtime";
	valueTypes2[0] = XDR_INT32;
	paramValues2[0] = (char *)&timeSent;
	
	/*Generation of random data and sending the datagrams*/
	try {
	  //for each input
	  for(int l=0;l<nInp;l++){
	    valfloat = ctp_inputs[l].input_rate;
	    //printf("Value sent for CTP input %s: % f\n", ctp_inputs[l].name, ctp_inputs[l].input_rate);
	    /*************************SENDING THE DATA*************************/
	    try{
	      timestamp = time(NULL);
	      apm->sendTimedParameters((char *)"CTP Inputs", ctp_inputs[l].name, nParameters, 
                paramNames, valueTypes, paramValues, timestamp);
	    } catch (runtime_error &e){
	      fprintf(stderr, "send operation failed: %s\n", e.what());
	      //exit(-1);
	    }
	  } //for l
	  /**********************epochtime*************************************/
	  printf("etime: %d\n ", etime);
	  timeSent = etime;
	  try {
	    timestamp = time(NULL);
	    apm -> sendTimedParameters((char *)"EPT", (char *)"epochtime", 1, paramNames2, valueTypes2, paramValues2, timestamp);
	  } catch(runtime_error &e) {
	    fprintf(stderr, "send operation failed: %s\n", e.what());
	  }
	} catch (runtime_error &e){
          fprintf(stderr, "%s\n", e.what());
        }
      } else {
	printf("Bad line received: WRONG INPUTS NUMBER (expected:48, got:%d)\n", q);
        condition = false;
      }
    } else { //Error in the command
      printf("\n\n bad line received \n\n");
      condition = false;
    }
    printf("\n"); fflush(stdout);
  }
  delete apm;
  return 0;
}
