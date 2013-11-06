#define MAXNAMELENGTH 81
#define MAXNAMES 600

#define funcall w32 (*)(w32, w32, w32, w32, w32, w32, w32, w32, w32, w32)

typedef struct{
  char name[MAXNAMELENGTH];
  w32 fls;   /* low 8 bits for par. type, higher bits
	       meaning	see flags description in cmdbase.c */
} Tpardesc;

typedef struct{
  char name[MAXNAMELENGTH];
 /* enum Ttokentype namet; */
  w32 fls;        /* see flags description in cmdbase.c */
  w32 (*fp)(w32,w32,w32,w32,w32,w32,w32,w32,w32,w32);    /* pointer to func */
  union {
    w32 intvar;
    char *strptr;
    w32 *intvar_ptr;
  };
  /*w32 intvar;     
      tVAR:    variable value or 
      tVMEADR: last read/write value or
      tFUN:    last result (int, w32, char,  NOT float ! )
      tSTRING: pointer to string in malloc() memory
      tSYMNAME: pointer to string "0x100" -BoardSpaceLength */
  float floatvar;   /* 
      tFUN: last result for tFUN
      tVAR: variable value */
  Tpardesc *pardesc;  /*
      tFUN: pointer to Tpardesc structure (NULL if no pars) */
                  /* if board name, the number of the board (if more identical
		     boards) */
  union{ w32 vmenp; char *bax; };   /* 
      tVMEADR:  vme address
      tFUN:     #of pars,
      tSYMNAME: name is board name, vmenp is the pointer to the
                default board base address in text form (for unix, wxx) 
                It can be changed in the time of cmdbase
		initialisation -according to supplied argument for cmdbase
                */
  char *usage;  /*
      tFUN:  help -function usage. NULL if no help available 
      tSTRING: if length of the string <4, the value of the string
      */
} Tname;

