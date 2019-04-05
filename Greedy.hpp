/*
 * Greedy.hpp
 *
 *  Created on: 2 de abr de 2019
 *      Author: haroldo
 */

#ifndef GREEDY_HPP_
#define GREEDY_HPP_

class InstanceSet;
class ResultsSet;
class Tree;
class GNodeData;

#include <cstddef>

class Greedy
{
public:
    Greedy (const InstanceSet *_iset, const ResultsSet *_rset);

    Tree *build();

    virtual ~Greedy ();
private:
    const InstanceSet *iset_;
    const ResultsSet *rset_;

    // prepare for branching on node and feature
    void prepareBranch( size_t n, size_t f );


    GNodeData **ndata;
    size_t tnodes; // total nodes in full btree
    size_t maxDepth;
};

#endif /* GREEDY_HPP_ */
