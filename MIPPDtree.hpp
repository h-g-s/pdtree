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

    void createBVars();
    void createDVars();
    void createAVars();
    void createLVars();
    void createZVars();
    //void createYVars();
    void createConsLnkBD();
    void createConsLnkAD();
    void createConsLNKZL();

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
};

#endif /* MIPPDTREE_HPP_ */
