/*
 * ResTestSet.cpp
 *
 *  Created on: 18 de mar de 2019
 *      Author: haroldo
 */

#include "ResTestSet.hpp"
#include <cstdlib>
#include <cstdio>

ResTestSet::ResTestSet(
    const std::unordered_map<std::string, size_t> &_instances,
    const std::unordered_map<std::string, size_t> &_algsettings,
    const char *fileName ) :
    res_(nullptr)
{
    FILE *f=fopen( fileName, "r" );
    char line[4096];

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

    while (char *s=fgets(line, 4096, f))
    {
        char instName[256];
        char algSetting[256];
        double res;
        int nr = sscanf(line, "%s,%s,%g", instName, algSetting, res);
        if (nr!=3)
        {
            fprintf(stderr, "invalid file format\n");
            exit(1);
        }

        auto iti = _instances.find(std::string(instName));
        if (iti == _instances.end())
            continue;

        auto ita = _algsettings.find(std::string(instName));
        if (ita == _algsettings.end())
            continue;

        res_[iti->second][ita->second] = res;
    }
    fclose(f);
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

