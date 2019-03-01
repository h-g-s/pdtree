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
enum FMRStrategy {Worse,       // worse result
                  WorseT2,     // abs worse result times 2
                  WorseInst,   // worse result from instance
                  WorseInstT2, // worse result from instance times 2
                  AverageInst
};

class ResultsSet
{
public:
    ResultsSet( const InstanceSet &_iset, const char *fileName, const enum FMRStrategy _fmrs = WorseInstT2 );

    // returns an specific result
    float get(size_t iIdx, size_t aIdx) const;

    const std::vector<std::string> &algsettings() const {
        return this->algsettings_;
    }

    virtual ~ResultsSet ();
private:
    const InstanceSet &iset_;

    // different algorithms and parameter settings
    std::vector< std::string > algsettings_;
    std::unordered_map< std::string, size_t > algsByName_;
    float **res_;
    const enum FMRStrategy fmrs_;
};

#endif /* RESULTSSET_HPP_ */
