/*
 * Node.hpp
 *
 *  Created on: 7 de mar de 2019
 *      Author: haroldo
 */

#ifndef NODE_HPP_
#define NODE_HPP_

#include <stddef.h>

#include "tinyxml2.h"

class InstanceSet;
class ResultsSet;
class XMLDocument;

class Node
{
public:
    Node( const InstanceSet *_iset, const ResultsSet *_rset );

    Node( const Node *_parent, size_t _nEl, const size_t *_el, size_t _idx );
    
    void branchOn( const size_t idxF, double normValue );

    void branchOnVal( const size_t idxF, const double val );

    Node **child() {
        return &(child_[0]);
    }

    const Node *parent() const {
        return parent_;
    }

    // number of instances in node
    size_t n_elements() const {
        return nEl_;
    }

    // instances on node
    const size_t *elements() const {
        return el_;
    }

    size_t bestAlg() const {
        return idxBestAlg;
    }

    size_t idx() const {
        return idx_;
    }

    // cost based on best algorithm
    double nodeCost() const {
        return nodeCost_;
    }
    
    // node depth  
    int depth() const {
        return depth_;
    }

    // branch feature idx or
    // numeric_limits<size_t>::max()
    size_t branchFeature() const {
        return idxFBranch;
    }

    // branch value
    double branchValue() const {
        return branchValue_;
    }

    const char *id() const {
        return &id_[0];
    }

    void writeXML(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *parent ) const;

    virtual ~Node ();
private:
    const InstanceSet *iset_;
    const ResultsSet *rset_;
    
    // instances on this node
    size_t nEl_;
    size_t *el_;
    
    // parent node
    const Node *parent_;

    // child nodes
    Node *child_[2];

    size_t idxFBranch;
    double branchValue_;
    
    size_t depth_; // node depth
    size_t idx_; // index on this depth
    char id_[64];
    
    size_t idxBestAlg;
    double nodeCost_;
    double avRank;

    void computeResultsNode();

    friend class Tree;
};

#endif /* NODE_HPP_ */

