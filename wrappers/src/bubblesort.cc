
#include "bubblesort.h"

// Constructor
/*!
  The constructor duplicates the data from the matrix pointer to datacopy_ object. It also generates the array containing the indices to be used during sorting. 

 */
BubbleSort::BubbleSort(Matrix *data){
  nrows_ = data->GetNumRows();
  ncols_ = data->GetNumCols();

  data_ = data;
  datacopy_ = new Matrix(nrows_, ncols_); // duplicate data

  // Copy the data to the duplicate container
  for(int i = 0; i<nrows_; i++){
    for(int j = 0; j<ncols_; j++){
      datacopy_ -> SetVal(i,j, data->GetVal(i,j));
    }
  }
  
  // Generate the index array
  indices_ = new int[nrows_];
  for(int i = 0; i<nrows_; i++){
    indices_[i] = i;
  }
 
 
}

/// Destructor
BubbleSort::~BubbleSort(){
  delete datacopy_;
  delete [] indices_;
  delete [] refColumn_;
}

/// Set the reference column number and extract the data of the reference column to the container refColumn_
void BubbleSort::SetRefColNum(int num){
  refColNum_ = num;
  
  // extract the reference column
  refColumn_ = new double[nrows_];
  
  for(int i = 0; i<nrows_; i++){
    refColumn_[i] = data_->GetVal(i, refColNum_);
  }

}


/// Main sorting body
/*!
  This function processes the reference column with the bubble sorting algorithm.

  Details of the bubble sort algortihm can be found from the following link: http://en.wikipedia.org/wiki/Bubble_sort
  
  \verbatim
  INPUT

  There are no inputs. The data to be sorted is already passed via the constructor

  OUTPUT:

  int         flag specifying whether or not the function succeeded
               = 0: success
	      != 0: something went wrong

  \endverbatim

 */
int BubbleSort::sort_data(){
  bool swapped = true;
  int j = 0;
  double tmp;
  int tmpid;

  while(swapped){
    swapped = false;
    j++;
    for(int i=0; i< nrows_- j; i++){
      if (refColumn_[i] > refColumn_[i+1]){
	tmp = refColumn_[i];
	refColumn_[i] = refColumn_[i+1];
	refColumn_[i+1] = tmp;
	swapped = true;

	tmpid = indices_[i];
	indices_[i] = indices_[i+1];
	indices_[i+1] = tmpid;
      }
    }
  }

  
  for (int i = 0; i < nrows_; i++){
    for(int j = 0; j < ncols_; j++){
      data_ -> SetVal(i,j, datacopy_->GetVal(indices_[i], j));
    }
  }
  

  return 0;

}
