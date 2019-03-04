/*
 * ResultsSet.hpp
 *
 *  Created on: 1 de mar de 2019
 *      Author: haroldo
 */

#ifndef RESULTSSET_HPP_
#define RESULTSSET_HPP_

#include "Dataset.hpp"
#include "InstanceSet.hpp"
#include <string>
#include <vector>
#include <cstddef>
#include <unordered_map>

/** strategy to fill missing results, if any */
enum FMRStrategy {Worse = 0,       // worse result
                  WorseT2,     // abs worse result times 2
                  WorseInst,   // worse result from instance
                  WorseInstT2, // worse result from instance times 2
                  AverageInst
};

enum Evaluation 
{
    Average = 0, // when results of executions in 
             // different instances are comparable,
             // such as execution time
             
    Rank     // if results for different instances
             // are not comparable, such as
             // objective functions with different
             // scales
};

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

    virtual ~ResultsSet ();

    static void configure_parameters(int argc, const char **argv);

    static void help();

    static void print_config();

    static enum FMRStrategy fmrStrategy;
    static enum Evaluation eval;

    // minimum absolute difference
    // between two results to change ranking
    static double rankEps;

    // minimum percentage difference between
    // two values to increase ranking
    static double rankPerc;
private:
    const InstanceSet &iset_;

    // different algorithms and parameter settings
    std::vector< std::string > algsettings_;
    std::unordered_map< std::string, size_t > algsByName_;
    float **res_;
    int **ranks_;
    const enum FMRStrategy fmrs_;

    void compute_rankings();
};

#endif /* RESULTSSET_HPP_ */
