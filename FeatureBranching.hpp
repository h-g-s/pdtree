/*
 * FeatureBranching.hpp
 *
 *  Created on: 25 de fev de 2019
 *      Author: haroldo
 */

#ifndef FEATUREBRANCHING_HPP_
#define FEATUREBRANCHING_HPP_

#include <unordered_map>
#include "Dataset.hpp"
#include <utility>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <cmath>

using namespace std;

template<typename T> class FeatureBranching {
public:
    FeatureBranching(const Dataset& _ds,
                     size_t _idxF,
                     FeatureBranching<T> *_parent = nullptr,
                     size_t _parBranch = 0,
                     size_t _minInstancesChild = 10,
                     size_t _maxEvBranches = 11
                     );

    virtual ~FeatureBranching ();
private:
    // sorted values and their positions in the dataset
    std::vector<std::pair<T, size_t>> sortedValues;

    void fillAndSortValues();
    void computeOcurrency();
    void computeBranchValues();


    const Dataset &ds_;
    const size_t idxF_;
    FeatureBranching<T> *parent_;
    size_t parBranch_;

    // elements in each branch (usually two)
    std::vector< std::unordered_set< size_t > > branchElements;

    // different values that can be used for branching
    // value, occurrences
    std::vector< std::pair<T, size_t> > diffValues;

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
FeatureBranching<T>::FeatureBranching(const Dataset& _ds,
                                      size_t _idxF,
                                      FeatureBranching<T> *_parent,
                                      size_t _parBranch,
                                      size_t _minInstancesChild,
                                      size_t _maxEvBranches) :
    ds_(_ds),
    idxF_(_idxF),
    parent_(_parent),
    parBranch_(_parBranch),
    branchElements( std::vector<std::unordered_set<size_t>>(2, unordered_set<size_t>()) ),
    minInstancesChild_(_minInstancesChild),
    maxEvBranches_(_maxEvBranches)
{
    this->fillAndSortValues();
}


template <typename T>
FeatureBranching<T>::~FeatureBranching ()
{
}

template <>
void FeatureBranching<int>::fillAndSortValues()
{
    if (this->parent_==nullptr)
    {
        const auto &ds = this->ds_;
        this->sortedValues.clear();
        for (size_t i=0 ; (i<ds.rows()) ; ++i)
            sortedValues.push_back(make_pair(ds.int_cell(i, idxF_), i));
    }
    {
        const auto &parElements = this->parent_->branchElements[this->parBranch_];
        const auto &ds = this->ds_;

        this->sortedValues.clear();
        for ( const auto &el : parElements)
            sortedValues.push_back(make_pair(ds.int_cell(el, idxF_), el));
    }

    std::sort(sortedValues.begin(), sortedValues.end());
}

template <>
void FeatureBranching<double>::fillAndSortValues()
{
    if (this->parent_==nullptr)
    {
        const auto &ds = this->ds_;
        this->sortedValues.clear();
        for (size_t i=0 ; (i<ds.rows()) ; ++i)
            sortedValues.push_back(make_pair(ds.float_cell(i, idxF_), i));
    }
    {
        const auto &parElements = this->parent_->branchElements[this->parBranch_];
        const auto &ds = this->ds_;

        this->sortedValues.clear();
        for ( const auto &el : parElements)
            sortedValues.push_back(make_pair(ds.float_cell(el, idxF_), el));
    }

    std::sort(sortedValues.begin(), sortedValues.end());
}

template <>
void FeatureBranching<const char *>::fillAndSortValues()
{
    if (this->parent_==nullptr)
    {
        const auto &ds = this->ds_;
        this->sortedValues.clear();
        for (size_t i=0 ; (i<ds.rows()) ; ++i)
            sortedValues.push_back(make_pair(ds.str_cell(i, idxF_), i));
    }
    {
        const auto &parElements = this->parent_->branchElements[this->parBranch_];
        const auto &ds = this->ds_;

        this->sortedValues.clear();
        for ( const auto &el : parElements)
            sortedValues.push_back(make_pair(ds.str_cell(el, idxF_), el));
    }

    std::sort(sortedValues.begin(), sortedValues.end());

}

template <typename T>
void FeatureBranching<T>::computeOcurrency()
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

template <typename T>
void FeatureBranching<T>::computeBranchValues()
{
    if (this->diffValues.size()<=this->maxEvBranches_)
    {
        size_t p = 0;
        for ( const auto v : diffValues)
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

#endif /* FEATUREBRANCHING_HPP_ */
