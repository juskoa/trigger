/*
  Simple SSM dump file creator - to produce some data for ssm_analyz testing
 */

/*
  Recognized keywords: %stop     - stop and close (or EOF)
                       %evnt     - new event
		       %tick     - data (%tick [tick],[SSM dato],[BCNT dato]
                                                       if < 0      if < 0
						       not filled  not filled
		       %ende     - end of event
		       %ctrl     - control word (ssdmp.exe control and setting)

  Recognized commands:
                       @prtd     - print dictionaries (debug only)
		       @vrbs     - increase verbosity
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vmewrap.h>
#include "ttcit_io.h"
#include "ttcit.h"
#include "ttcit_conf.h"

#define MAX_DICTION_PAIRS 50
#define MAX_DICTION_KLEN 6
#define MAX_LINE_LEN 254

enum { MY_FALSE = 0, MY_TRUE };

enum keywrd { KWSTOP = 1, KWEVNT, KWTICK, KWENDE, KWCTRL };
enum contwrd { CWPRTD = 1, CWVRBS };

/*
#define KWSTOP 1
#define KWEVNT 2
#define KWTICK 3
#define KWENDE 4
#define KWCTRL 5

#define CWPRTD 1
#define CWVRBS 2
*/

struct dictitem {
  char key[MAX_DICTION_KLEN];
  int val;
};

struct diction {
  int n;
  struct dictitem d[MAX_DICTION_PAIRS];
};

/*
  Functions 
 */

void add_diction(struct diction *dict, const char *key, int val);
int get_diction(struct diction *dict, char *key);
void print_diction(struct diction *dict);
int split_tick_itemw(char *rl,int *tick, int *ss, int *bc);

/*
  Global variables
 */

int verbose_flag;

struct ttcit_io_header IOheader;
struct ttcit_io_footer IOfooter;
struct ttcit_io_event IOevent;

struct diction dict;
struct diction ctrlw;

int main(){
  int retval = 0;
  int file_opened = 0;
  char fileName[] = TTCIT_MONITOR_FILE;
  int fd;
  int irc;

  char *s = NULL;
  char line[MAX_LINE_LEN];
  char comand[6];
  char restofl[MAX_LINE_LEN];
  int todo;

  int nomore;

  struct SSMbuffer ssm;
  struct BCfifo bc;
  int i;
  int tocon;

  int dtick, dssm, dbc;

  // struct ttcit_io_header *iohd_ptr = &IOheader;
  // struct diction *dict_ptr = &dict;

  verbose_flag = MY_FALSE;

  dict.n = 0;
  add_diction(&dict,"%stop\0",KWSTOP);
  add_diction(&dict,"%evnt\0",KWEVNT);
  add_diction(&dict,"%tick\0",KWTICK);
  add_diction(&dict,"%ende\0",KWENDE);
  add_diction(&dict,"%ctrl\0",KWCTRL);

  ctrlw.n = 0;
  add_diction(&ctrlw,"@prtd\0",CWPRTD);
  add_diction(&ctrlw,"@vrbs\0",CWVRBS);

  do{
    /*
      Open file
     */
    file_opened = MY_FALSE;
    fd = ttcit_io_open(&fileName[0], TTCIT_IO_WRITE);
    if(fd == -1){
      printf("Inut file %s cannot be opened -> STOP\n",&fileName[0]);
      retval = 1;
      break;
    }
    file_opened = MY_TRUE;

    /*
      Write file header
     */
    irc = ttcit_io_write_header(fd, &IOheader);
    if(irc <= 0){
      printf("Error on writing IO header IRC = %d  -> STOP\n",irc);
      retval = 1;
      break;
    }

    /*
      Process all input lines, act accordingly until EOF or %stop
     */
    while((s = gets(&line[0])) != NULL){
      if(verbose_flag){
	printf("Processing: %s\n",&line[0]);
      }
      strncpy(&comand[0],&line[0],5);
      comand[6] = '\0';
      strcpy(&restofl[0],&line[6]);
      todo = get_diction(&dict,&comand[0]);
      switch(todo){
      case KWSTOP:
	nomore = MY_TRUE;
	break;
      case KWEVNT:
	ssm.top = 0;
	for(i = 0; i < MEGA; i++){
	  ssm.buf[i] = 0x0;
	}
	bc.top = 0;
	for(i = 0; i < BCNT_FIFO_SIZ; i++){
	  bc.buf[i] = 0x0;
	}
	break;
      case KWTICK:
	dtick = -1;
	dssm = -1;
	dbc = -1;
	irc = split_tick_itemw(&restofl[0],&dtick,&dssm,&dbc);
	if(irc != 0){
	  printf("@tick line cannot be itemized IRC = %d\n",irc);
	  retval = -1;
	  nomore = MY_TRUE;
	  break;
	}
	if(dtick < 0){
	  printf("Ignoring negative tick %d\n",dtick);
	  break;
	}
	if(dssm > 0){
	  ssm.top = dtick + 1;
	  ssm.buf[dtick] = (unsigned long)dssm;
	}
	if(dbc > 0){
	  bc.buf[bc.top++] = (unsigned long)dbc;
	}
	break;
      case KWENDE:
	irc = ttcit_io_make_event(fd,&IOevent,&ssm,TTCIT_IO_SSM_SCOPE,&bc);
	if(irc < 0){
	  printf("New event cannot be written IRC = %d\n",irc);
	  retval = 1;
	  nomore = MY_TRUE;
	  break;
	}
	if(irc == 0){
	  printf("Empty buffer is not written, check input data\n");
	  break;
	}
	break;
      case KWCTRL:
	tocon = get_diction(&ctrlw,&restofl[0]);
	switch(tocon){
	case CWPRTD:
	  printf("Dictionary: COMMANDS:\n");
	  print_diction(&dict);
	  printf("Dictionary: CONTROL:\n");
	  print_diction(&ctrlw);
	  break;
	case CWVRBS:
	  verbose_flag = MY_TRUE;
	  break;
	default:
	  printf("Ignoring unrecognized command: %s\n",&restofl[0]);
	  break;
	}
	break;
      default:
	printf("Ignoring unrecognized command line: %s\n",&line[0]);
	break;
      }
      if(nomore){
	break;
      }
    }
    if(retval == 1){
      break;
    }

    /*
      Write file footer
     */
    irc = ttcit_io_write_footer(fd, &IOfooter);
    if(irc <= 0){
      printf("Error on writing IO footer IRC = %d  -> STOP\n",irc);
      retval = 1;
      break;
    }

  }while(0);

  /*
    Close file
   */
  if(file_opened){
    irc = ttcit_io_close(fd);
    if(irc != 0){
      printf("Error on closing file IRC = %d\n",irc);
      retval = 1;
    }
  }

  return retval;
}

void add_diction(struct diction *dict, const char *key, int val){
  do{
    if(dict->n >= MAX_DICTION_PAIRS){
      printf("Dictionary DICTION is full\n");
      break;
    }
    strncpy(&dict->d[dict->n].key[0],key,MAX_DICTION_KLEN);
    dict->d[dict->n].key[MAX_DICTION_KLEN] = '\0';
    dict->d[dict->n++].val = val;

  }while(0);
}

int get_diction(struct diction *dict, char *key){
  int retval = -1;
  int scm;
  int i;
  for(i = 0; i <= dict->n; i++){
    scm = strncmp(&dict->d[i].key[0],key,MAX_DICTION_KLEN);
    if(scm == 0){
      retval = dict->d[i].val;
      break;
    }
  }
  return retval;
}

void print_diction(struct diction *dict){
  int i;

  printf("Dictionary print\n");
  printf("==================\n");
  for(i = 0; i < dict->n; i++){
    printf("KEY: %s    VAL = %d\n",&dict->d[i].key[0],dict->d[i].val);
  }
  printf("==================\n");
}

int split_tick_itemw(char *rl, 
		     int *tick, int *ss, int *bc){
  int irc = 0;
  char tokstr[MAX_LINE_LEN];
  char *stick, *sssm, *sbc;
  int nchar;

  do{
    strncpy(&tokstr[0],rl,MAX_LINE_LEN);
    tokstr[MAX_LINE_LEN-1] = '\0';

    stick = strtok(&tokstr[0],",");
    if(stick == NULL){
      printf("No tick value given in %s\n",rl);
      irc = -1;
      break;
    }
    nchar = sscanf(stick,"%d",tick);
    if(nchar == 0){
      printf("Wrong TICK value in %s\n",&stick[0]);
      irc = -4;
      break;
    }

    sssm = strtok(NULL,",");
    if(sssm == NULL){
      printf("No SSM value given in %s\n",rl);
      irc = -2;
      break;
    }
    nchar = sscanf(sssm,"%d",ss);
    if(nchar == 0){
      printf("Wrong SSM value in %s\n",&sssm[0]);
      irc = -5;
      break;
    }

    sbc = strtok(NULL,",");
    if(sbc == NULL){
      printf("No BC value given in %s\n",rl);
      irc = -3;
      break;
    }
    nchar = sscanf(sbc,"%d",bc);
    if(nchar == 0){
      printf("Wrong BC value in %s\n",&sbc[0]);
      irc = -6;
      break;
    }


  }while(0);

  return irc;
}
