/*
 * Node.hpp
 *
 *  Created on: 7 de mar de 2019
 *      Author: haroldo
 */

#ifndef NODE_HPP_
#define NODE_HPP_

#include <vector>
#include "Instance.hpp"
#include "ResultsSet.hpp"
#include "Branching.hpp"

class Node
{
public:
    Node( const InstanceSet &_iset, const ResultsSet &_rset );

    Node( const Node *_parent, size_t _nEl, const size_t *_el );

    // searches for the best branch that does not violates any constraint,
    // if it exists performs it. returns the children nodes
    std::vector<Node> &perform_branch();

    // returns the best branch selected in this node
    // may be empty if the node is a leaf
    const Branching &best_branch() {
        return this->bestBranch_;
    }

    const std::vector<Node> &children() {
        return children_;
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

    virtual ~Node ();
private:
    const InstanceSet &iset_;
    const ResultsSet &rset_;

    size_t nEl_;
    size_t *el_;

    const Node *parent_;
    std::vector<Node> children_;
    Branching bestBranch_;
};

#endif /* NODE_HPP_ */
