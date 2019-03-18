#include <iostream>
#include "Parameters.hpp"
#include "Dataset.hpp"

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
    cout << "starting " << k << "-fold validation" << endl;

//Dataset data();
    //size_t testSize = 
    for ( int i=0 ; (i<k) ; ++i )
    {

    }
}

