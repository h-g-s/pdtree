/*
 * mpdt.cpp
 *
 *  Created on: 20 de mar de 2019
 *      Author: haroldo
 */

#include <cstdio>
#include <cstdlib>

#include "InstanceSet.hpp"
#include "MIPPDtree.hpp"
#include "Parameters.hpp"
#include "ResultsSet.hpp"
#include "Tree.hpp"
#include "Greedy.hpp"
#include "MIPSelAlg.hpp"


int main( int argc, char **argv )
{
    if (argc<3)
    {
        fprintf(stderr, "usage: mpdt instanceSet resultsSet [options]");
        exit(1);
    }

    Parameters::parse( argc, (const char **)argv );

    InstanceSet iset( argv[1], argv[2] );
    //iset.save("instances-norm.csv", true);
    ResultsSet rset( iset, argv[2] );
    //rset.save_csv("results.csv");
    //
    
    rset.saveFilteredDataSets("features-pp.csv", "results-pp.csv");

    rset.saveInstanceSum("instsum.csv");
    rset.saveAlgSummary("algsum.csv");

    MIPSelAlg msalg(&rset);
    msalg.optimize(1500);
    msalg.saveFilteredResults("results-msa.csv");

    exit(0);

    Greedy grd(&iset, &rset);
    Tree *greedyT = grd.build();
    greedyT->save("gtree.xml");
    greedyT->draw("gtree.gv");
    delete greedyT;


    MIPPDtree mpdt( &iset, &rset );

    const Tree *tree = mpdt.build( 60 );
    if (tree)
    {
        tree->draw("tree.gv");
        tree->save("tree.xml");
    }

    exit(0);
}
