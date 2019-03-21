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

#include <vector>
#include <string>

class MIPPDtree
{
public:
    MIPPDtree( const InstanceSet *_iset, const ResultsSet *_rset );

    virtual ~MIPPDtree ();
private:
    const InstanceSet *iset_;
    const ResultsSet *rset_;
    LinearProgram *mip;

    // penalty for more branches
    static double alpha;

    // max eps_j value
    std::vector< double > epsj;
    double emax;

    void computeEMax();

    void createBVars();
    void createDVars();
    void createAVars();
    void createLVars();
    void createZVars();
    //void createYVars();
    void createConsLnkBD();
    void createConsLnkAD();
    void createConsLNKZL();
    void createConsOneLeaf();
    void createConsSelectLeaf();

    std::vector< std::string > branchNodes;

    // feature names without special characters
    std::vector< std::string > featNames;

    // variable indexes
    std::vector< int > b; // b indexes
    std::vector< int > d; // d indexes
    std::vector< std::vector< int > > a; // a indexes
    std::vector< int > l;
    std::vector< std::vector< int > > z;

    size_t nLeafs;

    size_t nInsts;

    size_t nFeatures;

    // parents at leaf (0) and right (1)  for each leaf node
    // parents[l][0] has a list of all nodes which the left side branch has been followed to arrive at l
    // parents[l][1] has a list of all nodes which the right side branch has been followed to arrive at l
    std::vector< std::vector< std::vector< int > > > parents;
};

#endif /* MIPPDTREE_HPP_ */
