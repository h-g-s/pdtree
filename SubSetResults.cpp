/*
 * SubSetResults.cpp
 *
 *  Created on: 9 de mar de 2019
 *      Author: haroldo
 */

#include "SubSetResults.hpp"

#include <stdlib.h>
#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>
#include <vector>

#include "Parameters.hpp"

using namespace std;

SubSetResults::SubSetResults( const SubSetResults &other ) :
    nElSS(other.nElSS),
    idxBestAlg_(other.idxBestAlg_),
    resBestAlg_(other.resBestAlg_),
    rset_(other.rset_),
    eval_(other.eval_),
    sum_(nullptr)
{
    size_t nAlgs = rset_->algsettings().size();

    sum_ = (SumType *) malloc( sizeof(SumType)*nAlgs );
    assert( sum_ );
    for ( size_t i=0 ; (i<nAlgs) ; ++i )
        sum_[i] = other.sum_[i];
}


SubSetResults::SubSetResults ( const ResultsSet *_rset, const Evaluation _eval, bool addElements ) :
    nElSS(0),
    idxBestAlg_(numeric_limits<size_t>::max()),
    resBestAlg_(numeric_limits<double>::max()),
    rset_(_rset),
    eval_(_eval),
    sum_(nullptr)
{
    size_t nAlgs = rset_->algsettings().size();

    sum_ = (SumType *) malloc( sizeof(SumType)*nAlgs );
    assert( sum_ );
    for ( size_t i=0 ; (i<nAlgs) ; ++i )
        sum_[i] = 0.0;

    if (addElements)
    {
        vector< size_t > el( rset_->instances().size() );
        for ( size_t i=0 ; (i<rset_->instances().size()) ; ++i )
            el[i] = i;

        this->add( el.size(), &el[0] );
    }
}

void SubSetResults::add( size_t n, const size_t *el )
{
    nElSS += n;
    const size_t nAlgs = rset_->algsettings().size();
    switch (eval_)
    {
        case Average:
            for ( size_t ia=0 ; (ia<nAlgs) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] += (SumType)rset_->get( *e, ia );
            }
            break;
        case Rank:
            for ( size_t ia=0 ; (ia<nAlgs) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] += (SumType)rset_->rank( *e, ia );
            }
            break;
    }

    updateBest();
}

void SubSetResults::remove( size_t n, const size_t *el )
{
    assert( n <= nElSS );
    nElSS -= n;
    const size_t nAlgs = rset_->algsettings().size();

    switch (eval_)
    {
        case Average:
            for ( size_t ia=0 ; (ia<nAlgs ) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] -= (SumType)rset_->get( *e, ia );
            }
            break;
        case Rank:
            for ( size_t ia=0 ; (ia<nAlgs ) ; ++ia )
            {
                const size_t *endEl = el + n;
                const size_t *e = el;
                for ( ; (e<endEl) ; ++e )
                    sum_[ia] -= (SumType)rset_->rank( *e, ia );
            }
            break;
    }
    updateBest();
}

void SubSetResults::updateBest()
{
    idxBestAlg_ = numeric_limits<size_t>::max();
    resBestAlg_ = numeric_limits<double>::max();

    const size_t nAlg = rset_->algsettings().size();
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

double SubSetResults::resAlg( size_t idxAlg ) const
{
    return (double) ((sum_[idxAlg] / (SumType) this->nElSS)
            + (eval_==Rank ? 1.0 : 0.0) );
}

std::vector< size_t > SubSetResults::computeBestAlgorithms() const
{
    std::vector< size_t > res;
    res.reserve(rset_->algsettings().size());

    std::vector< pair< double, size_t > > sres;
    sres.reserve(rset_->algsettings().size());
    for ( size_t i=0 ; (i<rset_->algsettings().size()) ; ++i )
        sres.push_back( make_pair( resAlg(i) , i ) );

    sort( sres.begin(), sres.end() );
    for ( const auto &it : sres )
        res.push_back(it.second);

    return res;
}

SubSetResults::~SubSetResults ()
{
    free( sum_ );
}
