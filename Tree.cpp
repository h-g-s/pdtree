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
#include <iostream>
#include <sstream>
#include <vector>

#include "Node.hpp"
#include "SubSetResults.hpp"

using namespace std;

Tree::Tree ( const InstanceSet &_iset, const ResultsSet &_rset ) :
    iset_(_iset),
    rset_(_rset),
    root(nullptr)
{

}

std::string Tree::node_label( const Node *node ) const
{
    stringstream ss;
    string algsetting = rset_.algsettings()[node->ssres_.bestAlg()];
    double res = node->ssres_.bestAlgRes();

    ss << "     <table border=\"1\" cellspacing=\"1\" cellborder=\"1\">" << endl;
    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"Indigo\"><b>" << node->nEl_ << " instances:</b></font></td>" << endl;
    ss << "     </tr>" << endl;
    for ( int i=0 ; (i<(int)min(((int)5), ((int)node->nEl_))) ; ++i )
    {
        ss << "     <tr>" << endl;
        ss << "      <td align=\"left\"><font color=\"Indigo\">" << iset_.instance(node->el_[i]).name() << "</font></td>" << endl;
        ss << "     </tr>" << endl;
    }
    if (node->nEl_>5)
    {
        ss << "     <tr>" << endl;
        ss << "      <td align=\"center\"><font color=\"Indigo\">...</font></td>" << endl;
        ss << "     </tr>" << endl;
    }

    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"DarkGreen\">Best setting:</font></td>" << endl;
    ss << "     </tr>" << endl;
    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"DarkGreen\">" << algsetting <<  "</font></td>" << endl;
    ss << "     </tr>" << endl;
    ss << "     <tr>" << endl;
    ss << "      <td align=\"center\"><font color=\"DarkGreen\">" << res <<  "</font></td>" << endl;
    ss << "     </tr>" << endl;
    ss << "    </table>" << endl;;

    return ss.str();
}

void Tree::draw( const char *fileName )
{
    ofstream of(fileName);
    of << "digraph G {" << endl;
    for ( auto n : nodes_ )
    {
        of << "  \"" << n->id.c_str() << "\" [";
        of << "    label = <" << endl;
        of << node_label(n);
        of << "> shape=\"box\" ];" << endl;
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

        node->perform_branch();

        auto child = node->child();
        if (child.size() == 0)
            leafs_.push_back(node);
        else
            for ( auto c : child )
                queue.push_back(c);
    }
}

Tree::~Tree ()
{
    delete root;
}
