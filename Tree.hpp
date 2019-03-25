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
class ResTestSet;

class Tree
{
public:
    Tree( const InstanceSet &_iset, const ResultsSet &_rset, const ResTestSet *_rtest = nullptr );

    void build();

    // creates and returns the root node
    const Node *create_root();
    
    // branches by the normalized value
    std::vector< const Node * > branch( Node *node, size_t idxFeature, const double branchValue );

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
    const Node *node_instance( const Dataset *testd, size_t idxInst ) const;

    double cost_instance( const Dataset *testd, size_t idxInst, const ResTestSet *rtst ) const;

    std::string node_label( const Node *node ) const;

    const InstanceSet &iset_;
    const ResultsSet &rset_;

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
