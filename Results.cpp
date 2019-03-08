/*
 * Results.cpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include "Results.hpp"

#include <stddef.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "Dataset.hpp"

using namespace std;

Results::Results ( const InstanceSet &_iset, const char *resFile ) :
    iset_(_iset)
{
    Dataset dsres(resFile, false);

    if (dsres.types().size()<3)
    {
        cerr << "Results dataset should contain at least three fields: instance,algorithmAndSettings,result" << endl;
        abort();
    }

    /* checking contents consistenty */
    if (dsres.types()[0] != Datatype::String)
    {
        cerr << "First field in results dataset should be instance name (string)" << endl;
        abort();
    }

    const auto &typeLF = dsres.types()[dsres.types().size()-1];
    if (typeLF != Integer and typeLF != Float and typeLF != Char and typeLF != Short)
    {
        cerr << "Last field in results dataset should be a number with the execution evaluation (the smaller the better)." << endl;
        abort();
    }

    unordered_set< std::string > sAlgs;
    for ( size_t i=0 ; (i<dsres.rows()) ; ++i )
        sAlgs.insert( string(dsres.str_cell(i, 0)) );

    for ( const auto &alg : sAlgs )
        algs_.push_back(alg);

    std::sort(algs_.begin(), algs_.end());
}

const std::vector< std::string > &Results::algorithms() const
{
    return this->algs_;
}


Results::~Results ()
{
}

