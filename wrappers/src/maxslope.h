/*! MaxSlope is an abstract class which provides the framework to
   determine the most monotonic progress variable.
 */
#ifndef MAXSLOPE_H_
#define MAXSLOPE_H_

class Matrix;

class MaxSlope {
 public:
  virtual ~MaxSlope() {}
  /// Virtual function to be inherited by each monotonicity checking algorithm to determine the most monotonic progress variable
  virtual int MostMonotonic(int *monoAry, const int ncols, const int col) = 0;
};

#endif // MAXSLOPE_H_
