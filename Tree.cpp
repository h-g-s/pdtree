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

#include "Branching.hpp"
#include "Instance.hpp"
#include "Node.hpp"
#include "SubSetResults.hpp"
#include "tinyxml2.h"

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
        if (node->n_elements()==112)
        {
            ofstream of("insts.txt");
            for (size_t i=0 ; (i<node->n_elements()) ; ++i )
            {
                const auto inst = iset_.instance( node->elements()[i] );
                of << inst.name() << endl;
            }

            of.close();
        }*/
    
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

void Tree::save( const char *fileName ) const
{
    cout << fileName;
    tinyxml2::XMLDocument doc;
    XMLElement *tree = doc.NewElement("tree");
    doc.InsertFirstChild(tree);

    tree->SetAttribute("improvement", this->improvement);
    tree->SetAttribute("maxDepth", (int) this->maxDepth);
    tree->SetAttribute("minInstancesNode", (int) this->minInstancesNode);

    XMLElement *params = doc.NewElement("pdtreeParams");
    tree->InsertFirstChild(params);
    params->SetAttribute("eval", str_eval(Parameters::eval) );
    params->SetAttribute("fillMissingRes", str_fmrs(Parameters::fmrStrategy) );

    root->writeXML(&doc, tree);

    doc.SaveFile(fileName);
}

Tree::~Tree ()
{
    delete root;
}

