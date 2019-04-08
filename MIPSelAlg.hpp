/*
 * MIPSelAlg.hpp
 *
 *  Created on: 8 de abr de 2019
 *      Author: haroldo
 */

#ifndef MIPSELALG_HPP_
#define MIPSELALG_HPP_

class ResultsSet;

extern "C"
{
#include "lp.h"
}

class InstanceSet;

class MIPSelAlg
{
public:
    MIPSelAlg( const ResultsSet *_rset );

    virtual ~MIPSelAlg ();
private:
    const ResultsSet *rset_;
    const InstanceSet *iset_;

    void createYVars();
    void createXVars();
    void createConsSelK();
    void createConsLNKXY();

    // y var indexes
    int *y;

    int **x;

    LinearProgram *mip;
};

#endif /* MIPSELALG_HPP_ */
