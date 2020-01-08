/*
 * SArray.cc --
 *	Sorted array implementation
 *
 */

#ifndef _SArray_cc_
#define _SArray_cc_

#ifndef lint
static char SArray_Copyright[] = "Copyright (c) 1995-2006 SRI International.  All Rights Reserved.";
static char SArray_RcsId[] = "@(#)$Header: /home/srilm/devel/dstruct/src/RCS/SArray.cc,v 1.41 2007/07/16 23:41:39 stolcke Exp $";
#endif

#ifdef PRE_ISO_CXX
# include <new.h>
#else
# include <new>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "SArray.h"

#undef INSTANTIATE_SARRAY
#define INSTANTIATE_SARRAY(KeyT, DataT) \
	template <> DataT *SArray< KeyT, DataT >::removedData = 0; \
	template class SArray< KeyT, DataT >; \
	template class SArrayIter< KeyT, DataT >

#ifndef __GNUG__
template <class KeyT, class DataT>
DataT *SArray<KeyT,DataT>::removedData = 0;
#endif /* __GNUG__ */

#define BODY(b)	((SArrayBody<KeyT,DataT> *)b)

const double growSize = 1.1;

template <class KeyT, class DataT>
void
SArray<KeyT,DataT>::dump() const
{
    if (body == 0) {
	cerr << "EMPTY" << endl;
    } else {
	unsigned nEntries = numEntries();

	cerr << "maxEntries = " << BODY(body)->maxEntries;
	cerr << " nEntries = " << nEntries;

	for (unsigned i = 0; i < nEntries; i++) {
	    cerr << " " << i << ": ";
	    cerr << BODY(body)->data[i].key
#ifdef DUMP_VALUES
			/* 
			 * Only certain types can be printed.
			 */
		     << "->" << BODY(body)->data[i].value
#endif /* DUMP_VALUES */
		     ;
	}
	cerr << endl;
    }
}

template <class KeyT, class DataT>
void
SArray<KeyT,DataT>::memStats(MemStats &stats) const
{
    stats.total += sizeof(*this);

    if (body) {
	unsigned maxEntries = BODY(body)->maxEntries;

	stats.total += sizeof(*BODY(body)) +
			sizeof(BODY(body)->data[0]) *
				(maxEntries - 1);
	stats.wasted += sizeof(BODY(body)->data[0]) *
				(maxEntries - numEntries());
    }
}

template <class KeyT, class DataT>
void
SArray<KeyT,DataT>::alloc(unsigned size)
{
    body = (SArrayBody<KeyT,DataT> *)malloc(sizeof(*BODY(body)) +
			   (size - 1) * sizeof(BODY(body)->data[0]));
    assert(body != 0);

    BODY(body)->deleted = 0;
    BODY(body)->maxEntries = size;

    /*
     * Fill the array with dummy keys, marking the unused space
     */
    for (unsigned i = 0; i < size; i ++) {
	Map_noKey(BODY(body)->data[i].key);
    }
}

template <class KeyT, class DataT>
SArray<KeyT,DataT>::SArray(unsigned size)
    : body(0)
{
    if (size != 0) {
	alloc(size);
    }
}

template <class KeyT, class DataT>
void
SArray<KeyT,DataT>::clear(unsigned size)
{
    if (body) {
	unsigned maxEntries = BODY(body)->maxEntries;

	for (unsigned i = 0; i < maxEntries; i++) {
	    KeyT thisKey = BODY(body)->data[i].key;

	    if (Map_noKeyP(thisKey)) {
		break;
	    }
	    Map_freeKey(thisKey);
	}
	free(body);
	body = 0;
    }
    if (size != 0) {
	alloc(size);
    }
}

template <class KeyT, class DataT>
void
SArray<KeyT,DataT>::setsize(unsigned size)
{
    if (body == 0 && size != 0) {
	alloc(size);
    }
}

template <class KeyT, class DataT>
SArray<KeyT,DataT>::~SArray()
{
    clear(0);
}

template <class KeyT, class DataT>
unsigned
SArray<KeyT,DataT>::numEntries() const
{
    if (body == 0) {
	return 0;
    } else if (Map_noKeyP(BODY(body)->data[0].key)) {
	return 0;
    } else {
	/*
	 * Do a binary search for the end of the used memory
	 * lower always points to a filled entry
	 * upper always points to a free entry beyond the end of used entries
	 */
	unsigned lower, upper;

	lower = 0;			/* minimum index (inclusive) */
	upper = BODY(body)->maxEntries;	/* maximum index (exclusive) */

	while (lower + 1 < upper) {
	    unsigned middle = (lower + upper)/2;

	    if (Map_noKeyP(BODY(body)->data[middle].key)) {
		    upper = middle;
	    } else {
		    lower = middle;
	    }
	}

	/* lower + 1 == upper */
	return upper;
    }
}

template <class KeyT, class DataT>
SArray<KeyT,DataT>::SArray(const SArray<KeyT,DataT> &source)
    : body(0)
{
#ifdef DEBUG
    cerr << "warning: SArray copy constructor called\n";
#endif
    *this = source;
}

template <class KeyT, class DataT>
SArray<KeyT,DataT> &
SArray<KeyT,DataT>::operator= (const SArray<KeyT,DataT> &other)
{
#ifdef DEBUG
    cerr << "warning: SArray::operator= called\n";
#endif

    if (&other == this) {
	return *this;
    }

    /*
     * copy array entries from old to new 
     */
    if (other.body) {
	unsigned maxEntries = BODY(other.body)->maxEntries;
	clear(maxEntries);

	for (unsigned i = 0; i < maxEntries; i++) {
	    KeyT thisKey = BODY(other.body)->data[i].key;

	    if (Map_noKeyP(thisKey)) {
		break;
	    }

	    /*
	     * Copy key
	     */
	    BODY(body)->data[i].key = Map_copyKey(thisKey);

	    /*
	     * Initialize data, required for = operator
	     */
	    new (&(BODY(body)->data[i].value)) DataT;

	    /*
	     * Copy data
	     */
	    BODY(body)->data[i].value = BODY(other.body)->data[i].value;

	}
    } else {
	clear(0);
    }

    return *this;
}

/*
 * Returns index into data array that has the key which is either
 * equal to key, or indicates the place where key would be placed if it
 * occurred in the array.
 */
template <class KeyT, class DataT>
Boolean
SArray<KeyT,DataT>::locate(KeyT key, unsigned &index) const
{
    assert(!Map_noKeyP(key));

    if (body) {
	unsigned lower, upper;

	lower = 0;			/* minimum index (inclusive) */
	upper = BODY(body)->maxEntries;	/* maximum index (exclusive) */

	while (lower < upper) {
	    unsigned middle = (lower + upper)/2;

	    KeyT thisKey = BODY(body)->data[middle].key;

	    if (Map_noKeyP(thisKey)) {
		upper = middle;
		continue;
	    }
	    
	    int compare = SArray_compareKey(key, thisKey);

	    if (compare < 0) {
		    upper = middle;
	    } else if (compare > 0) {
		    lower = middle + 1;
	    } else {
		    index = middle;
		    return true;
	    }
	}

	/* we have lower == upper */
	if (lower == BODY(body)->maxEntries ||
	    Map_noKeyP(BODY(body)->data[lower].key) ||
	    SArray_compareKey(key, BODY(body)->data[lower].key) < 0)
	{
	    index = lower;
	} else  {
	    index = lower + 1;
	}
	return false;
    } else {
	return false;
    }
}

template <class KeyT, class DataT>
DataT *
SArray<KeyT,DataT>::find(KeyT key, Boolean &foundP) const
{
    unsigned index;

    if (foundP = locate(key, index)) {
	return &(BODY(body)->data[index].value);
    } else {
	return 0;
    }
}

template <class KeyT, class DataT>
KeyT
SArray<KeyT,DataT>::getInternalKey(KeyT key, Boolean &foundP) const
{
    unsigned index;
    static KeyT zeroKey;

    if (foundP = locate(key, index)) {
	return BODY(body)->data[index].key;
    } else {
	return zeroKey;
    }
}

template <class KeyT, class DataT>
DataT *
SArray<KeyT,DataT>::insert(KeyT key, Boolean &foundP)
{
    unsigned index;

    /*
     * Make sure there is room for at least one entry
     */
    if (body == 0) {
	alloc(1);
    }

    if (foundP = locate(key, index)) {
	return &(BODY(body)->data[index].value);
    } else {
	unsigned nEntries = numEntries();
	unsigned maxEntries = BODY(body)->maxEntries;

	/*
	 * Need to add an element.
	 * First, enlarge array if necessary
	 */
	if (nEntries == maxEntries) {
	    maxEntries = (int)(maxEntries * growSize);
	    if (maxEntries == nEntries) {
		maxEntries ++;
	    }
	    body = (SArrayBody<KeyT,DataT> *)realloc(body,
			sizeof(*BODY(body)) +
			   (maxEntries - 1) * sizeof(BODY(body)->data[0]));
	    assert(body != 0);

	    /*
	     * Fill new space with dummy keys
	     */
	    for (unsigned i = BODY(body)->maxEntries; i < maxEntries; i ++) {
		Map_noKey(BODY(body)->data[i].key);
	    }

	    BODY(body)->maxEntries = maxEntries;
	}

	memmove(&(BODY(body)->data[index + 1]),
		&(BODY(body)->data[index]),
		(nEntries - index) * sizeof(BODY(body)->data[0]));

	BODY(body)->data[index].key = Map_copyKey(key);

	/*
	 * Initialize data to zero, but also call constructors, if any
	 */
	memset(&(BODY(body)->data[index].value), 0,
		   sizeof(BODY(body)->data[index].value));
	new (&(BODY(body)->data[index].value)) DataT;

	return &(BODY(body)->data[index].value);
    }
}
  
template <class KeyT, class DataT>
DataT *
SArray<KeyT,DataT>::remove(KeyT key, Boolean &foundP)
{
    unsigned index;

    /*
     * Allocate pseudo-static memory for removed objects (returned by
     * remove()). We cannot make this static because otherwise 
     * the destructor for DataT would be called on it at program exit.
     */
    if (removedData == 0) {
	removedData = (DataT *)malloc(sizeof(DataT));
	assert(removedData != 0);
    }

    if (foundP = locate(key, index)) {
	unsigned nEntries = numEntries();

	Map_freeKey(BODY(body)->data[index].key);
	memcpy(removedData, &BODY(body)->data[index].value, sizeof(DataT));

	memmove(&(BODY(body)->data[index]),
		&(BODY(body)->data[index + 1]),
		(nEntries - index - 1) * sizeof(BODY(body)->data[0]));

	/*
	 * mark previous last slot as free
	 */
	Map_noKey(BODY(body)->data[nEntries-1].key);

	/*
	 * Indicate that deletion occurred
	 */
	BODY(body)->deleted = 1;

	return removedData;
    } else {
	return 0;
    }
}
  
#ifdef __GNUG__
static
#endif
int (*SArray_thisKeyCompare)();		/* used by SArrayIter() to communicate
					 * with compareIndex() */
#ifdef __GNUG__
static
#endif
void *SArray_thisBody;			/* ditto */


template <class KeyT, class DataT>
void
SArrayIter<KeyT,DataT>::sortKeys()
{
    /*
     * Store keys away and sort them to the user's orders.
     */
    unsigned *sortedIndex = new unsigned[numEntries];
    assert(sortedIndex != 0);

    unsigned i;
    for (i = 0; i < numEntries; i++) {
	sortedIndex[i] = i;
    }

    /*
     * Due to the limitations of the qsort interface we have to 
     * pass extra information to compareIndex in these global
     * variables - yuck. 
     */
    SArray_thisKeyCompare = (int (*)())sortFunction;
    SArray_thisBody = mySArrayBody;
    qsort(sortedIndex, numEntries, sizeof(*sortedIndex), compareIndex);

    /*
     * Save the keys for enumeration.  The reason we save the keys,
     * not the indices, is that indices may change during enumeration
     * due to deletions.
     */
    sortedKeys = new KeyT[numEntries];
    assert(sortedKeys != 0);

    for (i = 0; i < numEntries; i++) {
	sortedKeys[i] = mySArrayBody->data[sortedIndex[i]].key;
    }

    delete [] sortedIndex;
}

template <class KeyT, class DataT>
SArrayIter<KeyT,DataT>::SArrayIter(const SArray<KeyT,DataT> &sarray,
					int (*keyCompare)(KeyT, KeyT))
    : mySArrayBody(BODY(sarray.body)), current(0),
      numEntries(sarray.numEntries()), sortFunction(keyCompare)
{
    /*
     * Note: we access the underlying SArray through the body pointer,
     * not the top-level SArray itself.  This allows the top-level object
     * to be moved without affecting an ongoing iteration.
     * XXX: This only works because
     * - it is illegal to insert while iterating
     * - deletions don't cause reallocation of the data
     */
    if (sortFunction && mySArrayBody) {
	sortKeys();
    } else {
	sortedKeys = 0;
    }

    if (mySArrayBody) {
	mySArrayBody->deleted = 0;
    }
}

/*
 * This is the callback function passed to qsort for comparing array
 * entries. It is passed the indices into the data array, which are
 * compared according to the user-supplied function applied to the 
 * keys found at those locations.
 */
template <class KeyT, class DataT>
int
SArrayIter<KeyT,DataT>::compareIndex(const void *idx1, const void *idx2)
{
  return (*(compFnType)SArray_thisKeyCompare)
		(BODY(SArray_thisBody)->data[*(unsigned *)idx1].key,
		 BODY(SArray_thisBody)->data[*(unsigned *)idx2].key);
}

template <class KeyT, class DataT>
SArrayIter<KeyT,DataT>::~SArrayIter()
{
    delete [] sortedKeys;
}

template <class KeyT, class DataT>
void
SArrayIter<KeyT,DataT>::init()
{
    delete [] sortedKeys;
    sortedKeys = 0;

    current = 0;

    {
	/*
	 * XXX: fake SArray object to access numEntries()
	 */
	SArray<KeyT,DataT> mySArray(0);

	mySArray.body = mySArrayBody;
	numEntries = mySArray.numEntries();
	mySArray.body = 0;
    }

    if (mySArrayBody) {
	if (sortFunction) {
	    sortKeys();
	}
	mySArrayBody->deleted = 0;
    }
}

template <class KeyT, class DataT>
DataT *
SArrayIter<KeyT,DataT>::next(KeyT &key)
{
    if (mySArrayBody == 0) {
	return 0;
    } else {
	unsigned index;

	if (sortedKeys == 0) {
	    /*
	     * Detect deletion while iterating.
	     * A legal deletion can only affect the current entry, so
	     * adjust the current position to reflect the next entry was
	     * moved.
	     */
	    if (mySArrayBody->deleted) {
		numEntries --;
		current --;
	    }

	    if (current == numEntries) {
		return 0;
	    }

	    index = current++;
	} else {
	    if (current == numEntries) {
		return 0;
	    }

	    /*
	     * XXX: fake an SArray object to access locate()
	     */
	    SArray<KeyT,DataT> mySArray(0);

	    mySArray.body = mySArrayBody;
	    (void)mySArray.locate(sortedKeys[current++], index);
	    mySArray.body = 0;
	}
	mySArrayBody->deleted = 0;

	key = mySArrayBody->data[index].key;
	return &(mySArrayBody->data[index].value);
    }
}

#undef BODY

#endif /* _SArray_cc_ */
