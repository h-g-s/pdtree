/*
 * Instance.hpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_

#include <cstddef>
#include <string>
#include <vector>

class Dataset;

class Instance
{
public:
    Instance(size_t _idx) :
        idx_(_idx) {}

    const char *name() const;

    size_t idx() const { return idx_; }

    int int_feature( size_t idxFeature ) const;

    double float_feature( size_t idxFeature ) const;

    const char *str_feature( size_t idxFeature ) const;

    virtual ~Instance();

    size_t idx_;

    static std::vector< std::string > features;
    static Dataset *inst_dataset;
private:
};

#endif /* INSTANCE_HPP_ */
