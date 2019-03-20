/*
 * MIPPDtree.cpp
 *
 *  Created on: 20 de mar de 2019
 *      Author: haroldo
 */

#include "MIPPDtree.hpp"
#include "InstanceSet.hpp"

MIPPDtree::MIPPDtree( const InstanceSet *_iset, const ResultsSet *_rset ) :
    iset_( _iset ),
    rset_( _rset ),
    mip( lp_create() )
{

}

MIPPDtree::~MIPPDtree ()
{
    lp_free( &mip );
}

