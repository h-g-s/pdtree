/*
 * Parameters.cpp
 *
 *  Created on: 8 de mar de 2019
 *      Author: haroldo
 */

#include "Parameters.hpp"

#include <strings.h>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

using namespace std;

string Parameters::instancesFile = "";

string Parameters::mipPDTFile = "";
    
string Parameters::gtreeFile = "";

string Parameters::treeFile = "";

string Parameters::gtreeFileGV = "";

string Parameters::treeFileGV = "";
    
string Parameters::summFile = "";

string Parameters::resultsFile = "";

string Parameters::isetCSVNorm = "";

string Parameters::isetCSVNormR = "";

string Parameters::rsetCSV = "";

enum FMRStrategy Parameters::fmrStrategy = WorseInst;

double Parameters::fillMissingValue = 999999999;

enum Evaluation Parameters::eval = Rank;

double Parameters::rankEps = 1e-8;

double Parameters::rankPerc = 0.01;

size_t Parameters::storeTop = 5;

// minimum number of instances in a split
// for the branching to be valid
int Parameters::minElementsBranch = 3;

// maximum number of branches evaluated for a feature
// at a given depth
size_t Parameters::maxEvalBranches[MAX_DEPTH] = { 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999 };

size_t Parameters::maxDepth = 3;

// minimum percentage performance improvement
double Parameters::minPerfImprov = 0.01;

// maximum optimization time
double Parameters::maxSeconds = 300;

// minimum absolute performance improvement
double Parameters::minAbsPerfImprov = 1e-5;

static char FMRStrategyStr[6][16] = {
    "Worse",
    "WorseT2",
    "WorseInst",
    "WorseInstT2",
    "AverageInst",
    "Value" };

static char EvaluationStr[2][16] =
{
    "Average",
    "Rank"
};


const char *str_eval( const enum Evaluation eval )
{
    return EvaluationStr[eval];
}


const char *str_fmrs( const enum FMRStrategy fmrs  )
{
    return FMRStrategyStr[fmrs];
}

static enum FMRStrategy to_fmrs( const char *s);

static enum Evaluation to_eval( const char *s);

void Parameters::parse( int argc, const char **argv )
{
    instancesFile = string(argv[1]);
    resultsFile = string(argv[2]);

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

        size_t pNameLen = strlen(pName);

        if (strcasecmp(pName, "-mipPDTFile")==0)
        {
            Parameters::mipPDTFile = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-gtreeFile")==0)
        {
            Parameters::gtreeFile = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-treeFile")==0)
        {
            Parameters::treeFile = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-gtreeFileGV")==0)
        {
            Parameters::gtreeFileGV = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-treeFileGV")==0)
        {
            Parameters::treeFileGV = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-summFile")==0)
        {
            Parameters::summFile = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-isetCSVNorm")==0)
        {
            Parameters::isetCSVNorm = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-isetCSVNormR")==0)
        {
            Parameters::isetCSVNormR = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-rsetCSV")==0)
        {
            Parameters::rsetCSV = string(pValue);
            continue;
        }
        if (strcasecmp(pName, "-fmrs")==0)
        {
            Parameters::fmrStrategy = to_fmrs(pValue);
            continue;
        }
        if (strcasecmp(pName, "-fmrValue")==0)
        {
            Parameters::fillMissingValue = stod(string(pValue));
            continue;
        }
        if (strcasecmp(pName, "-eval")==0)
        {
            Parameters::eval = to_eval(pValue);
            continue;
        }
        if (strcasecmp(pName, "-rankEps")==0)
        {
            Parameters::rankEps = stod(string(pValue));
            continue;
        }
        if (strcasecmp(pName, "-rankPerc")==0)
        {
            Parameters::rankEps = stod(string(pValue));
            continue;
        }
        if (strcasecmp(pName, "-minElementsBranch")==0)
        {
            Parameters::minElementsBranch = stoi(string(pValue));
            continue;
        }
        if (strcasecmp(pName, "-maxDepth")==0)
        {
            Parameters::maxDepth = stoi(string(pValue));
            if (maxDepth>MAX_DEPTH)
            {
                cerr << "Max depth should be at most " << MAX_DEPTH << endl;
                abort();
            }
            continue;
        }
        if (strcmp(pName, "-minperfimprov")==0)
        {
            Parameters::minPerfImprov = stod(string(pValue));
            continue;
        }
        if (strcmp(pName, "-maxseconds")==0)
        {
            Parameters::maxSeconds = stod(string(pValue));
            continue;
        }
        if (strcmp(pName, "-minabsperfimprov")==0)
        {
            Parameters::minAbsPerfImprov = stod(string(pValue));
            continue;
        }
        if (pNameLen>15)
        {
            char pns[256];
            strcpy(pns, pName);
            pns[pNameLen-1] = '\0';
            if (strcmp(pns, "-maxevalbranches")==0)
            {
                char *s = strcasestr(pName, "s");
                assert(s);
                ++s;
                if (not isdigit(s[0]))
                {
                    cerr << "enter the maximum number of branches per level as -maxEvalBranchesLEVEL=int" << endl;
                    abort();
                }
                size_t level = (size_t)atoi(s);
                if (level>=MAX_DEPTH)
                {
                    cerr << "level should be 0, ..., " << MAX_DEPTH << endl;
                    abort();
                }
                assert(level<MAX_DEPTH);
                Parameters::maxEvalBranches[level] = stoi(string(pValue));
                continue;
            }
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
    cout << "\t-fmrValue=double" << endl;
    cout << "\t-eval=[Average, Rank]" << endl;
    cout << "\t-rankEps=float" << endl;
    cout << "\t-rankPerc=float" << endl;
    cout << "\t-minElementsBranch=int" << endl;
    cout << "\t-maxEvalBranchesLEVEL=int" << endl;
    cout << "\t-maxDepth=int" << endl;
    cout << "\t-minPerfImprov=double" << endl;
    cout << "\t-minAbsPerfImprov=double" << endl;

}

void Parameters::print()
{
    cout << "Parameter settings: " << endl;
    cout << "             fmrs=" << FMRStrategyStr[Parameters::fmrStrategy] << endl;
    cout << "         fmrValue=" << defaultfloat << setprecision(4) << Parameters::fillMissingValue << endl;
    cout << "             eval=" << EvaluationStr[Parameters::eval] << endl;
    cout << "         maxDepth=" << Parameters::maxDepth << endl;
    cout << "          rankEps=" << scientific << rankEps << endl;
    cout << "         rankPerc=" << fixed << setprecision(4) << rankPerc << endl;
    cout << "minElementsBranch=" << fixed << setprecision(0) << minElementsBranch << endl;
    cout << "  maxEvalBranches=[";
    for ( size_t i=0 ; i<MAX_DEPTH; ++i )
    {
        if (i)
            cout << ", ";
        cout << maxEvalBranches[i];
    }
    cout << "]" << endl;
    cout << "    minPerfImprov=" << fixed << setprecision(4) << minPerfImprov << endl;
    cout << " minAbsPerfImprov=" << defaultfloat << minAbsPerfImprov << endl;
}

