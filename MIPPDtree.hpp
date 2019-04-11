/*
 * MIPPDtree.hpp
 *
 *  Created on: 20 de mar de 2019
 *      Author: haroldo
 */

#ifndef MIPPDTREE_HPP_
#define MIPPDTREE_HPP_

extern "C"
{
#include "lp.h"
}
class InstanceSet;
class ResultsSet;
class Tree;

#include <vector>
#include <string>
#include <cfloat>
#include <climits>

class Tree;
class Node;

class MIPPDtree
{
public:
    MIPPDtree( const InstanceSet *_iset, const ResultsSet *_rset );

    void setInitialSolution( const Tree *tree );

    Tree *build( const int maxSeconds = INT_MAX );

    virtual ~MIPPDtree ();
private:
    const InstanceSet *iset_;
    const ResultsSet *rset_;

    size_t nLeafs;

    size_t nInsts;

    size_t nFeatures;

    size_t nAlgs;

    LinearProgram *mip;

    // penalty for more branches
    static double alpha;

    // max eps_j value
    std::vector< double > epsj;
    double emax;

    void setBinVarsNode( const Node *node, std::vector< std::string > &cnames );

    void computeEMax();

    std::vector< std::vector< int > > c;
    std::vector< std::vector<int> > w;
    std::vector< std::vector< std::vector< int > > > parents;

    // name of branchnodes
    std::vector< std::string > branchNodes;

    // feature names without special characters
    std::vector< std::string > featNames;

    std::vector< std::string > leafNodes;

    std::vector< int > b; // b indexes
    std::vector< int > d; // d indexes

    std::vector< std::vector< int > > z;

    void createBVars();
    void createDVars();
    void createAVars();
    void createConsLnkAD();
    void createLVars();
    std::vector< std::vector< int > > a; // a indexes
    std::vector< int > l;
    void createCVars();
    void createZVars();
    void createWVars();

    void createConsLnkBD();
    void createConsOneLeaf();
    void createConsLNKZL();
    void createConsBranchOrLeaf();
    void createConsOneLeafPath();
    void createConsSelectLeaf();
    void createConsOneLeafPerProb();
    void createConsLnkWCZ();
    void createConsSelOneW();
    void createConsBranchBeforeLeaf();
    void createConsSelAlgLeaf();

    // names for lp when in debug
    std::vector< std::string > insts;
    std::vector< std::string > algs;

    // variable indexes
    /*

    std::vector< int > l;

    void createConsLnkParent();



    // parents at leaf (0) and right (1)  for each leaf node
    // parents[l][0] has a list of all nodes which the left side branch has been followed to arrive at l
    // parents[l][1] has a list of all nodes which the right side branch has been followed to arrive at l
*/
};

#endif /* MIPPDTREE_HPP_ */

