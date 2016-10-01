#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* Address of n_th bucket in a hashset */
#define Nth_Vector_Addr(base, elemsize, n) ((vector *) (((char *) h->buckets) + (n * sizeof(vector))))

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert (elemSize > 0);
	assert (numBuckets > 0);
	assert (hashfn);
	assert (comparefn);

	h->elemSize = elemSize;
	h->numBuckets = numBuckets;
	h->size = 0;
	h->hashfn = hashfn;
	h->comparefn = comparefn;
	h->freefn = freefn;
	h->buckets = malloc(numBuckets * sizeof(vector));
	assert(h->buckets);

	int i;
	for(i = 0; i < numBuckets; ++i)
		VectorNew(Nth_Vector_Addr(h->buckets, sizeof(vector), i), elemSize, freefn, 2);
}

void HashSetDispose(hashset *h)
{
	int i;
	for (i = 0; i < h->numBuckets; ++i)
		VectorDispose(Nth_Vector_Addr(h->buckets, sizeof(vector), i));

	free(h->buckets);
}

int HashSetCount(const hashset *h)
{
	return h->size;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert (mapfn);

	int i;
	for (i = 0; i < h->numBuckets; ++i)
		VectorMap(Nth_Vector_Addr(h->buckets, sizeof(vector), i), mapfn, auxData);
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert (elemAddr);
	int hashCode = h->hashfn(elemAddr, h->numBuckets);
	assert (hashCode >= 0);
	assert (hashCode < h->numBuckets);

	vector *v = Nth_Vector_Addr(h->buckets, sizeof(vector), hashCode);
	int pos = VectorSearch(v, elemAddr, h->comparefn, 0, false);
	if (pos == -1)
	{
		VectorAppend(v, elemAddr);
		++h->size;
	}
	else
		VectorReplace(v, elemAddr, pos);
}

void HashSetDelete(hashset *h, const void *elemAddr){
	
	vector *v = Nth_Vector_Addr(h->buckets, sizeof(vector), hashCode);
	int pos = VectorSearch(v, elemAddr, h->comparefn, 0, false);
	VectorDelete(v, pos);

}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
	assert (elemAddr);
	int hashCode = h->hashfn(elemAddr, h->numBuckets);
	assert (hashCode >= 0);
	assert (hashCode < h->numBuckets);

	vector *v = Nth_Vector_Addr(h->buckets, sizeof(vector), hashCode);
	int pos = VectorSearch(v, elemAddr, h->comparefn, 0, false);
	if (pos == -1)
		return NULL;
	else
		return VectorNth(v, pos);
}
