#define NCLUST 6
/* info for DAQlogbook gathered [mostly] in ctpproxy and passed to the server
and consequently to daq db */
#define MAXRUN1MSG 500
typedef struct TDAQInfo {
w32 daqonoff;           // 0: CTP readout active
w32 masks[NCLUST];      // detectors in each CLUSTER
w32 inpmasks[NCLUST];   // input detectors feeding each CLUSTER
/*unsigned long long classmasks[NCLUST];  // classes for each CLUSTER
does not sent correctly to 64 bits. But w32 ok, i.e.:
*/
w32 classmasks01_32[NCLUST];
w32 classmasks33_64[NCLUST];
char run1msg[MAXRUN1MSG]; // orig. rcfg msg, as used before LS1
} TDAQInfo;

//daqlogbook.c:
int daqlogbook_open();
//int daqlogbook_update_triggerClassName(unsigned int runN, unsigned char classN, char *value);
//int daqlogbook_update_triggerClassCounter(unsigned int runN, unsigned char classN, unsigned int L2count);
int daqlogbook_update_triggerClassName(unsigned int run, unsigned char classId, 
  const char *className, unsigned int classGroupId, float classGroupTime, 
  const char *downscaling, const char **aliases);
int daqlogbook_update_triggerClassCounter(unsigned int run, unsigned char classId, 
  unsigned long long L0bCount, unsigned long long L0aCount, 
  unsigned long L1bCount, unsigned long L1aCount, 
  unsigned long L2bCount, unsigned long L2aCount, float ctpDuration);
int daqlogbook_update_triggerGlobalCounter(unsigned int run, unsigned long L2aCount, float ctpDuration);
int daqlogbook_update_triggerDetectorCounter(unsigned int run, const char *detector, unsigned long L2aCount);
int daqlogbook_update_triggerClusterCounter(unsigned int run, unsigned int cluster, unsigned long L2aCount);
int daqlogbook_insert_triggerInput(unsigned int run,
  unsigned int inputId, const char *inputName, unsigned int inputLevel);
int daqlogbook_update_triggerInputCounter(unsigned int run,
  unsigned int inputId, unsigned int inputLevel, unsigned long long inputCount);
int daqlogbook_add_comment(int runNor0,char *title,char *msg);
int daqlogbook_close();
int daqlogbook_update_LTUConfig(unsigned int run, const char *detector,
  unsigned int LTUFineDelay1, unsigned int LTUFineDelay2, 
  unsigned int LTUBCDelayAdd);
int daqlogbook_update_triggerConfig(int runn, char *mem, char *alignment);
int daqlogbook_update_cs(unsigned int runn, char *cs_string);
int daqlogbook_update_ACTConfig(unsigned int rundec, char *itemname,char *instname,char *version);
int daqlogbook_update_clusters(unsigned int runn, char *pname,
  TDAQInfo *daqi, unsigned int ignoredaqlog);

