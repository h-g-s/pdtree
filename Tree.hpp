/*
 * Tree.hpp
 *
 *  Created on: 11 de mar de 2019
 *      Author: haroldo
 */

#ifndef TREE_HPP_
#define TREE_HPP_

#include <string>
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"

class Node;

class Tree
{
public:
    Tree( const InstanceSet &_iset, const ResultsSet &_rset );

    void build();

    void draw( const char *fileName );

    virtual ~Tree ();
private:
    std::string node_label( const Node *node ) const;

    const InstanceSet &iset_;
    const ResultsSet &rset_;

    Node *root;

    std::vector< Node * > nodes_;

    std::vector< Node * > leafs_;
};

#endif /* TREE_HPP_ */