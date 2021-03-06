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
#include <cfloat>
#include <unordered_set>
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
    origRes_(nullptr),
    ranks_(nullptr),
    fmrs_(_fmrs),
    avInst(nullptr),
    stdDevInst_(nullptr),
    worseInst(nullptr),
    nTimeOutsInst(nullptr),
    avRes_(nullptr),
    rnkRes_(nullptr),
    defRes_(nullptr)
{
    clock_t start = clock();
    Dataset dsres(fileName, false);

    worseInst = new TResult[iset_.size()];

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

    res_ = new TResult*[iset_.size()];
    res_[0] = new TResult[iset_.size()*algsettings_.size()];
    for ( int i=1 ; (i<iset_.size()) ; ++i )
        res_[i] = res_[i-1] + algsettings_.size();
    std::fill( res_[0], res_[0]+(iset_.size()*algsettings_.size()), std::numeric_limits<TResult>::max() );
    origRes_ = new TResult*[iset_.size()];
    origRes_[0] = new TResult[iset_.size()*algsettings_.size()];
    for ( int i=1 ; (i<iset_.size()) ; ++i )
        origRes_[i] = origRes_[i-1] + algsettings_.size();
    std::fill( origRes_[0], origRes_[0]+(iset_.size()*algsettings_.size()), std::numeric_limits<TResult>::max() );

    ranks_ = new int*[iset_.size()];
    ranks_[0] = new int[iset_.size()*algsettings_.size()];
    for ( int i=1 ; (i<iset_.size()) ; ++i )
        ranks_[i] = ranks_[i-1] + algsettings_.size();
    std::fill( ranks_[0], ranks_[0]+(iset_.size()*algsettings_.size()), std::numeric_limits<int>::max() );

    // first pass on all results, checking worse values and
    // missing ones
    size_t nRows = dsres.rows();
    size_t colResult = dsres.headers().size()-1;
    auto worse = std::numeric_limits<TResult>::min();
    std::vector< long double > sumInst( iset_.size(), 0.0 );
    std::vector< size_t > nResInst( iset_.size(), 0 );

    // one instance and algorithm may appear more than once
    long double **sumRes = new long double *[iset_.size()];
    sumRes[0] = new long double[iset_.size()*algsettings_.size()];
    for (size_t i=0 ; (i<iset_.size()*algsettings_.size()) ; ++i )
        sumRes[0][i] = 0.0;
    for (int i=1 ; (i<iset_.size()) ; ++i )
        sumRes[i] = sumRes[i-1] + algsettings_.size();
    int **nRes = new int *[iset_.size()];
    nRes[0] = new int[iset_.size()*algsettings_.size()];
    for (int i=0 ; (i<iset_.size()*(int)algsettings_.size()) ; ++i )
        nRes[0][i] = 0;
    for (int i=1 ; (i<iset_.size()) ; ++i )
        nRes[i] = nRes[i-1] + algsettings_.size();

    size_t ir = 0;
    fill(worseInst, worseInst+iset_.size(), -DBL_MIN);

    for ( size_t i=0 ; (i<nRows) ; ++i )
    {
        string iname = string(dsres.str_cell(i, 0));
        if (not _iset.has(iname))
            continue;

        const auto r = (TResult)dsres.float_cell(i, colResult);

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

    this->timeOut = worse;

    // if there are more results per instance and algorithm
    for ( int i=0 ; i<iset_.size() ; ++i )
        for ( size_t j=0 ; j<algsettings_.size()  ; ++j )
            if (nRes[i][j]>=2)
                res_[i][j] = (TResult)(((long double)sumRes[i][j]) / ((long double)nRes[i][j]));
            else
                res_[i][j] = (TResult)(((long double)sumRes[i][j]));

    // average per instance and algorithm
    delete[] sumRes[0];
    delete[] sumRes;
    delete[] nRes[0];
    delete[] nRes;

    // computing average per instance
    std::vector< TResult > avgInst = vector<TResult>(iset_.size());
    for ( int i=0 ; (i<iset_.size()) ; ++i )
        if (nResInst[i])
            avgInst[i] = (TResult)(((long double)sumInst[i])/((long double)nResInst[i]));
        else
            avgInst[i] = worse;

    size_t nMissing = 0;
    for ( int i=0 ; (i<iset_.size()) ; ++i )
    {
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
        {
            auto r = res_[i][j];
            if (r == numeric_limits<TResult>::max())
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
                    case FMRStrategy::Value:
                        res_[i][j] = Parameters::fillMissingValue;
                        break;
                }
                nMissing++;
            }
        }
    }

    // storing original result before additional changes
    memcpy(origRes_[0], res_[0], sizeof(TResult)*(iset_.size()*algsettings_.size()));

    // second pass, checking number of timeouts per instance

    nTimeOutsInst = new int[iset_.size()];
    memset( nTimeOutsInst, 0, sizeof(int)*iset_.size());

    fill(worseInst, worseInst+iset_.size(), -DBL_MIN);

    for ( int i=0 ; (i<iset_.size()) ; ++i )
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            if (res_[i][j] == timeOut)
                ++nTimeOutsInst[i];

    if (Parameters::bestIsZero)
    {
        // storing difference from best result per instance
        vector< double > bestResInst( iset_.size(), DBL_MAX );
        for ( int i=0 ; (i<iset_.size()) ; ++i )
            for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
                bestResInst[i] = min(bestResInst[i], (double)res_[i][j]);

        for ( int i=0 ; (i<iset_.size()) ; ++i )
        {
            for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            {
                res_[i][j] = res_[i][j]-bestResInst[i];
                worseInst[i] = max(worseInst[i], res_[i][j]);
                assert(res_[i][j] >= 0.0-1e-9);
            }
        }
    }
    
    if (Parameters::normalizeResults)
    {
        // normalizing
        TResult multPrec = pow(10.0, RES_PRECISION);
        for ( int i=0 ; (i<iset_.size()) ; ++i )
        {
            for (size_t ia=0 ; (ia<algsettings_.size()) ; ++ia )
            {
                if (worse==0.0)
                    res_[i][ia] = 0.0;
                else
                    res_[i][ia] = (res_[i][ia]/worse);

                res_[i][ia] = floor((res_[i][ia] * multPrec) + 0.5) / multPrec;

                assert( res_[i][ia]>=0.0-1e-10 );
                assert( res_[i][ia]<=1.0+1e-10 );
            }
        }        
    }
    
    lowerBound = 0.0;
    
    for ( int i=0 ; (i<(int)iset_.size()) ; ++i )
    {
        auto bestI = numeric_limits<TResult>::max();
        
        for ( int ia=0 ; (ia<(int)algsettings_.size()) ; ++ia )
            bestI = min( (TResult)res_[i][ia], bestI );
        
        lowerBound += bestI;
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

    compute_rankings( algsettings_.size(), iset_.size(), (const TResult **)res_, ranks_ );

    cout << "done in " << fixed << setprecision(2) <<
            (((double)clock()-startr) / ((double)CLOCKS_PER_SEC)) << endl;
    
    avInst = new TResult[iset_.size()];
    stdDevInst_ = new TResult[iset_.size()];
    

    vector< pair< TResult, size_t > > algsByAv;
    vector< pair< TResult, size_t > > algsByAvRnk;

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
 
    for ( int i=0 ; (i<iset_.size()) ; ++i )
    {
        long double sum = 0.0;
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            sum += res_[i][j];
        avInst[i] = sum / ((long double)algsettings_.size());
    }

    for ( int i=0 ; (i<iset_.size()) ; ++i )
    {
        long double ds = 0.0;
        for ( size_t ia=0 ; (ia<algsettings_.size()) ; ++ia )
            ds += pow( avInst[i] - res_[i][ia], 2.0 );

        stdDevInst_[i] =(TResult) (ds /= ((long double) algsettings_.size()-1.0));

    }

    nRankOne = new int[algsettings_.size()];
    nLastRank = new int[algsettings_.size()];
    avAlg_ = new TResult[algsettings_.size()];
    long double *sumAlg = new long double[algsettings_.size()];

    fill(nRankOne, nRankOne+algsettings_.size(), 0);
    fill(nLastRank, nLastRank+algsettings_.size(), 0);
    fill(sumAlg, sumAlg+algsettings_.size(), 0.0);
    for ( int i=0 ; i<iset_.size(); ++i )
    {
        int lastRank = -1;
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
        {
            if (ranks_[i][j]==0)
                nRankOne[j]++;

            lastRank = max(lastRank, ranks_[i][j]);
        }

        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            if (ranks_[i][j]==lastRank)
                nLastRank[j]++;


        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            sumAlg[j] += (long double)res_[i][j];
    }

    for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
        avAlg_[j] = sumAlg[j] / (long double)iset_.size();
    
    vector< pair<int, size_t > > algsByNro;
    for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
        algsByNro.push_back( make_pair(nRankOne[j]*-1, j) );

    sort( algsByNro.begin(), algsByNro.end() );

    for (size_t i=0 ; (i<Parameters::storeTop and i<algsByNro.size()) ; ++i )
        topAlgByRnkOne.push_back(algsByNro[i].second);

    delete[] sumAlg;
}

TResult ResultsSet::get(size_t iIdx, size_t aIdx) const
{
    assert( (int)iIdx<(int)iset_.size() );
    assert( aIdx<algsettings_.size() );
    return res_[iIdx][aIdx];
}

ResultsSet::~ResultsSet ()
{
    delete avRes_;
    delete rnkRes_;

    delete[] avAlg_;
    delete[] nRankOne;
    delete[] nLastRank;
    delete[] ranks_[0];
    delete[] ranks_;
    delete[] res_[0];
    delete[] res_;
    delete[] origRes_[0];
    delete[] origRes_;
    delete[] avInst;
    delete[] stdDevInst_;
    delete[] worseInst;
    delete[] nTimeOutsInst;
}

void ResultsSet::compute_rankings( size_t nAlgs, size_t nInsts, const TResult **res, int **rank )
{
    vector< pair< TResult, size_t> > resInst = vector< pair< TResult, size_t> >(nAlgs);

    for ( size_t i=0 ; (i<nInsts) ; ++i )
    {
        for ( size_t j=0 ; (j<nAlgs) ; ++j )
            resInst[j] = make_pair( res[i][j], j );

        std::sort(resInst.begin(), resInst.end() );

        auto startValRank = resInst.begin()->first;

        int currRank = 0;
        for ( size_t j=0 ; (j<nAlgs) ; ++j )
        {
            size_t iAlg = resInst[j].second;
            const auto res = resInst[j].first;
            const double pr = fabs(res)*Parameters::rankPerc;

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
    assert((int)iIdx<(int)iset_.size());
    assert(iAlg<algsettings_.size());

    int r = ranks_[iIdx][iAlg];
    assert( r>=0 && r<((int)algsettings_.size()) );
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

void ResultsSet::save_csv( const char *fileName ) const
{
    ofstream of(fileName);

    for ( const auto &alg : algsettings_ )
        of << "," << alg;
    of << endl;

    for ( const auto &inst : iset_.instances( ))
    {
        of << inst.name();
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            of << "," << this->res(inst.idx_, j);
        of << endl;
    }

    of.close();
}

int ResultsSet::algsetting_rank( size_t idxInst, int rank ) const
{
    for ( int i=0 ; (i<(int)algsettings_.size()) ; ++i )
    {
        if (this->ranks_[idxInst][i] == rank)
            return i;
    }
    return -1;
}

void ResultsSet::saveInstanceSum(const char *fileName) const
{
    FILE *f=fopen(fileName, "w");

    fprintf(f, "instance,average,best setting,time best setting,nTimeOuts,stdDev\n");
    for ( const auto &inst : this->instances() )
    {
        int bestAS = algsetting_rank( inst.idx(), 0 );
        assert(bestAS>=0 && bestAS<(int)algsettings_.size());
        fprintf( f, "%s,%g,%s,%g,%d,%g\n", inst.name(), avInst[inst.idx()], algsettings()[bestAS].c_str(), this->res_[inst.idx()][bestAS], nTimeOutsInst[inst.idx()], stdDevInst_[inst.idx()] );
    }

    fclose(f);
}

void ResultsSet::saveAlgSummary(const char *fileName) const
{
    FILE *f=fopen(fileName, "w");
    fprintf(f, "algsetting,nBestRank,nWorseRank\n");
    for ( size_t ia=0 ; (ia<algsettings_.size()) ; ++ia )
        fprintf(f, "%s,%d,%d\n", algsettings_[ia].c_str(), nRankOne[ia], nLastRank[ia]);
    fclose(f);
}

void ResultsSet::saveFilteredDataSets(const char *featuresFile, const char *resultsFile)
{
    FILE *f=fopen(featuresFile, "w");
    fprintf(f, "instance");
    for ( const auto feat : iset_.features() )
        fprintf(f, ",%s", feat.c_str());
    fprintf(f, "\n");
    std::unordered_set< size_t > inclInstances;
    for ( int ip=0 ; ip<iset_.size(); ++ip )
    {
        if (this->stdDevInst_[ip]<=MIN_STD_DEV)
            continue;
        inclInstances.insert(ip);
        fprintf(f, "%s", iset_.instance(ip).name());
        size_t idxF=0;
        //fprintf(f, ",%g", stdDevInst(inst.idx()));
        for ( const auto feat : iset_.features() )
        {
            if (iset_.feature_is_integer(idxF))
                fprintf(f, ",%d", iset_.instance(ip).int_feature(idxF));
            else
                fprintf(f, ",%g", iset_.instance(ip).float_feature(idxF));
            ++idxF;
        }
        fprintf(f, "\n");
    }
    fclose(f);

    // filtering algorithms
    unordered_set< size_t > inclAlgs;
    for ( size_t ia=0 ; (ia<algsettings_.size()) ; ++ia )
    {
        if ( nLastRank[ia] >= ((double)algsettings_.size())*0.98 )
            continue;
        inclAlgs.insert(ia);
    }

    f=fopen(resultsFile, "w");
    for ( size_t idxInst : inclInstances )
    {
        for ( size_t ia : inclAlgs )
            fprintf(f, "%s,%s,%g\n", iset_.instance(idxInst).name(), algsettings_[ia].c_str(), this->res(idxInst, ia) );
    }

    fclose(f);

}

double ResultsSet::origRes(size_t iIdx, size_t iAlg) const
{
    return origRes_[iIdx][iAlg];
}

double ResultsSet::res(size_t iIdx, size_t iAlg) const
{
    switch (Parameters::eval)
    {
    case Average:
        return this->get(iIdx, iAlg);
        break;
    case Rank:
        return this->rank(iIdx, iAlg);
        break;
    }

    return 0.0;
}
