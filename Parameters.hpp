/*
 * Parameters.hpp
 *
 *  Created on: 8 de mar de 2019
 *      Author: haroldo
 */

#ifndef PARAMETERS_HPP_
#define PARAMETERS_HPP_

#include <stddef.h>

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

class Parameters
{
public:
    static void parse( int argc, const char **argv );

    static void print();

    static void help();

    // how missing results in the results set
    // will be filled
    static enum FMRStrategy fmrStrategy;

    // based on average or rank
    static enum Evaluation eval;

    // minimum absolute difference
    // between two results to change ranking
    static double rankEps;

    // minimum percentage difference between
    // two values to increase ranking
    static double rankPerc;

    // compute top "storeTop" configurations, just to
    // display summary
    static size_t storeTop;

};

#endif /* PARAMETERS_HPP_ */
