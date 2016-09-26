#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

/* Address of n_th element in a vector */
#define Nth_Elem_Addr(base, elemsize, n) (((char *) base) + (n * elemsize))

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
	assert (elemSize > 0);
	assert (initialAllocation >= 0);

	v->elems = malloc(initialAllocation * elemSize);
	assert (v->elems);

	if (initialAllocation == 0)	v->initialAllocationLength = 4;		// as default
	else	v->initialAllocationLength = initialAllocation;

	v->logLength = 0;
	v->alocLength = initialAllocation;
	v->elemSize = elemSize;
	v->freeFn = freeFn;
}

void VectorDispose(vector *v)
{
	if (v->freeFn != NULL)
	{
		int i;
		for (i = 0; i < v->logLength; ++i)
			v->freeFn(Nth_Elem_Addr(v->elems, v->elemSize, i));
	}
	free(v->elems);
}

int VectorLength(const vector *v)
{
	return v->logLength;
}

void *VectorNth(const vector *v, int position)
{
	assert (position >= 0);
	assert (position < v->logLength);

	return Nth_Elem_Addr(v->elems, v->elemSize, position);
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
	assert (position >= 0);
	assert (position < v->logLength);

	void *addr = Nth_Elem_Addr(v->elems, v->elemSize, position);

	if (v->freeFn != NULL)
		v->freeFn(addr);

	memcpy(addr, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
	assert (position >= 0);
	assert (position <= v->logLength);
	if (v->logLength == v->alocLength)
		growAllocationSpace(v);

	char *addr = Nth_Elem_Addr(v->elems, v->elemSize, position);
	memmove(addr + v->elemSize, addr, (v->logLength - position) * v->elemSize);
	memcpy(addr, elemAddr, v->elemSize);
	++v->logLength;
}

void VectorAppend(vector *v, const void *elemAddr)
{
	if (v->logLength == v->alocLength)
		growAllocationSpace(v);

	memcpy(Nth_Elem_Addr(v->elems, v->elemSize, v->logLength), elemAddr, v->elemSize);
	++v->logLength;
}

void VectorDelete(vector *v, int position)
{
	assert (position >= 0);
	assert (position < v->logLength);

	char *addr = Nth_Elem_Addr(v->elems, v->elemSize, position);
	if (v->freeFn != NULL)
		v->freeFn(addr);

	--v->logLength;
	memmove(addr, addr + v->elemSize, (v->logLength - position) * v->elemSize);
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
	assert (compare);

	qsort(v->elems, v->logLength, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
	assert (mapFn);

	int i;
	for (i = 0; i < v->logLength; ++i)
		mapFn(Nth_Elem_Addr(v->elems, v->elemSize, i), auxData);
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
	assert (startIndex >= 0);
	assert (startIndex <= v->logLength);
	assert (searchFn);
	assert (key);
	if (v->logLength == 0) return kNotFound;

	void *res;
	if (isSorted)
		res = bsearch(key, Nth_Elem_Addr(v->elems, v->elemSize, startIndex), v->logLength - startIndex, v->elemSize, searchFn);
	else
		{size_t nmemb = v->logLength - startIndex; res = lfind(key, Nth_Elem_Addr(v->elems, v->elemSize, startIndex), &nmemb, v->elemSize, searchFn);}

	if (res == NULL)
		return kNotFound;

	return ((char *) res - (char *) v->elems) / v->elemSize;
} 

void growAllocationSpace(vector *v)
{
	v->alocLength += v->initialAllocationLength;
	v->elems = realloc(v->elems, v->alocLength * v->elemSize);	
	assert (v->elems);
}