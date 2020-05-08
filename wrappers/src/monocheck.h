/*! This is the header file monocheck.h
   
   See monocheck.cc for detailed information about monotonicity
   checking.
 */
#ifndef MONOCHECK_H_
#define MONOCHECK_H_

#include <stdio.h>

class Matrix;

class MonoCheck {
 public:
  MonoCheck(const Matrix &progVar);
  ~MonoCheck();
  int CheckStrictMonoticity(int *monoAry, const int ncols, int col);

 private:
  const int nrows_; // number of rows in progVar matrix
  const int ncols_; // number of cols in progVar matrix
  const Matrix &progVar_; // matrix containing progress variables sorted in increasing order by temperature
};

#endif // MONOCHECK_H_
