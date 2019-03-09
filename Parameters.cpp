/*
 * Parameters.cpp
 *
 *  Created on: 8 de mar de 2019
 *      Author: haroldo
 */

#include "Parameters.hpp"

#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

enum FMRStrategy Parameters::fmrStrategy = WorseInst;

enum Evaluation Parameters::eval = Average;

double Parameters::rankEps = 1e-8;

double Parameters::rankPerc = 0.01;

size_t Parameters::storeTop = 5;

static char FMRStrategyStr[5][16] = {
    "Worse",
    "WorseT2",
    "WorseInst",
    "WorseInstT2",
    "AverageInst" };

static char EvaluationStr[2][16] =
{
    "Average",
    "Rank"
};

static enum FMRStrategy to_fmrs( const char *s);

static enum Evaluation to_eval( const char *s);

void Parameters::parse( int argc, const char **argv )
{
    for ( int i=3 ; (i<argc) ; ++i )
    {
        if (argv[i][0]!='-')
            continue;

        char paramStr[256], pName[256], pValue[256];
        strcpy(paramStr, argv[i]);

        char *sp = strstr(paramStr, "=");
        if (sp==nullptr)
            throw "To set parameters use -parameter=value";

        ++sp;
        strcpy(pValue, sp);

        --sp; *sp = '\0';
        strcpy(pName, paramStr);
        char *s = pName;
        while (*s != '\0')
        {
            *s = tolower(*s);
            ++s;
        }

        s = pValue;
        while (*s != '\0')
        {
            *s = tolower(*s);
            ++s;
        }


        if (strcmp(pName, "-fmrs")==0)
        {
            Parameters::fmrStrategy = to_fmrs(pValue);
            continue;
        }
        if (strcmp(pName, "-eval")==0)
        {
            Parameters::eval = to_eval(pValue);
            continue;
        }
        if (strcmp(pName, "-rankEps")==0)
        {
            Parameters::rankEps = stod(string(pValue));
            continue;
        }
        if (strcmp(pName, "-rankPerc")==0)
        {
            Parameters::rankEps = stod(string(pValue));
            continue;
        }
    }

}

static enum FMRStrategy to_fmrs( const char *s )
{
    char slc[256];
    for ( size_t i=0 ; i<5 ; ++i )
    {
        strcpy(slc, FMRStrategyStr[i]);
        char *st = slc;
        while (*st != '\0')
        {
            *st = tolower(*st);
            ++st;
        }

        if (strcmp(slc, s)==0)
            return (FMRStrategy)i;
    }

    throw "Fill missing results strategy invalid: " + string(s);

    return FMRStrategy::AverageInst;
}


static enum Evaluation to_eval( const char *s)
{
    char slc[256];
    for ( size_t i=0 ; i<2 ; ++i )
    {
        strcpy(slc, EvaluationStr[i]);
        char *st = slc;
        while (*st != '\0')
        {
            *st = tolower(*st);
            ++st;
        }

        if (strcmp(slc, s)==0)
            return (Evaluation)i;
    }

    throw "Invalid evaluation criterion: " + string(s);

    return Evaluation::Rank;
}

void Parameters::help()
{
    cout << "\t-fmrs=[Worse, WorseT2, WorseInst, WorseInstT2, AverageInst]" << endl;
    cout << "\t-eval=[Average, Rank]" << endl;
    cout << "\t-rankEps=float" << endl;
    cout << "\t-rankPerc=float" << endl;

}

void Parameters::print()
{
    cout << "Results evaluation settings: " << endl;
    cout << "      fmrs=" << FMRStrategyStr[Parameters::fmrStrategy] << endl;
    cout << "      eval=" << EvaluationStr[Parameters::eval] << endl;
    cout << "   rankEps=" << scientific << rankEps << endl;
    cout << "  rankPerc=" << fixed << setprecision(4) << rankPerc << endl;
}

