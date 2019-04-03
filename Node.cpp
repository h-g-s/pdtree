/*
 * Node.cpp
 *
 *  Created on: 7 de mar de 2019
 *      Author: haroldo
 */

#include "Node.hpp"
#include "Dataset.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <limits>
#include <cfloat>

#include "Dataset.hpp"
#include "FeatureBranching.hpp"
#include "Parameters.hpp"
#include "ResultsSet.hpp"
#include "tinyxml2.h"
#include "InstanceSet.hpp"


using namespace std;

Node::Node( const InstanceSet *_iset, const ResultsSet *_rset ) :
    iset_(_iset),
    rset_(_rset),
    nEl_(iset_->size()),
    el_(new size_t[iset_->size()]),
    parent_(nullptr),
    idxFBranch(numeric_limits<size_t>::max()),
    branchValue_(0.0),
    depth_(0),
    idx_(0),
    idxBestAlg(numeric_limits<size_t>::max()),
    nodeCost_(DBL_MAX),
    avRank(DBL_MAX)
{
    child_[0] = child_[1] = nullptr; 
    for (size_t i=0 ; (i<nEl_) ; ++i )
        el_[i] = i;

    strcpy( id_, "root" );

    computeResultsNode();
}

Node::Node( const Node *_parent, size_t _nEl, const size_t *_el, size_t _idx ) :
    iset_(_parent->iset_),
    rset_(_parent->rset_),
    nEl_(_nEl),
    el_(new size_t[_nEl]),
    parent_(_parent),
    idxFBranch(numeric_limits<size_t>::max()),
    branchValue_(0.0),
    depth_(_parent->depth_+1),
    idx_(_idx),
    idxBestAlg(numeric_limits<size_t>::max()),
    nodeCost_(DBL_MAX),
    avRank(DBL_MAX)
{
    child_[0] = child_[1] = nullptr;
    memcpy(el_, _el, sizeof(size_t)*nEl_ );
    sprintf( this->id_, "nL%zuI%zu", depth_, idx_ );

    computeResultsNode();
}

using namespace tinyxml2;

static void addElement( tinyxml2::XMLDocument *doc,  XMLElement *el, const char *name, const double value )
{
    auto el2 = doc->NewElement(name);
    el->InsertEndChild(el2);
    el2->SetText(to_string(value).c_str());
}

static void addElement( tinyxml2::XMLDocument *doc,  XMLElement *el, const char *name, const int value )
{
    auto el2 = doc->NewElement(name);
    el->InsertEndChild(el2);
    el2->SetText(to_string(value).c_str());
}

static void addElement( tinyxml2::XMLDocument *doc,  XMLElement *el, const char *name, const char *value )
{
    auto el2 = doc->NewElement(name);
    el->InsertEndChild(el2);
    el2->SetText(value);
}

void Node::writeXML(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *parent ) const
{
    XMLElement *node = doc->NewElement("node");
    assert( node!=NULL);
    parent->InsertEndChild(node);

    node->SetAttribute("id", this->id_ );

    addElement( doc, node, "depth", (int) this->depth_);
    addElement( doc, node, "bestAlg", rset_->algsettings()[idxBestAlg].c_str() );
    addElement( doc, node, "bestAlgCost", this->nodeCost_ );

    XMLElement *insts = doc->NewElement("instances");
    node->InsertEndChild(insts);
    for ( size_t i=0 ; (i<n_elements()) ; ++i )
    {
        auto inst = iset_->instance(elements()[i]);
        auto instEl = doc->NewElement("instance");
        instEl->SetAttribute("name", inst.name() );
        insts->InsertEndChild(instEl);
    }

    if (idxFBranch!=numeric_limits<size_t>::max())
    {
        auto elb = doc->NewElement("branching");
        elb->SetAttribute( "feature", iset_->features()[idxFBranch].c_str() );
        elb->SetAttribute( "value", branchValue_ );
        node->InsertEndChild(elb);
    }

    node->SetAttribute("id", this->id_ );

    if (child_[0]!=nullptr)
    {
        assert( child_[1]!=nullptr );
        child_[0]->writeXML(doc, node);
        child_[1]->writeXML(doc, node);
    }
}

void Node::branchOn( const size_t idxF, const double normValue )
{
    assert( idxF<iset_->features().size());
    assert( normValue >= 0.0-1e-10 && normValue <= 1.0+1e+10 );

    double bestDiff = DBL_MAX;
    double bv = DBL_MAX;
    for ( size_t i=0 ; (i<nEl_) ; ++i )
    {
        size_t idxInst = el_[i];
        const double v = iset_->norm_feature_val(idxInst, idxF);
        const double diff = abs(v-normValue);
        if (diff<bestDiff)
        {
            bestDiff = diff;
            const Instance &inst = iset_->instance(idxInst);
            bv = inst.float_feature(idxF);
        } // all diferences
    } // all elements

    assert( bestDiff <= 1e-5 );
    
    this->idxFBranch = idxF;
    this->branchValue_ = bv;

    vector< vector<size_t> > elb = vector< vector<size_t> >(2);

    for ( size_t i=0 ; (i<nEl_) ; i++ )
    {
        const Instance &inst = iset_->instance(el_[i]);
        if (inst.float_feature(idxF)<=bv)
            elb[0].push_back(el_[i]);
        else
            elb[1].push_back(el_[i]);
    }

    assert( elb[0].size() >= Parameters::minElementsBranch );
    assert( elb[1].size() >= Parameters::minElementsBranch );

    child_[0] = new Node( (const Node *)this, elb[0].size(), &elb[0][0], this->idx_*2 );
    child_[1] = new Node( (const Node *)this, elb[1].size(), &elb[1][0], this->idx_*2+1 );
}

void Node::computeResultsNode()
{
    if (nEl_<Parameters::minElementsBranch)
    {
        fprintf( stderr, "Node has only %zu elements, less than the minimum %zu.\n", nEl_, Parameters::minElementsBranch );
        abort();
    }

    const size_t nAlgs = rset_->algsettings().size();
    long double *sumAlg = new long double[nAlgs];
    for (size_t i=0 ; (i<nAlgs) ; ++i )
        sumAlg[i] = 0.0;
    
    for ( size_t ie=0 ; (ie<nEl_) ; ++ie )
        for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
            sumAlg[idxAlg] += rset_->res(el_[ie], idxAlg);

    idxBestAlg = numeric_limits<size_t>::max();
    nodeCost_ = DBL_MAX;
    for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
    {
        sumAlg[idxAlg] = sumAlg[idxAlg] / ((long double)nEl_);
        if (sumAlg[idxAlg]<nodeCost_)
        {
            nodeCost_ = sumAlg[idxAlg];
            idxBestAlg = idxAlg;
        }
    }
    
    if (Parameters::eval == Rank)
        avRank = nodeCost_;
    else
    {
        for (size_t i=0 ; (i<nAlgs) ; ++i )
            sumAlg[i] = 0.0;

        for ( size_t ie=0 ; (ie<nEl_) ; ++ie )
            for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
                sumAlg[idxAlg] += rset_->rank(el_[ie], idxAlg);

        avRank = DBL_MAX;
        for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
        {
            sumAlg[idxAlg] = sumAlg[idxAlg] / ((long double)nEl_);
            avRank = min( (long double)avRank, sumAlg[idxAlg]);
        }
    }
    
    delete[] sumAlg;
}

void Node::branchOnVal( const size_t idxF, const double val )
{
    assert( idxF<iset_->features().size());

    this->branchValue_ = val;

    vector< vector<size_t> > elb = vector< vector<size_t> >(2);

    for ( size_t i=0 ; (i<nEl_) ; i++ )
    {
        const Instance &inst = iset_->instance(el_[i]);
        if (inst.float_feature(idxF)<=val)
            elb[0].push_back(el_[i]);
        else
            elb[1].push_back(el_[i]);
    }

    assert( elb[0].size() >= Parameters::minElementsBranch );
    assert( elb[1].size() >= Parameters::minElementsBranch );

    child_[0] = new Node( (const Node *)this, elb[0].size(), &elb[0][0], this->idx_*2 );
    child_[1] = new Node( (const Node *)this, elb[1].size(), &elb[1][0], this->idx_*2+1 );
}

Node::~Node ()
{
    delete[] el_;
    if (child_[0])
    {
        delete child_[0];
        assert(child_[1]);
        delete child_[1];
    }
}

