    /*
 * MIPSelAlg.cpp
 *
 *  Created on: 8 de abr de 2019
 *      Author: haroldo
 */

#include "MIPSelAlg.hpp"
#include "Parameters.hpp"
#include "ResultsSet.hpp"
#include "pdtdefines.hpp"
#include "Tree.hpp"
#include "Node.hpp"
#include <vector>
#include <limits>
#include <cmath>
#include <cassert>
#include <cstring>

using namespace std;

// minimum number of parameter settings that should be allocated to a problem

static char **to_char_vec( const vector< string > names );

MIPSelAlg::MIPSelAlg( const ResultsSet *_rset ) :
    rset_(_rset),
    iset_(&rset_->instanceSet()),
    nSelAlg_(0),
    selAlg_(new int[_rset->algsettings().size()]),
    y( new int[rset_->algsettings().size()] ),
    x( new int*[iset_->size()]),
    mip(lp_create())
{
    x[0] = new int[iset_->size()*rset_->algsettings().size()];
    for ( auto i=1 ; (i<iset_->size()) ; ++i )
        x[i] = x[i-1] + rset_->algsettings().size();

    createYVars();
    createXVars();
    createConsSelK();
    createConsLNKXY();
    createConsSelNAlgs();
    createConsSelMinProbAlg();
}

void MIPSelAlg::createYVars()
{
    vector< string > cnames;
    vector< double > obj;
    for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
    {
        char cn[256];
        sprintf(cn, "y(%zu)", ia);
        y[ia] = lp_cols(mip)+cnames.size();
        cnames.push_back(cn);
        obj.push_back(((double)Parameters::minElementsBranch)*rset_->avAlg(ia));
    }

    char **cns = to_char_vec(cnames);
    lp_add_bin_cols(mip, obj.size(), &obj[0], cns);
    free(cns);
}

void MIPSelAlg::createXVars()
{
    vector< string > cnames;
    vector< double > obj;
    vector< double > lb;
    vector< double > ub;
    for ( auto ip=0 ; ip<iset_->size() ; ++ip )
    {
        for ( auto ia=0 ; (ia<(int)rset_->algsettings().size()) ; ++ia )
        {
            char cn[256];
            sprintf(cn, "x(%d,%d)", ip, ia);
            x[ip][ia] = lp_cols(mip)+cnames.size();
            cnames.push_back(cn);
            obj.push_back(rset_->res(ip,ia));
            lb.push_back(0.0);
            if (rset_->stdDevInst(ip)<=MIN_STD_DEV)
                ub.push_back(0.0);
            else
                ub.push_back(1.0);
        }
    }
    char **cns = to_char_vec(cnames);
    lp_add_bin_cols(mip, obj.size(), &obj[0], cns);
    free(cns);
}

MIPSelAlg::~MIPSelAlg ()
{
    delete[] x[0];
    delete[] x;
    delete[] y;
    delete[] selAlg_;
    lp_free( &mip );
}

void MIPSelAlg::createConsSelK()
{
    vector< int > idx(rset_->algsettings().size());
    vector< double > coef(idx.size(), 1.0);

    for ( auto ip=0 ; (ip<iset_->size()) ; ++ip )
    {
        for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
            idx[ia] = x[ip][ia];
        char rName[256];
        sprintf(rName, "selK(%d)", ip);
        lp_add_row(mip, rset_->algsettings().size(), &idx[0], &coef[0], rName, 'G', Parameters::afMinAlgsInst);
    }
}

void MIPSelAlg::createConsLNKXY()
{
    for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
    {
        vector< int > idx(iset_->size() + 1);
        vector< double  > coef(idx.size(), 1.0);

        for (auto ip=0 ; ip<iset_->size(); ++ip)
            idx[ip] = x[ip][ia];

        *idx.rbegin() = y[ia];
        *coef.rbegin() = -((double)Parameters::minElementsBranch);

        char rName[256];
        sprintf(rName, "lnkYX(%zu)", ia);
        lp_add_row( mip, idx.size(), &idx[0], &coef[0], rName, 'G', 0.0);

        sprintf(rName, "lnkXY(%zu)", ia);
        *coef.rbegin() = -((double)iset_->size());
        lp_add_row( mip, idx.size(), &idx[0], &coef[0], rName, 'L', 0.0);
    }
}

void MIPSelAlg::createConsSelNAlgs()
{
    vector< int > idx( rset_->algsettings().size(), 0 );

    for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
        idx[ia] = y[ia];

    vector< double > coef(rset_->algsettings().size(), 1.0);

    char rName[256]; sprintf(rName, "nAlgs");
    lp_add_row(mip, idx.size(), &idx[0], &coef[0], rName, 'E', Parameters::maxAlgs);
}

void MIPSelAlg::optimize(int maxSeconds)
{
    lp_set_max_seconds(mip, maxSeconds);
    lp_write_lp(mip, "aaa.lp");
    lp_set_mip_emphasis(mip, LP_ME_FEASIBILITY);
    int status = lp_optimize(mip);
    if (status!=LP_OPTIMAL && status!=LP_FEASIBLE)
    {
        printf("No solution found for MIPSelAlg\n");
        return;
    }

    double *x = lp_x(mip);

    for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
    {
        if (fabs(x[y[ia]])<=0.99)
            continue;

        selAlg_[nSelAlg_++] = ia;
    }

    assert(nSelAlg_ == Parameters::maxAlgs);
}

void MIPSelAlg::createConsSelMinProbAlg()
{
    vector< int > idx(iset_->size()+1);
    vector< double > coef(idx.size(), 1.0);
    (*coef.rbegin()) = -Parameters::minElementsBranch;

    for ( auto ia=0 ; (ia<(int)rset_->algsettings().size()) ; ++ia )
    {
        for ( auto ip=0 ; (ip<iset_->size()) ; ++ip )
            idx[ip] = x[ip][ia];

        (*idx.rbegin()) = y[ia];
        char rName[256];
        sprintf(rName, "minInstAlg(%d)", ia);
        lp_add_row(mip, idx.size(), &idx[0], &coef[0], rName, 'G', 0.0);
    }

}

void MIPSelAlg::saveFilteredResults(const char *fileName) const
{
    FILE *f = fopen(fileName, "w");
    fprintf(f, "instance,algsetting,result\n");
    for (auto ip=0 ; (ip<iset_->size()) ; ++ip)
    {
        if (rset_->stdDevInst(ip)<=MIN_STD_DEV)
            continue;

        for (int isa=0 ; (isa<nSelAlg_) ; ++isa)
        {
            size_t ia = selAlg_[isa];
            fprintf(f, "%s,%s,%.4f\n", iset_->instance(ip).name(), rset_->algsettings()[ia].c_str(), rset_->res(ip, ia));
        }
    }
    fclose(f);
}

static char **to_char_vec( const vector< string > names )
{
    size_t spaceVec = (sizeof(char*)*names.size());
    size_t totLen = names.size(); // all \0
    for ( const auto &str : names )
        totLen += str.size();
    totLen *= sizeof(char);

    char **r = (char **)malloc(spaceVec+totLen);
    assert( r );
    r[0] = (char *)(r + names.size());
    for ( size_t i=1 ; (i<names.size()) ; ++i )
        r[i] = r[i-1] + names[i-1].size() + 1;

    for ( size_t i=0 ; (i<names.size()) ; ++i )
        strcpy(r[i], names[i].c_str());

    return r;
}
