/*
 * InstanceSet.hpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "Dataset.hpp"
#include "Instance.hpp"

#ifndef INSTANCESET_HPP_
#define INSTANCESET_HPP_

class InstanceSet
{
public:
    // constructs an instance set considering instances from fileName which
    // save some experimental result in resultsFileName
    // if ifold and kfold are informed (k>=2), then the i-th training subset
    // from a k-fold validation is built
    InstanceSet (const char *fileName, const char *resultsFileName = nullptr, int ifold = -1, int kfold=-1 );
    
    /* instance by index */
    const Instance &instance( size_t idx ) const;

    /* queries if a instance is stored */
    bool has(const std::string &iname) const;

    /* vector of instances */
    const std::vector< Instance > &instances() const {
        return instances_;
    }

    /* instance by name */
    const Instance &inst_by_name( const std::string &name ) const;

    /* features of each instance */
    const std::vector< std::string > &features() const;

    /* type of each feature */
    const std::vector< Datatype > &types() const;

    // if feature is of some integral type
    bool feature_is_integer( size_t idxF ) const;

    // if feature is of some integral type
    bool feature_is_float( size_t idxF ) const;

    size_t size() const;

    virtual ~InstanceSet ();

    // training dataset
    Dataset *inst_dataset_;

    Dataset *test_dataset_;
private:
    std::vector<std::string> features_;
    std::vector<Instance> instances_;
    std::vector<Datatype> types_;
    std::unordered_map< std::string, size_t > instByName_;
};

#endif /* INSTANCESET_HPP_ */
