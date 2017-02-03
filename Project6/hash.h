#ifndef _hash_
#define _hash_

void freeFn(void *elemAddr);

int cmp(const void *elemAddr1, const void *elemAddr2);

int hash(const void *elemAddr, int numBuckets);


#endif
