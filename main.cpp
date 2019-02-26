#include <iostream>
#include "Dataset.hpp"
#include <unordered_map>
#include <map>
#include <quadmath.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

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

    __float128 sumDelta;
    double average;
    size_t nFeas;
    size_t nInfeas;
    size_t count;
};


int main(int argc, char **argv)
{
    if (argc<3)
    {
        cerr << "usage: ddtre instances.csv relaxations.csv" << endl;
        exit(1);
    }
    Dataset inst(argv[1]);

    Dataset relax(argv[2]);

    map<string, map<string, pair<__float128, long int> > > resInstSt;
    const size_t colSt = relax.headers().size()-2;
    const size_t colDelta = relax.headers().size()-1;

    map< string , StrategyResults > resSt;


    for (size_t i=0 ; (i<relax.rows()) ; ++i)
    {

        string iname = string(relax.str_cell(i, 0));
        string stname = string(relax.str_cell(i, colSt));
        __float128 delta = relax.float_cell(i, colDelta);
        auto itI = resInstSt.find(iname);
        if (itI==resInstSt.end())
        {
            resInstSt[iname][stname] = make_pair( delta, (long int) 1 );
        }
        else
        {
            auto itS = itI->second.find(stname);
            if (itS==itI->second.end())
            {
                itI->second[stname] = make_pair( delta, (long int) 1 );
            }
            else
            {
                __float128 cdelta = itS->second.first;
                long int ccount = itS->second.second;

                itS->second = make_pair(cdelta + delta, ccount+1);
            }
        }

        {
            auto itS = resSt.find(stname);
            if (itS==resSt.end())
            {
                resSt[stname] = make_pair(delta, (long int)1);

            }
            else
            {
                __float128 cdelta = itS->second.first;
                long int ccount = itS->second.second;
                itS->second = make_pair(cdelta + delta, ccount+1);
            }
        }
    }

    // sorted strategy results
    vector< pair< __float128, string > > sstr;

    for ( auto it=resSt.begin() ; (it!=resSt.end()) ; ++it )
        sstr.push_back( make_pair(it->second.first /  (__float128)it->second.second, it->first) );

    std::sort(sstr.begin(), sstr.end());

    cout << "overall strategy results, ranked from the best to the worse: " << endl;
    for (auto it=sstr.begin() ; (it!=sstr.end()) ; ++it )
        cout << it->second << "," << (long double)it->first << endl;

    return 1;
}
