#define MNPART 6      // Maximum number of partitions
#define NCON 4        // # of connectors at FO
#ifdef DBMAIN
//char errorReason[ERRMSGL]; not needed -see main_ctp.c and test.c
#endif
//
//------------------------------------------------------------------
// Input/output
void xcountersStart(w32 run, w32 clgroup);
//------------------------------------------------------------------
// Reads partition from *.cfg from disk.
Tpartition *readDatabase2Tpartition(char *filename); //cfg2ctp.c
//------------------------------------------------------------------
//  Partitions[] manipulations
//-------------------------------------------------------------------
// delete partition from list Partitions[]
int deletePartitions(Tpartition *part);
// add partition to Partitions[]
int addPartitions(Tpartition *part);
// clear all ClusterTables
int clearClusterTablePartitions();
//  get partition with name from Partitions[] list
Tpartition *getPartitions(char *name, Tpartition *parray[]);
//-------------------------------------------------------------------
//  Check Resources.
//------------------------------------------------------------------
// check conflict of resources of part against existing Partitions[]
Tpartition *checkTS();
int checkResources(Tpartition *part);
int checkClusters(Tpartition *part);
int checkDetectors(Tpartition *part);
int checkRBIF(Tpartition *part,Tpartition *parray[]);
//--------------------------------------------------------
// HW manipulations
// add partition part to HW
int addPartitions2HW();
int addClasses2HW();
int addFO2HW();
w32 clusterPart2HW(w32 pclustercode,Tpartition *part,w32 *ret);
int Detector2Connector(int idet,int *ifo,int *iconnector);
/*---------------------------------------------------------------
  Hardware operations
-----------------------------------------------------------------*/
// Set daq busy to all classes
void setALLDAQBusy();
void unsetALLDAQBusy();
// Set DAQ busy to partition after there was alldaq busy
int setPartDAQBusy(Tpartition *part, int detectors);
// Unset DAQ busy to partition
int unsetPartDAQBusy(Tpartition *part, int detectors);
// enable BC-triggers by disabling RND1
void ctp_Disablernd1(int usecs);
// read ctp, ltu counters:
void readALLcnts(Tpartition *part, char xse);
int generateXOD(Tpartition *part,char x, w32 *orbitn);

/*----------------------------------------------------------------
  Routines for smi:
------------------------------------------------------------------*/
int ctp_Initproxy();
int ctp_Endproxy();
int ctp_PausePartition(char *name, int detectors);
int ctp_SyncPartition(char *name, char * errorReason, w32 *orbitn);
int ctp_ResumePartition(char *name, int detectors);
int ctp_LoadPartition(char *name,char *mask, int run_number, char *ACT_CONFIG, char *errorReason);
int ctp_InitPartition(char *name,char *mask, int run_number, char *ACT_CONFIG, char *errorReason);
int ctp_StartPartition(char *name, char *errorReason);
int ctp_StopPartition(char *name);

