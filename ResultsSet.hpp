/*
 * ResultsSet.hpp
 *
 *  Created on: 1 de mar de 2019
 *      Author: haroldo
 */

#ifndef RESULTSSET_HPP_
#define RESULTSSET_HPP_

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "InstanceSet.hpp"
#include "Parameters.hpp"
#include "pdtdefines.hpp"

class SubSetResults;

class ResultsSet
{
public:
    ResultsSet( const InstanceSet &_iset,
                const char *fileName,
                const enum FMRStrategy _fmrs = WorseInstT2 );

    // returns a specific result
    TResult get(size_t iIdx, size_t aIdx) const;

    // returns the ranking of instance
    // iIdx, for algorithm/parameter setting iAlg
    int rank(size_t iIdx, size_t iAlg) const;

    // returns the algsetting of i-th rank (starting from zero)
    // for some instance of -1 if it does
    // not exists
    int algsetting_rank( size_t idxInst, int rank ) const;
    
    // return normal result or rank 
    // depending on param settings
    double res(size_t iIdx, size_t iAlg) const;

    const std::vector<std::string> &algsettings() const {
        return this->algsettings_;
    }

    const std::vector< Instance > &instances() const {
        return iset_.instances();
    }

    void print_summarized_results();

    virtual ~ResultsSet ();

    void save_csv(const char *fileName) const;

    void compute_summarized_results();

    SubSetResults &results_eval( const Evaluation _eval ) const;

    const SubSetResults &results() const {
        return *this->defRes_;
    }

    // summary information per instance in a csv file
    void saveInstanceSum(const char *fileName) const;

private:
    const InstanceSet &iset_;

    // different algorithms and parameter settings
    std::vector< std::string > algsettings_;
    std::unordered_map< std::string, size_t > algsByName_;
    TResult **res_;
    int **ranks_;
    const enum FMRStrategy fmrs_;

    TResult *avInst;
    int *nRankOne;

    std::vector< size_t > topAlgByRnkOne;

    // algorithms results
    SubSetResults *avRes_;
    SubSetResults *rnkRes_;
    // points to one of the two previous
    SubSetResults *defRes_;

    friend class Tree;
    friend class ResTestSet;
    static void compute_rankings( size_t nAlgs, size_t nInsts, const TResult **res, int **rank );
};

#endif /* RESULTSSET_HPP_ */

