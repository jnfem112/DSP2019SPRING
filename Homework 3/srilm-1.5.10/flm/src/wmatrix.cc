/*
 * wmatrix.cc
 *
 * Jeff Bilmes <bilmes@ee.washington.edu>
 */

#ifndef lint
static char Copyright[] = "Copyright (c) 2003-2006 SRI International.  All Rights Reserved.";
static char RcsId[] = "@(#)$Header: /home/srilm/devel/flm/src/RCS/wmatrix.cc,v 1.8 2006/08/12 06:26:03 stolcke Exp $";
#endif

#ifdef PRE_ISO_CXX
# include <iostream.h>
#else
# include <iostream>
using namespace std;
#endif
#include <string.h>
#include <ctype.h>

#include "FNgramSpecs.h"

#include "Trie.cc"
#include "Array.cc"
#include "FactoredVocab.h"
#include "Debug.h"
#include "FDiscount.h"
#include "hexdec.h"


WidMatrix::WidMatrix()
{
  for (int i=0;i<maxNumFactors;i++) {
    wid_factors[i] = new VocabIndex[maxNumParentsPerChild+1];
  }
}

WidMatrix::~WidMatrix()
{
  for (int i=0;i<maxNumFactors;i++) {
    delete [] wid_factors[i];
  }
}

WordMatrix::WordMatrix()
{
  for (int i=0;i<maxNumFactors;i++) {
    word_factors[i] = new VocabString[maxNumParentsPerChild+1];
  }
}

WordMatrix::~WordMatrix()
{
  for (int i=0;i<maxNumFactors;i++) {
    delete [] word_factors[i];
  }
}

void
WordMatrix::print(FILE* f)
{
  for (int i=0;word_factors[i][FNGRAM_WORD_TAG_POS] != 0;i++) {
    fprintf(f,"%d ",i);
    for (int j=0;word_factors[i][j] != 0;j++) {
      fprintf(f,"%s ",word_factors[i][j]);
    }
    fprintf(f,"\n");
  }
}

