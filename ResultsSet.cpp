/*
 * ResultsSet.cpp
 *
 *  Created on: 1 de mar de 2019
 *      Author: haroldo
 */

#include <limits>
#include <cmath>
#include <ctime>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <algorithm>
#include "ResultsSet.hpp"

using namespace std;

// default values
enum FMRStrategy ResultsSet::fmrStrategy = WorseInstT2;
enum Evaluation ResultsSet::eval = Average;
double ResultsSet::rankEps = 1e-8;
double ResultsSet::rankPerc = 0.01;
size_t ResultsSet::storeTop = 5;

static char FMRStrategyStr[5][16] = {
    "Worse",
    "WorseT2",
    "WorseInst",
    "WorseInstT2",
    "AverageInst" };

static char EvaluationStr[2][16] =
{
    "Average",
    "Rank"
};

static enum FMRStrategy to_fmrs( const char *s);

static enum Evaluation to_eval( const char *s);

void ResultsSet::configure_parameters(int argc, const char **argv)
{
    for ( int i=3 ; (i<argc) ; ++i )
    {
        if (argv[i][0]!='-')
            continue;

        char paramStr[256], pName[256], pValue[256];
        strcpy(paramStr, argv[i]);

        char *sp = strstr(paramStr, "=");
        if (sp==nullptr)
            throw "To set parameters use -parameter=value";

        ++sp;
        strcpy(pValue, sp);

        --sp; *sp = '\0';
        strcpy(pName, paramStr);
        char *s = pName;
        while (*s != '\0')
        {
            *s = tolower(*s);
            ++s;
        }

        s = pValue;
        while (*s != '\0')
        {
            *s = tolower(*s);
            ++s;
        }


        if (strcmp(pName, "-fmrs")==0)
        {
            ResultsSet::fmrStrategy = to_fmrs(pValue);
            continue;
        }
        if (strcmp(pName, "-eval")==0)
        {
            ResultsSet::eval = to_eval(pValue);
            continue;
        }
        if (strcmp(pName, "-rankEps")==0)
        {
            ResultsSet::rankEps = stod(string(pValue));
            continue;
        }
        if (strcmp(pName, "-rankPerc")==0)
        {
            ResultsSet::rankEps = stod(string(pValue));
            continue;
        }
    }
}

ResultsSet::ResultsSet( const InstanceSet &_iset, const char *fileName, const enum FMRStrategy _fmrs ) :
    iset_(_iset),
    res_(nullptr),
    ranks_(nullptr),
    fmrs_(_fmrs),
    avAlg(nullptr),
    avRnkAlg(nullptr),
    avInst(nullptr)
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
        for ( size_t j=1 ; (j<dsres.headers().size()-2) ; ++j )
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

    size_t ir = 0;
    bool msgITwice = false;
    for ( size_t i=0 ; (i<nRows) ; ++i )
    {
        string iname = string(dsres.str_cell(i, 0));
        if (not _iset.has(iname))
            continue;
        if (res_[iIdx[ir]][aIdx[ir]] != numeric_limits<float>::max() and (not msgITwice))
        {
            cout << "warning: result for instance " << iset_.instance(iIdx[ir]).name() <<
              " and parameter setting " << algsettings_[aIdx[ir]] <<
              " appear twice in the results file, disable this message for the next repeated entries. " << endl ;
            msgITwice = true;
            continue;
        }

        const float r = (float)dsres.float_cell(i, colResult);

        res_[iIdx[ir]][aIdx[ir]] = r;
        worse = max( worse, r );
        worseInst[iIdx[ir]] = max( worseInst[iIdx[ir]], r );
        sumInst[iIdx[ir]] += ((long double)r);
        ++nResInst[iIdx[ir]];

        ++ir;
    }

    // computing average per instance
    std::vector< float > avgInst = vector<float>(iset_.size());
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

    double secs = ((double)(clock()-start)) / ((double)CLOCKS_PER_SEC);
    cout << ir << " results loaded in " << setprecision(3) << secs << " seconds" << endl;

    cout << "Computing ranking and summarized results ... ";

    compute_rankings();

    avAlg = new float[algsettings_.size()];
    avRnkAlg  = new float[algsettings_.size()];
    avInst = new float[iset_.size()];

    vector< pair< float, size_t > > algsByAv;
    vector< pair< float, size_t > > algsByAvRnk;

    for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
    {
        long double sum = 0.0;
        long double sumRk = 0.0;

        for ( size_t i=0 ; (i<iset_.size()) ; ++i )
        {
            sum += res_[i][j];
            sumRk += ranks_[i][j];
        }

        avAlg[j] = ((long double)sum) / ((long double)iset_.size());
        algsByAv.push_back( make_pair(avAlg[j], j) );
        avRnkAlg[j] = ((long double)sumRk) / ((long double)iset_.size());
        algsByAvRnk.push_back( make_pair(avRnkAlg[j], j) );
    }

    sort( algsByAv.begin(), algsByAv.end() );
    for ( size_t i=0 ; (i<algsByAv.size() and i<storeTop) ; ++i )
        topAlgByAv.push_back(algsByAv[i].second);

    sort( algsByAvRnk.begin(), algsByAvRnk.end() );
    for ( size_t i=0 ; (i<algsByAvRnk.size() and i<storeTop) ; ++i )
        topAlgByAvRnk.push_back(algsByAvRnk[i].second);
 
    for ( size_t i=0 ; (i<iset_.size()) ; ++i )
    {
        long double sum = 0.0;
        for ( size_t j=0 ; (j<algsettings_.size()) ; ++j )
            sum += res_[i][j];
        avInst[i] = sum / ((long double)algsettings_.size());
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
    delete[] avAlg;
    delete[] avRnkAlg;
    delete[] avInst;
}

static enum FMRStrategy to_fmrs( const char *s ) 
{
    char slc[256];
    for ( size_t i=0 ; i<5 ; ++i )
    {
        strcpy(slc, FMRStrategyStr[i]);
        char *st = slc;
        while (*st != '\0')
        {
            *st = tolower(*st);
            ++st;
        }

        if (strcmp(slc, s)==0)
            return (FMRStrategy)i;
    }

    throw "Fill missing results strategy invalid: " + string(s);

    return FMRStrategy::AverageInst;
}


static enum Evaluation to_eval( const char *s)
{
    char slc[256];
    for ( size_t i=0 ; i<2 ; ++i )
    {
        strcpy(slc, EvaluationStr[i]);
        char *st = slc;
        while (*st != '\0')
        {
            *st = tolower(*st);
            ++st;
        }

        if (strcmp(slc, s)==0)
            return (Evaluation)i;
    }

    throw "Invalid evaluation criterion: " + string(s);

    return Evaluation::Rank;
}


void ResultsSet::help()
{
    cout << "\t-fmrs=[Worse, WorseT2, WorseInst, WorseInstT2, AverageInst]" << endl;
    cout << "\t-eval=[Average, Rank]" << endl;
}

void ResultsSet::print_config()
{
    cout << "      fmrs=" << FMRStrategyStr[ResultsSet::fmrStrategy] << endl;
    cout << "      eval=" << EvaluationStr[ResultsSet::eval] << endl;
    cout << "   rankEps=" << scientific << rankEps << endl;
    cout << "  rankPerc=" << fixed << setprecision(4) << rankPerc << endl;
}

void ResultsSet::compute_rankings()
{
    size_t nAlgs = algsettings_.size();
    vector< pair< float, size_t> > resInst = vector< pair< float, size_t> >(nAlgs);

    for ( size_t i=0 ; (i<iset_.size()) ; ++i )
    {
        for ( size_t j=0 ; (j<nAlgs) ; ++j )
            resInst[j] = make_pair( res_[i][j], j );

        std::sort(resInst.begin(), resInst.end() );

        float startValRank = resInst.begin()->first;

        int currRank = 0;
        for ( size_t j=0 ; (j<nAlgs) ; ++j )
        {
            size_t iAlg = resInst[j].second;
            const float res = resInst[j].first;
            const double pr = fabsf(res)*rankPerc;

            if ((res>=startValRank+rankEps) and (res>=startValRank+pr))
            {
                ++currRank;
                ranks_[i][iAlg] = currRank;
                startValRank = res;
            }
            else
                ranks_[i][iAlg] = currRank;
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
    cout << "Top algorithms/parameter settings considering the whole instance set: " << endl;
    cout << "Average results                              Rank based results" << endl;
    cout << " # algorithm/p. setting                      res          algorithm/p. setting                      res" << endl;
    cout << "== ========================================= ============ ========================================= ============" << endl;
    for ( size_t i=0 ; (i<topAlgByAv.size()) ; ++i )
    {
        cout << setw(2) << right << i+1 << " " <<
                setw(41) << left << algsettings_[topAlgByAv[i]] << " " <<
                setw(12) << defaultfloat << right << avAlg[topAlgByAv[i]] << " " <<
                setw(41) << left << algsettings_[topAlgByAvRnk[i]] << " " <<
                setw(12) << setprecision(3) << fixed << right << avRnkAlg[topAlgByAvRnk[i]] << endl;
    }
    cout << endl;
}

