#include <iostream>
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"

using namespace std;

int main(int argc, const char **argv)
{
    if (argc<3)
    {
        cerr << "usage: ddtre instances.csv algresults.csv -param1=value1 -param2=value2 ..." << endl;
        cout << "options:" << endl;
        ResultsSet::help();
        exit(1);
    }

    try
    {
        cout << "Loading instances set ... "  << endl;
        InstanceSet iset(argv[1], argv[2]);
        Instance::inst_dataset = iset.inst_dataset_;
        cout << endl;

        cout << "ResultsSet settings: " << endl;
        ResultsSet::configure_parameters(argc, (const char **)argv);
        ResultsSet::print_config();
        cout << "Loading results set ... "  << endl;
        ResultsSet rset( iset, argv[2] );
        rset.print_summarized_results();
    } catch (std::string &str)
    {
        cout << endl; cerr << endl;
        cerr << "ERROR: " << str << endl;
        return 1;
    }

    return 0;
}
