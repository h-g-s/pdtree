/*
 * ResultsSet.cpp
 *
 *  Created on: 1 de mar de 2019
 *      Author: haroldo
 */

#include "ResultsSet.hpp"

#include <bits/types/clock_t.h>
#include <sys/time.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <utility>

#include "Dataset.hpp"
#include "Instance.hpp"
#include "SubSetResults.hpp"


using namespace std;

ResultsSet::ResultsSet( const InstanceSet &_iset, const char *fileName, const enum FMRStrategy _fmrs ) :
    iset_(_iset),
    res_(nullptr),
    ranks_(nullptr),
    fmrs_(_fmrs),
    avInst(nullptr),
    avRes_(nullptr),
    rnkRes_(nullptr),
    defRes_(nullptr)
{
    clock_t start = clock();
    Dataset dsres(fileName, false);

    if (dsres.headers().size()<3)
        throw "Results file should have at least 3 columns: instance,algorithmAndParamSettings,result";

    if (dsres.types()[0]!=String)
        throw "First field in results file should be an instance name (string)";

    if (not dsres.col_is_number(dsres.types().size()-1))
        throw "Last column in results file should be a number with the performance result";

    vector< size_t > iIdx;
    iIdx.reserve(dsres.rows());

    // algorithm x parameter setting index
    vector< size_t > aIdx;
    aIdx.reserve(dsres.rows());

    // checking instance indexes
    for ( size_t i=0 ; (i<dsres.rows()) ; ++i )
    {
        string iname = string(dsres.str_cell(i, 0));
        if (not _iset.has(iname))
            continue;
        iIdx.push_back(iset_.inst_by_name(iname).idx_);
    }

    // storing different algorithms and settings
    for ( size_t i=0 ; (i<dsres.rows()) ; ++i )
    {
        string iname = string(dsres.str_cell(i, 0));
        if (not _iset.has(iname))
            continue;
        string asname="";
        for ( size_t j=1 ; (j<dsres.headers().size()-1) ; ++j )
        {
            if (j>=2)
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
                    throw "Unexpected valued in column type";
                    break;
            }
        }

        auto it = algsByName_.find(asname);
        if (it==algsByName_.end())
        {
            algsByName_[asname] = algsettings_.size();
            aIdx.push_back(algsettings_.size());
            algsettings_.push_back(asname);
        }
        else
            aIdx.push_back(it->second);
    }

    res_ = new float*[iset_.size()];
    res_[0] = new float[iset_.size()*algsettings_.size()];
    for ( size_t i=1 ; (i<iset_.size()) ; ++i )
        res_[i] = res_[i-1] + algsettings_.size();
    std::fill( res_[0], res_[0]+(iset_.size()*algsettings_.size()), std::numeric_limits<float>::max() );

    ranks_ = new int*[iset_.size()];
    ranks_[0] = new int[iset_.size()*algsettings_.size()];
    for ( size_t i=1 ; (i<iset_.size()) ; ++i )
        ranks_[i] = ranks_[i-1] + algsettings_.size();
    std::fill( ranks_[0], ranks_[0]+(iset_.size()*algsettings_.size()), std::numeric_limits<int>::max() );

    // first pass on all results, checking worse values and
    // missing ones
    size_t nRows = dsres.rows();
    size_t colResult = dsres.headers().size()-1;
    float worse = std::numeric_limits<float>::min();
    std::vector< float > worseInst(iset_.size(), std::numeric_limits<float>::min());
    std::vector< long double > sumInst( iset_.size(), 0.0 );
    std::vector< size_t > nResInst( iset_.size(), 0 );

    // one instance and algorithm may appear more than once
    long double **sumRes = new long double *[iset_.size()];
    sumRes[0] = new long double[iset_.size()*algsettings_.size()];
    for (size_t i=0 ; (i<iset_.size()*algsettings_.size()) ; ++i )
        sumRes[0][i] = 0.0;
    for (size_t i=1 ; (i<iset_.size()) ; ++i )
        sumRes[i] = sumRes[i-1] + algsettings_.size();
    int **nRes = new int *[iset_.size()];
    nRes[0] = new int[iset_.size()*algsettings_.size()];
    for (size_t i=0 ; (i<iset_.size()*algsettings_.size()) ; ++i )
        nRes[0][i] = 0;
    for (size_t i=1 ; (i<iset_.size()) ; ++i )
        nRes[i] = nRes[i-1] + algsettings_.size();

    size_t ir = 0;
    for ( size_t i=0 ; (i<nRows) ; ++i )
    {
        string iname = string(dsres.str_cell(i, 0));
        if (not _iset.has(iname))
            continue;

        const float r = (float)dsres.float_cell(i, colResult);

        const size_t ii = iIdx[ir];
        const size_t ia = aIdx[ir];

        sumRes[ii][ia] += (long double)r;
        nRes[ii][ia]++;

        worse = max( worse, r );
        worseInst[ii] = max( worseInst[ii], r );
        sumInst[ii] += ((long double)r);
        ++nResInst[ii];

        ++ir;
    }

    for ( size_t i=0 ; i<iset_.size() ; ++i )
        for ( size_t j=0 ; j<algsettings_.size()  ; ++j )
            if (nRes[i][j])
                res_[i][j] = (float)(((long double)sumRes[i][j]) / ((long double)nRes[i][j]));

    // average per instance and algorithm
    delete[] sumRes[0];
    delete[] sumRes;
    delete[] nRes[0];
    delete[] nRes;

    // computing average per instance
    std::vector< float > avgInst = vector<float>(iset_.size());
    for ( size_t i=0 ; (i<iset_.size()) ; ++i )
        if (nResInst[i])
            avgInst[i] = (float)(((long double)sumInst[i])/((long double)nResInst[i]));
        else
            avgInst[i] = worse;

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
                        res_[i][j] = worse + fabs(worse);;
                        break;
                    case FMRStrategy::WorseInst:
                        res_[i][j] = worseInst[i];
                        break;
                    case FMRStrategy::WorseInstT2:
                        res_[i][j] = worseInst[i]+fabs(worseInst[i]);;
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

    double secs = ((double)(clock()-start)) / ((double)CLOCKS_PER_SEC);
    cout << ir << " results loaded in " << setprecision(3) << secs << " seconds" << endl;

    clock_t startr = clock();
    cout << "Computing ranking and summarized results ... ";

    compute_rankings( algsettings_.size(), iset_.size(), (const float **)res_, ranks_ );

    cout << "done in " << fixed << setprecision(2) <<
            (((double)clock()-startr) / ((double)CLOCKS_PER_SEC)) << endl;

    avInst = new float[iset_.size()];

    vector< pair< float, size_t > > algsByAv;
    vector< pair< float, size_t > > algsByAvRnk;

    avRes_ = new SubSetResults( this, Average );
    rnkRes_ = new SubSetResults( this, Rank );
    switch (Parameters::eval)
    {
        case Average:
            defRes_ = avRes_;
            break;
        case Rank:
            defRes_ = rnkRes_;
            break;
    }
 
    for ( size_t i=0 ; (i<iset_.size()) ; ++i )
    {
        long double sum = 0.0;
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            sum += res_[i][j];
        avInst[i] = sum / ((long double)algsettings_.size());
    }

    nRankOne = new int[algsettings_.size()];
    fill(nRankOne, nRankOne+algsettings_.size(), 0);
    for ( size_t i=0 ; i<iset_.size(); ++i )
    {
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
        {
            if (ranks_[i][j]==0)
                nRankOne[j]++;
        }
    }

    vector< pair<int, size_t > > algsByNro;
    for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
        algsByNro.push_back( make_pair(nRankOne[j]*-1, j) );

    sort( algsByNro.begin(), algsByNro.end() );

    for (size_t i=0 ; (i<Parameters::storeTop and i<algsByNro.size()) ; ++i )
        topAlgByRnkOne.push_back(algsByNro[i].second);
}

float ResultsSet::get(size_t iIdx, size_t aIdx) const
{
    assert( iIdx<iset_.size() );
    assert( aIdx<algsettings_.size() );
    return res_[iIdx][aIdx];
}

ResultsSet::~ResultsSet ()
{
    delete avRes_;
    delete rnkRes_;

    delete[] nRankOne;
    delete[] ranks_[0];
    delete[] ranks_;
    delete[] res_[0];
    delete[] res_;
    delete[] avInst;
}


void ResultsSet::compute_rankings( size_t nAlgs, size_t nInsts, const float **res, int **rank )
{
    vector< pair< float, size_t> > resInst = vector< pair< float, size_t> >(nAlgs);

    for ( size_t i=0 ; (i<nInsts) ; ++i )
    {
        for ( size_t j=0 ; (j<nAlgs) ; ++j )
            resInst[j] = make_pair( res[i][j], j );

        std::sort(resInst.begin(), resInst.end() );

        float startValRank = resInst.begin()->first;

        int currRank = 0;
        for ( size_t j=0 ; (j<nAlgs) ; ++j )
        {
            size_t iAlg = resInst[j].second;
            const float res = resInst[j].first;
            const double pr = fabsf(res)*Parameters::rankPerc;

            if ((res>=startValRank+Parameters::rankEps) and (res>=startValRank+pr))
            {
                ++currRank;
                rank[i][iAlg] = currRank;
                startValRank = res;
            }
            else
                rank[i][iAlg] = currRank;
        } // all algorithms
    } // all instances
}

int ResultsSet::rank(size_t iIdx, size_t iAlg) const
{
    assert(iIdx<iset_.size());
    assert(iAlg<algsettings_.size());

    int r = ranks_[iIdx][iAlg];
    assert( r>=0 and r<((int)algsettings_.size()) );
    return r;
}

void ResultsSet::print_summarized_results() 
{
    cout << endl;
    cout << "Top algorithms/parameter settings considering the whole instance set: " << endl << endl;
    cout << "Average results" << endl;
    cout << " # algorithm/p. setting                                     res        " << endl;
    cout << "== ======================================================== ===========" << endl;
    auto bestAlgsAv = avRes_->computeBestAlgorithms();
    for ( size_t i=0 ; (i<min(bestAlgsAv.size(), Parameters::storeTop)) ; ++i )
    {
        cout << setw(2) << right << i+1 << " " <<
                setw(55) << left << algsettings_[bestAlgsAv[i]] << " " <<
                setw(12) << setprecision(6) << defaultfloat << right << avRes_->resAlg(bestAlgsAv[i]) << " " << endl;
    }
    cout << endl;
    cout << "Average rank" << endl;
    cout << " # algorithm/p. setting                                     res     " << endl;
    cout << "== ======================================================== ========" << endl;
    auto bestAlgsRnk = rnkRes_->computeBestAlgorithms();
    for ( size_t i=0 ; (i<min(bestAlgsRnk.size(), Parameters::storeTop)) ; ++i )
    {
        cout << setw(2) << right << i+1 << " " <<
                setw(55) << left << algsettings_[bestAlgsRnk[i]] << " " <<
                setw(9) << setprecision(3) << fixed << right << rnkRes_->resAlg(bestAlgsRnk[i]) << " " << endl;
    }

    cout << endl;
    cout << "Algorithms per number of first ranked results" << endl;
    cout << " # algorithm/p. setting                                     res      " << endl;
    cout << "== ======================================================== ========" << endl;
    for ( size_t i=0 ; (i<topAlgByRnkOne.size()) ; ++i )
    {
        cout << setw(2) << right << i+1 << " " <<
                setw(55) << left << algsettings_[topAlgByRnkOne[i]] << " " <<
                setw(9) << setprecision(3) << fixed << right << nRankOne[topAlgByRnkOne[i]] << " " << endl;
    }
}

SubSetResults &ResultsSet::results_eval( const Evaluation _eval ) const
{
    switch (_eval)
    {
        case Average:
            return *(this->avRes_);
        case Rank:
            return *(this->rnkRes_);
    }

    return *(this->defRes_);
}

void ResultsSet::save_csv( const char *fileName, Evaluation _eval ) const
{
    ofstream of(fileName);

    for ( const auto &alg : algsettings_ )
        of << "," << alg;
    of << endl;

    for ( const auto &inst : iset_.instances( ))
    {
        of << inst.name();

        switch (_eval)
        {
            case Average:
                for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
                    of << "," << this->get(inst.idx_, j);
                break;
            case Rank:
                for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
                    of << "," << this->rank(inst.idx_, j);
                break;
        }
        of << endl;
    }

    of.close();
}

