/*
 * Tree.cpp
 *
 *  Created on: 11 de mar de 2019
 *      Author: haroldo
 */

#include "Tree.hpp"

#include <stddef.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
#include <unordered_map>

#include "Branching.hpp"
#include "Instance.hpp"
#include "Node.hpp"
#include "SubSetResults.hpp"
#include "tinyxml2.h"
#include "ResTestSet.hpp"

using namespace std;

Tree::Tree ( const InstanceSet &_iset, const ResultsSet &_rset ) :
    iset_(_iset),
    rset_(_rset),
    root(nullptr),
    resultLeafs(0.0),
    improvement(0.0),
    maxDepth(0.0),
    minInstancesNode(numeric_limits<size_t>::max())
{

}

std::string Tree::node_label( const Node *node ) const
{
    stringstream ss;
    string algsetting = rset_.algsettings()[node->ssres_.bestAlg()];
    double res = node->ssres_.bestAlgRes();

    ss << "     <table border=\"1\" cellspacing=\"1\" cellborder=\"1\" bgcolor=\"LightYellow\">" << endl;
    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"Indigo\"><b>" << node->nEl_ << " instances:</b></font></td>" << endl;
    ss << "     </tr>" << endl;
    for ( int i=0 ; (i<(int)min(((int)5), ((int)node->nEl_))) ; ++i )
    {
        ss << "     <tr>" << endl;
        ss << "      <td align=\"left\"><font color=\"Indigo\"><i>" << iset_.instance(node->el_[i]).name() << "</i></font></td>" << endl;
        ss << "     </tr>" << endl;
    }
    if (node->nEl_>5)
    {
        ss << "     <tr>" << endl;
        ss << "      <td align=\"center\"><font color=\"Indigo\">...</font></td>" << endl;
        ss << "     </tr>" << endl;
    }

    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"DarkGreen\"><b>Best algorithm setting:</b></font></td>" << endl;
    ss << "     </tr>" << endl;
    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"Black\">" << algsetting <<  "</font></td>" << endl;
    ss << "     </tr>" << endl;
    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"DarkGreen\">" << setprecision(14) << res <<  "</font></td>" << endl;
    ss << "     </tr>" << endl;
    ss << "    </table>" << endl;;

    return ss.str();
}

void Tree::draw( const char *fileName ) const
{
    ofstream of(fileName);
    of << "digraph G {" << endl;
    of << " graph [fontname = \"helvetica\"];" << endl;
    of << " node [fontname = \"helvetica\"];" << endl;
    of << " edge [fontname = \"helvetica\"];" << endl;

    map< size_t, vector< Node * > > levelNodes;
    map< size_t, double > perfLevel;
    map< size_t, int > nInstLevel;
    for ( auto n : nodes_ )
        nInstLevel[n->depth] += n->nEl_;

    for ( auto n : nodes_ )
    {
        levelNodes[n->depth].push_back(n);
        perfLevel[n->depth] += (n->ssres_.bestAlgRes()) * (((double)n->nEl_) / ((double)nInstLevel[n->depth]));
    }

    for ( auto nl : levelNodes )
    {
        size_t level = nl.first;
        const auto &nodes = nl.second;
        of << "  subgraph clusterdepth" << level << " {" << endl;
        of << "    style=filled;" << endl;
        of << "    color=\"PaleGreen\";" << endl;
        of << "    label=< <b>Depth " << level << " cost: " << perfLevel[level] << "</b> >;" << endl;
        for ( auto n : nodes )
        {
            of << "  \"" << n->id.c_str() << "\" [";
            of << "    label = <" << endl;
            of << node_label(n);
            of << "> shape=\"box\" fillcolor=\"LightYellow\" ];" << endl;
        }
 
        of << "  }" << endl;
    }

   of << endl;

    for ( auto n : nodes_ )
    {
        bool left = true;
        for ( auto c : n->child() )
        {
            string lbl = iset_.features()[n->best_branch().idxF_] +
                    (left ? string("≤") : string(">")) + n->best_branch().as_str();
            of << "    " << n->id << " -> " << c->id << " [label=\""<< lbl << "\"];" << endl;
            left = not left;
        }
    }

    of << "}" << endl;

    of.close();
}

void Tree::build()
{
    if (root != nullptr)
    {
        cerr << "tree already built" << endl;
        abort();
    }

    root = new Node( iset_, rset_ );

    vector< Node * > queue;
    queue.push_back( root );

    while (queue.size())
    {
        Node *node = queue.back();
        nodes_.push_back(node);
        queue.pop_back();

        /*
        if (node->n_elements()==79)
        {
            ofstream of("insts.txt");
            for (size_t i=0 ; (i<node->n_elements()) ; ++i )
            {
                const auto inst = iset_.instance( node->elements()[i] );
                of << inst.name() << endl;
            }

            of.close();
        } */
    
        this->maxDepth = max( this->maxDepth, node->depth+1 );
        this->minInstancesNode = min( this->minInstancesNode, node->n_elements() );

        node->perform_branch();

        auto child = node->child();
        if (child.size() == 0)
            leafs_.push_back(node);
        else
            for ( auto c : child )
                queue.push_back(c);
    }

    double rootRes = root->result().bestAlgRes();
    long double resLeafs = 0.0;
    for ( auto l : leafs_ )
        resLeafs  += l->result().bestAlgRes() * ( (long double) l->n_elements() / (long double) root->n_elements() );

    this->resultLeafs = (double) resLeafs;

    if (resultLeafs!=0.0)
        improvement = rootRes / resultLeafs;
    else
        improvement = 0.0;
}

using namespace tinyxml2;

static void addElement( tinyxml2::XMLDocument *doc,  XMLElement *el, const char *name, const double value )
{
    auto el2 = doc->NewElement(name);
    el->InsertEndChild(el2);
    el2->SetText(to_string(value).c_str());
}

static void addElement( tinyxml2::XMLDocument *doc,  XMLElement *el, const char *name, const int value )
{
    auto el2 = doc->NewElement(name);
    el->InsertEndChild(el2);
    el2->SetText(to_string(value).c_str());
}

static void addElement( tinyxml2::XMLDocument *doc,  XMLElement *el, const char *name, const char *value )
{
    auto el2 = doc->NewElement(name);
    el->InsertEndChild(el2);
    el2->SetText(value);
}

void Tree::save( const char *fileName ) const
{
    cout << fileName;
    tinyxml2::XMLDocument doc;
    XMLElement *tree = doc.NewElement("tree");
    doc.InsertFirstChild(tree);

    addElement(&doc, tree, "improvement", this->improvement );
    addElement(&doc, tree, "maxDepth", (int)this->maxDepth );
    addElement(&doc, tree, "minInstancesNode", (int)this->minInstancesNode );
    addElement(&doc, tree, "instancesFile", Parameters::instancesFile.c_str() );
    addElement(&doc, tree, "experimentsFile", Parameters::resultsFile.c_str() );
    addElement(&doc, tree, "nInstances", (int)iset_.instances().size() );
    addElement(&doc, tree, "nAlgorithms", (int)rset_.algsettings().size() );

    XMLElement *params = doc.NewElement("PDTreeParameters");
    tree->InsertEndChild(params);

    addElement(&doc, params, "eval", str_eval(Parameters::eval) );
    addElement(&doc, params, "fillMissingRes", str_fmrs(Parameters::fmrStrategy) );
    addElement(&doc, params, "rankEps", Parameters::rankEps );
    addElement(&doc, params, "rankPerc", Parameters::rankPerc );
    addElement(&doc, params, "minElementsBranch", (int)Parameters::minElementsBranch );
    addElement(&doc, params, "maxDepth", (int)Parameters::maxDepth );
    addElement(&doc, params, "minPerfImprov", (int)Parameters::minPerfImprov );
    addElement(&doc, params, "minAbsPerfImprov", (int)Parameters::minAbsPerfImprov );

    root->writeXML(&doc, tree);

    doc.SaveFile(fileName);
}

const Node *Tree::node_instance( const Dataset *testd, size_t idxInst ) const
{
    const Node *node = this->root;
    while (node->bestBranch_.found())
    {
        assert(node->child_.size()==2);

        // check if instance is at left or right
        size_t idxF = node->bestBranch_.idxF_;
        std::string colName = node->iset_.inst_dataset_->headers()[idxF];
        size_t idxFtd = iset_.test_dataset_->colIdx(colName);
        assert( iset_.types()[idxF] == iset_.test_dataset_->types()[idxFtd] );

        if (iset_.feature_is_float(idxF))
        {
            double vinst = testd->float_cell(idxInst, idxFtd);
            if (vinst<=node->bestBranch_.value_.vfloat)
                node = node->child_[0];
            else
                node = node->child_[1];
        }
        else
        {
            if (iset_.feature_is_integer(idxF))
            {
                int vinst = testd->int_cell(idxInst, idxFtd);
                if (vinst<=node->bestBranch_.value_.vint)
                    node = node->child_[0];
                else
                    node = node->child_[1];
            }
            else
            {
                cerr << "branch type not supported" << endl;
                exit(1);
            }
        }
    } // continue branch

    return node;
}

double Tree::cost_instance( const Dataset *testd, size_t idxInst, const ResTestSet *rtst ) const
{
    const Node *nodeInst = this->node_instance(testd, idxInst);
    size_t idxAlg = nodeInst->ssres_.bestAlg();

    return rtst->get(idxInst, idxAlg);
}

double Tree::evaluate( const Dataset *testData ) const
{
    unordered_map< std::string, size_t > insts;
    for ( size_t idxInst = 0 ; (idxInst<iset_.test_dataset_->rows()) ; ++idxInst )
        insts[iset_.test_dataset_->str_cell(idxInst, 0)] = idxInst;
    unordered_map< std::string, size_t > algs;
    size_t idxa = 0;
    for ( const auto &alg : rset_.algsettings() )
        algs[alg] = idxa++;

    ResTestSet rtst( insts, algs, Parameters::resultsFile.c_str() );

    long double sum = 0.0;

    for ( size_t i=0 ; (i<testData->rows()) ; ++i )
    {
        const double instCost = cost_instance(iset_.test_dataset_, i, &rtst);
        //cout << "test instance " << testData->str_cell(i, 0) << " cost: " << instCost << endl;
        sum += instCost;
    }

    long double res = (sum) / ((long double)testData->rows());
    return res;
}

Tree::~Tree ()
{
    delete root;
}

