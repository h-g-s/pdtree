/*
 * InstanceSet.hpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include "Dataset.hpp"
#include "Instance.hpp"

#ifndef INSTANCESET_HPP_
#define INSTANCESET_HPP_

class InstanceSet
{
public:
    InstanceSet (const char *fileName, const char *resultsFileName = nullptr);

    /* instance by index */
    const Instance &instance( size_t idx ) const;

    /* queries if a instance is stored */
    bool has(const std::string &iname) const;

    /* instance by name */
    const Instance &inst_by_name( const std::string &name ) const;

    /* features of each instance */
    const std::vector< std::string > &features() const;

    /* type of each feature */
    const std::vector< Datatype > &types() const;

    size_t size() const;

    virtual ~InstanceSet ();
private:
    Dataset inst_dataset_;
    std::vector<std::string> features_;
    std::vector<Instance> instances_;
    std::vector<Datatype> types_;
    std::unordered_map< std::string, size_t > instByName_;
};

#endif /* INSTANCESET_HPP_ */
