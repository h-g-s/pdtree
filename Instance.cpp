/*
 * Instance.cpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include "Instance.hpp"

#include "Dataset.hpp"

using namespace std;

Dataset *Instance::inst_dataset = nullptr;
vector< string > Instance::features;

const char *Instance::name() const
{
    return inst_dataset->str_cell(this->idx_, 0);
}

int Instance::int_feature( size_t idxFeature ) const
{
    return Instance::inst_dataset->int_cell(this->idx_, (size_t) idxFeature+1);
}

double Instance::float_feature( size_t idxFeature ) const
{
    return Instance::inst_dataset->float_cell(this->idx_, (size_t) idxFeature+1);
}

const char *Instance::str_feature( size_t idxFeature ) const
{
    return Instance::inst_dataset->str_cell(this->idx_, (size_t) idxFeature+1);
}



Instance::~Instance ()
{
}

