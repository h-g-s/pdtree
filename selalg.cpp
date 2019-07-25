/*
 * mpdt.cpp
 *
 *  Created on: 20 de mar de 2019
 *      Author: haroldo
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include "InstanceSet.hpp"
#include "MIPPDtree.hpp"
#include "Parameters.hpp"
#include "ResultsSet.hpp"
#include "Tree.hpp"
#include "Greedy.hpp"
#include "MIPSelAlg.hpp"

using namespace std;

int main( int argc, char **argv )
{
    if (argc<3)
    {
        fprintf(stderr, "usage: selalg instanceSet resultsSet [options]");
        exit(1);
    }

    Parameters::parse( argc, (const char **)argv );
    Parameters::print();
    cout << endl;

    cout << "reading instances ... " << endl;
    InstanceSet iset( argv[1], argv[2] );
    if (Parameters::isetCSVNorm.size())
        iset.save(Parameters::isetCSVNorm.c_str(), true);
    if (Parameters::isetCSVNormR.size())
        iset.saveNormRank(Parameters::isetCSVNormR.c_str());
    cout << endl;

    cout << "reading results ... " << endl;
    ResultsSet rset( iset, argv[2] );
    if (Parameters::rsetCSV.size())
        rset.save_csv(Parameters::rsetCSV.c_str());
    cout << endl;
    
    int newMEB = (int) ceil(((double)iset.size())*((double)Parameters::minPercElementsBranch));
    if (newMEB>Parameters::minElementsBranch)
    {
        cout << "minElementsBranch increased to " << newMEB << defaultfloat <<
            setprecision(3) << ", " << Parameters::minPercElementsBranch*100.0 << "\% of instance set size" << endl;
        Parameters::minElementsBranch = newMEB;
    }    

    MIPSelAlg msa(&rset);

    msa.optimize(Parameters::maxSeconds);

    string fileName = "res-" + to_string(rset.algsettings().size()) + 
            "-" + to_string(Parameters::maxAlgs)  + ".csv";
    
    msa.saveFilteredResults(fileName.c_str());

    exit(0);
}
