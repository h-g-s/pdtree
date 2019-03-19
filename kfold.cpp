#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
#include "Parameters.hpp"
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"
#include "Tree.hpp"
#include "ResTestSet.hpp"

using namespace std;

// 1: basic log
// 2: very detailed log, saving files
int save_log = 2;

int main( int argc, const char **argv )
{
    if (argc<4)
    {
        cerr << "usage: kfold dataset results k [options]" << endl;
        exit(1);
    }
    int k = atoi(argv[3]);
    Parameters::parse(argc, argv);
    Parameters::print();

    FILE *fkfold = NULL;
    if (save_log>=2)
        fkfold=fopen("kfold.csv", "w");

    for ( int i=0 ; (i<k) ; ++i )
    {
        cout << "performing step " << i+1 << " of " <<
             k << "-fold validation" << endl;
        InstanceSet trainSet( argv[1], argv[2], i, k );
        ResultsSet trainRes( trainSet, argv[2], Parameters::fmrStrategy );
        const Dataset *test_data = trainSet.test_dataset_;
        unordered_map< string, size_t > instsTest;
        for ( size_t ii=0 ; ii<test_data->rows() ; ++ii )
            instsTest[test_data->str_cell(ii, 0)] = ii;
        unordered_map< string, size_t > algs;
        size_t idxa=0;
        for ( const auto &alg : trainRes.algsettings() )
            algs[alg] = idxa++;

        ResTestSet resTestSet(instsTest, algs, argv[2]);
        Tree tree(trainSet, trainRes, &resTestSet);
        tree.build();
        double rtest = tree.evaluate(trainSet.test_dataset_);
        cout << "result train: " << tree.leafResults() << " result test: " << rtest << endl << endl;

        if (fkfold)
            fprintf(fkfold, "%d,%g,%g\n", i+1, tree.leafResults(), rtest );

        if (save_log>=2)
        {
            char trainName[256]; sprintf(trainName, "train-%d-%d.csv", i+1, k);
            trainSet.inst_dataset_->write_csv(trainName);

            char testName[256]; sprintf(testName, "test-%d-%d.csv", i+1, k);
            trainSet.test_dataset_->write_csv(testName);
            
            char testResName[256]; sprintf(testResName, "restest-%d-%d.csv", i+1, k);
            resTestSet.save(testResName);

            char treeXMLName[256]; sprintf(treeXMLName, "tree-%d-%d.xml", i+1, k);
            tree.save(treeXMLName);

            char treeGVName[256]; sprintf(treeGVName, "tree-%d-%d.gv", i+1, k);
            tree.draw(treeGVName);

            char treePDFName[256]; sprintf(treePDFName, "tree-%d-%d.pdf", i+1, k);

            char drawCmd[1024] = "";
            sprintf(drawCmd, "dot -Tpdf %s -o %s", treeGVName, treePDFName );
            system(drawCmd);
        }
    }

    if (fkfold)
        fclose(fkfold);
}

