/*
 * InstanceSet.hpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include <cstddef>
#include <string>
#include <map>
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

    int size() const;

    double norm_feature_val( size_t idxInst, size_t idxF ) const;

    double norm_feature_val_rank( size_t idxInst, size_t idxF ) const;

    double value_by_norm_val_rank( size_t idxF, const double nv ) const;

    // number branching possibilities for a feature
    int rankingsFeature( size_t idxF ) const {
        return rankingsF_[idxF];
    }

    int nElementsFeatRank( size_t idxFeature, size_t rank ) const {
        return nElementsFeatRank_[idxFeature][rank];
    }

    int nValidBranchingsFeature( size_t idxF ) const {
        return nValidBF_[idxF];
    }

    virtual ~InstanceSet ();

    void save(const char *fileName, bool normalized = true) const;

    void saveNormRank(const char *fileName) const;

    // training dataset
    Dataset *inst_dataset_;

    Dataset *test_dataset_;
private:
    std::vector<std::string> features_;
    std::vector<Instance> instances_;
    std::vector<Datatype> types_;
    std::unordered_map< std::string, size_t > instByName_;

    // valid branchings per feature
    std::vector< int > nValidBF_;

    // normalized value feature rank
    std::vector< std::map< double, int > > featureValRank;
    std::vector< std::map< int, double > > featureRankVal;

    // elements by feature and rank
    std::vector< std::vector< int > > nElementsFeatRank_;

    std::vector< int > rankingsF_;

    int **instFeatRank; // per instance

    std::vector< std::pair<double, double> > limitsFeature;
};

#endif /* INSTANCESET_HPP_ */
