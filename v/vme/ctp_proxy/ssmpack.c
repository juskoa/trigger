/*----------------------------------------*/ int main(int argc, char **argv) {
int rc;
char f1[80], f2[80];
if((argc<3) || (argc>4)) {
  printf("Usage:\n
ssmpacking name.dmp name.pck      -pack name.dmp file to name.pck\n
ssmpacking -u name.pck name.dmp   -unpack name.pck file to name.dmp\n
");
exit(8);
};
if(strcmp(argv[1],"-u")==0) {
  ix=2;
} else {
  ix=1;
};
strcpy(f1, argv[ix]); strcpy(f2, argv[ix+1]);
}
