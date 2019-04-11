/*
 * Greedy.cpp
 *
 *  Created on: 2 de abr de 2019
 *      Author: haroldo
 */

#include <vector>
#include <cassert>
#include <set>
#include <cfloat>
#include <algorithm>
#include <utility>
#include <cmath>
#include <cstring>

#include "Greedy.hpp"
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"
#include "Tree.hpp"
#include "Parameters.hpp"
#include "SubSetResults.hpp"
#include "Node.hpp"

using namespace std;

typedef struct 
{
    size_t el;
    double val;
} ElVal;

class SplitInfo
{
public:
    SplitInfo( size_t nAlgs, size_t nInsts ) :
        sumRL( new long double[nAlgs] ),
        sumRR( new long double[nAlgs] ),
        elv(new ElVal[nInsts]),
        splitCost(DBL_MAX),
        idxFeature(numeric_limits<size_t>::max()),
        nElLeft(0)
    {}

    virtual ~SplitInfo() {
        delete[] sumRL;
        delete[] sumRR;
    }

    long double *sumRL;
    long double *sumRR;
    ElVal *elv;
    long double splitCost;
    size_t idxFeature;
    int nElLeft;
};

class GNodeData {
public:
    GNodeData( const InstanceSet *_iset, const ResultsSet *_rset ) :
        iset_(_iset),
        rset_(_rset),
        idx(0),
        sumResR( new long double[rset_->algsettings().size()] ),
        sumResL( new long double[rset_->algsettings().size()] ),
        elv( new ElVal[iset_->size()] ),
        nEl(iset_->size()),
        nElLeft(0),
        splitCost(DBL_MAX),
        idxFeature(numeric_limits<size_t>::max()),
        bestSplit(SplitInfo(rset_->algsettings().size(), iset_->size()))
    { 
        for ( auto i=0 ; (i<iset_->size()) ; ++i )
            elv[i].el = i;
        for ( size_t i=0 ; (i<rset_->algsettings().size()) ; ++i )
            sumResR[i] = rset_->results().sum()[i];
        for ( size_t i=0 ; (i<rset_->algsettings().size()) ; ++i )
            sumResL[i] = 0.0;
    }

    virtual ~GNodeData() {
        delete[] elv;
        delete[] sumResR;
        delete[] sumResL;
    }

    void moveInstanceLeft( size_t idxInst )
    {
        for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
        {
            sumResR[ia] -= ((long double)rset_->res(idxInst, ia));
            sumResL[ia] += ((long double)rset_->res(idxInst, ia));

#ifdef DEBUG
            assert( sumResR[ia] >= -1e-5 );
            assert( sumResL[ia] >= -1e-5 );
#endif
        }
    }

    void updateBestAlg() {
        assert( nElLeft >= Parameters::minElementsBranch && (nEl-nElLeft)>=Parameters::minElementsBranch );
        long double costBestAlgL = DBL_MAX;

        for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
            if (sumResL[ia]<costBestAlgL)
                costBestAlgL = sumResL[ia];

        long double costBestAlgR = DBL_MAX;

        for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
            if (sumResR[ia]<costBestAlgR)
                costBestAlgR = sumResR[ia];

        splitCost = costBestAlgL + costBestAlgR;

        if (splitCost<bestSplit.splitCost) {
            bestSplit.splitCost = splitCost;
            bestSplit.nElLeft = this->nElLeft;
            memcpy( bestSplit.sumRL, this->sumResL, sizeof(long double)*rset_->algsettings().size() );
            memcpy( bestSplit.sumRR, this->sumResR, sizeof(long double)*rset_->algsettings().size() );
            bestSplit.idxFeature = idxFeature;
            memcpy( bestSplit.elv, this->elv, sizeof(ElVal)*this->nEl );
        }
    }

    double cutValue() const {
        // is in a valid branching position
        assert( nElLeft >=1 && nElLeft<nEl );

        return elv[((int)nElLeft)-1].val;
    }

    bool next() {
        assert(nElLeft>=0 && nElLeft<nEl);
        moveInstanceLeft( elv[nElLeft].el );

goNext:
        ++nElLeft;
        if ( ((int)nElLeft)>((int)nEl)-((int)Parameters::minElementsBranch) )
            return false;

        const double diff = elv[nElLeft].val - elv[nElLeft-1].val;
        assert( diff>=0.0 );

        if (diff>=1e-10 and nElLeft>=Parameters::minElementsBranch)
                return true;
        else
        {
            moveInstanceLeft( elv[nElLeft].el );
            goto goNext;
        }

        return false;
    }

    const InstanceSet *iset_;
    const ResultsSet *rset_;
    
    size_t idx;
    long double *sumResR;
    long double *sumResL;
    ElVal *elv;
    int nEl;

    int nElLeft;

    long double splitCost;

    // feature being branched on
    size_t idxFeature;

    SplitInfo bestSplit;
};

Greedy::Greedy (const InstanceSet *_iset, const ResultsSet *_rset) :
    iset_(_iset),
    rset_(_rset),
    ndata(nullptr),
    tnodes(0),
    maxDepth(Parameters::maxDepth)
{
    for ( size_t i=0 ; (i<maxDepth) ; ++i )
        tnodes += (size_t)pow( 2.0, i)+1e-10;

    ndata = new GNodeData*[tnodes];
    for ( size_t i=0 ; (i<tnodes) ; ++i ) 
    {
        ndata[i] = new GNodeData(iset_, rset_);
        ndata[i]->idx = 0;
    }
}

Tree *Greedy::build()
{
    vector< pair< size_t, Node *> > nqueue;

    Tree *res = new Tree(iset_, rset_);

    Node *root = res->create_root();

    nqueue.push_back( make_pair((size_t)0, root) );

    while (nqueue.size())
    {
        pair< size_t, Node *> np = nqueue.back();
        Node *node = np.second;
        nqueue.pop_back();

        size_t idxLeft = 2*np.first+1;
        size_t idxRight = 2*np.first+2;

        // if children will be will within max depth
        if (idxLeft >= tnodes) 
            break;

        GNodeData *gnd = ndata[np.first];

        gnd->bestSplit.splitCost = DBL_MAX;
        gnd->bestSplit.idxFeature = numeric_limits<size_t>::max();
        for ( size_t idxFeature=0 ; (idxFeature<iset_->features().size()) ; ++idxFeature )
        {
            prepareBranch( np.first, idxFeature );
            while (gnd->next())
                gnd->updateBestAlg();
        }
        
        // found a valid branch
        if (gnd->bestSplit.idxFeature != numeric_limits<size_t>::max())
        {
            // recover to best state
            gnd->nElLeft = gnd->bestSplit.nElLeft;
            memcpy( gnd->sumResL, gnd->bestSplit.sumRL, sizeof(long double)*rset_->algsettings().size() );
            memcpy( gnd->sumResR, gnd->bestSplit.sumRR, sizeof(long double)*rset_->algsettings().size() );
            gnd->splitCost = gnd->bestSplit.splitCost;
            memcpy(gnd->elv , gnd->bestSplit.elv, sizeof(ElVal)*gnd->nEl );
            gnd->idxFeature = gnd->bestSplit.idxFeature;
            // update child node elements
            auto *gndLeft = ndata[idxLeft];
            gndLeft->nEl = gnd->nElLeft;
            for ( int ie=0 ; (ie<gnd->nElLeft) ; ++ie )
                gndLeft->elv[ie].el = gnd->elv[ie].el;
            auto *gndRight = ndata[idxRight];
            gndRight->nEl = gnd->nEl-gnd->nElLeft;
            size_t ii = 0;
            for ( int ie=gnd->nElLeft ; (ie<gnd->nEl) ; ++ie,++ii )
                gndRight->elv[ii].el = gnd->elv[ie].el;

            node->branchOnVal( gnd->idxFeature, gnd->cutValue() );

            nqueue.push_back( make_pair( (size_t) idxLeft, node->child()[0]) );
            nqueue.push_back( make_pair( (size_t) idxRight, node->child()[1]) );

            res->addNode(node->child()[0]);
            res->addNode(node->child()[1]);
        }
    }
    
    res->computeCost();

    return res;
}

bool compElVal(const ElVal &lhs, const ElVal &rhs) { return lhs.val < rhs.val; }

void Greedy::prepareBranch( size_t n, size_t f )
{
    GNodeData *gnd = ndata[n];

    gnd->nElLeft = 0;
    gnd->idxFeature = f;

    if (gnd->idx==0)
    {
        // root node
        for ( size_t i=0 ; (i<rset_->algsettings().size()) ; ++i )
            gnd->sumResR[i] = rset_->results().sum()[i];
        for ( size_t i=0 ; (i<rset_->algsettings().size()) ; ++i )
            gnd->sumResL[i] = 0.0;

    }
    else
    {
        size_t parent = (((int)n)-1)/2;
        bool isLeft = (n%2);
        const long double *srp = (isLeft) ? ndata[parent]->sumResL : ndata[parent]->sumResR;
        memcpy( gnd->sumResR, srp, sizeof(long double)*rset_->algsettings().size() );
        for ( size_t i=0 ; (i<rset_->algsettings().size() ) ; ++i )
            gnd->sumResL[i] = 0.0;
    }

    for ( int i=0 ; (i<gnd->nEl) ; ++i )
        gnd->elv[i].val = iset_->instance(gnd->elv[i].el).float_feature(f);
    
    std::sort( gnd->elv, gnd->elv+gnd->nEl, compElVal );

    gnd->nElLeft = 0;
#ifdef DEBUG
    {
        set< size_t > sEl;
        for ( int i=0 ; (i<gnd->nEl) ; ++i )
            sEl.insert(gnd->elv[i].el);
        assert( (int)sEl.size() == gnd->nEl );
    }
#endif
}

Greedy::~Greedy ()
{
    for ( size_t i=0 ; (i<tnodes) ; ++i )
        delete ndata[i];
    delete[] ndata;
}
