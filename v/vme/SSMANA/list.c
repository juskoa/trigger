struct list {
	struct list *next;
	struct list *prev;
	int poradie;
	int namesize;
	char *name;
};
void memerror()
{
 printf("Not enough memory. Exiting. \n");
 exit(EXIT_FAILURE);
}
/*
Rules for this list:
- list is ordered according to poradie;
- if poradie is same the last coming is last in list
- the first memeber of list unvisible foe user is poradie=0,
  so you should not enter items with poradie <=0
- f points to the end of list, sorting starts from end, as natural
  for ssm problem
- prev is going towards beginning (poradie=0)
- next is going towards end (big poradie)    
*/
struct list *addlist(struct list *f,int poradie,char *text)
/* -f points by definition to the end of list 
   -list is ordered in poradie */
{
 int namesize;
 namesize=strlen(text)+1; /* due to /0 at the end of string */
 /* printf("len of text=%i %s \n",namesize,text); */
 if( f==NULL ){    /*Frst member */
  struct list *prvy;
  prvy=(struct list *) malloc(sizeof(struct list));
  if(!prvy) memerror();
  prvy->name= NULL;
  prvy->prev = NULL;
  prvy->poradie=0;	  	
  
  f=(struct list *) malloc(sizeof(struct list));
  if(!f) memerror();
  f->name= (char *) malloc(namesize*sizeof(char));
  if(!(f->name)) memerror();
  f->next=NULL;
  f->prev=prvy;
  f->poradie=poradie;
  f->namesize=namesize;
  strcpy(f->name,text);
  
  prvy->next=f;
  return f;
 }else{
  struct list *novy,*last;
  novy=(struct list *) malloc(sizeof(struct list));
  if(!novy) memerror();
  novy->name= (char *) malloc(namesize*sizeof(char));
  if(!(novy->name)) memerror();
  last=f;  
  while((poradie < f->poradie)) f=f->prev; 	  	   	  
  if(f->next){  /* Not at the beginning */
    novy->prev=f;
    novy->next=f->next;
    (f->next)->prev=novy;
    f->next=novy;   
  }else{                /* End */
    novy->prev=f;
    novy->next=NULL;
    f->next=novy;
    last=novy;
  }
  
  novy->poradie=poradie;
  novy->namesize=namesize;
  strcpy(novy->name,text);
  return last;  
 }
}
/* Print and delete list */
int printlist(struct list *f,FILE *ff)
{
 if(!ff){
  printf("printlist: file pointer is null \n");
  return 0;
 }
 if(!f){
   printf("printlist: List is empty. \n");
   return 0;
 }  
 struct list *first;
 while(f->prev) f=f->prev; /* Going to start */
 first=f->next;
 free(f->name);
 free(f);
 f=first;
 while(f){
   fprintf(ff,"%7i: %s \n",f->poradie,f->name);
   /*printf("%7i:  %s \n",f->poradie,f->name);*/
   /* printf("%7i: name=%i ",f->poradie,f->namesize);
   for(i=0;i<f->namesize;i++)printf("%c",f->name[i]);
   printf("\n");
   */
   first=f->next;
   free(f->name);
   free(f);
   f=first;
 }
 fclose(ff);
 return 1;
}
int sizeoflist(struct list *f)
{
 if(f){
  int s=0;
  while((f=f->prev))s++;
  return s; 
 }else return 0;	 
}

