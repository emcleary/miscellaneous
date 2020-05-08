
%module fittogrid

%{
#ifndef SWIG_FILE_WITH_INIT
#define SWIG_FILE_WITH_INIT
#include "interpolator.h"
#include "fittogrid.h"
#include "sorting.h"
#include "standardsort.h"
#endif
%}

%include "numpy.i"
%init %{
  import_array();
%}

%apply (double * IN_ARRAY1, int DIM1) {(const double * in_array, int size_in)}

%inline %{
  int fittogrid_func(Matrix4D * datain, const double * in_array, int size_in, Interpolator *interp, Matrix3D * dataout, int nthreads, int ex) {
    return fittogrid(datain, in_array, interp, dataout, nthreads, ex);
  }
%}


