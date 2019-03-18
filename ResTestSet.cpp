/*
 * ResTestSet.cpp
 *
 *  Created on: 18 de mar de 2019
 *      Author: haroldo
 */

#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <cfloat>
#include <cmath>
#include "ResTestSet.hpp"
#include "Parameters.hpp"

using namespace std;

ResTestSet::ResTestSet(
    const std::unordered_map<std::string, size_t> &_instances,
    const std::unordered_map<std::string, size_t> &_algsettings,
    const char *fileName ) :
    res_(nullptr)
{
    FILE *f=fopen( fileName, "r" );
    char line[4096] = "";

    // ignoring headers
    if (!fgets(line, 4096, f))
    {
        fprintf(stderr, "empty results file");
        exit(1);
    }

    res_ = new float*[_instances.size()];
    res_[0] = new float[_instances.size()*_algsettings.size()];
    for ( size_t i=1 ; (i<_instances.size()) ; ++i )
        res_[i] = res_[i-1] + _algsettings.size();

    char **loaded = new char*[_instances.size()];
    loaded[0] = new char[_instances.size()*_algsettings.size()];
    for ( size_t i=0 ; i<(_instances.size()*_algsettings.size()) ; ++i )
        loaded[0][i] = false;
    for ( size_t i=1 ; i<(_instances.size()) ; ++i )
        loaded[i] = loaded[i-1] + _algsettings.size();

    long double sum = 0.0;
    size_t nRes = 0;
    double worseRes = DBL_MIN;

    long double *sumInst = new long double[_instances.size()];
    for ( size_t i=0 ; (i<_instances.size()) ; ++i )
        sumInst[i] = 0.0;
    size_t *nResInst = new size_t[_instances.size()];
    for ( size_t i=0 ; (i<_instances.size()) ; ++i )
        nResInst[i] = 0;
    double *worseInst = new double[_instances.size()];
    for ( size_t i=0 ; (i<_instances.size()) ; ++i )
        worseInst[i] = DBL_MIN;
    double *avgInst = new double[_instances.size()];
    for ( size_t i=0 ; (i<_instances.size()) ; ++i )
        avgInst[i] = 1e20;

    while (char *s=fgets(line, 4096, f))
    {
        char instName[256]="";
        char algSetting[256]="";
        float res=DBL_MAX;
        int nr = sscanf(s, "%s,%s,%g", instName, algSetting, &res);
        if (nr!=3)
        {
            fprintf(stderr, "invalid file format\n");
            exit(1);
        }

        auto iti = _instances.find(std::string(instName));
        if (iti == _instances.end())
            continue;

        auto ita = _algsettings.find(std::string(algSetting));
        if (ita == _algsettings.end())
            continue;

        loaded[iti->second][ita->second] = true;
        res_[iti->second][ita->second] = res;

        sum += res;
        nRes++;

        sumInst[iti->second] += res;
        nResInst[iti->second]++;
        worseInst[iti->second] = max( (double)worseInst[iti->second], (double)res);
        worseRes = max( worseRes, (double)res);
    }
    fclose(f);

    for ( size_t i=0 ; (i<_instances.size()) ; ++i )
        if (nResInst[i])
            avgInst[i] = ((long double)sumInst[i]) / ((long double)nResInst[i] );
        else
            avgInst[i] = worseRes;

    for ( size_t i=0 ; (i<_instances.size()) ; ++i )
    {
        for ( size_t j=0 ; (j<_algsettings.size()) ; ++j )
        {
            if (loaded[i][j])
                continue;

            switch (Parameters::fmrStrategy)
            {
                case FMRStrategy::Worse:
                    res_[i][j] = worseRes;
                    break;
                case FMRStrategy::WorseT2:
                    res_[i][j] = worseRes + fabs(worseRes);
                    break;
                case FMRStrategy::WorseInst:
                    res_[i][j] = worseInst[i];
                    break;
                case FMRStrategy::WorseInstT2:
                    res_[i][j] = worseInst[i]+fabs(worseInst[i]);
                    break;
                case FMRStrategy::AverageInst:
                    res_[i][j] = avgInst[i];
                    break;
            }
        }
    }

    delete[] avgInst;
    delete[] worseInst;
    delete[] loaded[0];
    delete[] loaded;
    delete[] sumInst;
    delete[] nResInst;
}

float ResTestSet::get( size_t idxInst, size_t idxAlgSetting ) const
{
    return res_[idxInst][idxAlgSetting];
}

ResTestSet::~ResTestSet ()
{
    delete[] res_[0];
    delete[] res_;
}

