/*
 * FeatureBranching.hpp
 *
 *  Created on: 25 de fev de 2019
 *      Author: haroldo
 */

#ifndef FEATUREBRANCHING_HPP_
#define FEATUREBRANCHING_HPP_

#include <stddef.h>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <cmath>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Instance.hpp"
#include "InstanceSet.hpp"
#include "Parameters.hpp"
#include "ResultsSet.hpp"
#include "SubSetResults.hpp"

using namespace std;

template<typename T> class FeatureBranching {
public:
    FeatureBranching(const InstanceSet& _iset, // complete instance set
                     const ResultsSet& _rset, // complete results set
                     size_t _idxF, // feature where branching will be evaluated
                     const size_t *_elements, // subset of instances (nullptr if all),
                     size_t _nElements, // number of elements
                     const SubSetResults &_ssr,
                     size_t _minInstancesChild = Parameters::minElementsBranch, // splittings leaving few instances in a not will be forbiden
                     size_t _maxEvBranches = 11 // maximum number of values to branch
                     );

    // current branching value
    T branch_value() const {
        return branchValue;
    }

    // evaluation for this branching
    double evaluation() const;

    // go to the best value for branching
    // and do this branching
    bool next();

    // instances at each side of the branching (0 left, 1 right)
    const size_t *branch_elements( size_t iBranch ) const;

    // number of instances at each side of the branching (0 left, 1 right)
    const size_t n_branch_elements( size_t iBranch ) const;

    size_t idxF() const {
        return this->idxF_;
    }

    // returns branch values and positions in the sorted vector
    const std::vector< pair<T, size_t> > &branch_values() const {
        return this->branchingV;
    }

    virtual ~FeatureBranching ();
private:
    // evaluates the current splitting
    // checks everything if in debug mode
    void evaluate();

    void fillAndSortValues( std::vector< pair<T, size_t> > &sortedValues  );
    void computeOcurrency( std::vector< pair<T, size_t> > &sortedValues, std::vector< pair<T, size_t> > &diffValues);
    void computeBranchValues( std::vector< pair<T, size_t> > &sortedValues, std::vector< pair<T, size_t> > &diffValues);

    const InstanceSet &iset_;
    const ResultsSet &rset_;
    const size_t idxF_;
    size_t *elements_;
    size_t n_elements_;

    size_t minInstancesChild_;
    size_t maxEvBranches_;

    // branching values and starting
    // positions in sortedValues vector
    std::vector< std::pair<T, size_t> > branchingV;

    // index of current branch value
    size_t idxBv;

    // current branching value
    T branchValue;

    size_t nElLeft;

    SubSetResults ssrLeft;
    SubSetResults ssrRight;

    double splittingEval;
    friend class Branching;
    friend class SubSetResults;
};

template class FeatureBranching<int>;
template class FeatureBranching<double>;
template class FeatureBranching<const char*>;

template <typename T>
FeatureBranching<T>::FeatureBranching(const InstanceSet& _iset, // complete instance set
                                      const ResultsSet& _rset, // complete results set
                                      size_t _idxF, // feature where branching will be evaluated
                                      const size_t *_elements, // subset of instances (nullptr if all),
                                      size_t _nElements, // number of elements
                                      const SubSetResults &_ssr,
                                      size_t _minInstancesChild,
                                      size_t _maxEvBranches
                                      ) :
    iset_(_iset),
    rset_(_rset),
    idxF_(_idxF),
    elements_(new size_t[_nElements]),
    n_elements_(_nElements),
    minInstancesChild_(_minInstancesChild),
    maxEvBranches_(_maxEvBranches),
    idxBv(0),
    branchValue(numeric_limits<T>::max()),
    ssrLeft( &rset_, Parameters::eval, false ),
    ssrRight( _ssr  ),
    splittingEval(0.0)
{
    assert( ssrRight.nElSS == n_elements_ );
    assert( _nElements<=iset_.size( ));
    assert( elements_ != nullptr and _nElements>=2 );
    assert( idxF_ < iset_.features().size() );
    assert( maxEvBranches_ > 0);
    memcpy( elements_, _elements, sizeof(size_t)*n_elements_ );
    {

        // sorted values and their positions in the dataset
        std::vector<std::pair<T, size_t>> sortedValues;

        // different values that can be used for branching
        // value, occurrences
        fillAndSortValues(sortedValues);

        for ( size_t i=0 ; (i<n_elements_) ; ++i )
            elements_[i] = sortedValues[i].second;

        // vector in the form: value, number of occurrences
        std::vector< std::pair<T, size_t> > diffValues;
        computeOcurrency(sortedValues, diffValues);
        computeBranchValues(sortedValues, diffValues);
    }

    if (branchingV.size()==0)
        return;

    // arranging data for first branch
    branchValue = branchingV[0].first;
    size_t pCut = branchingV[0].second;
    ssrLeft.add( pCut+1, elements_ );
    ssrRight.remove( pCut+1, elements_ );
    nElLeft = pCut+1;

    evaluate();

    assert(nElLeft == n_branch_elements(0));
    assert(n_elements_-nElLeft == n_branch_elements(1));
    assert( n_branch_elements(0) == ssrLeft.nElSS );
    assert( n_branch_elements(1) == ssrRight.nElSS );
}

template <>
void FeatureBranching<int>::fillAndSortValues(std::vector< pair<int, size_t> > &sortedValues)
{
    assert( sortedValues.size() == 0 );
    const size_t *endEl = this->elements_ + this->n_elements_;
    for ( const size_t *el = this->elements_ ; el<endEl; ++el )
    {
        const Instance &inst = iset_.instances()[*el];
        sortedValues.push_back(make_pair( inst.int_feature(idxF_), inst.idx() ));
    }

    std::sort(sortedValues.begin(), sortedValues.end());
}

template <>
void FeatureBranching<double>::fillAndSortValues(std::vector< pair<double, size_t> > &sortedValues)
{
    assert( sortedValues.size() == 0 );
    const size_t *endEl = this->elements_ + this->n_elements_;
    for ( const size_t *el = this->elements_ ; el<endEl; ++el )
    {
        const Instance &inst = iset_.instances()[*el];
        sortedValues.push_back(make_pair( inst.float_feature(idxF_), inst.idx() ));
    }

    std::sort(sortedValues.begin(), sortedValues.end());
}

static bool compare_str( const pair<const char *, size_t > &v1, const pair<const char *, size_t > &v2  )
{
    int r = strcmp(v1.first, v2.first);
    return (r<0);
}

template <>
void FeatureBranching<const char *>::fillAndSortValues(std::vector< pair<const char *, size_t> > &sortedValues)
{
    assert( sortedValues.size() == 0 );
    const size_t *endEl = this->elements_ + this->n_elements_;
    for ( const size_t *el = this->elements_ ; el<endEl; ++el )
    {
        const Instance &inst = iset_.instances()[*el];
        sortedValues.push_back(make_pair( inst.str_feature(idxF_), inst.idx() ));
    }

    std::sort(sortedValues.begin(), sortedValues.end(), compare_str);
}

template <typename T>
void FeatureBranching<T>::computeOcurrency( std::vector< pair<T, size_t> > &sortedValues, std::vector< pair<T, size_t> > &diffValues )
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
void FeatureBranching<const char *>::computeOcurrency( std::vector< pair<const char *, size_t> > &sortedValues, std::vector< pair<const char*, size_t> > &diffValues)
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
void FeatureBranching<T>::computeBranchValues( std::vector< pair<T, size_t> > &sortedValues, std::vector< pair<T, size_t> > &diffValues )
{
    if (diffValues.size()<=this->maxEvBranches_)
    {
        size_t p = 0;
        for ( const auto &v : diffValues)
        {
            size_t cEl = p + v.second;
            if (cEl>=this->minInstancesChild_ and ((int)sortedValues.size())-((int)cEl)>=((int)this->minInstancesChild_))
                this->branchingV.push_back(make_pair(v.first, cEl-1));

            p += v.second;
        }
        return;
    }

    std::set< std::pair<T, size_t> > difbv;

    for (size_t ib=0 ; (ib<this->maxEvBranches_); ++ib)
    {
        size_t idealNEl =  floor((((double)ib+1.0)/((double)this->maxEvBranches_+1.0))*((double)sortedValues.size())+0.5);

        if (idealNEl<this->minInstancesChild_ or ((int)sortedValues.size())-((int)idealNEl)<((int)this->minInstancesChild_))
            continue;

        int bestDist = numeric_limits<int>::max();
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
                bestP = nEl-1;
            }
            else
                break;
        }
        difbv.insert(make_pair(bestV, bestP));
    }

    branchingV.insert( branchingV.end(), difbv.begin(), difbv.end() );

}

template <typename T>
void FeatureBranching<T>::evaluate()
{
    const long double percLeft = ((long double)nElLeft) / ((long double)n_elements_);
    const long double percRight =  ((long double)(n_elements_-nElLeft)) / ((long double)n_elements_);

    splittingEval = (double) ( percLeft  * (long double)ssrLeft.bestAlgRes() +
                               percRight * (long double)ssrRight.bestAlgRes() );
}

template <typename T>
double FeatureBranching<T>::evaluation() const
{
    return splittingEval;
}

template <typename T>
    bool
    FeatureBranching<T>::next()
{
    ++idxBv;
    if (idxBv>=branchingV.size())
        return false;

    this->branchValue = branchingV[idxBv].first;

    {
        // elements at left
        const size_t *el = elements_ + branchingV[idxBv-1].second+1;
        size_t nEl = (branchingV[idxBv].second+1) - (branchingV[idxBv-1].second+1);
        ssrLeft.add( nEl, el );
        ssrRight.remove( nEl, el );
    }

    this->nElLeft = branchingV[idxBv].second+1;
    evaluate();
    assert(nElLeft == n_branch_elements(0));
    assert(n_elements_-nElLeft == n_branch_elements(1));

    assert( n_branch_elements(0) == ssrLeft.nElSS );
    assert( n_branch_elements(1) == ssrRight.nElSS );

    return true;
}

// instances at each side of the branching (0 left, 1 right)
template <typename T>
const size_t *FeatureBranching<T>::branch_elements( size_t iBranch ) const
{
    assert( iBranch<2 );
    return elements_+ (iBranch*nElLeft);
}

// number of instances at each side of the branching (0 left, 1 right)
template <typename T>
const size_t FeatureBranching<T>::n_branch_elements( size_t iBranch ) const
{
    assert( iBranch<2 );
    // left or right
    return (1-iBranch)*nElLeft + iBranch*(n_elements_-nElLeft);
}

template <typename T>
FeatureBranching<T>::~FeatureBranching ()
{
    delete[] elements_;
}

#endif /* FEATUREBRANCHING_HPP_ */
