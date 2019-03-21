/*
 * MIPPDtree.cpp
 *
 *  Created on: 20 de mar de 2019
 *      Author: haroldo
 */

#include "MIPPDtree.hpp"

#include <stddef.h>
#include <stdlib.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <sstream>
#include <ctype.h>

#include "Parameters.hpp"
#include "InstanceSet.hpp"

using namespace std;

static char **to_char_vec( const std::vector< std::string > names );

double MIPPDtree::alpha = 0.001;

static std::string clean_str( const char *str )
{
    string res = "";
    size_t len = strlen(str);
    for ( size_t i=0 ; (i<len) ; ++i )
    {
        if (isalnum(str[i]))
            res.push_back(str[i]);
        else
            if ((str[i])=='%')
                res += "perc";
    }
    return res;
}

MIPPDtree::MIPPDtree( const InstanceSet *_iset, const ResultsSet *_rset ) :
    iset_( _iset ),
    rset_( _rset ),
    mip( lp_create() ),
    nLeafs( floor(pow( 2.0, Parameters::maxDepth )+1e-5) ),
    nInsts(_iset->size())
{
    // branch nodes
    for ( size_t d=0 ; (d<Parameters::maxDepth) ; ++d )
    {
        size_t nNodes = floor(pow(2.0, d)+1e-9);
        for ( size_t n=0 ; (n<nNodes) ; ++n )
        {
            stringstream ss;
            ss << "n(" << d << "," << n << ")";
            branchNodes.push_back(ss.str());
        }
    }

    createBVars();
    createDVars();
    createAVars();
    createLVars();
    createZVars();
    createConsLnkBD();
    createConsLnkAD();
    createConsLNKZL();
}

void MIPPDtree::createBVars()
{
    b = vector< int >( branchNodes.size() );
    vector< double > lb( branchNodes.size(), 0.0 );
    vector< double > ub( branchNodes.size(), 1.0 );
    vector< double > obj( branchNodes.size(), 0.0 );
    vector< char > isint( branchNodes.size(), 0 );
    vector< string > cnames(branchNodes.size());
    for ( size_t i=0 ; (i<branchNodes.size()) ; ++i )
    {
        b[i] = lp_cols(mip) + i;
        cnames[i] = "b(" + branchNodes[i] + ")";
    }

    char **names = to_char_vec(cnames);
    lp_add_cols( mip, cnames.size(), &obj[0], &lb[0], &ub[0], &isint[0], names );

    free( names );
}

static char **to_char_vec( const std::vector< std::string > names )
{
    size_t spaceVec = (sizeof(char*)*names.size());
    size_t totLen = names.size(); // all \0
    for ( const auto &str : names )
        totLen += str.size();

    char **r = (char **)malloc(spaceVec+totLen);
    assert( r );
    r[0] = (char *)(r + names.size());
    for ( size_t i=1 ; (i<names.size()) ; ++i )
        r[i] = r[i-1] + names[i].size() + 1;

    for ( size_t i=0 ; (i<names.size()) ; ++i )
        strcpy(r[i], names[i].c_str());

    return r;
}

void MIPPDtree::createDVars()
{
    vector< double > lb( branchNodes.size(), 0.0 );
    vector< double > ub( branchNodes.size(), 1.0 );
    vector< double > obj( branchNodes.size(), MIPPDtree::alpha );
    vector< char > isint( branchNodes.size(), 1 );
    vector< string > cnames(branchNodes.size());
    for ( size_t i=0 ; (i<branchNodes.size()) ; ++i )
    {
        cnames[i] = "d(" + branchNodes[i] + ")";
        d[i] = lp_cols(mip) + i;
    }

    char **names = to_char_vec(cnames);
    lp_add_cols( mip, cnames.size(), &obj[0], &lb[0], &ub[0], &isint[0], names );

    free( names );
}

void MIPPDtree::createConsLnkBD()
{
    for ( size_t i=0 ; (i<branchNodes.size()) ; ++i )
    {
        double coef[] = { 1.0, -1.0 };
        int idx[] = { b[i], d[i] };
        char rname[256] = "";
        sprintf(rname, "lnkbd(%zu)", i );
        lp_add_row(mip, 2, idx, coef, rname, 'L', 0.0);
    }

}

void MIPPDtree::createAVars()
{
    vector< string > vnames;
    a = vector< vector< int > >( iset_->features().size(), std::vector< int >( branchNodes.size() ) );
    for ( size_t idxF=0 ; (idxF<iset_->features().size()) ; ++idxF )
    {
        for ( size_t idxN=0 ; (idxN<branchNodes.size()) ; ++idxN )
        {
            char vname[512];
            sprintf(vname, "a(%s,%s)",
                    clean_str(iset_->features()[idxF].c_str()).c_str() ,
                    branchNodes[idxN].c_str());
            a[idxF][idxN] = lp_cols(mip) + vnames.size();
            vnames.push_back(vname);
        }
    }

    vector< double > lb( vnames.size(), 0.0 );
    vector< double > ub( vnames.size(), 1.0 );
    vector< double > obj( vnames.size(), 0.0 );
    vector< char > isint( vnames.size(), 1 );

    char **names = to_char_vec(vnames);
    lp_add_cols( mip, vnames.size(), &obj[0], &lb[0], &ub[0], &isint[0], names );

    free( names );
}

void MIPPDtree::createConsLnkAD()
{
    for ( size_t idxN=0 ; (idxN<branchNodes.size()) ; ++idxN )
    {
        vector< int > idx( iset_->features().size()+1 );
        vector< double > coef( iset_->features().size()+1, 1.0 );
        *coef.rbegin() = -1.0;
        *idx.rbegin() = d[idxN];
        for ( size_t idxF=0 ; (idxF<iset_->features().size()) ; ++idxF )
            idx[idxF] = a[idxF][idxN];

        char rname[256];
        sprintf( rname, "lnkAD(%s)", branchNodes[idxN].c_str() );
        lp_add_row(mip, iset_->features().size(), &idx[0], &coef[0], rname, 'E', 0.0 );
    }
}

void MIPPDtree::createLVars()
{
    l = vector< int >(nLeafs);
    vector< string > cnames;
    for ( size_t i=0 ; (i<nLeafs) ; ++i )
    {
        char cName[256];
        sprintf( cName, "l(%zu)", i );
        l[i] = lp_cols(mip) + cnames.size();
        cnames.push_back(cName);
    }
    vector< double > obj( cnames.size(), 0.0 );
    char **names = to_char_vec(cnames);

    lp_add_bin_cols(mip, cnames.size(), &obj[0], names );
    free(names);
}

void MIPPDtree::createZVars()
{
    z = vector< vector<int> >( nInsts, vector<int>(nLeafs) );
    vector< string > cnames;
    for ( size_t i=0 ; (i<nInsts) ; ++i )
    {
        for ( size_t l=0 ; (l<nLeafs) ; ++l )
        {
            char cName[256];
            sprintf(cName, "z(%zu,%zu)", i, l );
            z[i][l] = lp_cols(mip)+cnames.size();
            cnames.push_back( cName );
        }
    }
    vector< double > obj( cnames.size(), 0.0 );
    char **names = to_char_vec(cnames);

    lp_add_bin_cols(mip, cnames.size(), &obj[0], names );
    free(names);
}

void MIPPDtree::createConsLNKZL()
{
    for ( size_t i=0 ; (i<nInsts) ; ++i )
    {
        for ( size_t j=0 ; (j<nLeafs) ; ++j )
        {
            int idx[] = { z[i][j], l[j] };
            double coef[] = { 1.0, -1.0 };
            char rName[256];
            sprintf( rName, "lnkZLUp(%zu,%zu)", i, j );
            lp_add_row(mip, 2, idx, coef, rName, 'L', 0.0);
        }
    }

    for ( size_t j=0 ; (j<nLeafs) ; ++j )
    {
        char rName[256];
        sprintf( rName, "minElementsLeaf(%zu)", j );
        vector< int > idx( nInsts+1 );
        vector< double > coef( nInsts+1, 1.0 );
        *idx.rbegin() = l[j];
        *coef.rbegin() = -1.0;
        for ( size_t i=0 ; (i<nInsts) ; ++i )
            idx[i] = z[i][j];

        lp_add_row( mip, nInsts+1, &idx[0], &coef[0], rName, 'G', 0.0 );
    }

}

MIPPDtree::~MIPPDtree ()
{
    lp_free( &mip );
}
