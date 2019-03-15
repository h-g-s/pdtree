/*
 * Node.hpp
 *
 *  Created on: 7 de mar de 2019
 *      Author: haroldo
 */

#ifndef NODE_HPP_
#define NODE_HPP_

#include <stddef.h>
#include <vector>

#include "Branching.hpp"
#include "InstanceSet.hpp"
#include "tinyxml2.h"

class XMLDocument;

class Node
{
public:
    Node( const InstanceSet &_iset, const ResultsSet &_rset );

    Node( const Node *_parent, size_t _nEl, const size_t *_el, const SubSetResults &_ssres, size_t _idx );

    // searches for the best branch that does not violates any constraint,
    // if it exists performs it. returns the children nodes
    std::vector<Node *> &perform_branch();

    // returns the best branch selected in this node
    // may be empty if the node is a leaf
    const Branching &best_branch() {
        return this->bestBranch_;
    }

    const std::vector<Node *> &child() {
        return child_;
    }

    const Node *parent() const {
        return parent_;
    }

    size_t n_elements() const {
        return nEl_;
    }

    const size_t *elements() const {
        return el_;
    }

    const SubSetResults &result( const Evaluation eval_ = Parameters::eval ) {
        if (eval_==Parameters::eval)
            return this->ssres_;
        
        if (ossr==nullptr)
        {
            switch (eval_)
            {
                case Average:
                    this->ossr = new SubSetResults( &rset_, Average, true, this->n_elements(), this->elements() );
                    break;
                case Rank:
                    this->ossr = new SubSetResults( &rset_, Rank, true, this->n_elements(), this->elements() );
                    break;
            }
        }

        return *ossr;
    }

    void writeXML(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *parent ) const;

    virtual ~Node ();
private:
    const InstanceSet &iset_;
    const ResultsSet &rset_;
    const SubSetResults ssres_;
    // other subset results (rank or average)
    SubSetResults *ossr;

    size_t nEl_;
    size_t *el_;

    const Node *parent_;
    std::vector< Node * > child_;
    Branching bestBranch_;

    size_t depth; // node depth
    size_t idx; // index on this depth
    std::string id;

    friend class Tree;
};

#endif /* NODE_HPP_ */
