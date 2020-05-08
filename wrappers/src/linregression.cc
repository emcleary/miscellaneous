/*! LinRegression is a class that determines the most monotonic
 progress variable with respect to temperature (or another specified
 column). It calculates the slope of the best linear approximation for
 each progress variable and selects the largest magnitude.
 
 The slope is given by 
 {sum_i=1_i=N (C_i-C_ave)(T_i-T_ave)}/{sum_i=1_i=N (T_i-T_ave)^2}
 */
#include "linregression.h"

#include <assert.h>
#include <stdlib.h>
#include <cmath>

#include "maxslope.h"
#include "matrix.h"

/// Constructor
LinRegression::LinRegression(const Matrix &progVar)
  : nrows_(progVar.GetNumRows()),
    ncols_(progVar.GetNumCols()),
    progVar_(progVar) {
  slopes_ = new double[ncols_];
}

/// Destructor
LinRegression::~LinRegression() {
  delete [] slopes_;
}

/// Method to find the most monotonic progress variable
/*!  MostMonotonic calculates the slope of the best linear
approximation for each progress variable which is strictly increasing
or strictly decreasing. The output array monoAry must be of length
ncols, where each cell holds a value of 3 if C is strictly monotonic
and has the largest slope, 2 if C is strictly monotonic but does not
have the largest slope, and 0 for non-monotonic C. col is the
reference column.
*/
int LinRegression::MostMonotonic(int *monoAry, const int ncols, const int col){
  assert(ncols == ncols_);

  if ((col < 0) || (col >= ncols)) {
    printf("Column %d is not a valid column number.\n", col);
    printf("The specified column must lie within 0 < col < %d.\n", ncols); 
    exit(1);
  }

  assert(monoAry != NULL && "monoAry input is a null pointer.\n");

  double *monoDomain = new double[nrows_];
  assert(progVar_.GetCol(col, monoDomain) == 0); // Domain over which monotonicity is checked (usually the temperature column of progVar_ - it is specified by the input "col")

  // Calculate average domain value (usually average temperature)
  double Tsum = 0.0;
  for (int itr=0; itr<nrows_; ++itr) {
    Tsum = Tsum + monoDomain[itr];
  }
  double Tave = Tsum/nrows_;

  for (int j=0; j<ncols; ++j) { // Loop over cells in monoAry
    if (monoAry[j] == 3) { // Monotonic progress variable
      double *progVarCol = new double[nrows_];
      assert(progVar_.GetCol(j, progVarCol) == 0);

      // Calculate average progress variable
      double Csum = 0.0;
      for (int i=0; i<nrows_; ++i) {
	Csum = Csum + progVarCol[i];
      }
      double Cave = Csum/nrows_;

      // Calculate slope of best fit line
      double sumNumerator = 0.0;
      double sumDenominator = 0.0;
      double slope = 0.0;

      for (int i=0; i<nrows_; ++i) {
	sumNumerator = sumNumerator + (progVarCol[i]-Cave)*(monoDomain[i]-Tave);
	sumDenominator = sumDenominator + (monoDomain[i]-Tave)*(monoDomain[i]-Tave);
      }

      if (sumDenominator != 0.0) {
	slope = sumNumerator/sumDenominator;
      }
      else {
	printf("Unable to calculate slope of best fit line.\n");
	exit(1);
      }

      slopes_[j] = slope; // Store slope 

      delete [] progVarCol;
    }
    else { // Not monotonic
      slopes_[j] = 0.0; // Set slope to 0 to indicate a non-monotonic progress variable
    }
  }

  /*
  // Print slopes for testing purposes
  printf("Slopes from linear regression for strictly monotonic C:\n");
  for (int j = 0; j<ncols; ++j) {
    printf("%6.3f\t", slopes_[j]);
  }
  printf("\n");
  */

  // Find slope with the maximum magnitude in slopes_ array & store
  // index of location
  double maxSlope = 0.0; // Stores value of maximum slope of best fit line for monotonic progress variables
  int index = -1; // Stores location of maximum slope value 

  for (int j=0; j<ncols; ++j) {
    if(monoAry[j] == 3 && std::abs(slopes_[j]) > maxSlope) {
      maxSlope = slopes_[j];
      index = j;
    }
  }

  assert(index >= 0 && "No progress variable is monotonic.\n");

  // Rewrite monoAry to have values of 3 for the best monotonic
  // progress variable, 2 for progress variables that are monotonic
  // but not the best, and 0 otherwise
  for (int j=0; j<ncols; ++j) {
    if (j == index) { // Best progress variable
      monoAry[j] = 3;
    }
    else {
      if (monoAry[j] == 3) {
	monoAry[j] = 2;
      }
    }
  }

  delete [] monoDomain;

  return 0;
}
