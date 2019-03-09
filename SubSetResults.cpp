/*
 * SubSetResults.cpp
 *
 *  Created on: 9 de mar de 2019
 *      Author: haroldo
 */

#include "SubSetResults.hpp"

#include <stddef.h>
#include <stdlib.h>
#include <cassert>
#include <vector>
#include <limits>

using namespace std;

SubSetResults::SubSetResults ( const ResultsSet &_rset, const Evaluation _eval ) :
    nElSS(0),
    idxBestAlg_(numeric_limits<size_t>::max()),
    resBestAlg_(numeric_limits<double>::max()),
    rset_(_rset),
    eval_(_eval),
    sum_(nullptr)
{
    size_t nAlgs = rset_.algsettings().size();

    sum_ = (SumType *) malloc( sizeof(SumType)*nAlgs );
    assert( sum_ );
    for ( size_t i=0 ; (i<nAlgs) ; ++i )
        sum_[i] = 0.0;
}

void SubSetResults::add( size_t n, const size_t *el )
{
    nElSS += n;
    const size_t nAlgs = rset_.algsettings().size();
    switch (eval_)
    {
        case Average:
            for ( size_t ia=0 ; (ia<nAlgs) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] += (SumType)rset_.get( *e, ia );
            }
            break;
        case Rank:
            for ( size_t ia=0 ; (ia<nAlgs) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] += (SumType)rset_.rank( *e, ia );
            }
            break;
    }

    updateBest();
}

void SubSetResults::remove( size_t n, const size_t *el )
{
    assert( n <= nElSS );
    nElSS -= n;
    const size_t nAlgs = rset_.algsettings().size();

    switch (eval_)
    {
        case Average:
            for ( size_t ia=0 ; (ia<nAlgs ) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] -= (SumType)rset_.get( *e, ia );
            }
            break;
        case Rank:
            for ( size_t ia=0 ; (ia<nAlgs ) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] -= (SumType)rset_.rank( *e, ia );
            }
            break;
    }
    updateBest();
}

void SubSetResults::updateBest()
{
    idxBestAlg_ = numeric_limits<size_t>::max();
    resBestAlg_ = numeric_limits<double>::max();

    const size_t nAlg = rset_.algsettings().size();
    for ( size_t i=0 ; (i<nAlg) ; ++i )
    {
        if (sum_[i]<resBestAlg_)
        {
            resBestAlg_ = sum_[i];
            idxBestAlg_ = i;
        }
    }

    resBestAlg_ /= (SumType)nElSS;
    if (eval_==Rank)
        resBestAlg_ += 1.0;
}

SubSetResults::~SubSetResults ()
{
    free( sum_ );
}
