/*
 * Node.cpp
 *
 *  Created on: 7 de mar de 2019
 *      Author: haroldo
 */

#include "Node.hpp"

#include <cstdlib>
#include <iostream>

#include "FeatureBranching.hpp"

using namespace std;

Node::Node( const InstanceSet &_iset, const ResultsSet &_rset ) :
    iset_(_iset),
    rset_(_rset),
    nEl_(iset_.size()),
    el_(new size_t[iset_.size()]),
    parent_(nullptr)
{
    for (size_t i=0 ; (i<nEl_) ; ++i )
        el_[i] = i;
}

Node::Node( const Node *_parent, size_t _nEl, const size_t *_el ) :
    iset_(_parent->iset_),
    rset_(_parent->rset_),
    nEl_(_nEl),
    el_(new size_t[_nEl]),
    parent_(_parent)
{

}

std::vector<Node> &Node::perform_branch()
{
    if (children_.size())
    {
        cerr << "branch was already done" << endl;
        abort();
    }

    const size_t nFeatures = iset_.size();
    for ( size_t idxF=0 ; (idxF<nFeatures) ; ++idxF )
    {
        if (iset_.feature_is_integer(idxF))
        {
            FeatureBranching<int> fbi(iset_, rset_, idxF, el_, nEl_, 5, 11);
            if (fbi.branch_values().size())
            {
                do
                {
                    bestBranch_.update_best_branching(fbi);
                } while (fbi.next());
            }
        }
        else
        {
            if (iset_.types()[idxF]==Float)
            {
                FeatureBranching<double> fbf(iset_, rset_, idxF, el_, nEl_, 5, 11);
                if (fbf.branch_values().size())
                {
                    do
                    {
                        bestBranch_.update_best_branching(fbf);
                    } while (fbf.next());
                }
            }
            else
            {
                cerr << "type not handled yet." << endl;
                abort();
            }
        }
    }

    if (bestBranch_.found())
    {
        children_.push_back( Node(this, bestBranch_.branches()[0].size(), &(bestBranch_.branches()[0][0])) );
        children_.push_back( Node(this, bestBranch_.branches()[1].size(), &(bestBranch_.branches()[1][0])) );
    }

    return children_;
}

Node::~Node ()
{
    delete[] el_;
}

