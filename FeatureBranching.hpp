/*
 * FeatureBranching.hpp
 *
 *  Created on: 25 de fev de 2019
 *      Author: haroldo
 */

#ifndef FEATUREBRANCHING_HPP_
#define FEATUREBRANCHING_HPP_

#include <unordered_map>
#include "InstanceSet.hpp"
#include <utility>
#include <map>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;

template<typename T> class FeatureBranching {
public:
    FeatureBranching(const InstanceSet& _iset, // complete instance set
                     size_t _idxF, // feature where branching will be evaluated
                     std::vector< size_t > *_elements=nullptr, // subset of instances (nullptr if all)
                     size_t _minInstancesChild = 10,
                     size_t _maxEvBranches = 11
                     );

    virtual ~FeatureBranching ();
private:
    // sorted values and their positions in the dataset
    std::vector<std::pair<T, size_t>> sortedValues;

    void fillAndSortValues();
    void computeOcurrency(std::vector< pair<T, size_t> > &diffValues);
    void computeBranchValues(std::vector< pair<T, size_t> > &diffValues);

    const InstanceSet &iset_;
    const size_t idxF_;
    std::vector< size_t > *elements_;

    // elements in each branch (usually two branches)
    std::vector< std::vector< size_t > > branchElements;


    size_t minInstancesChild_;
    size_t maxEvBranches_;

    // branching values and starting
    // positions in sortedValues vector
    std::vector< std::pair<T, size_t> > branchingV;
};

template class FeatureBranching<int>;
template class FeatureBranching<double>;
template class FeatureBranching<const char*>;

template <typename T>
FeatureBranching<T>::FeatureBranching(const InstanceSet& _iset, // complete instance set
                                      size_t _idxF, // feature where branching will be evaluated
                                      std::vector< size_t > *_elements, // subset of instances (nullptr if all)
                                      size_t _minInstancesChild,
                                      size_t _maxEvBranches
                                      ) :
    iset_(_iset),
    idxF_(_idxF),
    elements_(_elements),
    branchElements( std::vector<std::vector<size_t>>(2, vector<size_t>()) ),
    minInstancesChild_(_minInstancesChild),
    maxEvBranches_(_maxEvBranches)
{
    {
        // different values that can be used for branching
        // value, occurrences
        fillAndSortValues();

        // vector in the form: value, number of occurrences
        std::vector< std::pair<T, size_t> > diffValues;
        computeOcurrency(diffValues);
        computeBranchValues(diffValues);
    }


}

template <>
void FeatureBranching<int>::fillAndSortValues()
{
    this->sortedValues.clear();
    if (this->elements_==nullptr)
    {
        for ( const auto &inst : iset_.instances())
            sortedValues.push_back( make_pair( inst.int_feature(idxF_), inst.idx() ));
    }
    else
    {
        const auto parElements = (*this->elements_);
        for ( const auto &el : parElements)
        {
            const Instance &inst = iset_.instances()[el];
            sortedValues.push_back(make_pair( inst.int_feature(idxF_), inst.idx() ));
        }
    }

    std::sort(sortedValues.begin(), sortedValues.end());
}

template <>
void FeatureBranching<double>::fillAndSortValues()
{
    this->sortedValues.clear();
    if (this->elements_==nullptr)
    {
        for ( const auto &inst : iset_.instances())
            sortedValues.push_back( make_pair( inst.float_feature(idxF_), inst.idx() ));
    }
    else
    {
        const auto parElements = (*this->elements_);
        for ( const auto &el : parElements)
        {
            const Instance &inst = iset_.instances()[el];
            sortedValues.push_back(make_pair( inst.float_feature(idxF_), inst.idx() ));
        }
    }

    std::sort(sortedValues.begin(), sortedValues.end());
}

bool compare_str( const pair<const char *, size_t > &v1, const pair<const char *, size_t > &v2  )
{
    int r = strcmp(v1.first, v2.first);
    return (r<0);
}

template <>
void FeatureBranching<const char *>::fillAndSortValues()
{
    this->sortedValues.clear();
    if (this->elements_==nullptr)
    {
        for ( const auto &inst : iset_.instances())
            sortedValues.push_back( make_pair( inst.str_feature(idxF_), inst.idx() ));
    }
    else
    {
        const auto parElements = (*this->elements_);
        for ( const auto &el : parElements)
        {
            const Instance &inst = iset_.instances()[el];
            sortedValues.push_back(make_pair( inst.str_feature(idxF_), inst.idx() ));
        }
    }

    std::sort(sortedValues.begin(), sortedValues.end(), compare_str);
}

template <typename T>
void FeatureBranching<T>::computeOcurrency( std::vector< pair<T, size_t> > &diffValues )
{


    std::unordered_map<T, size_t> occurrences;

    auto p = sortedValues.begin();
    while (p!=sortedValues.end())
    {
        T v = p->first;
        size_t n = 0;
        for ( ; (p!=sortedValues.end()&&v==p->first) ; ++p )
            ++n;
        if (n)
            occurrences[v] = n;

    }
    diffValues.clear();
    for (const auto &it : occurrences )
        diffValues.push_back(make_pair(it.first, it.second));

    std::sort(diffValues.begin(), diffValues.end());
}

struct StrComparator : public std::binary_function<const char *, const char *, bool>
{
    bool operator()(const char *s1, const char *s2) const
    {
        return (strcmp(s1,s2) < 0);
    }
};

template <>
void FeatureBranching<const char *>::computeOcurrency(std::vector< pair<const char*, size_t> > &diffValues)
{
    map<const char *, size_t, StrComparator> occurrences;

    auto p = sortedValues.begin();
    while (p!=sortedValues.end())
    {
        const char *v = p->first;
        size_t n = 0;
        for ( ; (p!=sortedValues.end()&&(strcmp(v,p->first)==0)) ; ++p )
            ++n;
        if (n)
            occurrences[v] = n;

    }
    diffValues.clear();
    for (const auto &it : occurrences )
        diffValues.push_back(make_pair(it.first, it.second));

    std::sort(diffValues.begin(), diffValues.end(), compare_str);
}

template <typename T>
void FeatureBranching<T>::computeBranchValues( std::vector< pair<T, size_t> > &diffValues )
{
    if (diffValues.size()<=this->maxEvBranches_)
    {
        size_t p = 0;
        for ( const auto &v : diffValues)
        {
            size_t cEl = p + v.second;
            if (cEl>=this->minInstancesChild_ and ((int)this->sortedValues.size())-((int)cEl)>=((int)this->minInstancesChild_))
                this->branchingV.push_back(make_pair(v.first, p));

            p += v.second;
        }
        return;
    }

    for (size_t ib=0 ; (ib<this->maxEvBranches_); ++ib)
    {
        size_t idealNEl =  floor((((double)ib)/((double)this->maxEvBranches_+1.0))*((double)this->sortedValues.size())+0.5);

        if (idealNEl<this->minInstancesChild_ or ((int)this->sortedValues.size())-((int)idealNEl)<((int)this->minInstancesChild_))
            continue;

        size_t bestDist = numeric_limits<size_t>::max();
        T bestV = T();
        size_t bestP = 0;

        size_t nEl = 0;
        for (const auto &v : diffValues)
        {
            nEl += v.second;
            if ( abs(((int)nEl)-((int)idealNEl)) < ((int)bestDist))
            {
                bestDist = abs(((int)nEl)-((int)idealNEl));
                bestV = v.first;
                bestP = nEl-v.second;
            }
            else
                break;
        }
        this->branchingV.push_back(make_pair(bestV, bestP));
    }
}

template <typename T>
FeatureBranching<T>::~FeatureBranching ()
{
}

#endif /* FEATUREBRANCHING_HPP_ */
