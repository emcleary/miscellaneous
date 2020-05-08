#ifndef MATRIX3D_H_
#define MATRIX3D_H_

#include <stdio.h>

class Matrix3D {
 public:
  Matrix3D(int dim1, int dim2, int dim3);
  ~Matrix3D();
  double GetVal(int i, int j, int k) const;
  void SetVal(int i, int j, int k, double vol);
  int GetNumDim1() const;
  int GetNumDim2() const;
  int GetNumDim3() const;
 private:
  double *matrix_;
  const int dim1_;
  const int dim2_;
  const int dim3_;
  
};


#endif // MATRIX3D_H_
