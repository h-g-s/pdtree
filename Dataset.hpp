/*
 * Instance.hpp
 *
 *  Created on: 24 de fev de 2019
 *      Author: haroldo
 */

#ifndef DATASET_HPP_
#define DATASET_HPP_

#include <stddef.h>
#include <string>
#include <vector>

enum Datatype { Char=0, 
                Short, 
                Integer, 
                Float, 
                String, 
                Empty, 
                N_DATA_TYPES };

class Dataset {
public:
    /** reads a dataset from "fileName", checks for columns with only one feature if
     * deleteFeatures = true
     */
	Dataset(const char *fileName, bool deleteFeatures_ = true);

	/** generates a copy of the dataset
	 * including a subset of records
	 */
	Dataset(const Dataset &other, std::vector<bool> _included );

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

    bool col_is_number( size_t col ) const;

    // sets the cell contents, converting to the
    // column type
    void cell_set(size_t row, size_t col, const std::string &str);

    void cell_set(size_t row, size_t col, const int val);

    void cell_set(size_t row, size_t col, const double val);

    void write_csv( const char *fileName );

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

enum Datatype str_type(const std::string &str);

#endif /* DATASET_HPP_ */
