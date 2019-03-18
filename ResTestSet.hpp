/*
 * ResTestSet.hpp
 *
 *  Created on: 18 de mar de 2019
 *      Author: haroldo
 */

#ifndef RESTESTSET_HPP_
#define RESTESTSET_HPP_

#include <unordered_map>
#include <string>

class ResTestSet
{
public:
    ResTestSet(
            const std::unordered_map<std::string, size_t> &_instances,
            const std::unordered_map<std::string, size_t> &_algsettings,
            const char *fileName
    );

    float get( size_t idxInst, size_t idxAlgSetting ) const;

    virtual ~ResTestSet ();
private:
    float **res_;
};

#endif /* RESTESTSET_HPP_ */
