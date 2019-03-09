/*
 * Parameters.hpp
 *
 *  Created on: 8 de mar de 2019
 *      Author: haroldo
 */

#ifndef PARAMETERS_HPP_
#define PARAMETERS_HPP_

#include <stddef.h>

#include "ResultsSet.hpp"

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
