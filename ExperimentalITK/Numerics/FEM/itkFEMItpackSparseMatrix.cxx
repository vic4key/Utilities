/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFEMItpackSparseMatrix.cxx,v $
  Language:  C++
  Date:      $Date: 2008/10/18 00:22:49 $
  Version:   $Revision: 1.1.1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkFEMItpackSparseMatrix.h"
#include "itpack.h"

#include <iostream>

namespace itk {
namespace fem {


ItpackSparseMatrix::ItpackSparseMatrix()
{
  m_MatrixFinalized = 0;
  m_MatrixInitialized = 0;
  m_NZ = 0;
  m_N = 0;
  /* m_IER = 0; */     /* initialize */
  m_MODE = 1;    /* add to existing entries when building matrix */
  m_LEVEL = -1;  /* no error messages */
  m_NOUT = 0;    /* output unit number */

  m_IA = 0;
  m_JA = 0;
  m_IWORK = 0;
  m_A = 0;
}

ItpackSparseMatrix::ItpackSparseMatrix(integer order)
{
  m_MatrixFinalized = 0;
  m_MatrixInitialized = 0;
  m_NZ = 0;
  m_N = order;
  /* m_IER = 0; */     /* initialize */
  m_MODE = 1;    /* add to existing entries when building matrix */
  m_LEVEL = -1;  /* no error messages */
  m_NOUT = 0;    /* output unit number */

  m_IA = 0;
  m_JA = 0;
  m_IWORK = 0;
  m_A = 0;
}


ItpackSparseMatrix::ItpackSparseMatrix(integer order, integer maxNonZeroValues)
{
   m_MatrixFinalized = 0;
   m_MatrixInitialized = 0;
   m_N = order;
   m_NZ = maxNonZeroValues;
   /* m_IER = 0; */      /* initialize */
   m_MODE = 1;     /* add to existing entries when building matrix */
   m_LEVEL = -1;   /* no error messages */
   m_NOUT = 0;     /* output unit number */

   m_IA = 0;
   m_JA = 0;
   m_IWORK = 0;
   m_A = 0;

}


void ItpackSparseMatrix::Initialize()
{

  /* is matrix ready for initialization */
  if ( (m_N <= 0) || (m_NZ <= 0) ) 
  { 
    /* FIX ME: error handling */
    throw FEMException(__FILE__, __LINE__, "ItpackSparseMatrix::Initialize");
  }

  /* initialize itpack variables */
  if (m_IA != 0)
  {
    delete [] m_IA;
  }
  if (m_JA != 0)
  {
    delete [] m_JA;
  }
  if (m_IWORK != 0)
  {
    delete [] m_IWORK;
  }
  if (m_A != 0)
  {
    delete [] m_A;
  }
  m_IA =    new integer [ m_N + 1 ];
  m_JA =    new integer [ m_NZ ];
  m_IWORK = new integer [ m_NZ ];
  m_A =     new doublereal [ m_NZ ];

  int i;
  for (i=0; i<m_NZ; i++) 
  {
    m_JA[i] = 0;
    m_IWORK[i] = 0;
    m_A[i] = 0.0;
  }
  for (i=0; i<=m_N; i++) 
  {
    m_IA[i] = 0;
  }

  /* initialize sparse matrix storage via itpack routine */
  sbini_( &m_N, &m_NZ, m_IA, m_JA, m_A, m_IWORK );
  
  /* set info flags */
  m_MatrixInitialized = 1;
  m_MatrixFinalized = 0;

  /* Do this to avoid itpack ignorance (unless it's somehow my ignorance) */
//  for (i=0; i<m_N; i++)
//  {
//    this->Set(i,i,0.0);
//  }

  return;
}

void ItpackSparseMatrix::Clear()
{

  /* free variables */
  if (m_IA != 0)
  {
    delete [] m_IA;
  }
  if (m_JA != 0)
  {
    delete [] m_JA;
  }
  if (m_IWORK != 0)
  {
    delete [] m_IWORK;
  }
  if (m_A != 0)
  {
    delete [] m_A;
  }
 
  m_MatrixFinalized = 0;
  m_MatrixInitialized = 0;
  m_N = 0;
  m_NZ = 0;
  /* m_IER = 0;   */
  m_MODE = 1;   
  m_LEVEL = -1;  
  m_NOUT = 0;    

  m_IA = 0;
  m_JA = 0;
  m_IWORK = 0;
  m_A = 0;
}

void ItpackSparseMatrix::Finalize()
{

  /* check */
  if ( (m_MatrixFinalized != 0) || (m_MatrixInitialized == 0) ) 
  {
    throw FEMException(__FILE__, __LINE__, "ItpackSparseMatrix::Finalize");
  }

  //this->PrintCompressedRow();

  /* finalize */
  sbend_( &m_N, &m_NZ, m_IA, m_JA, m_A, m_IWORK );
  
  //this->PrintCompressedRow();
 
  /* set info flag */
  m_MatrixFinalized = 1;

  return;
}


void ItpackSparseMatrix::UnFinalize()
{

  /* check if this op makes sense*/
  if ( (m_MatrixFinalized == 0) || (m_MatrixInitialized == 0) )
  {
    throw FEMException(__FILE__, __LINE__, "ItpackSparseMatrix::UnFinalize");
  }

  integer IER = 0;

  sbagn_(&m_N, &m_NZ, m_IA, m_JA, m_A, m_IWORK, &m_LEVEL, &m_NOUT, &IER);

  if (IER > 0)
  {
    throw FEMExceptionItpackSparseMatrixSbagn(__FILE__, __LINE__, "ItpackSparseMatrix::UnFinalize", IER);
  }

  /* set info flag */
  m_MatrixFinalized = 0;

  return;
}


void ItpackSparseMatrix::Set(integer i, integer j, doublereal value)
{

  /* check for dynamic form */
  if (m_MatrixInitialized == 0)
  {

    /* initialize if prepared */
    if ( (m_N <= 0) || (m_NZ <= 0) )
    {
      throw FEMException(__FILE__, __LINE__, "ItpackSparseMatrix::Set");
    }
    else 
    {
      this->Initialize();
    }
  }

  if (m_MatrixFinalized == 1) 
  {
    this->UnFinalize();
  }

  /* replace an existing value */
  m_MODE = 0;

  /* add entry (itpack expects 1-based indices */
  integer IER;
  integer fortranI = i+1;
  integer fortranJ = j+1;
  sbsij_(&m_N, &m_NZ, m_IA, m_JA, m_A, m_IWORK, &fortranI, &fortranJ, &value, &m_MODE, &m_LEVEL, &m_NOUT, &IER);

  if (IER > 700)
  {
    throw FEMExceptionItpackSparseMatrixSbsij(__FILE__, __LINE__, "ItpackSparseMatrix::Set", IER);
  }


  return;
}


void ItpackSparseMatrix::Add(integer i, integer j, doublereal value)
{


  /* ignore add zero */
  if (value == 0.0) 
  {
    return;
  }

  /* check for dynamic form */
  if (m_MatrixInitialized == 0)
  {

    /* initialize if prepared */
    if ( (m_N <= 0) || (m_NZ <= 0) )
    {
      throw FEMException(__FILE__, __LINE__, "ItpackSparseMatrix::Add");
    }
    else 
    {
      this->Initialize();
    }
  }
  if (m_MatrixFinalized != 0) 
  {
    this->UnFinalize();
  }

  /* add to an existing value */
  m_MODE = 1;

  /* add entry (itpack expects 1-based indices */
  integer IER;
  integer fortranI = i+1;
  integer fortranJ = j+1;
  sbsij_(&m_N, &m_NZ, m_IA, m_JA, m_A, m_IWORK, &fortranI, &fortranJ, &value, &m_MODE, &m_LEVEL, &m_NOUT, &IER);

  static int count = 0;

  if (IER == 0)
  {
    count++;
  }

  if (IER > 700)
  {
    std::cout << "count = " << count << std::endl;
    std::cout << "m_NZ = " << m_NZ << std::endl;
    std::cout << "IER = " << IER << std::endl;
    throw FEMExceptionItpackSparseMatrixSbsij(__FILE__, __LINE__, "ItpackSparseMatrix::Add", IER);
  }

  return;
}

doublereal ItpackSparseMatrix::Get(integer i, integer j)
{

  doublereal returnValue = 0.0; /* set to default return value */
  integer fortranJ = j+1;
  integer lower;
  integer upper;

  /* check for readiness */
  if (m_MatrixInitialized != 0)
  {  

    /* ensure matrix is in readable form */
    if (m_MatrixFinalized == 0)
    {
      this->Finalize();
    }

    /* get search bounds in appropriate row */
    lower = m_IA[i]-1;
    upper = m_IA[i+1]-1;

    /* Find value if it exists */
    for (int k=lower; k<upper; k++)
    {
      if (m_JA[k] == fortranJ)
      {
        returnValue = m_A[k];
      }
    }
  }

  return returnValue;
}


doublereal* ItpackSparseMatrix::GetA()
{
  if (m_MatrixInitialized == 0) return 0;
  if (m_MatrixFinalized == 0) Finalize();

  return m_A;
}


integer* ItpackSparseMatrix::GetIA()
{
  if (m_MatrixInitialized == 0) return 0;
  if (m_MatrixFinalized == 0) Finalize();

  return m_IA;
}


integer* ItpackSparseMatrix::GetJA()
{
  if (m_MatrixInitialized == 0) return 0;
  if (m_MatrixFinalized == 0) Finalize();

  return m_JA;
}


void ItpackSparseMatrix::mult(doublereal* vector, doublereal* result)
{

  /* finalize matrix */
  if (m_MatrixFinalized == 0)
  {
    this->Finalize();
  }

  /* loop and temp variables */
  int lower;
  int upper;
  int i;
  int j;

  /* prepare result vector */
  //if (result)  delete [] result;
  //result = new doublereal [ m_N ];
  
  for (i=0; i<m_N; i++)
  {
    result[i] = 0.0;
  }

  /* perform the mult operation */
  for (i=0; i<m_N; i++)
  {

    lower = m_IA[i]-1;
    upper = m_IA[i+1]-1;
    
    
    for (j=lower; j<upper; j++)
    {
      result[i] += m_A[j] * vector[ m_JA[j] - 1 ];
    }
  }

  return;
}


void ItpackSparseMatrix::mult(ItpackSparseMatrix* rightMatrix, ItpackSparseMatrix* resultMatrix)
{

    /* ensure appropriate matrix sizes */
  if (m_N != rightMatrix->GetOrder())
  {
    return;
  }

  /* finalize matrix */
  if (m_MatrixFinalized == 0)
  {
    this->Finalize();
  }


  /* loop and temp variables */
  int lower;          /* lower bounds for column indices vector */
  int upper;          /* upper bounds for column indices vector */
  int i;              /* loop over rows */
  int j;              /* loop over columns */
  int k;              /* iterate through row */
  doublereal summed;  /* temp holder for row.column */

  /* perform the mult operation */
  for (i=0; i<m_N; i++)
  {
    for (j=0; j<m_N; j++)
    {

      /* bounds of values located in current row */
      lower = m_IA[i]-1;
      upper = m_IA[i+1]-1;
    
      // sum up row*column elements
      summed = 0.0;
      for (k=lower; k<upper; k++)
      {
        summed += m_A[k] * rightMatrix->Get( m_JA[k]-1, j );
      }

      // insert sum to result matrix
      if (summed != 0.0)
      {
        resultMatrix->Set(i,j,summed);
      }
      
    }
  }
  
}


void ItpackSparseMatrix::SetCompressedRow(integer* ia, integer* ja, doublereal *a) 
{
  m_IA = ia;
  m_JA = ja;
  m_A = a;
  m_MatrixFinalized = 1;
  m_MatrixInitialized = 1;
}

/*
void ItpackSparseMatrix::PrintCompressedRow()
{

  int unfin = 0;

  //if (m_MatrixFinalized == 0) {
  //  this->Finalize();
  //  unfin = 1;
  //}

  int i;
  for (i=0; i<=m_N; i++) {
    std::cout << m_IA[i] << " ";
  }
  std::cout << std::endl;
  for (i=0; i<m_NZ; i++) {
    std::cout << m_JA[i] << " ";
  }
  std::cout << std::endl;
  for (i=0; i<m_NZ; i++) {
    std::cout << m_A[i] << " ";
  }
  std::cout << std::endl;

  //if (unfin) this->UnFinalize();

}
*/


ItpackSparseMatrix::~ItpackSparseMatrix()
{
  delete [] m_IA;
  delete [] m_JA;
  delete [] m_A;
  delete [] m_IWORK;
}


FEMExceptionItpackSparseMatrixSbagn::FEMExceptionItpackSparseMatrixSbagn(const char *file, unsigned int lineNumber, std::string location, integer errorCode) :
  FEMException(file,lineNumber)
{
  std::string solverError;

  if (errorCode == 703) 
  {
    solverError = "maximumNumberOfNonZeroValuesInMatrix is too small";
  }
  else
  {
    solverError = "Unknown error code returned";
  }

  OStringStream buf;
  buf << "Error: " << solverError;

  SetDescription(buf.str().c_str());
  
  SetLocation(location);
}



FEMExceptionItpackSparseMatrixSbsij::FEMExceptionItpackSparseMatrixSbsij(const char *file, unsigned int lineNumber, std::string location, integer errorCode) :
  FEMException(file,lineNumber)
{
    std::string solverError;

    switch (errorCode) 
    {
    case 701 :
      solverError = "Improper index of matrix";
      break;
    case 702 :
      solverError = "maximumNumberOfNonZeroValuesInMatrix is too small";
      break;
    default :
      solverError = "Unknown error code returned";
    }

  OStringStream buf;
  buf << "Error: " << solverError;

  SetDescription(buf.str().c_str());

  SetLocation(location);
}


}}  // end namespace itk::fem

