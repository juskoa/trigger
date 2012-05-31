#include <string.h>
void do_partitionCtpConfigItem(char *name, char *partitionCtpConfigItem) {
//strncpy(partitionCtpConfigItem,"/part ",sizeof(partitionCtpConfigItem));
strcpy(partitionCtpConfigItem,"/part ");
strcat(partitionCtpConfigItem, name);
  //sizeof(partitionCtpConfigItem)-strlen(partitionCtpConfigItem));
strcat(partitionCtpConfigItem,"/CTP config");
  //sizeof(partitionCtpConfigItem)-strlen(partitionCtpConfigItem));
}

