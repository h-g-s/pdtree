/*
 * Results.hpp
 *
 *  Created on: 27 de fev de 2019
 *      Author: haroldo
 */

#ifndef RESULTS_HPP_
#define RESULTS_HPP_

#include <string>
#include <vector>
#include "InstanceSet.hpp"

class Results
{
public:
    Results( const InstanceSet &_iset, const char *resFile );

    const std::vector< std::string > &algorithms() const;

    virtual ~Results ();
private:
    const InstanceSet &iset_;
    std::vector< std::string > algs_;
};

#endif /* RESULTS_HPP_ */
