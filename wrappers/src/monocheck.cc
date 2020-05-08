/*!  monocheck is a class which checks whether a specified progress
 variable is strictly increasing or strictly decreasing with respect
 to temperature (or another specified column). That is, C(T1)<C(T2) or
 C(T1)>C(T2) for T1<T2, where T1 and T2 are any two temperatures and C
 is the progress variable.
 */
#include <assert.h>
#include <stdlib.h>

#include "monocheck.h"
#include "matrix.h"

/// Constructor
MonoCheck::MonoCheck(const Matrix &progVar)
  : nrows_(progVar.GetNumRows()),
    ncols_(progVar.GetNumCols()),
    progVar_(progVar)
{}

/// Destructor
MonoCheck::~MonoCheck()
{}

/// Method to check whether each progress variable is strictly monotonic
/*!
CheckStrictMonoticity checks the monotonicity of each column (AKA
progress variable "C") in progVar with respect to column
"col". The output array monoAry must be of length ncols_, where
each cell holds a value of 3 if C is strictly increasing or
strictly decreasing and 0 otherwise.

\verbatim
INPUTS:

int *monoAry        array containing integer flags that denote the monotonicity of candidate progress variables

const int ncols     number of columns of monoAry

const int col       the reference column


OUTPUT:

int                 flag specifying whether or not the function succeeded
                     = 0: success
		    != 0: something went wrong

\endverbatim

*/
int MonoCheck::CheckStrictMonoticity(int *monoAry, const int ncols, int col){
  assert(ncols == ncols_);

  if ((col < 0) || (col >= ncols)) {
    printf("Column %d is not a valid column number.\n", col);
    printf("The specified column must lie within 0 < col < %d.\n", ncols); 
    exit(1);
  }

  assert(monoAry != NULL && "monoAry input is a null pointer.\n");

  double *monoDomain = new double[nrows_];
  assert(progVar_.GetCol(col, monoDomain) == 0); // Domain over which monotonicity is checked (usually the temperature column of progVar_ - it is specified by the input "col")

  for (int j=0; j<ncols; ++j) { // Loop over cells in monoAry
    if (j == col) {
      monoAry[j] = 0; // Cell representing domain
    }
    else {
      double *progVarCol = new double[nrows_];
      assert(progVar_.GetCol(j, progVarCol) == 0);
      int biggerCount = 0; // Keeps track of the number of times each
			   // element in progVarCol is larger than the
			   // previous element
      int smallerCount = 0; // Keeps track of the number of times each
			    // element in progVarCol is smaller than
			    // the previous element

      for (int i=1; i<nrows_; ++i) {
	if (monoDomain[i] > monoDomain[i-1]) {
	  if (progVarCol[i] > progVarCol[i-1]) {
	    biggerCount = biggerCount + 1;
	  }
	  else {
	    if (progVarCol[i] < progVarCol[i-1]) {
	      smallerCount = smallerCount + 1;
	    }
	  }
	}
	else {
	  printf("Column %d is not sorted in strictly increasing order.\n", col);
	  exit(1);
	}
      }

      if (biggerCount == nrows_-1 || smallerCount == nrows_-1) {
	monoAry[j] = 3; // Progress variable is strictly increasing or strictly decreasing
      }
      else {
	monoAry[j] = 0; // Progress variable is not strictly increasing or strictly decreasing
      }
      
      delete [] progVarCol;
    }
  }

  delete [] monoDomain;

  return 0;
}
