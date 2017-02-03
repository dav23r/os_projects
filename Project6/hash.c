#include "hash.h"
#include "config_service.h"
#include <stdlib.h>


void freeFn(void *elemAddr){
  struct config *c = (struct config *)elemAddr;
  free(c->vhost);
  free(c->document_root);
  free(c->cgi_bin);
  free(c->ip);
  free(c->port);
  free(c->log);
}

int cmp(const void *elemAddr1, const void *elemAddr2){
  char *a=(char *)elemAddr1;
  char *b=((struct config*)elemAddr2)->vhost;
  int rs = strcmp(a, b);
  return rs;
}

int hash(const void *elemAddr, int numBuckets) {
  return 1;
  char *str = (char*)elemAddr;
  int hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  if (hash < 0) hash *= -1;
  return hash%numBuckets;
}
