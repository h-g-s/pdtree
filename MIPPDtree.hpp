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

    void createBVars();

    std::vector< std::string > branchNodes;
};

#endif /* MIPPDTREE_HPP_ */
