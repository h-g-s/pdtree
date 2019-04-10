/*
 * InstanceSet.cpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#include <fstream>
#include <cfloat>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <algorithm>
#include <ctime>
#include <unordered_set>
#include <algorithm>
#include "InstanceSet.hpp"
#include "pdtdefines.hpp"
#include "Parameters.hpp"

using namespace std;

static std::unordered_set< std::string > check_instances_with_results( const char *resFN );

InstanceSet::InstanceSet (const char *fileName, const char *resultsFileName, int ifold, int kfold ) :
    inst_dataset_(new Dataset(fileName)),
    test_dataset_(nullptr),
    instFeatRank(nullptr)
{
    if (kfold>=2)
    {
        if (ifold<0 or ifold>=kfold)
        {
            cerr << "invalid training partition " << ifold << endl;
            exit(1);
        }
    }
    clock_t start = clock();
    auto ires = check_instances_with_results(resultsFileName);
    {
        vector< bool > included;
        vector< bool > inclTest;
        vector< size_t > elements;
        included.reserve(inst_dataset_->rows());
        size_t nel = 0;
        for ( size_t i=0 ; (i<inst_dataset_->rows()) ; ++i )
        {
            string iname = string(inst_dataset_->str_cell(i, 0));
            bool hasResult = ires.find(iname)!=ires.end();
            included.push_back( hasResult );
            if (hasResult)
            {
                ++nel;
                elements.push_back(i);
            }
        }
        if ( kfold>=2 )
        {
            if ((int)nel<kfold)
            {
                cerr << "cannot perform kfold (k=" << kfold 
                    << ") validation since there are results only for " 
                    << ires.size() << "instances." << endl;
                exit(1);
            }

            // removing elements from test dataset
            size_t tsize = floor(((double)elements.size()) * (((double)1.0)/((double)kfold)));
            size_t tstart = tsize * ifold;
            size_t tend = tstart + tsize;
            for ( size_t i=tstart ; (i<tend) ; ++i )
                included[elements[i]] = false;

            inclTest = vector< bool >( included.size(), false );
            for ( size_t i=tstart ; (i<tend) ; ++i )
                inclTest[elements[i]] = true;
        }

        Dataset *ds = new Dataset(*inst_dataset_, included);
        if (kfold>=2)
            test_dataset_ = new Dataset( *inst_dataset_, inclTest );

        delete inst_dataset_;
        inst_dataset_ = ds;
    }

    auto itf = inst_dataset_->headers().begin(); ++itf;
    features_ = vector<string>(itf, inst_dataset_->headers().end() );
    auto itt = inst_dataset_->types().begin(); ++itt;
    types_ = vector<Datatype>(itt, inst_dataset_->types().end());
    assert(features_.size()==types_.size());

    instances_.reserve(ires.size());

    size_t discarded = 0;
    size_t idxInst = 0;
    for ( size_t i=0 ; (i<inst_dataset_->rows()) ; ++i )
    {
        string iname = string(inst_dataset_->str_cell(i, 0));
        if (ires.find(iname)==ires.end())
        {
            ++discarded;
            continue;
        }
        instances_.push_back( Instance(idxInst) );
        auto it = instByName_.find(iname);
        if (it!=instByName_.end())
            throw string("instance " + iname + " appears twice in the instance list");
        instByName_[iname] = idxInst;
        ++idxInst;
    }
    double secs = (double(clock()-start)) / ((double)CLOCKS_PER_SEC);
    cout << instances_.size() << " instances loaded in " << setprecision(3) << secs << endl;
    if (discarded)
        cout << discarded << " instances were discarded because no experiments were performed with them" << endl;

    if (Instance::inst_dataset != nullptr)
    {
        cerr << "only one instance dataset is allowed at time" << endl;
        exit(1);
    }

    limitsFeature = vector< pair<double, double> >( features().size(), make_pair( (double) DBL_MAX, (double)DBL_MIN ) );


    Instance::inst_dataset = this->inst_dataset_;

    for ( size_t idxF=0 ; (idxF<features().size()) ; ++idxF )
    {
        if (feature_is_integer(idxF))
        {
            for ( const auto &inst : instances() )
            {
                limitsFeature[idxF].first = min( limitsFeature[idxF].first, (double)inst.int_feature(idxF) );
                limitsFeature[idxF].second = max( limitsFeature[idxF].second, (double)inst.int_feature(idxF) );
            }
        }
        else
        {
            if (feature_is_float(idxF))
            {
                for ( const auto &inst : instances() )
                {
                    limitsFeature[idxF].first = min( limitsFeature[idxF].first, (double)inst.float_feature(idxF) );
                    limitsFeature[idxF].second = max( limitsFeature[idxF].second, (double)inst.float_feature(idxF) );
                }
            }
        }
    } // all features

    featureValRank = vector< map< double, int > >( features().size(), map< double, int >() );
    featureRankVal = vector< map< int, double > >( features().size(), map< int, double >());

    rankingsF_ = vector< int >( features().size(), 0 );

    nElementsFeatRank_ = vector< vector< int > >( features_.size() );

    nValidBF_ = vector< int >(features().size(), 0);

    for ( size_t idxF=0 ; (idxF<features().size()) ; ++idxF )
    {
        std::unordered_map< double, int > values;
        for ( const auto &inst : instances() )
        {
            const double v = norm_feature_val(inst.idx(), idxF);
            values[v]++;
        }

        std::vector< pair<double, int> > svalues;
        for ( const auto &v : values )
            svalues.push_back( make_pair(v.first, v.second) );

        std::sort( svalues.begin(), svalues.end() );

        double prev = svalues.begin()->first -1;

        int rank = -1;
        int nElLeft = 0;
        double vsplit = 0;
        double minDif = 1e-10;
        bool undefBorder = true;
        for ( const auto v : svalues )
        {
            nElLeft += v.second;

            if ( (v.first-prev>=minDif) && (nElLeft>=Parameters::minElementsBranch) && (undefBorder||((size()-nElLeft)>=Parameters::minElementsBranch))  )
            {
                rank++;
                vsplit = v.first;

                if ( ((int)size())-nElLeft >= ((int)Parameters::minElementsBranch) )
                    nValidBF_[idxF]++;
                else
                    undefBorder = false;

            }
            else
                rank = max(rank, 0);

            while ( ((int)nElementsFeatRank_[idxF].size()) <= rank )
                nElementsFeatRank_[idxF].push_back(0);

            nElementsFeatRank_[idxF][rank] += v.second;

            featureValRank[idxF][v.first] = rank;
            featureRankVal[idxF][rank] = vsplit;
            prev = vsplit;
        }

        rankingsF_[idxF] = rank+1;

#ifdef DEBUG
        {
            int sum = 0;
            for ( int ir=0 ; (ir<(int)rankingsF_[idxF]) ; ++ir )
                sum += nElementsFeatRank_[idxF][ir];
            assert(sum == (int)size());
        }
#endif
    }

    instFeatRank = new int*[instances_.size()];
    instFeatRank[0] = new int[instances_.size()*features().size()];
    for ( int i=1 ; (i<(int)instances().size()) ; ++i )
        instFeatRank[i] = instFeatRank[i-1] + features().size();
    for ( int i=0 ; (i<(int)instances().size()) ; ++i )
    {
        for ( int f=0 ; (f<(int)features().size()) ; ++f )
        {
            auto itv = featureValRank[f].find(norm_feature_val(i,f));
            assert(itv!=featureValRank[f].end());
            instFeatRank[i][f] = itv->second;
        }
    }
}

int InstanceSet::size() const
{
    return ((int)inst_dataset_->rows());
}

const Instance &InstanceSet::inst_by_name( const std::string &name ) const
{
    auto it = instByName_.find(name);
    if (it == instByName_.end())
        throw string("instance ") + name + string(" not found");

    return instances_[it->second];
}

const Instance &InstanceSet::instance( size_t idx ) const
{
    return instances_[idx];
}

const std::vector<std::string>&
InstanceSet::features () const
{
    return this->features_;
}

const std::vector<Datatype>&
InstanceSet::types () const
{
    return this->types_;
}

InstanceSet::~InstanceSet ()
{
    if (inst_dataset_)
        delete inst_dataset_;

    Instance::inst_dataset = nullptr;

    delete[] instFeatRank[0];
    delete[] instFeatRank;

    if (test_dataset_)
        delete test_dataset_;
}

static string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}


static std::unordered_set< std::string > check_instances_with_results( const char *resFN )
{
    unordered_set< string > res;

    if (resFN==nullptr)
        return res;

    ifstream f(resFN);
    string line;
    while (getline(f, line))
    {
        line = trim(line);
        if (line.size()==0)
            continue;
        auto it = line.find(',');
        if (it==line.size())
            throw "Results file should have the following format: instance,algAndSettings,result";

        auto iname = line.substr(0, it);
        if (iname.size()==0)
            throw "Empty instance name";
        res.insert(iname);
    }
    f.close();

    return res;
}

bool InstanceSet::feature_is_integer( size_t idxF ) const
{
    assert( idxF<features().size() );

    return (types_[idxF] <= Integer);
}

bool InstanceSet::feature_is_float( size_t idxF ) const
{
    assert( idxF<features().size() );

    return (types_[idxF] == Float);
}

double InstanceSet::norm_feature_val( size_t idxInst, size_t idxF ) const
{
    if (feature_is_integer(idxF))
    {
        double v = instance(idxInst).int_feature(idxF) - limitsFeature[idxF].first;
        double interval = limitsFeature[idxF].second - limitsFeature[idxF].first;

        double r = v/interval;

        assert( (r>=0.0-1e-9) and (r<=1.0+1e-9) );

        return r;
    }
    else
    {
        if (feature_is_float(idxF))
        {
            double v = instance(idxInst).float_feature(idxF) - limitsFeature[idxF].first;
            double interval = limitsFeature[idxF].second - limitsFeature[idxF].first;

            double r = v/interval;

            assert( (r>=0.0-1e-9) and (r<=1.0+1e-9) );

            return r;
        }
    }

    cerr << "cannot get normalized value for field " << features_[idxF] << endl;
    abort();

    return 0.0;
}

double InstanceSet::norm_feature_val_rank( size_t idxInst, size_t idxF ) const
{
    if (rankingsFeature(idxF)<=1)
        return 0.0;
    double rankingsF = rankingsFeature(idxF)-1.0;

    const double v = ((double)instFeatRank[idxInst][idxF]) / rankingsF;
    assert( v>=0.0-1e-10 );
    assert( v<=1.0+1e-10 );

    return v;
}

void InstanceSet::save(const char *fileName, bool normalized) const
{
    FILE *f = fopen(fileName, "w");
    assert(f);
    fprintf(f, "name");
    for ( size_t i=0 ; (i<features_.size()) ; ++i )
        fprintf(f, ",%s", features_[i].c_str());
    fprintf( f, "\n" );
    for ( const auto &inst : instances_ )
    {
        fprintf(f, "%s", inst.name() );
        for ( size_t i=0 ; (i<features_.size()) ; ++i )
        {
            if (normalized)
                fprintf(f, ",%g", norm_feature_val(inst.idx(), i));
            else
                fprintf(f, ",%g", inst.float_feature(i));
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void InstanceSet::saveNormRank(const char *fileName) const
{
    FILE *f = fopen(fileName, "w");
    assert(f);
    fprintf(f, "name");
    for ( size_t i=0 ; (i<features_.size()) ; ++i )
        fprintf(f, ",%s", features_[i].c_str());
    fprintf( f, "\n" );
    for ( const auto &inst : instances_ )
    {
        fprintf(f, "%s", inst.name() );
        for ( size_t i=0 ; (i<features_.size()) ; ++i )
        {
            fprintf(f, ",%g", this->norm_feature_val_rank(inst.idx(), i) );
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

double InstanceSet::value_by_norm_val_rank(size_t idxF, const double nv) const
{
    assert( nv>=0.0-1e-9 );
    assert( nv<=1.0+1e-9 );

    double rankingsF = rankingsFeature(idxF)-1.0;

    int rank = (int)floor( rankingsF*nv + 0.5 );
    auto it = featureValRank[idxF].find(rank);
    assert(it!=featureValRank[idxF].end());

    assert( it->second >= 0.0-1e-9);
    assert( it->second <= 1.0+1e-9);

    return it->second;
}

bool InstanceSet::has(const std::string &iname) const
{
    auto it = instByName_.find(iname);
    if (it==instByName_.end())
        return false;

    return true;
}
