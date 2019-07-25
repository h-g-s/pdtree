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

int Parameters::maxAlgs = 100;

int Parameters::afMinAlgsInst = 5;

bool Parameters::onlyGreedy = false;

enum Evaluation Parameters::eval = Rank;

bool Parameters::bestIsZero = false;

bool Parameters::normalizeResults = false;

double Parameters::rankEps = 1e-8;

double Parameters::rankPerc = 0.01;

size_t Parameters::storeTop = 60;

// minimum number of instances in a split
// for the branching to be valid
int Parameters::minElementsBranch = 3;

// minimum percentage of all instances that should
// be at one side of the branch to allow branch
// (increases minElementsBranch if necessary)
double Parameters::minPercElementsBranch = 0.1;

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
        
//         char *s = pName;
//         while (*s != '\0')
//         {
//             *s = tolower(*s);
//             ++s;
//         }
// 
//         s = pValue;
//         while (*s != '\0')
//         {
//             *s = tolower(*s);
//             ++s;
//         }


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
        if (strcasecmp(pName, "-maxAlgs")==0)
        {
            Parameters::maxAlgs = stoi(string(pValue));
            continue;
        }
        if (strcasecmp(pName, "-afMinAlgsInst")==0)
        {
            Parameters::afMinAlgsInst = stoi(string(pValue));
            continue;
        }

        if (strcasecmp(pName, "-eval")==0)
        {
            Parameters::eval = to_eval(pValue);
            continue;
        }
        if (strcasecmp(pName, "-bestIsZero")==0)
        {
            Parameters::bestIsZero = (bool)atoi(pValue);
            continue;
        }
        if (strcasecmp(pName, "-onlyGreedy")==0)
        {
            Parameters::onlyGreedy = (bool)atoi(pValue);
            continue;
        }
 
        if (strcasecmp(pName, "-normalizeResults")==0)
        {
            Parameters::normalizeResults = (bool)atoi(pValue);
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
        if (strcasecmp(pName, "-minPercElementsBranch")==0)
        {
            Parameters::minPercElementsBranch = stod(string(pValue));
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
        if (strcasecmp(pName, "-minPerfImprov")==0)
        {
            Parameters::minPerfImprov = stod(string(pValue));
            continue;
        }
        if (strcasecmp(pName, "-maxSeconds")==0)
        {
            Parameters::maxSeconds = stod(string(pValue));
            continue;
        }
        if (strcasecmp(pName, "-minAbsPerfImprov")==0)
        {
            Parameters::minAbsPerfImprov = stod(string(pValue));
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
    cout << "                 fmrs=" << FMRStrategyStr[Parameters::fmrStrategy] << endl;
    cout << "             fmrValue=" << defaultfloat << setprecision(4) << Parameters::fillMissingValue << endl;
    cout << "                 eval=" << EvaluationStr[Parameters::eval] << endl;
    cout << "           bestIsZero=" << Parameters::bestIsZero << endl;
    cout << "           onlyGreedy=" << Parameters::onlyGreedy << endl;
    cout << "     normalizeResults=" << Parameters::normalizeResults << endl;
    cout << "             maxDepth=" << Parameters::maxDepth << endl;
    cout << "              rankEps=" << scientific << rankEps << endl;
    cout << "             rankPerc=" << fixed << setprecision(4) << rankPerc << endl;
    cout << "    minElementsBranch=" << fixed << setprecision(0) << minElementsBranch << endl;
    cout << "minPercElementsBranch=" << fixed << setprecision(3) << minPercElementsBranch << endl;
    cout << "        minPerfImprov=" << fixed << setprecision(4) << minPerfImprov << endl;
    cout << "     minAbsPerfImprov=" << defaultfloat << minAbsPerfImprov << endl;
}

