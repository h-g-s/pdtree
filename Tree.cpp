/*
 * Tree.cpp
 *
 *  Created on: 11 de mar de 2019
 *      Author: haroldo
 */

#include "Tree.hpp"

#include <cstddef>
#include <cfloat>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>
#include <vector>
#include <cstring>
#include <unordered_map>

#include "InstanceSet.hpp"
#include "ResultsSet.hpp"
#include "Instance.hpp"
#include "Node.hpp"
#include "SubSetResults.hpp"
#include "tinyxml2.h"
#include "ResTestSet.hpp"

using namespace std;

Tree::Tree( const InstanceSet *_iset, const ResultsSet *_rset, const ResTestSet *_rtest ) :
    iset_(_iset),
    rset_(_rset),
    root_(nullptr),
    avCostRoot(DBL_MAX),
    avCostLeafs(DBL_MAX),
    avRankRoot(DBL_MAX),
    avRankLeafs(DBL_MAX),
    costImprovement(DBL_MAX),
    rankImprovement(DBL_MAX),
    maxDepth(0),
    minInstancesNode(INT_MAX),
    rtest_(_rtest)
{

}

const char *Tree::node_label( const Node *node ) const
{
    stringstream ss;
    string algsetting = rset_->algsettings()[node->bestAlg() ];
    double res = node->nodeCost();

    ss << "     <table border=\"1\" cellspacing=\"1\" cellborder=\"1\" bgcolor=\"LightYellow\">" << endl;
    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"Indigo\"><b>" << node->nEl_ << " instances:</b></font></td>" << endl;
    ss << "     </tr>" << endl;
    for ( int i=0 ; (i<(int)min(((int)5), ((int)node->nEl_))) ; ++i )
    {
        ss << "     <tr>" << endl;
        ss << "      <td align=\"left\"><font color=\"Indigo\"><i>" << iset_->instance(node->el_[i]).name() << "</i></font></td>" << endl;
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

    strncpy( (char*)&nLabel[0], (const char *) ss.str().c_str(), 8192 );

    return &nLabel[0];
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
        nInstLevel[n->depth_] += n->nEl_;

    for ( auto n : nodes_ )
    {
        levelNodes[n->depth_].push_back(n);
        perfLevel[n->depth_] += (n->nodeCost()) * (((double)n->nEl_) / ((double)nInstLevel[n->depth_]));
    }

    for ( auto nl : levelNodes )
    {
        size_t level = nl.first;
        const auto &nodes = nl.second;
        of << "  subgraph clusterdepth" << level << " {" << endl;
        of << "    style=filled;" << endl;
        of << "    color=\"PaleGreen\";" << endl;
        of << "    label=< <b>Depth " << level << " cost: " << perfLevel[level] << "</b> >;" << endl;

        for ( auto ni=nodes.rbegin() ; (ni!=nodes.rend()) ; ++ni )
        {
            auto n = *ni;
            of << "  \"" << n->id() << "\" [";
            of << "    label = <" << endl;
            of << node_label(n);
            of << "> shape=\"box\" fillcolor=\"LightYellow\" ];" << endl;
        }
 
        of << "  }" << endl;
    }

   of << endl;

    for ( auto n : nodes_ )
    {
        const Node **childs = (const Node **)n->child();
        if (childs[0])
        {
            assert( childs[1] );

            {
                const Node *c = childs[0];
                char lbl[256];
                sprintf(lbl, "%s≤%g", iset_->features()[n->branchFeature()].c_str(), n->branchValue());
                of << "    " << n->id() << " -> " << c->id() << " [label=\""<< lbl << "\"];" << endl;
            }
            {
                const Node *c = childs[1];
                char lbl[256];
                sprintf(lbl, "%s>%g", iset_->features()[n->branchFeature()].c_str(), n->branchValue());
                of << "    " << n->id() << " -> " << c->id() << " [label=\""<< lbl << "\"];" << endl;
            }
        }
    }

    of << "}" << endl;

    of.close();
}

Node *Tree::create_root()
{
    assert( root_ == nullptr );
    root_ = new Node(this->iset_, this->rset_);

    nodes_.push_back( root_ );

    return root_;
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

    addElement(&doc, tree, "maxDepth", (int)this->maxDepth );
    addElement(&doc, tree, "minInstancesNode", (int)this->minInstancesNode );
    addElement(&doc, tree, "instancesFile", Parameters::instancesFile.c_str() );
    addElement(&doc, tree, "experimentsFile", Parameters::resultsFile.c_str() );
    addElement(&doc, tree, "nInstances", (int)iset_->instances().size() );
    addElement(&doc, tree, "nAlgorithms", (int)rset_->algsettings().size() );
    addElement(&doc, tree, "bestAlgRoot", rset_->algsettings()[root_->bestAlg()].c_str() );
    addElement(&doc, tree, "avCostRoot", this->avCostRoot );
    addElement(&doc, tree, "avCostLeafs", this->avCostLeafs );
    addElement(&doc, tree, "costImprovement", this->costImprovement );
    addElement(&doc, tree, "avRankRoot", this->avRankRoot );
    addElement(&doc, tree, "avRankLeafs", this->avRankLeafs );
    addElement(&doc, tree, "rankImprovement", this->rankImprovement );

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

    root_->writeXML(&doc, tree);

    doc.SaveFile(fileName);
}

void Tree::addNode( Node *_node )
{
    this->nodes_.push_back(_node);
    this->minInstancesNode = min(this->minInstancesNode, (int)_node->n_elements());
    this->maxDepth = max(this->maxDepth, _node->depth());
}

double Tree::evaluate( const Dataset *testData ) const
{
    double res = 0.0;
    
    return res;
}

void Tree::computeCost()
{
    root_->setCostRoot();
    
    this->avCostRoot = root_->nodeCost_;
    this->avRankRoot = root_->avRank;
    
    long double avCostLeafs = 0.0;
    long double avRankLeafs = 0.0;
    
    for ( auto &n : nodes_ )
    {
        if ( (!n->isLeaf()) || (n->n_elements()<=0))
            continue;
        
        n->computeCost();
        avCostLeafs += (long double)n->nodeCost_ / (long double) n->n_elements() ;
        avRankLeafs += (long double)n->avRank / (long double) n->n_elements() ;
    }
    
    this->costImprovement = avCostRoot / avCostLeafs;
    this->rankImprovement = avRankRoot / avRankLeafs;
}

Tree::~Tree ()
{
    delete root_;
}

