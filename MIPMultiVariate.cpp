#include "MIPMultiVariate.hpp"
#include "InstanceSet.hpp"
#include "ResultsSet.hpp"
#include <string>
#include <vector>
#include <cassert>
#include <cstring>

using namespace std;

static char **to_char_vec( const vector< string > names );

MIPMultiVariate::MIPMultiVariate( const InstanceSet *_iset, const ResultsSet *_rset ) :
    iset_( _iset ),
    rset_( _rset ),
    nAlgs(rset_->algsettings().size()),
    nFeat(iset_->features().size()),
    mip(lp_create())
{
    createYvars();
    createZvars();
    createAvars();
    createWvars();
    b = lp_cols(mip);
    char bn[] = "b";
    lp_add_col(mip, 0.0, 0.0, 1.0, 0, bn, 0, NULL, NULL);

    createConsLnkAYB();
    createConsSelAlgSide();
    createConsSelAlgProblem();
    createConsLNKWYZ();
}

void MIPMultiVariate::createYvars()
{
    y = vector< int >(iset_->instances().size());
    vector<string> cnames;
    for (const auto &inst : iset_->instances())
    {
        string name = "y("+to_string(inst.idx())+")";
        y[cnames.size()] = cnames.size();
        cnames.push_back(name);
    }
    vector< double > obj(cnames.size(), 0.0);
    char **names = to_char_vec(cnames);
    lp_add_bin_cols(mip, cnames.size(), &obj[0], names);
    free(names);
}

void MIPMultiVariate::createZvars()
{
    vector< string > cnames;
    for ( int ia=0 ; ia<nAlgs ; ++ia )
    {
        vector< int > idx(2);
        char name[256];
        sprintf(name, "z(%d,l)", ia);
        idx[0] = lp_cols(mip) + cnames.size();
        cnames.push_back(string(name));

        sprintf(name, "z(%d,r)", ia);
        idx[1] = lp_cols(mip) + cnames.size();
        cnames.push_back(string(name));

        w.push_back( idx );
    }
    vector< double > obj(cnames.size(), 0.0);
    char **names = to_char_vec(cnames);
    lp_add_bin_cols(mip, cnames.size(), &obj[0], names);
    free(names);
}

void MIPMultiVariate::createAvars()
{
    vector< string > cnames;

    for ( int iFeat=0 ; iFeat<nFeat ; ++iFeat )
    {
        char name[256];
        sprintf(name, "a(%d)", iFeat);
        a.push_back(lp_cols(mip)+cnames.size());
        cnames.push_back(string(name));
    }

    vector< double > obj(cnames.size(), 0.0);
    vector< char > is_int(cnames.size(), 0);
    char **names = to_char_vec(cnames);
    lp_add_cols_same_bound(mip, cnames.size(), &obj[0], 0.0, 1.0, &is_int[0], names );
    free(names);
}

void MIPMultiVariate::createConsLnkAYB()
{
    for ( const auto &inst : iset_->instances() )
    {
        vector< int > idx;
        vector< double > coef;
        for ( int iFeat=0 ; (iFeat<nFeat) ; ++iFeat )
        {
            idx.push_back(a[iFeat]);
            coef.push_back(iset_->norm_feature_val_rank(inst.idx(), iFeat));
        }

        idx.push_back(b);
        coef.push_back(-1.0);

        idx.push_back(y[inst.idx()]);
        coef.push_back(1.0);

        char name[256];
        sprintf(name, "lnkAYBp1(%d)", (int)inst.idx());
        lp_add_row(mip, idx.size(), &idx[0], &coef[0], name, 'L', 1.0);

        const double eps = 1.0 / (double)iset_->size();

        *coef.rbegin() = 1.0 + eps;
        sprintf(name, "lnkAYBp2(%d)", (int)inst.idx());
        lp_add_row(mip, idx.size(), &idx[0], &coef[0], name, 'G', eps);
    }
}


void MIPMultiVariate::createConsSelAlgSide()
{
    vector< int >sides = vector<int>({0, 1});

    for ( const auto &side : sides )
    {
        char name[256];
        sprintf(name, "selAlgSide(%d)", side);

        vector< int > idx;
        for ( int ia=0 ; (ia<nAlgs); ++ia )
            idx.push_back(z[ia][side]);

        vector<double> coef(idx.size(), 1.0);
        lp_add_row(mip, idx.size(), &idx[0], &coef[0], name, 'E', 1.0);
    }
}

void MIPMultiVariate::createWvars()
{
    vector< string > cnames;
    vector< double > obj;
    for ( const auto &inst : iset_->instances() ) 
    {
        vector< int > idx;

        for ( int ia=0 ; (ia<nAlgs) ; ++ia )
        {
            idx.push_back(lp_cols(mip)+cnames.size());
            obj.push_back(rset_->res(inst.idx(), ia)*1000.0);
            char name[256];
            sprintf(name, "w(%d,%d)", (int)inst.idx(), ia);
            cnames.push_back(name);
        }
        w.push_back(idx);
    }
    char **names = to_char_vec(cnames);
    lp_add_bin_cols(mip, cnames.size(), &obj[0], names);
    free(names);
}

void MIPMultiVariate::createConsSelAlgProblem()
{
    for ( const auto &inst : iset_->instances() )
    {
        vector< int > idx;
        for ( int ia=0 ; ia<nAlgs ; ++ia )
            idx.push_back(w[inst.idx()][ia]);

        vector< double > coef(idx.size(), 1.0);
        char name[256];
        sprintf(name, "selAlgProb(%d)", (int)inst.idx());
        lp_add_row(mip, idx.size(), &idx[0], &coef[0], name, 'E', 1.0);
    }
}

void MIPMultiVariate::createConsLNKWYZ()
{
    for ( const auto &inst : iset_->instances() )
    {
        for ( int ia=0 ; (ia<nAlgs) ; ++ia )
        {
            int idx[] = {w[inst.idx()][ia], y[inst.idx()], z[ia][0]};
            char name[256];
            {
                double coef[] = { 1.0, -0.5, -0.5};

                sprintf(name, "lnkWYZ(%d,%d)l", (int)inst.idx(), ia);

                lp_add_row(mip, 3, idx, coef, name, 'L', 0.0);
            }
            {
                double coef[] = { 1.0, 0.5, -0.5};

                sprintf(name, "lnkWYZ(%d,%d)r", (int)inst.idx(), ia);

                lp_add_row(mip, 3, idx, coef, name, 'L', 0.5);
            }
        } // algorithms
    } // instances
}

MIPMultiVariate::~MIPMultiVariate()
{
    lp_free(&mip);
}

static char **to_char_vec( const vector< string > names )
{
    size_t spaceVec = (sizeof(char*)*names.size());
    size_t totLen = names.size(); // all \0
    for ( const auto &str : names )
        totLen += str.size();
    totLen *= sizeof(char);

    char **r = (char **)malloc(spaceVec+totLen);
    assert( r );
    r[0] = (char *)(r + names.size());
    for ( size_t i=1 ; (i<names.size()) ; ++i )
        r[i] = r[i-1] + names[i-1].size() + 1;

    for ( size_t i=0 ; (i<names.size()) ; ++i )
        strcpy(r[i], names[i].c_str());

    return r;
}


