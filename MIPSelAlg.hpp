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

    void optimize(int maxSeconds);

    int nSelAlg() const {
        return nSelAlg_;
    }

    const int *selAlg() const {
        return selAlg_;
    }

    void saveFilteredResults(const char *fileName) const;

    virtual ~MIPSelAlg ();
private:
    const ResultsSet *rset_;
    const InstanceSet *iset_;

    int nSelAlg_;
    int *selAlg_;

    void createYVars();
    void createXVars();
    void createConsSelK();
    void createConsLNKXY();
    void createConsSelNAlgs();
    void createConsSelMinProbAlg();

    // y var indexes
    int *y;

    int **x;

    LinearProgram *mip;
};

#endif /* MIPSELALG_HPP_ */
