#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <cmath>
#include <quadmath.h>
#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <unordered_set>
#include "Dataset.hpp"
#include "Instance.hpp"
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"

using namespace std;

int main(int argc, char **argv)
{
    if (argc<3)
    {
        cerr << "usage: ddtre instances.csv algresults.csv" << endl;
        exit(1);
    }

    // loading instance data
    InstanceSet iset(argv[1], argv[2]);

    ResultsSet rset( iset, argv[2] );

    return 0;
}


