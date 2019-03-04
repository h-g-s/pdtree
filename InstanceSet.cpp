/*
 * InstanceSet.cpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include "InstanceSet.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <ctime>
#include <unordered_set>

using namespace std;

static std::unordered_set< std::string > check_instances_with_results( const char *resFN );

InstanceSet::InstanceSet (const char *fileName, const char *resultsFileName) :
    inst_dataset_(fileName),
    features_( vector<string>( inst_dataset_.headers().begin()++, inst_dataset_.headers().end() ) ),
    instances_(vector< Instance >(inst_dataset_.rows(), Instance(0) )),
    types_(vector<Datatype>(inst_dataset_.types().begin()++, inst_dataset_.types().end()))
{
    clock_t start = clock();
    auto ires = check_instances_with_results(resultsFileName);

    size_t discarded = 0;
    for ( size_t i=0 ; (i<instances_.size()) ; ++i )
    {
        instances_[i].idx_ = i;
        const auto iname = string(instances_[i].name());
        if (ires.find(iname)==ires.end())
        {
            ++discarded;
            continue;
        }
        auto it = instByName_.find(iname);
        if (it!=instByName_.end())
            throw string("instance " + iname + " appears twice in the instance list");
        instByName_[iname] = i;
    }
    double secs = (double(clock()-start)) / ((double)CLOCKS_PER_SEC);
    if (discarded)
    {
        cout << instances_.size() << " instances loaded in " << setprecision(3) << secs
             << discarded << " were discarded because no experiments were performed with them" << endl;
    }
    else
    {
        cout << instances_.size() << " instances loaded in " << setprecision(3) << secs << endl;
    }
}

size_t InstanceSet::size() const
{
    return inst_dataset_.rows();
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

bool InstanceSet::has(const std::string &iname) const
{
    auto it = instByName_.find(iname);
    if (it==instByName_.end())
        return false;

    return true;
}

