/*
 * MIPMultiVariate.hpp
 *
 *  Created on: 29 de jul de 2019
 *      Author: haroldo
 */

extern "C"
{
#include "lp.h"
}
class InstanceSet;
class ResultsSet;
class Tree;
#include <vector>

#ifndef MIPMULTIVARIATE_HPP
#define MIPMULTIVARIATE_HPP

class MIPMultiVariate
{
    public:
        // an initial feasible tree can be informed to extract useful
        // features for an initial solution
        MIPMultiVariate( const InstanceSet *_iset, const ResultsSet *_rset, const Tree *_tree = NULL );

        void optimize();

        virtual ~MIPMultiVariate();
    private:
        const InstanceSet *iset_;
        const ResultsSet *rset_;
        
        void createYvars();
        void createZvars();
        void createAvars();
        void createWvars();

        void createConsLnkAYB();
        void createConsSelAlgSide();
        void createConsSelAlgProblem();
        void createConsLNKWYZ();

        void fillInitialSolution();

        int nAlgs;
        int nFeat;
        
        std::vector<int> y;
        std::vector< std::vector< int > > z;
        std::vector<int> a;
        int b;
        std::vector< std::vector< int > > w;

        const Tree *tree;

        LinearProgram *mip;
};

#endif

