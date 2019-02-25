/*
 * Instance.hpp
 *
 *  Created on: 24 de fev de 2019
 *      Author: haroldo
 */

#ifndef DATASET_HPP_
#define DATASET_HPP_

#include <string>
#include <vector>
#include <map>

enum Datatype { Char=0, 
                Short, 
                Integer, 
                Float, 
                String, 
                Empty, 
                N_DATA_TYPES };

class Dataset {
public:
	Dataset(const char *fileName);

	size_t rows() const { return rows_; }

	const std::vector<std::string> &headers() const {
	    return headers_;
	}

	const std::vector<Datatype> &types() const {
	    return cTypes_;
	}

    int int_cell(size_t row, size_t col) const;

    double float_cell(size_t row, size_t col) const;

    const char *str_cell(size_t row, size_t col) const;

    // sets the cell contents, converting to the
    // column type
    void cell_set(size_t row, size_t col, const std::string &str);

	virtual ~Dataset();
private:
        std::vector< std::string > headers_;

        std::vector< enum Datatype > cTypes_;
        std::vector< size_t > cSizes_; // in bytes
        std::vector< size_t > cShift_; // in bytes
        size_t rowSize;
        char *data;
        size_t rows_;
};

#endif /* DATASET_HPP_ */
