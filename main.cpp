#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <cmath>
#include <quadmath.h>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <unordered_set>
#include "Dataset.hpp"
#include "Instance.hpp"
#include "InstanceSet.hpp"

using namespace std;

/*
class StrategyResults
{
public:
    StrategyResults() :
        sumDelta(0.0),
        average(0.0),
        nFeas(0),
        nInfeas(0),
        count(0) {}

    const StrategyResults &operator = (const StrategyResults &other) {
        this->sumDelta = other.sumDelta;
        this->average = other.average;
        this->nFeas = other.nFeas;
        this->nInfeas = other.nInfeas;
        this->count = other.count;

        return *this;
    }

    void computeAverage() {
        const __float128 av = this->sumDelta / ((__float128) this->count);
        this->average = (double) av;
    }

    void addResult(const double &value) {
        if (fabs(value + 1e+8)<1e-6)
            this->nFeas++;

        this->sumDelta += ((__float128)value);
        this->count++;
    }

    __float128 sumDelta;
    double average;
    size_t nFeas;
    size_t nInfeas;
    size_t count;
}; */


int main(int argc, char **argv)
{
    if (argc<3)
    {
        cerr << "usage: ddtre instances.csv algresults.csv" << endl;
        exit(1);
    }

    // loading instance data
    InstanceSet iset( argv[1] );

    // loading results
    {
        Dataset algRes(argv[2]);

        // checking different algorithms/parameter settings
        unordered_set<string> algs;
        for ( size_t i=0 ; (i<algRes.rows()) ; ++i )
            algs.insert( algRes.str_cell(i, 0));
    }



    /*
    Dataset relax(argv[2]);

    map<string, map<string, StrategyResults > > resInstSt;
    const size_t colSt = relax.headers().size()-2;
    const size_t colDelta = relax.headers().size()-1;
    unordered_set< string > algorithms;

    map< string , StrategyResults > resSt;

    for (size_t i=0 ; (i<relax.rows()) ; ++i)
    {

        string iname = string(relax.str_cell(i, 0));
        string stname = string(relax.str_cell(i, colSt));
        double delta = relax.float_cell(i, colDelta);
        auto itI = resInstSt.find(iname);
        algorithms.insert(stname);
        if (itI==resInstSt.end())
        {
            StrategyResults stres;
            stres.addResult(delta);
            resInstSt[iname][stname] = stres;
        }
        else
        {
            auto itS = itI->second.find(stname);
            if (itS==itI->second.end())
            {
                StrategyResults stres;
                stres.addResult(delta);
                itI->second[stname] = stres;
            }
            else
            {
                itS->second.addResult(delta);
            }
        }

        {
            auto itS = resSt.find(stname);
            if (itS==resSt.end())
            {
                StrategyResults stres;
                stres.addResult(delta);
                resSt[stname] = stres;

            }
            else
            {
                itS->second.addResult(delta);
            }
        }
    }

    // checking if all instances and strategies
    // appear in the result dataset.
    size_t instWR = 0;
    size_t instStWR = 0;
    for ( const auto &inst : instances )
    {
        auto iti = resInstSt.find( string(inst.name()) );
        if (iti==resInstSt.end())
        {
            instWR++;
            continue;
        }
        for ( const auto &alg : algorithms )
        {
            auto its = iti->second.find(alg);
            if (its==iti->second.end())
            {
                instStWR++;
            }
        }
    }

    if (instWR)
    {
        cout << instWR << " instances without any results in the dataset" << endl;
    }
    else
    {
        cout << "all instances have results" << endl;
    }

    if (instStWR)
    {
        cout << instStWR << " instances x strategies (from a total of " << instances.size()*algorithms.size() << ") without any results in the dataset" << endl;
    }
    else
    {
        cout << "all instances x strategies have at least one result" << endl;
    }

    size_t minSt = numeric_limits<size_t>::max(), maxSt = 0;
    size_t nISt = 0, totSamp = 0;
    for ( const auto &itI : resInstSt )
    {
        for ( const auto &stI : itI.second )
        {
            totSamp += stI.second.count;
            minSt = min(minSt, stI.second.count);
            maxSt = max(maxSt, stI.second.count);
            ++nISt;
        }
    }

    cout << "average samples per instance and strategy: " <<
         ((double)totSamp / ((double) instances.size()*algorithms.size()) ) <<
         " min " << minSt << " max " << maxSt << endl;

    // sorted strategy results
    vector< pair< __float128, string > > sstr;

    for ( auto &stRes : resSt )
    {
        stRes.second.computeAverage();
        sstr.push_back( make_pair( stRes.second.average, stRes.first ) );
    }

    std::sort(sstr.begin(), sstr.end());

    {
        ofstream ofs("strAvg.csv");
        for (auto it=sstr.begin() ; (it!=sstr.end()) ; ++it )
            ofs << it->second << "," << (long double)it->first << endl;
        ofs.close();
    }

    {
        sstr.clear();
        for ( auto &stRes : resSt )
        {
            stRes.second.computeAverage();
            sstr.push_back( make_pair( stRes.second.nFeas, stRes.first ) );
        }
        std::sort(sstr.begin(), sstr.end());

        ofstream ofs("strFeas.csv");
        for (auto it=sstr.begin() ; (it!=sstr.end()) ; ++it )
            ofs << it->second << "," << (long double)it->first << endl;
        ofs.close();
    }

    {
        ofstream of("diving-inst-st.csv");
        for ( auto &itI : resInstSt )
        {
            for ( auto &stI : itI.second )
            {
                stI.second.computeAverage();


            }
        }
        of.close();
    }
*/
    return 1;
}
