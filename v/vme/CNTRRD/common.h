typedef unsigned int w32;

typedef struct {
  char cname[20];
  int addr;
  char board[8];
  char ltuname[20];
  char type;
} Tsorted;
typedef struct {
  int reladdr;
  w32 prevcs;
  w32 currcs;
} Tcnt1;

void epoch2date(time_t epoch, char *dmyhms);
void vme2volt(w32 vme, int *volts);
int parseLine(char *line, Tsorted *pl);
