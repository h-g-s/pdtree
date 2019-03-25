/*
 * MIPPDtree.cpp
 *
 *  Created on: 20 de mar de 2019
 *      Author: haroldo
 */

#include "MIPPDtree.hpp"

#include <stddef.h>
#include <stdlib.h>
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "InstanceSet.hpp"
#include "Parameters.hpp"
#include "ResultsSet.hpp"
#include "Tree.hpp"

#define SEL_LEAF_SCAL 1000.0

using namespace std;

#define Left 0
#define Right 1

static char **to_char_vec( const vector< string > names );

double MIPPDtree::alpha = 0.001;

static string clean_str( const char *str )
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
    nLeafs( floor(pow( 2.0, Parameters::maxDepth )+1e-5) ),
    nInsts(_iset->size()),
    nFeatures(_iset->features().size()),
    nAlgs(_rset->algsettings().size()),
    mip( lp_create() ),
    c(vector< vector<int> >(nLeafs, vector< int >(nAlgs))),
    w(vector< vector<int> >( nInsts, vector<int>(nAlgs) ) ),
    parents( vector< vector< vector< int > > >( nLeafs, vector< vector<int> >(2)) )
{
    computeEMax();
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

    size_t idxNL = floor(pow( 2.0, Parameters::maxDepth)+1e-5) - 1;
    for ( int iln=0 ; iln<(int)nLeafs; ++iln,++idxNL )
    {
        int pNodeIdx = idxNL;
        do
        {
            int pside = 1-(pNodeIdx % 2);
            pNodeIdx=(pNodeIdx-1)/2;
            parents[iln][pside].push_back(pNodeIdx);
        } while (pNodeIdx>0);
    }

    createBVars();
    createDVars();
    createAVars();
    createLVars();
    createZVars();
    createConsLnkBD();
    createConsLnkAD();
    createConsLNKZL();
    createConsOneLeaf();
    createConsSelectLeaf();
    createCVars();
    createWVars();
    createConsLnkParent();
    createConsLnkWCZ();
    createConsSelCLeaf();

    lp_write_lp(mip, "mm.lp");
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

void MIPPDtree::createDVars()
{
    d = vector< int >( branchNodes.size() );
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
    a = vector< vector< int > >( iset_->features().size(), vector< int >( branchNodes.size() ) );
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
        lp_add_row(mip, iset_->features().size()+1, &idx[0], &coef[0], rname, 'E', 0.0 );
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
        for ( size_t il=0 ; (il<nLeafs) ; ++il )
        {
            char cName[256];
            sprintf(cName, "z(%zu,%zu)", i, il );
            z[i][il] = lp_cols(mip)+cnames.size();
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
        *coef.rbegin() = -((int)Parameters::minElementsBranch);
        for ( size_t i=0 ; (i<nInsts) ; ++i )
            idx[i] = z[i][j];

        lp_add_row( mip, nInsts+1, &idx[0], &coef[0], rName, 'G', 0.0 );
    }

}

void MIPPDtree::createConsSelectLeaf()
{
    for ( size_t i=0 ; (i<nInsts) ; ++i )
    {
        for ( size_t idxL=0 ; (idxL<nLeafs) ; ++idxL )
        {
            for ( const auto leftN : parents[idxL][Left] )
            {
                vector< int > idx;
                vector< double > coef;
                idx.reserve( iset_->features().size()+2 );
                coef.reserve( iset_->features().size()+2 );

                for ( size_t idxF=0 ; (idxF<nFeatures) ; ++idxF )
                {
                    const double c = iset_->norm_feature_val( i, idxF )*SEL_LEAF_SCAL;
                    if (fabs((c<=1e-2)))
                        continue;

                    idx.push_back(a[idxF][leftN]);
                    coef.push_back(c);
                }

                idx.push_back(b[leftN]);
                coef.push_back(-1.0);

                idx.push_back( z[i][idxL] );
                coef.push_back( 1.0*SEL_LEAF_SCAL );

                char rName[256];
                sprintf(rName, "selNAL(%zu,%zu,%d)", i, idxL, leftN );

                lp_add_row(mip, idx.size(), &idx[0], &coef[0], rName, 'L', SEL_LEAF_SCAL*1.0 );
            } // parents at left

            for ( const auto rightN : parents[idxL][Right] )
            {
                vector< int > idx;
                vector< double > coef;
                idx.reserve( iset_->features().size()+2 );
                coef.reserve( iset_->features().size()+2 );

                for ( size_t idxF=0 ; (idxF<nFeatures) ; ++idxF )
                {
                    double c = (iset_->norm_feature_val( i, idxF )-epsj[idxF])*SEL_LEAF_SCAL;
                    if (fabs((c<=1e-2)))
                        continue;

                    idx.push_back(a[idxF][rightN]);
                    coef.push_back(c );
                }

                idx.push_back(b[rightN]);
                coef.push_back(-1.0);

                idx.push_back( z[i][idxL] );
                coef.push_back( (-(1.0+emax))*SEL_LEAF_SCAL );

                char rName[256];
                sprintf(rName, "selNRL(%zu,%zu,%d)", i, idxL, rightN );

                lp_add_row(mip, idx.size(), &idx[0], &coef[0], rName, 'G', (-(1.0+emax))*SEL_LEAF_SCAL );
            } // parents at right
        } // leafs
    } // instances
}

void MIPPDtree::computeEMax()
{
    emax = DBL_MIN;
    epsj = vector< double >(nFeatures, DBL_MAX);
    for ( size_t idxF=0 ; (idxF<nFeatures) ; ++idxF )
    {
        unordered_set<double> values;
        for ( size_t i=0 ; (i<nInsts) ; ++i )
            values.insert( iset_->norm_feature_val(i, idxF) );

        vector< double > sv(values.begin(), values.end());
        sort( sv.begin(), sv.end());

        if (sv.size()==1)
            continue;

        for ( size_t p=1 ; (p<sv.size()) ; ++p )
        {
            const double diff = sv[p]-sv[p-1];
            if ( diff <= 1e-7 )
                continue;
            epsj[idxF] = min(epsj[idxF], sv[p]-sv[p-1] );
        }

        emax = max(epsj[idxF], emax);
    }
    printf("%g", emax);
}

void MIPPDtree::createConsOneLeaf()
{
    for ( size_t i=0 ; (i<nInsts) ; ++i  )
    {
        char rName[256];
        sprintf(rName, "selLeaf(%zu)", i);

        vector< int >idx( nLeafs );
        for ( size_t idxLeaf=0 ; (idxLeaf<nLeafs) ; ++idxLeaf )
            idx[idxLeaf] = z[i][idxLeaf];

        vector< double >coef( nLeafs, 1.0 );

        lp_add_row(mip, idx.size(), &idx[0], &coef[0], rName, 'E', 1.0);
    }
}

void MIPPDtree::createCVars()
{
    vector< string > cnames;
    for ( size_t idxL=0 ; (idxL<nLeafs) ; ++idxL )
    {
        for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
        {
            char cName[256];
            sprintf(cName, "c(%zu,%zu)", idxL, idxAlg );
            c[idxL][idxAlg] = lp_cols(mip) + cnames.size();
            cnames.push_back( string(cName) );
        }
    }

    vector< double > obj( cnames.size(), 0.0 );
    char **names = to_char_vec(cnames);

    lp_add_bin_cols(mip, cnames.size(), &obj[0], names );
    free(names);
}

void MIPPDtree::createConsLnkParent()
{
    for ( int idxb=1 ; (idxb<(int)branchNodes.size()) ; ++idxb )
    {
        int parent = (idxb-1)/2;

        int idx[] = {d[idxb], d[parent]};
        double coef[] = {1.0, -1,0};
        char rName[256];
        sprintf(rName, "parent(%d)", idxb);
        lp_add_row( mip, 2, idx, coef, rName, 'L', 0.0 );
    }
}

void MIPPDtree::createWVars()
{
    vector< string > cnames;
    vector< double > obj;
    for ( size_t idxInst=0 ; (idxInst<nInsts) ; ++idxInst )
    {
        for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
        {
            char cName[256];
            sprintf( cName, "w(%zu,%zu)", idxInst, idxAlg );
            w[idxInst][idxAlg] = lp_cols(mip) + cnames.size();
            cnames.push_back(cName);
            obj.push_back( rset_->get(idxInst, idxAlg) );
        }
    }

    char **names = to_char_vec(cnames);

    lp_add_bin_cols(mip, cnames.size(), &obj[0], names );
    free(names);
}

void MIPPDtree::createConsLnkWCZ()
{
    for ( size_t i=0 ; (i<nInsts) ; ++i )
    {
        for ( size_t idxL=0 ; (idxL<nLeafs) ; ++idxL )
        {
            vector< int > idx;
            vector< double > coef;

            for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
            {
                idx.push_back(w[i][idxAlg]);
                coef.push_back((((int)idxAlg)+1));
                idx.push_back(c[idxL][idxAlg]);
                coef.push_back(-(((int)idxAlg+1)));
            }

            idx.push_back( z[i][idxL] );
            coef.push_back( -((int)nAlgs) );

            char rName[256]; sprintf( rName, "lnkWCZL(%zu,%zu)", i, idxL );
            lp_add_row( mip, idx.size(), &idx[0], &coef[0], rName, 'G', -((int)nAlgs) );

            *coef.rbegin() = nAlgs;
            sprintf( rName, "lnkWCZU(%zu,%zu)", i, idxL );
            lp_add_row( mip, idx.size(), &idx[0], &coef[0], rName, 'L', nAlgs );
        }

    }
}

void MIPPDtree::createConsSelCLeaf()
{
    for ( size_t idxL=0 ; (idxL<nLeafs) ; ++idxL )
    {
        vector< int > idx( nAlgs + 1 );
        vector< double > coef( nAlgs +1 , 1.0 );
        
        for ( size_t idxAlg=0 ; (idxAlg<nAlgs) ; ++idxAlg )
            idx[idxAlg] = c[idxL][idxAlg];

        *idx.rbegin() = l[idxL];
        *coef.rbegin() = -1.0;

        char rName[256];
        sprintf(rName, "selParamL(%zu)", idxL);
        lp_add_row(mip, idx.size(), &idx[0], &coef[0], rName, 'E', 0.0 );
    }
}

Tree *MIPPDtree::build()
{
    return nullptr;
}

MIPPDtree::~MIPPDtree ()
{
    lp_free( &mip );
}
