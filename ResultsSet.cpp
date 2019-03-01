/*
 * ResultsSet.cpp
 *
 *  Created on: 1 de mar de 2019
 *      Author: haroldo
 */

#include <limits>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cassert>
#include "ResultsSet.hpp"

using namespace std;

ResultsSet::ResultsSet( const InstanceSet &_iset, const char *fileName, const enum FMRStrategy _fmrs ) :
    iset_(_iset),
    res_(nullptr),
    fmrs_(_fmrs)
{
    Dataset dsres(fileName, false);

    if (dsres.headers().size()<3)
        throw "Results file should have at least 3 columns: instance,algorithmAndParamSettings,result";

    if (dsres.types()[0]!=String)
        throw "First field in results file should be an instance name (string)";

    if (not dsres.col_is_number(dsres.types().size()-1))
        throw "Last column in results file should be a number with the performance result";

    vector< size_t > iIdx;
    iIdx.resize(dsres.rows());

    // algorithm x parameter setting index
    vector< size_t > aIdx;
    aIdx.resize(dsres.rows());

    // checking instance indexes
    for ( size_t i=0 ; (i<dsres.rows()) ; ++i )
        iIdx[i] = iset_.inst_by_name(dsres.str_cell(i, 0)).idx_;

    // storing algorithm and settings
    for ( size_t i=0 ; (i<dsres.rows()) ; ++i )
    {
        string asname="";
        for ( size_t j=0 ; (j<dsres.headers().size()-2) ; ++j )
        {
            if (j)
                asname += ";";
            switch (dsres.types()[j])
            {
                case String:
                    asname += dsres.str_cell(i, j);
                    break;
                case Integer:
                    asname += to_string(dsres.int_cell(i, j));
                    break;
                case Short:
                    asname += to_string(dsres.int_cell(i, j));
                    break;
                case Char:
                    asname += to_string(dsres.int_cell(i, j));
                    break;
                case Float:
                    asname += to_string(dsres.float_cell(i, j));
                    break;
                case Empty:
                    break;
                case N_DATA_TYPES:
                    break;
            }
        }

        auto it = algsByName_.find(asname);
        if (it==algsByName_.end())
        {
            algsByName_[asname] = algsettings_.size();
            aIdx[i] = algsettings_.size();
            algsettings_.push_back(asname);
        }
        else
            aIdx[i] = it->second;
    }

    res_ = new float*[iset_.size()];
    res_[0] = new float[iset_.size()*algsettings_.size()];
    for ( size_t i=1 ; ; ++i )
        res_[i] = res_[i-1] +  algsettings_.size();

    std::fill( res_[0], res_[0]+(iset_.size()*algsettings_.size()), std::numeric_limits<float>::max() );

    // first pass on all results, cgecking worse values and
    // missing ones
    size_t nRows = dsres.rows();
    size_t colResult = dsres.headers().size()-1;
    float worse = std::numeric_limits<float>::min();
    std::vector< float > worseInst(iset_.size(), std::numeric_limits<float>::min());
    std::vector< long double > sumInst( iset_.size(), 0.0 );
    std::vector< size_t > nResInst( iset_.size(), 0 );
    for ( size_t i=0 ; (i<nRows) ; ++i )
    {
        if (res_[iIdx[i]][aIdx[i]] != numeric_limits<float>::max())
        {
            cout << "warning: result for instance " << iset_.instance(iIdx[i]).name() <<
              " and parameter setting " << algsettings_[aIdx[i]] <<
              " appear twice in the results file " << endl ;
            continue;
        }

        const float r = (float)dsres.float_cell(i, colResult);

        res_[iIdx[i]][aIdx[i]] = r;
        worse = max( worse, r );
        worseInst[iIdx[i]] = max( worseInst[iIdx[i]], r );
        sumInst[iIdx[i]] += r;
        ++nResInst[iIdx[i]];
    }

    std::vector< float > avgInst = vector<float>(iset_.size(), 0.0);
    for ( size_t i=0 ; (i<iset_.size()) ; ++i )
        avgInst[i] = (float)(((long double)sumInst[i])/((long double)nResInst[i]));

    size_t nMissing = 0;
    for ( size_t i=0 ; (i<iset_.size()) ; ++i )
    {
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
        {
            auto r = res_[i][j];
            if (r == numeric_limits<float>::max())
            {
                switch (fmrs_)
                {
                    case FMRStrategy::Worse:
                        res_[i][j] = worse;
                        break;
                    case FMRStrategy::WorseT2:
                        res_[i][j] = fabs(worse)*2.0;
                        break;
                    case FMRStrategy::WorseInst:
                        res_[i][j] = worseInst[i];
                        break;
                    case FMRStrategy::WorseInstT2:
                        res_[i][j] = fabs(worseInst[i])*2.0;
                        break;
                    case FMRStrategy::AverageInst:
                        res_[i][j] = avgInst[i];
                        break;
                }
                nMissing++;
            }
        }
    }

    if (nMissing)
    {
        const double percm = ( (((double)nMissing))/(((double)iset_.size()*algsettings_.size())) )*100.0;
        cout << "warning : there are " << nMissing << " results for instance x algorithm/parameter settings missing (" \
             << setprecision(2) << percm \
             << "%)" << endl;
    }
}

float ResultsSet::get(size_t iIdx, size_t aIdx) const
{
    assert( iIdx<iset_.size() );
    assert( aIdx<algsettings_.size() );
    return res_[iIdx][aIdx];
}

ResultsSet::~ResultsSet ()
{
    delete[] res_[0];
    delete[] res_;
}
