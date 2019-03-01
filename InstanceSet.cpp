/*
 * InstanceSet.cpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include "InstanceSet.hpp"
#include <iostream>

using namespace std;

InstanceSet::InstanceSet (const char *fileName, const char *resultsFileName) :
    inst_dataset_(fileName),
    features_( vector<string>( inst_dataset_.headers().begin()++, inst_dataset_.headers().end() ) ),
    instances_(vector< Instance >(inst_dataset_.rows(), Instance(0) )),
    types_(vector<Datatype>(inst_dataset_.types().begin()++, inst_dataset_.types().end()))
{
    for ( size_t i=0 ; (i<instances_.size()) ; ++i )
    {
        instances_[i].idx_ = i;
        const auto iname = string(instances_[i].name());
        auto it = instByName_.find(iname);
        if (it!=instByName_.end())
        {
            cerr << "instance " << iname << " appears twice in the instance list" << endl;
            abort();
        }
        instByName_[iname] = i;
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

