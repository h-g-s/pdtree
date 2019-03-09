/*
 * SubSetResults.hpp
 *
 *  Created on: 9 de mar de 2019
 *      Author: haroldo
 */


#ifndef SUBSETRESULTS_HPP_
#define SUBSETRESULTS_HPP_

#include "ResultsSet.hpp"
#include "Parameters.hpp"

typedef long double SumType;

class SubSetResults
{
public:
    SubSetResults ( const ResultsSet &_rset,
                    const Evaluation _eval = Parameters::eval
                    );

    void add( size_t n, const size_t *el );

    void remove( size_t n, const size_t *el );

    size_t bestAlg() const {
        return this->idxBestAlg_;
    }

    double bestAlgRes() const {
        return this->resBestAlg_;
    }

    virtual ~SubSetResults ();
private:
    void updateBest();

    size_t nElSS;

    size_t idxBestAlg_;
    double resBestAlg_;

    const ResultsSet &rset_;
    Evaluation eval_;
    SumType *sum_;
};

#endif /* SUBSETRESULTS_HPP_ */
