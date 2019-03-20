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

#include "Parameters.hpp"

using namespace std;

static char **to_char_vec( const std::vector< std::string > names );

MIPPDtree::MIPPDtree( const InstanceSet *_iset, const ResultsSet *_rset ) :
    iset_( _iset ),
    rset_( _rset ),
    mip( lp_create() )
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


}

void MIPPDtree::createBVars()
{
    vector< double > lb( branchNodes.size(), 0.0 );
    vector< double > ub( branchNodes.size(), 1.0 );
    vector< double > obj( branchNodes.size(), 0.0 );
    vector< char > isint( branchNodes.size(), 0 );
    vector< string > cnames(branchNodes.size());
    for ( size_t i=0 ; (i<branchNodes.size()) ; ++i )
        cnames[i] = "b(" + branchNodes[i] + ")";

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


MIPPDtree::~MIPPDtree ()
{
    lp_free( &mip );
}


