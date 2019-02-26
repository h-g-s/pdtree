/*
 * Instance.cpp
 *
 *  Created on: 24 de fev de 2019
 *      Author: haroldo
 */

#include "Dataset.hpp"

#include <fstream>
#include <vector>
#include <set>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <unordered_set>

using namespace std;

#include <algorithm>
#include <cctype>
#include <locale>
#include <iterator>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <chrono>


// trim from start (in place)
static inline void ltrim(std::string &s);

// trim from end (in place)
static inline void rtrim(std::string &s);

// trim from both ends (in place)
static inline void trim(std::string &s);

enum Datatype str_type(const string &str);

class CSVRow
{
    public:
        std::string const& operator[](std::size_t index) const
        {
            return m_data[index];
        }
        std::size_t size() const
        {
            return m_data.size();
        }
        void readNextRow(std::istream& str)
        {
            std::string         line;
            std::getline(str, line);

            std::stringstream   lineStream(line);
            std::string         cell;

            m_data.clear();
            while(std::getline(lineStream, cell, ','))
            {
                trim(cell);
                m_data.push_back(cell);
            }
            // This checks for a trailing comma with no data after it.
            if (!lineStream && cell.empty())
            {
                // If there was a trailing comma then add an empty element.
                m_data.push_back("");
            }
        }
    private:
        std::vector<std::string>    m_data;
};

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}

Dataset::Dataset(const char *fileName) :
    data(nullptr)
{
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    ifstream ifile;
    ifile.open(fileName, ios::in);

    vector<CSVRow> rows;

    CSVRow row;

    // reading header
    ifile >> row;
    this->headers_ = vector< string >(row.size());
    for (size_t i=0 ; (i<row.size()) ; ++i )
    {
        this->headers_[i] = row[i];
        trim(this->headers_[i]);
    }
    
    size_t n = this->headers_.size();
    vector<unordered_set<string>> difValues = vector<unordered_set<string>>(n, unordered_set<string>());
    int line = 1;

    vector<vector<size_t>> colTypes = vector<vector<size_t>>(n, vector<size_t>(N_DATA_TYPES, 0));

    // for char types
    vector<size_t> colSizes = vector<size_t>(n, 0);

    while(ifile >> row)
    {
        if (row.size()!=n)
        {
            cerr << "error reading line " << line << \
            ", number of columns should be " << n << \
            " but there are " << row.size() << " columns." << endl;
            abort();
        }

        for ( size_t i=0 ; (i<n) ; ++i )
        {
            difValues[i].insert(row[i]);
            ++colTypes[i][str_type(row[i])];
            colSizes[i] = max(colSizes[i], row[i].size());
        }

        ++line;
    }
    ifile.close();

    this->rowSize = 0;
    
    // checking columns that can be safely deleted
    std::vector< bool > deleteColumn = vector< bool >(n, false);
    size_t nDel = 0;
    for ( size_t i=0 ; (i<difValues.size()) ; ++i )
    {
        if (difValues[i].size()<=1)
        {
            deleteColumn[i] = true;
            nDel++;
        }
    }
    if (nDel)
    {
        cout << "the following columns have always the same value and will be deleted:" << endl;
        for ( size_t i=0 ; (i<n) ; ++i )
            if (deleteColumn[i])
                cout << "\t" << this->headers_[i] << endl;
    }
    
    this->cTypes_ = vector<Datatype>(n, Empty);
    this->cSizes_ = vector<size_t>(n, 0);

    size_t idx = 0;
    for ( size_t i=0 ; (i<n); ++i )
    {
        if (deleteColumn[i])
            continue;
        if (colTypes[i][String])
        {
            this->cTypes_[idx] = String;
            this->cSizes_[idx] = colSizes[i]+1;
        }
        else
        {
            if (colTypes[i][Float])
            {
                this->cTypes_[idx] = Float;
                this->cSizes_[idx] = sizeof(double);
            }
            else
            {
                if (colTypes[i][Integer])
                {
                    this->cTypes_[idx] = Integer;
                    this->cSizes_[idx] = sizeof(int);
                }
                else
                {
                    if (colTypes[i][Short])
                    {
                        this->cTypes_[idx] = Short;
                        this->cSizes_[idx] = sizeof(short int);
                    }
                    else
                    {
                        this->cTypes_[idx] = Char;
                        this->cSizes_[idx] = sizeof(signed char);
                    }
                } // smaller than integer
            } // not float
        } // all columns

        this->headers_[idx] = this->headers_[i];
        this->rowSize += this->cSizes_[idx];
        ++idx;
    }
    
    n = idx;
    this->headers_.resize(n);
    this->cTypes_.resize(n);
    this->cSizes_.resize(n);
    this->cShift_ = vector< size_t >(this->cSizes_.size(), 0);
    for (size_t i=1 ; (i<this->cSizes_.size()) ; ++i)
        this->cShift_[i] = this->cShift_[i-1] + this->cSizes_[i-1];


    this->rows_ = line;
    size_t dataSize = this->rowSize*line;

    string unity = "bytes";
    double hSize = dataSize;
    if (hSize>1024)
    {
        hSize /= 1024;
        unity = "Kb";
    }
    if (hSize>1024)
    {
        hSize /= 1024;
        unity = "Mb";
    }
    if (hSize>1024)
    {
        hSize /= 1024;
        unity = "Gb";
    }
    cout << "dataset will ocupy " << setprecision(2) << fixed << hSize << " " << unity << endl;

    this->data = (char *)malloc(dataSize);
    if (this->data==nullptr)
    {
        cerr << "No memory for dataset." << endl;
        abort();
    }

    ifile.open(fileName, ios::in);
    ifile >> row;

    size_t r = 0;
    while (ifile >> row)
    {
        size_t idx = 0;
        for ( size_t i=0 ; i<row.size(); ++i )
        {
            if (idx==this->headers_.size())
            {
                cerr << "line " << r+2 << " contains too many columns." << endl;
                abort();
            }
            if (deleteColumn[i])
                continue;

            this->cell_set(r, idx, row[i]);
            ++idx;
        }
        ++r;
    }
    ifile.close();
    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
    cout << "dataset was read in " << setprecision(3) << time_span.count() << " seconds." << endl;
}

void Dataset::cell_set(size_t row, size_t col, const std::string &str)
{
    assert(row<rows_);
    assert(col<this->headers_.size());
    char *p = (char *)this->data + this->rowSize*row + this->cShift_[col];
    switch (this->cTypes_[col])
    {
        case String:
        {
            char *s = (char*)p;
            strncpy(s, str.c_str(), this->cSizes_[col]);
            break;
        }
        case Char:
        {
            signed char *v = (signed char *)p;
            *v = (signed char) stoi(str);
            break;
        }
        case Short:
        {
            short int *v = (short int *)p;
            *v = (short int) stoi(str);
            break;
        }
        case Integer:
        {
            int *v = (int*)p;
            *v = stoi(str);
            break;
        }
        case Float:
        {
            double *v = (double *)p;
            *v = stod(str);
            break;
        }
        default:
        {
            cerr << "type not handled." << endl;
            abort();
        }
    }
}


int Dataset::int_cell(size_t row, size_t col) const
{
    assert(row<this->rows_);
    assert(col<this->headers_.size());
    assert(this->cTypes_[col]==Integer || this->cTypes_[col]==Short || this->cTypes_[col]==Char);
    char *p = (char *)this->data + this->rowSize*row + this->cShift_[col];

    switch (this->cTypes_[col])
    {
        case Integer:
        {
            int *v = (int *)p;
            return (*v);
            break;
        }
        case Short:
        {
            short int *v = (short int *)p;
            return (int)(*v);
            break;
        }
        case Char:
        {
            signed char *v = (signed char *)p;
            return (int)(*v);
            break;
        }
        default:
        {
            cerr << "unexpected value for data type" << endl;
            abort();
            break;
        }

    }

    return 0;
}

double
Dataset::float_cell (size_t row, size_t col) const
{
    assert(row<this->rows_);
    assert(col<this->headers_.size());
    assert(this->cTypes_[col]==Float);
    char *p = (char *)this->data + this->rowSize*row + this->cShift_[col];
    double *v = (double *)p;
    return *v;
}

const char*
Dataset::str_cell (size_t row, size_t col) const
{
    assert(row<this->rows_);
    assert(col<this->headers_.size());
    assert(this->cTypes_[col]==String);
    char *p = (char *)this->data + this->rowSize*row + this->cShift_[col];
    const char *s = (const char *)p;
    return s;
}

Dataset::~Dataset()
{
    if (this->data)
        free(this->data);
}


// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
                }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
                }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

enum Datatype str_type(const string &str)
{
    bool hasNum = false;
    int nPoints = 0;

    for ( size_t i=0 ; (i<str.size()) ; ++i )
    {
        if (str[i] == '.')
            ++nPoints;
        else
        {
            if (isdigit(str[i]) or str[i]=='e' or str[i]=='+' or str[i]=='-')
                hasNum = true;
            else
                return String;
        }
    }

    if (hasNum)
    {
        switch (nPoints)
        {
            case 0:
            {
                int v = 0;
                try
                {
                    v = stoi(str);
                }
                catch (const std::exception& e)
                {
                    return String;
                }
                if (v>=-127 && v<=127)
                    return Char;
                if (v>=-32767 && v<=32767)
                    return Short;
                return Integer;
                break;
            }
            case 1:
            {
                try
                {
                    stod(str);
                }
                catch (const std::exception& e)
                {
                    return String;
                }
                return Float;
                break;
            }
            default:
            {
                return String;
                break;
            }
        }
    }

    return Empty;
}
