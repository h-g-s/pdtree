/*
 * FeatureBranching.hpp
 *
 *  Created on: 25 de fev de 2019
 *      Author: haroldo
 */

#ifndef FEATUREBRANCHING_HPP_
#define FEATUREBRANCHING_HPP_

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cassert>
#include <map>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"

using namespace std;

template<typename T> class FeatureBranching {
public:
    FeatureBranching(const InstanceSet& _iset, // complete instance set
                     const ResultsSet& _rset, // complete results set
                     size_t _idxF, // feature where branching will be evaluated
                     std::unordered_set< size_t > *_elements=nullptr, // subset of instances (nullptr if all)
                     size_t _minInstancesChild = 10, // splittings leaving few instances in a not will be forbiden
                     size_t _maxEvBranches = 11 // maximum number of values to branch
                     );

    // current branching value
    T branch_value() const {
        return branchValue;
    }

    // evaluation for this branching
    long double evaluation() const;

    // go to the best value for branching
    // and do this branching
    bool next();

    // instances at each side of the branching
    const std::vector< std::unordered_set<size_t> > &branch_elements() const {
        return branchElements;
    }

    virtual ~FeatureBranching ();
private:
    // evaluates the current splitting
    // checks everything if in debug mode
    void evaluate();

    // sorted values and their positions in the dataset
    std::vector<std::pair<T, size_t>> sortedValues;

    void fillAndSortValues();
    void computeOcurrency(std::vector< pair<T, size_t> > &diffValues);
    void computeBranchValues(std::vector< pair<T, size_t> > &diffValues);

    void addElementsBranch( size_t iBranch, size_t start, size_t end );
    void removeElementsBranch( size_t iBranch, size_t start, size_t end );

    const InstanceSet &iset_;
    const ResultsSet &rset_;
    const size_t idxF_;
    std::unordered_set< size_t > *elements_;

    // elements in each branch (usually two branches)
    std::vector< std::unordered_set< size_t > > branchElements;

    std::vector< std::vector< long double > > branchSum;

    std::vector< std::vector< long double > > branchSumRnk;

    size_t minInstancesChild_;
    size_t maxEvBranches_;

    // branching values and starting
    // positions in sortedValues vector
    std::vector< std::pair<T, size_t> > branchingV;

    // index of current branch value
    size_t idxBv;

    // best algorithm for branch
    std::vector< size_t > branchBestAlg;

    long double splittingEval;

    // current branching value
    T branchValue;
};

template class FeatureBranching<int>;
template class FeatureBranching<double>;
template class FeatureBranching<const char*>;

template <typename T>
FeatureBranching<T>::FeatureBranching(const InstanceSet& _iset, // complete instance set
                                      const ResultsSet& _rset, // complete results set
                                      size_t _idxF, // feature where branching will be evaluated
                                      std::unordered_set< size_t > *_elements, // subset of instances (nullptr if all)
                                      size_t _minInstancesChild,
                                      size_t _maxEvBranches
                                      ) :
    iset_(_iset),
    rset_(_rset),
    idxF_(_idxF),
    elements_(_elements),
    branchElements( std::vector<std::unordered_set<size_t>>(2, unordered_set<size_t>()) ),
    branchSum( vector< vector<long double> >(2, vector< long double >( rset_.algsettings().size(), 0.0 ) ) ),
    branchSumRnk( vector< vector<long double> >(2, vector< long double >( rset_.algsettings().size(), 0.0 ) ) ),
    minInstancesChild_(_minInstancesChild),
    maxEvBranches_(_maxEvBranches),
    idxBv(0),
    branchBestAlg( std::vector<size_t>(2, std::numeric_limits<size_t>::max()) ),
    branchValue(numeric_limits<T>::max())
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

    if (branchingV.size()==0)
        return;


    // arranging data for first branch
    branchValue = branchingV[0].first;
    size_t pCut = branchingV[0].second;
    addElementsBranch(0, 0, pCut+1);
    addElementsBranch(1, pCut+1, sortedValues.size());
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

static bool compare_str( const pair<const char *, size_t > &v1, const pair<const char *, size_t > &v2  )
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
                this->branchingV.push_back(make_pair(v.first, cEl-1));

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
                bestP = nEl-1;
            }
            else
                break;
        }
        this->branchingV.push_back(make_pair(bestV, bestP));
    }
}

template <typename T>
void FeatureBranching<T>::addElementsBranch( size_t iBranch, size_t start, size_t end )
{
    assert( iBranch < 2);
    assert( start < end ); // at least one
    assert( start < sortedValues.size() );
    assert( end <= sortedValues.size() );
    for ( size_t p=start ; p<end ; ++p )
    {
        size_t instIdx = sortedValues[p].second;
        assert( instIdx < iset_.size() );

        // adding instance results
        for ( size_t iAlg=0 ; (iAlg<rset_.algsettings().size()) ; ++iAlg )
            branchSum[iBranch][iAlg] += rset_.get( instIdx, iAlg);

        for ( size_t iAlg=0 ; (iAlg<rset_.algsettings().size()) ; ++iAlg )
            branchSumRnk[iBranch][iAlg] += rset_.rank( instIdx, iAlg);

#ifdef DEBUG
        assert( branchElements[iBranch].find(instIdx) == branchElements[iBranch].end() );
        assert( branchElements[1-iBranch].find(instIdx) != branchElements[1-iBranch].end() );
#endif
        branchElements[iBranch].insert(instIdx);
    }
}

template <typename T>
void FeatureBranching<T>::removeElementsBranch( size_t iBranch, size_t start, size_t end )
{
    assert( iBranch < 2);
    assert( start < end ); // at least one
    assert( start < sortedValues.size() );
    assert( end <= sortedValues.size() );
    for ( size_t p=start ; p<end ; ++p )
    {
        size_t instIdx = sortedValues[p].second;
        assert( instIdx < iset_.size() );

        // adding instance results
        for ( size_t iAlg=0 ; (iAlg<rset_.algsettings().size()) ; ++iAlg )
            branchSum[iBranch][iAlg] -= rset_.get( instIdx, iAlg);

        for ( size_t iAlg=0 ; (iAlg<rset_.algsettings().size()) ; ++iAlg )
            branchSumRnk[iBranch][iAlg] -= rset_.rank( instIdx, iAlg);

#ifdef DEBUG
        assert( branchElements[iBranch].find(instIdx) == branchElements[iBranch].end() );
        assert( branchElements[1-iBranch].find(instIdx) != branchElements[1-iBranch].end() );
#endif
        branchElements[iBranch].erase(instIdx);
    }
}

template <typename T>
void FeatureBranching<T>::evaluate()
{
    long double totalElements = (long double)(branchElements[0].size()+branchElements[1].size());

    splittingEval = 0.0;
    switch (ResultsSet::eval)
    {
        case Evaluation::Average:
            for ( size_t iBranch=0 ; (iBranch<2) ; ++iBranch )
            {
                const long double weight = ((long double)branchElements[iBranch].size()) / totalElements;
                branchBestAlg[iBranch] = numeric_limits<size_t>::max();
                long double bestRBranch = numeric_limits<long double>::max();
                for ( size_t iAlg=0 ; (iAlg<rset_.algsettings().size()) ; ++iAlg )
                {
                    if (branchSum[iBranch][iAlg]<bestRBranch)
                    {
                        bestRBranch = branchSum[iBranch][iAlg];
                        branchBestAlg[iBranch] = iAlg;
                    }
                }
                splittingEval += weight*bestRBranch;
            }
            break;
        case Evaluation::Rank:
            for ( size_t iBranch=0 ; (iBranch<2) ; ++iBranch )
            {
                const long double weight = ((long double)branchElements[iBranch].size()) / totalElements;
                branchBestAlg[iBranch] = numeric_limits<size_t>::max();
                long double bestRBranch = numeric_limits<long double>::max();
                for ( size_t iAlg=0 ; (iAlg<rset_.algsettings().size()) ; ++iAlg )
                {
                    if (branchSum[iBranch][iAlg]<bestRBranch)
                    {
                        bestRBranch = branchSumRnk[iBranch][iAlg];
                        branchBestAlg[iBranch] = iAlg;
                    }
                }
                splittingEval += weight*bestRBranch;
            }
            break;
    }
}

template <typename T>
long double FeatureBranching<T>::evaluation() const
{
    return splittingEval;
}

template <typename T>
bool FeatureBranching<T>::next()
{
    ++idxBv;
    if (idxBv>=branchingV.size())
        return false;

    // doing branch
    addElementsBranch(0, branchingV[idxBv-1].second, branchingV[idxBv].second+1 );
    removeElementsBranch(1, branchingV[idxBv-1].second, branchingV[idxBv].second+1);
    evaluate();

    return true;
}

template <typename T>
FeatureBranching<T>::~FeatureBranching ()
{
}

#endif /* FEATUREBRANCHING_HPP_ */
