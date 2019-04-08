/*
 * MIPSelAlg.cpp
 *
 *  Created on: 8 de abr de 2019
 *      Author: haroldo
 */

#include "MIPSelAlg.hpp"
#include "Parameters.hpp"
#include "ResultsSet.hpp"
#include <vector>
#include <cassert>
#include <cstring>

using namespace std;

#define K 3

static char **to_char_vec( const vector< string > names );


MIPSelAlg::MIPSelAlg( const ResultsSet *_rset ) :
    rset_(_rset),
    iset_(&rset_->instanceSet()),
    y( new int[rset_->algsettings().size()] ),
    x( new int*[iset_->size()]),
    mip(lp_create())
{
    x[0] = new int[iset_->size()*rset_->algsettings().size()];
    for ( size_t i=1 ; (i<iset_->size()) ; ++i )
        x[i] = x[i-1] + rset_->algsettings().size();

    createYVars();
    createXVars();
    createConsSelK();
    createConsLNKXY();
}

void MIPSelAlg::createYVars()
{
    vector< string > cnames;
    for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
    {
        char cn[256];
        sprintf(cn, "y(%zu)", ia);
        y[ia] = lp_cols(mip)+cnames.size();
        cnames.push_back(cn);
    }
    vector< double > obj(cnames.size(), Parameters::minElementsBranch);

    char **cns = to_char_vec(cnames);
    lp_add_bin_cols(mip, obj.size(), &obj[0], cns);
    free(cns);
}

void MIPSelAlg::createXVars()
{
    vector< string > cnames;
    vector< double > obj;
    for ( size_t ip=0 ; ip<iset_->size() ; ++ip )
    {
        for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
        {
            char cn[256];
            sprintf(cn, "x(%zu,%zu)", ip, ia);
            x[ip][ia] = lp_cols(mip)+cnames.size();
            cnames.push_back(cn);
            obj.push_back(rset_->res(ip,ia));
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
    lp_free( &mip );
}

void MIPSelAlg::createConsSelK()
{
    vector< int > idx(rset_->algsettings().size());
    vector< double > coef(idx.size(), 1.0);

    for ( size_t ip=0 ; (ip<iset_->size()) ; ++ip )
    {
        for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
        {
            idx[ia] = x[ip][ia];

            char rName[256];
            sprintf(rName, "selK(%zu)", ip);
            lp_add_row(mip, rset_->algsettings().size(), &idx[0], &coef[0], rName, 'G', K);
        }
    }
}

void MIPSelAlg::createConsLNKXY()
{
    for ( size_t ia=0 ; (ia<rset_->algsettings().size()) ; ++ia )
    {
        vector< int > idx(iset_->size() + 1);
        vector< double  > coef(idx.size(), 1.0);

        for (size_t ip=0 ; ip<iset_->size(); ++ip)
            idx[ip] = x[ip][ia];

        *idx.rbegin() = y[ia];
        *coef.rbegin() = -Parameters::minElementsBranch;

        char rName[256];
        sprintf(rName, "lnkYX(%zu)", ia);
        lp_add_row( mip, idx.size(), &idx[0], &coef[0], rName, 'G', 0.0);

        sprintf(rName, "lnkXY(%zu)", ia);
        *coef.rbegin() = -iset_->size();
        lp_add_row( mip, idx.size(), &idx[0], &coef[0], rName, 'L', 0.0);
    }
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
