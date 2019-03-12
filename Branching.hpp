/*
 * Branching.hpp
 *
 * Stores the best branching and the splitting of the elements
 *
 *  Created on: 7 de mar de 2019
 *      Author: Haroldo
 */

#ifndef BRANCHING_HPP_
#define BRANCHING_HPP_

#include <stddef.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "Dataset.hpp"
#include "FeatureBranching.hpp"
#include "SubSetResults.hpp"

typedef union  {
    int vint;
    double vfloat;
    char vstr[128];
} BranchValue;

class Branching
{
public:
    Branching( ) :
        idxF_(std::numeric_limits<size_t>::max()),
        eval_(std::numeric_limits<double>::max()),
        type_(Empty) {
    }

    double eval() const {
        return eval_;
    }

    const std::vector< std::vector<size_t> > &branches() const {
        return elements_;
    }

    // constructor for integers
    Branching( size_t _idxF,
               int _branchValue,
               double _eval,
               size_t _nElLeft,
               const size_t _elLeft,
               size_t _nElRight,
               const size_t _elRight ) :
                   idxF_(_idxF),
                   eval_(_eval),
                   type_(Integer)

    {
        this->value_.vint = _branchValue;
        this->elements_.resize(2);
        this->elements_[0] = std::vector< size_t >(_elLeft, _elLeft+_nElLeft);
        this->elements_[1] = std::vector< size_t >(_elRight, _elRight+_nElRight);
    }

    // constructor for double
    Branching( size_t _idxF,
               double _branchValue,
               double _eval,
               size_t _nElLeft,
               const size_t _elLeft,
               size_t _nElRight,
               const size_t _elRight ) :
                   idxF_(_idxF),
                   eval_(_eval),
                   type_(Float)

    {
        this->value_.vfloat = _branchValue;
        this->elements_.resize(2);
        this->elements_[0] = std::vector< size_t >(_elLeft, _elLeft+_nElLeft);
        this->elements_[1] = std::vector< size_t >(_elRight, _elRight+_nElRight);
    }

    // constructor for string
    Branching( size_t _idxF,
               const std::string &_branchValue,
               double _eval,
               size_t _nElLeft,
               const size_t _elLeft,
               size_t _nElRight,
               const size_t _elRight ) :
                   idxF_(_idxF),
                   eval_(_eval),
                   type_(String)
    {
        this->set(_branchValue);
        this->elements_.resize(2);
        this->elements_[0] = std::vector< size_t >(_elLeft, _elLeft+_nElLeft);
        this->elements_[1] = std::vector< size_t >(_elRight, _elRight+_nElRight);
    }

    void set( int value ) {
        value_.vint = value;
    }

    void set( double value ) {
        value_.vfloat = value;
    }

    void set( const std::string &value ) {
        assert(value.size()<sizeof(BranchValue));
        strcpy( &(value_.vstr[0]), value.c_str() );
    }

    int get_int() const {
        assert( type_==Integer );
        return value_.vint;
    }

    double get_float() const {
        assert( type_== Float );
        return value_.vfloat;
    }

    const char *get_str() const {
        assert( type_== String );
        return &(value_.vstr[0]);
    }

    std::string as_str() const {
        switch (type_)
        {
            case Datatype::String:
                return string(value_.vstr);
            case Datatype::Float:
            {
                stringstream ss;
                ss << value_.vfloat;
                return ss.str();
            }
            case Datatype::Integer:
            {
                stringstream ss;
                ss << value_.vint;
                return ss.str();
            }
            default:
            {
                cerr << "type not suported" << endl;
                abort();
            }
        }

        return string();
    }

    Branching &operator=(const Branching &other) {
        this->idxF_ = other.idxF_;
        memcpy( (&this->value_), &(other.value_), sizeof(BranchValue) );
        this->eval_ = other.eval_;
        this->type_ = other.type_;
        this->elements_ = other.elements_;

        return *this;
    }

    bool found() const {
        return type_ != Empty;
    }

    // updates best branching with the current
    // value in featurebranching<int>
    void update_best_branching( const FeatureBranching<int> &fb ) {
        // feature did not produced any branch
        if (fb.branch_values().size() == 0)
            return;

        if ( ((double)fb.evaluation())<this->eval_)
        {
            this->idxF_ = fb.idxF();
            this->eval_ = (double)fb.evaluation();
            this->type_ = Integer;
            this->value_.vint = fb.branch_value();
            if (this->elements_.size()==0)
                this->elements_.resize(2);

            const size_t *elLeft = fb.branch_elements(0);
            const size_t nElLeft = fb.n_branch_elements(0);

            const size_t *elRight = fb.branch_elements(1);
            const size_t nElRight = fb.n_branch_elements(1);
            this->elements_[0] = std::vector<size_t>( elLeft, elLeft+nElLeft );
            this->elements_[1] = std::vector<size_t>( elRight, elRight+nElRight );

            this->ssrLeft = SubSetResults(fb.ssrLeft);
            this->ssrRight = SubSetResults(fb.ssrRight);
        }
    }

    // updates best branching with the current
    // value in featurebranching<int>
    void update_best_branching( const FeatureBranching<double> &fb ) {
        // feature did not produced any branch
        if (fb.branch_values().size() == 0)
            return;

        if ( ((double)fb.evaluation())<this->eval_)
        {
            this->idxF_ = fb.idxF();
            this->eval_ = (double)fb.evaluation();
            this->type_ = Float;
            this->value_.vfloat = fb.branch_value();
            if (this->elements_.size()==0)
                this->elements_.resize(2);

            const size_t *elLeft = fb.branch_elements(0);
            const size_t nElLeft = fb.n_branch_elements(0);

            const size_t *elRight = fb.branch_elements(1);
            const size_t nElRight = fb.n_branch_elements(1);
            this->elements_[0] = std::vector<size_t>( elLeft, elLeft+nElLeft );
            this->elements_[1] = std::vector<size_t>( elRight, elRight+nElRight );
        }
    }

    Datatype type() const {
        return type_;
    }

    virtual ~Branching ();
private:
    // feature
    size_t idxF_;

    BranchValue value_;

    // cost of this branching
    double eval_;

    Datatype type_;

    std::vector< std::vector< size_t > > elements_;

    SubSetResults ssrLeft;
    SubSetResults ssrRight;

    friend class Node;
    friend class Tree;
};

#endif /* BRANCHING_HPP_ */
