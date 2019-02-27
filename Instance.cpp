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


Instance::~Instance ()
{
}

