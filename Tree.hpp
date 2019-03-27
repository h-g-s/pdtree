/*
 * Tree.hpp
 *
 *  Created on: 11 de mar de 2019
 *      Author: haroldo
 */

#ifndef TREE_HPP_
#define TREE_HPP_

#include "InstanceSet.hpp"
#include "ResultsSet.hpp"

class Node;
class ResTestSet;
class InstanceSet;
class ResultsSet;

class Tree
{
public:
    Tree( const InstanceSet *_iset, const ResultsSet *_rset, const ResTestSet *_rtest = nullptr );

    void addNode( Node *_node );

    // creates and returns the root node
    Node *create_root();
    
    void draw( const char *fileName ) const;

    // saves tree in XML
    void save( const char *fileName ) const;

    // evaluate in a set of training instances
    double evaluate( const Dataset *testData ) const;

    double leafResults() const {
        return resultLeafs;
    }

    virtual ~Tree ();
private:
    char nLabel[8192];
    const char *node_label( const Node *node ) const;

    const InstanceSet *iset_;
    const ResultsSet *rset_;

    Node *root;

    double resultLeafs;

    double improvement;

    std::vector< Node * > nodes_;

    std::vector< Node * > leafs_;

    size_t maxDepth;

    size_t minInstancesNode;

    const ResTestSet *rtest_;
};

#endif /* TREE_HPP_ */
