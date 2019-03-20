/*
 * MIPPDtree.hpp
 *
 *  Created on: 20 de mar de 2019
 *      Author: haroldo
 */

#ifndef MIPPDTREE_HPP_
#define MIPPDTREE_HPP_

#include "lp.h"
class InstanceSet;
class ResultsSet;

class MIPPDtree
{
public:
    MIPPDtree( const InstanceSet *_iset, const ResultsSet *_rset );

    virtual ~MIPPDtree ();
private:
    const InstanceSet *iset_;
    const ResultsSet *rset_;
    LinearProgram *mip;
};

#endif /* MIPPDTREE_HPP_ */
