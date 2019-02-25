#include <iostream>
#include "Dataset.hpp"

using namespace std;

int main(int argc, char **argv)
{
    if (argc<2)
    {
        cerr << "enter dataset file" << endl;
        exit(1);
    }
    Dataset inst(argv[1]);
    return 1;
}
