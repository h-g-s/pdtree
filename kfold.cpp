#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "Parameters.hpp"
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"
#include "Tree.hpp"

using namespace std;

int main( int argc, const char **argv )
{
    if (argc<4)
    {
        cerr << "usage: kfold dataset results k" << endl;
        exit(1);
    }
    int k = atoi(argv[3]);
    Parameters::parse(argc, argv);
    Parameters::print();

    for ( int i=0 ; (i<k) ; ++i )
    {
        cout << "performing step " << i << " of " <<
             k << "-fold validation" << endl;
        InstanceSet trainSet( argv[1], argv[2], i, k );
        ResultsSet trainRes( trainSet, argv[2], Parameters::fmrStrategy );
        Tree tree(trainSet, trainRes);
        tree.build();
    }
}

