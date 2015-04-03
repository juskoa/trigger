#define LOG_INFO 'I'
#define LOG_WARNING 'W'
#define LOG_ERROR 'E'
#define LOG_FATAL 'F'
void infolog_trg(char severity, char *msg);
void infolog_trg2(int errcode, char *errsource, int errline);
void infolog_trgboth2(int errcode, char *errsource, int errline);
void infolog_trgboth(char severity, char *msg);
void infolog_SetStream(char *stream, int run);
void infolog_SetFacility(char *facility);

