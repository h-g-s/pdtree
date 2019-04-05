/*
 * pdtdefines.hpp
 *
 *  Created on: 4 de abr de 2019
 *      Author: haroldo
 */

#ifndef PDTDEFINES_HPP_
#define PDTDEFINES_HPP_

typedef double TResult;

// considering normalized feature values v_i,
// a split of value b will be
// at the left:   v_i <= b for all i in this branch
// at the right:  v_i >= b+minDiffBranches for all i in this branch
#define minDiffBranches  1e-4

#endif /* PDTDEFINES_HPP_ */
