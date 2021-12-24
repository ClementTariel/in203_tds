#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "ProdMatMat.hpp"

namespace {
void prodSubBlocks(int iRowBlkA, int iColBlkB, int iColBlkA, int szBlock,
                   const Matrix& A, const Matrix& B, Matrix& C) {
  
  for (int k = iColBlkA; k < std::min(A.nbCols, iColBlkA + szBlock); k++) 
    # pragma omp parallel for shared(A,B,C,szBlock,iColBlkA,iColBlkB,iRowBlkA) 
    for (int j = iColBlkB; j < std::min(B.nbCols, iColBlkB + szBlock); j++)   
      for (int i = iRowBlkA; i < std::min(A.nbRows, iRowBlkA + szBlock); ++i)
        
        C(i, j) += A(i, k) * B(k, j);
}
const int szBlock = 1024;
}  // namespace

Matrix operator*(const Matrix& A, const Matrix& B) {
  Matrix C(A.nbRows, B.nbCols, 0.0);
  //prodSubBlocks(0, 0, 0, std::max({A.nbRows, B.nbCols, A.nbCols}), A, B, C);
  # pragma omp parallel for shared(A,B,C,szBlock) 
  for (int j = 0; j < B.nbCols; j+=szBlock)
    for (int i = 0; i < B.nbRows; i+=szBlock)
      for (int k = 0; k < A.nbCols; k+=szBlock)
        prodSubBlocks(i, j, k, szBlock, A, B, C);
  return C;
}
