/*
 * InstanceSet.cpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include "InstanceSet.hpp"
#include <fstream>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <algorithm>
#include <ctime>
#include <unordered_set>

using namespace std;

static std::unordered_set< std::string > check_instances_with_results( const char *resFN );

InstanceSet::InstanceSet (const char *fileName, const char *resultsFileName, int ifold, int kfold ) :
    inst_dataset_(new Dataset(fileName)),
    test_dataset_(nullptr)
{
    if (kfold>=2)
    {
        if (ifold<0 or ifold>=kfold)
        {
            cerr << "invalid training partition " << ifold << endl;
            exit(1);
        }
    }
    clock_t start = clock();
    auto ires = check_instances_with_results(resultsFileName);
    {
        vector< bool > included;
        vector< bool > inclTest;
        vector< size_t > elements;
        included.reserve(inst_dataset_->rows());
        size_t nel = 0;
        for ( size_t i=0 ; (i<inst_dataset_->rows()) ; ++i )
        {
            string iname = string(inst_dataset_->str_cell(i, 0));
            bool hasResult = ires.find(iname)!=ires.end();
            included.push_back( hasResult );
            if (hasResult)
            {
                ++nel;
                elements.push_back(i);
            }
        }
        if ( kfold>=2 )
        {
            if ((int)nel<kfold)
            {
                cerr << "cannot perform kfold (k=" << kfold 
                    << ") validation since there are results only for " 
                    << ires.size() << "instances." << endl;
                exit(1);
            }

            // removing elements from test dataset
            size_t tsize = floor(((double)elements.size()) * (((double)1.0)/((double)kfold)));
            size_t tstart = tsize * ifold;
            size_t tend = tstart + tsize;
            for ( size_t i=tstart ; (i<tend) ; ++i )
                included[elements[i]] = false;

            inclTest = vector< bool >( included.size(), false );
            for ( size_t i=tstart ; (i<tend) ; ++i )
                inclTest[elements[i]] = true;
        }

        Dataset *ds = new Dataset(*inst_dataset_, included);
        if (kfold>=2)
            test_dataset_ = new Dataset( *inst_dataset_, inclTest );

        delete inst_dataset_;
        inst_dataset_ = ds;
    }

    auto itf = inst_dataset_->headers().begin(); ++itf;
    features_ = vector<string>(itf, inst_dataset_->headers().end() );
    auto itt = inst_dataset_->types().begin(); ++itt;
    types_ = vector<Datatype>(itt, inst_dataset_->types().end());
    assert(features_.size()==types_.size());

    instances_.reserve(ires.size());

    size_t discarded = 0;
    size_t idxInst = 0;
    for ( size_t i=0 ; (i<inst_dataset_->rows()) ; ++i )
    {
        string iname = string(inst_dataset_->str_cell(i, 0));
        if (ires.find(iname)==ires.end())
        {
            ++discarded;
            continue;
        }
        instances_.push_back( Instance(idxInst) );
        auto it = instByName_.find(iname);
        if (it!=instByName_.end())
            throw string("instance " + iname + " appears twice in the instance list");
        instByName_[iname] = idxInst;
        ++idxInst;
    }
    double secs = (double(clock()-start)) / ((double)CLOCKS_PER_SEC);
    cout << instances_.size() << " instances loaded in " << setprecision(3) << secs << endl;
    if (discarded)
        cout << discarded << " instances were discarded because no experiments were performed with them" << endl;
}

size_t InstanceSet::size() const
{
    return inst_dataset_->rows();
}

const Instance &InstanceSet::inst_by_name( const std::string &name ) const
{
    auto it = instByName_.find(name);
    if (it == instByName_.end())
        throw string("instance ") + name + string(" not found");

    return instances_[it->second];
}

const Instance &InstanceSet::instance( size_t idx ) const
{
    return instances_[idx];
}

const std::vector<std::string>&
InstanceSet::features () const
{
    return this->features_;
}

const std::vector<Datatype>&
InstanceSet::types () const
{
    return this->types_;
}

InstanceSet::~InstanceSet ()
{
    if (inst_dataset_)
        delete inst_dataset_;

    if (test_dataset_)
        delete test_dataset_;
}

static string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}


static std::unordered_set< std::string > check_instances_with_results( const char *resFN )
{
    unordered_set< string > res;

    if (resFN==nullptr)
        return res;

    ifstream f(resFN);
    string line;
    while (getline(f, line))
    {
        line = trim(line);
        if (line.size()==0)
            continue;
        auto it = line.find(',');
        if (it==line.size())
            throw "Results file should have the following format: instance,algAndSettings,result";

        auto iname = line.substr(0, it);
        if (iname.size()==0)
            throw "Empty instance name";
        res.insert(iname);
    }
    f.close();

    return res;
}

bool InstanceSet::feature_is_integer( size_t idxF ) const
{
    assert( idxF<features().size() );

    return (types_[idxF] <= Integer);
}

bool InstanceSet::feature_is_float( size_t idxF ) const
{
    assert( idxF<features().size() );

    return (types_[idxF] == Float);
}


bool InstanceSet::has(const std::string &iname) const
{
    auto it = instByName_.find(iname);
    if (it==instByName_.end())
        return false;

    return true;
}

