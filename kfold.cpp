#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include "Parameters.hpp"
#include "InstanceSet.hpp"


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

        char trainFN[256];
        sprintf(trainFN, "train-%d-%d.csv", i, k);
        trainSet.inst_dataset_->write_csv(trainFN);

        char testFN[256];
        sprintf(testFN, "test-%d-%d.csv", i, k);
        trainSet.test_dataset_->write_csv(testFN);
    }
}

