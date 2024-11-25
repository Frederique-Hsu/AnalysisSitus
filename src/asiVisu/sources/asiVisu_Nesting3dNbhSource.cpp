//-----------------------------------------------------------------------------
// Created on: 12 October 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// Own include
#include <asiVisu_Nesting3dNbhSource.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkDataObject.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

using namespace asiAlgo::nesting;

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_Nesting3dNbhSource)

//-----------------------------------------------------------------------------

asiVisu_Nesting3dNbhSource::asiVisu_Nesting3dNbhSource()
//
: vtkUnstructuredGridAlgorithm()
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline.
}

//-----------------------------------------------------------------------------

asiVisu_Nesting3dNbhSource::~asiVisu_Nesting3dNbhSource()
{}

//-----------------------------------------------------------------------------

void asiVisu_Nesting3dNbhSource::SetInputNbh(const Nbh3d& nbh)
{
  m_nbh = nbh;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

int asiVisu_Nesting3dNbhSource::RequestData(vtkInformation*        asiVisu_NotUsed(request),
                                            vtkInformationVector** asiVisu_NotUsed(inputVector),
                                            vtkInformationVector*  outputVector)
{
  if ( m_nbh.IsEmpty() )
  {
    vtkErrorMacro( << "Invalid input: empty neighborhood." );
    return 0;
  }

  /* ================
   *  Prepare output.
   * ================ */

  // Get the output unstructured grid data from the information vector.
  vtkUnstructuredGrid* pOutputGrid = vtkUnstructuredGrid::GetData(outputVector);
  pOutputGrid->Allocate();
  pOutputGrid->SetPoints( vtkSmartPointer<vtkPoints>::New() );

  /* ===========
   *  Add cells.
   * =========== */

  const Handle(Container3dGrid)&
    grid = m_nbh.container->GetGrid();

  this->populateAxis(grid, m_nbh.iPos, pOutputGrid); // i+
  this->populateAxis(grid, m_nbh.iNeg, pOutputGrid); // i-
  this->populateAxis(grid, m_nbh.jPos, pOutputGrid); // j+
  this->populateAxis(grid, m_nbh.jNeg, pOutputGrid); // j-
  this->populateAxis(grid, m_nbh.kPos, pOutputGrid); // k+
  this->populateAxis(grid, m_nbh.kNeg, pOutputGrid); // k-

  return 1;
}

//-----------------------------------------------------------------------------

void asiVisu_Nesting3dNbhSource::populateAxis(const Handle(Container3dGrid)&              grid,
                                              const std::vector<asiAlgo::nesting::Pos3d>& nodes,
                                              vtkUnstructuredGrid*                        pData)
{
  if ( nodes.empty() )
    return;

  for ( size_t k = 0; k < nodes.size() - 1; ++k )
  {
    const Pos3d& ijk_curr = nodes[k];
    const Pos3d& ijk_next = nodes[k + 1];

    gp_Pnt P1 = grid->Access(ijk_curr.i, ijk_curr.j, ijk_curr.k).P;
    gp_Pnt P2 = grid->Access(ijk_next.i, ijk_next.j, ijk_next.k).P;

    this->registerLine(P1, P2, pData);
  }
}

//-----------------------------------------------------------------------------

vtkIdType asiVisu_Nesting3dNbhSource::registerLine(const gp_Pnt&        ptStart,
                                                   const gp_Pnt&        ptEnd,
                                                   vtkUnstructuredGrid* pData)
{
  std::vector<vtkIdType> pids =
  {
    this->addPoint(ptStart, pData),
    this->addPoint(ptEnd,   pData)
  };

  vtkIdType cellID =
    pData->InsertNextCell( VTK_LINE, (int) pids.size(), &pids[0] );

  return cellID;
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_Nesting3dNbhSource::addPoint(const gp_Pnt&        coords,
                                       vtkUnstructuredGrid* pData)
{
  // Access points array.
  vtkPoints*
    points = pData->GetPoints();

  // Add the point to the VTK data set.
  vtkIdType
    resPid = points->InsertNextPoint( coords.X(), coords.Y(), coords.Z() );

  return resPid;
}
