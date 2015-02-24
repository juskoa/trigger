#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "infolog.h"
#ifndef NOINFOLOGGER
#ifdef CPLUSPLUS
extern "C" {
#include "infoLogger.h"
}
#else
#include "infoLogger.h"
#endif
#endif
/*------------------------------------------------------------- infolog()
severity: LOG_INFO, LOG_ERROR, LOG_FATAL  from 22.3.2012: LOG_WARNING, LOG_DEBUG
msg: message (no \n at the end!)
Note:
SYSTEM: see infoLoggerStandalone.sh: DATE_INFOLOGGER_SYSTEM=TRG
FACILITY: set once in:
    ltu_proxy/main(), dummy_ctp.c/main() ttcmi/ttcmi.c:
        infolog_SetFacility("ltu_DETNAME")
STREAM: set in ltu_utils Setglobalmode/Setstdalonemode
        infoSetStream(PARTIITON_NAME/dimservername);

In CERNLAB the following should be started to setup environment:
. /opt/infoLogger/infoLoggerStandalone.sh
*/
void ctplog(char severity, char *msg) {
if((severity==LOG_ERROR) || (severity==LOG_FATAL)) {
  prtError(msg);
} else if(severity==LOG_INFO) {
  prtLog(msg);
} else {
  prtWarning(msg);
};
}

#ifndef NOINFOLOGGER
void my_infoLogS_f(char severity, char *msg) {
int rc;
rc= infoLogger_msg_xt(UNDEFINED_STRING,UNDEFINED_INT,UNDEFINED_INT,UNDEFINED_STRING,severity,INFOLOGLEVEL_OPS,msg);
}
#endif

void infolog_trg(char severity, char *msg) {
#ifndef NOINFOLOGGER
  my_infoLogS_f(severity, msg);
#else
  ctplog(severity, msg);
#endif
}
void infolog_trgboth(char severity, char *msg) {
#ifndef NOINFOLOGGER
  my_infoLogS_f(severity, msg);
#endif
ctplog(severity, msg);
}
void ecs2dcs(char *ecs, char *dcs) {
if(strcmp(ecs, "SPD")==0) { strcpy(dcs,"SPD");
} else if(strcmp(ecs, "SDD")==0) { strcpy(dcs,"SDD");
} else if(strcmp(ecs, "SSD")==0) { strcpy(dcs,"SSD");
} else if(strcmp(ecs, "TPC")==0) { strcpy(dcs,"TPC");
} else if(strcmp(ecs, "TRD")==0) { strcpy(dcs,"TRD");
} else if(strcmp(ecs, "TOF")==0) { strcpy(dcs,"TOF");
} else if(strcmp(ecs, "HMPID")==0) { strcpy(dcs,"HMP");
} else if(strcmp(ecs, "PHOS")==0) { strcpy(dcs,"PHS");
} else if(strcmp(ecs, "CPV")==0) { strcpy(dcs,"CPV");
} else if(strcmp(ecs, "PMD")==0) { strcpy(dcs,"PMD");
} else if(strcmp(ecs, "MUON_TRK")==0) { strcpy(dcs,"MCH");
} else if(strcmp(ecs, "MUON_TRG")==0) { strcpy(dcs,"MTR");
} else if(strcmp(ecs, "FMD")==0) { strcpy(dcs,"FMD");
} else if(strcmp(ecs, "T0")==0) { strcpy(dcs,"T00");
} else if(strcmp(ecs, "V0")==0) { strcpy(dcs,"V00");
} else if(strcmp(ecs, "ZDC")==0) { strcpy(dcs,"ZDC");
} else if(strcmp(ecs, "ACORDE")==0) { strcpy(dcs,"ACO");
} else if(strcmp(ecs, "EMCAL")==0) { strcpy(dcs,"EMC");
} else if(strcmp(ecs, "DAQ")==0) { strcpy(dcs,"DAQ");
} else { strcpy(dcs,"");
};
}
void infolog_SetStream(char *stream, int run) {
/* use run:0 if runnumber not known, or 
          -1 if no update of DATE_RUN_NUMBER requested
ctp_proxy
*/
#ifndef NOINFOLOGGER
char crun[20];
char det3[4];
//infoSetStream(stream);  commented from 22.3.2012 (not more in infoLogger)
if((strncmp(stream,"PHYSICS_x",8)==0) ||
   (strncmp(stream,"TEST_x",5)==0) ||
   (stream[0]=='\0')) {
  infoLogger_setParam(INFOLOG_PARAM_FIELD_PARTITION,stream);
} else {
  ecs2dcs(stream,det3);
  if(det3[0]!='\0') {
    infoLogger_setParam(INFOLOG_PARAM_FIELD_DETECTOR,det3);
  } else {
    // should not happen:
    infoLogger_setParam(INFOLOG_PARAM_FIELD_DETECTOR,stream);
  };
};
if(run>0) {
  sprintf(crun, "%d",run);
} else {
  crun[0]='\0';
};
setenv("DATE_RUN_NUMBER", crun, 1);
// following from 24.2.2015 (Sylvain advice - env. is reread only once
// per 60 secs)
infoLogger_setParam_int(INFOLOG_PARAM_FIELD_RUN_NUMBER,run);
#endif
}
void infolog_SetFacility(char *facility) {
#ifndef NOINFOLOGGER
infoSetFacility(facility);
#endif
}

