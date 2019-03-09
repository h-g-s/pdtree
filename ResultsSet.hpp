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

class SubSetResults;

class ResultsSet
{
public:
    ResultsSet( const InstanceSet &_iset,
                const char *fileName,
                const enum FMRStrategy _fmrs = WorseInstT2 );

    // returns a specific result
    float get(size_t iIdx, size_t aIdx) const;

    // returns the ranking of instance
    // iIdx, for algorithm/parameter setting iAlg
    int rank(size_t iIdx, size_t iAlg) const;

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
private:
    const InstanceSet &iset_;

    // different algorithms and parameter settings
    std::vector< std::string > algsettings_;
    std::unordered_map< std::string, size_t > algsByName_;
    float **res_;
    int **ranks_;
    const enum FMRStrategy fmrs_;

    float *avInst;
    int *nRankOne;

    std::vector< size_t > topAlgByRnkOne;

    // algorithms results
    SubSetResults *avRes_;
    SubSetResults *rnkRes_;
    SubSetResults *defRes_;

    void compute_rankings();
};

#endif /* RESULTSSET_HPP_ */

