/*
 * BayesMix.cc --
 *	Bayesian mixture language model
 *
 */

#ifndef lint
static char Copyright[] = "Copyright (c) 1995-2006 SRI International.  All Rights Reserved.";
static char RcsId[] = "@(#)$Header: /home/srilm/devel/lm/src/RCS/BayesMix.cc,v 1.12 2006/08/12 06:46:11 stolcke Exp $";
#endif

#ifdef PRE_ISO_CXX
# include <iostream.h>
#else
# include <iostream>
using namespace std;
#endif
#include <stdlib.h>
#include <math.h>

#include "BayesMix.h"

/*
 * Debug levels used
 */
#define DEBUG_MIX_WEIGHTS	2

BayesMix::BayesMix(Vocab &vocab, LM &lm1, LM &lm2,
			    unsigned int clength, Prob prior, double llscale)
    : LM(vocab), lm1(lm1), lm2(lm2),
      clength(clength), prior(prior), llscale(llscale)
{
    if (prior < 0.0 || prior > 1.0) {
	cerr << "warning: mixture prior out of range: " << prior << endl;
	prior = 0.5;
    }
}

LogP
BayesMix::wordProb(VocabIndex word, const VocabIndex *context)
{
    Prob lm1Prob = LogPtoProb(lm1.wordProb(word, context));
    Prob lm2Prob = LogPtoProb(lm2.wordProb(word, context));

    Prob lm1Weight = prior *
			LogPtoProb(llscale * lm1.contextProb(context, clength));
    Prob lm2Weight = (1.0 - prior) *
			LogPtoProb(llscale * lm2.contextProb(context, clength));

    /*
     * If both LMs don't know this context revert to the prior
     */
    if (lm1Weight == 0.0 && lm2Weight == 0.0) {
	lm1Weight = prior;
	lm2Weight = 1.0 - prior;
    }

    if (running() && debug(DEBUG_MIX_WEIGHTS)) {
	if (clength > 0) {
	    dout() << "[post=" << (lm1Weight/(lm1Weight + lm2Weight))<< "]";
	}
    }

    return ProbToLogP((lm1Weight * lm1Prob + lm2Weight * lm2Prob) /
						(lm1Weight + lm2Weight));
}

void *
BayesMix::contextID(VocabIndex word, const VocabIndex *context,
							unsigned &length)
{
    /*
     * Return the context ID of the component model that uses the longer
     * context. We must use longest context regardless of predicted word
     * because mixture models don't support contextBOW().
     */
    unsigned len1, len2;

    void *cid1 = lm1.contextID(context, len1);
    void *cid2 = lm2.contextID(context, len2);

    if (len2 > len1) {
	length = len2;
	return cid2;
    } else {
	length = len1;
	return cid1;
    }
}

Boolean
BayesMix::isNonWord(VocabIndex word)
{
    /*
     * A non-word in either of our component models is a non-word.
     * This ensures that state names, hidden vocabulary, etc. are not
     * treated as regular words in the respectively other component.
     */
    return lm1.isNonWord(word) || lm2.isNonWord(word);
}

void
BayesMix::setState(const char *state)
{
    /*
     * Global state changes are propagated to the component models
     */
    lm1.setState(state);
    lm2.setState(state);
}

