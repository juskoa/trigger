#define LOG_INFO 'I'
#define LOG_ERROR 'E'
#define LOG_FATAL 'F'
void infolog_trg(char level, char *msg);
void infolog_trgboth(char level, char *msg);
void infolog_SetStream(char *stream, int run);
void infolog_SetFacility(char *facility);

