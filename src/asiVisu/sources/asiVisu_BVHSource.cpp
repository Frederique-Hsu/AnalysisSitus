//-----------------------------------------------------------------------------
// Created on: 01 August 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Sergey Slyadnev
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
#include <asiVisu_BVHSource.h>

// asiVisu includes
#include <asiVisu_Utils.h>

// asiAlgo includes
#include <asiAlgo_BVHIterator.h>

// VTK includes
#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkDataObject.h>
#include <vtkDoubleArray.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_BVHSource)

//-----------------------------------------------------------------------------

asiVisu_BVHSource::asiVisu_BVHSource()
: vtkUnstructuredGridAlgorithm (),
  m_iLevel                     (-1) // no selected level
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline.
}

//-----------------------------------------------------------------------------

asiVisu_BVHSource::~asiVisu_BVHSource()
{}

//-----------------------------------------------------------------------------

void asiVisu_BVHSource::SetInputBVH(const opencascade::handle<BVH_Tree<double, 3>>& bvh)
{
  m_bvh = bvh;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

void asiVisu_BVHSource::SetInputLevel(const int level)
{
  m_iLevel = level;
  //
  this->Modified();
}

//-----------------------------------------------------------------------------

int asiVisu_BVHSource::RequestData(vtkInformation*        asiVisu_NotUsed(request),
                                   vtkInformationVector** asiVisu_NotUsed(inputVector),
                                   vtkInformationVector*  outputVector)
{
  if ( m_bvh.IsNull() )
  {
    vtkErrorMacro( << "Invalid input: nullptr BVH." );
    return 0;
  }

  /* ================
   *  Prepare output.
   * ================ */

  // Get the output unstructured grid data from the information vector.
  vtkUnstructuredGrid* pOutputGrid = vtkUnstructuredGrid::GetData(outputVector);
  pOutputGrid->Allocate();
  pOutputGrid->SetPoints( vtkSmartPointer<vtkPoints>::New() );

  // Prepare array for nodal scalars.
  vtkPointData*                   pPointData = pOutputGrid->GetPointData();
  vtkSmartPointer<vtkDoubleArray> scalarsArr = asiVisu_Utils::InitDoubleArray(ARRNAME_BVH_N_SCALARS);
  //
  pPointData->SetScalars(scalarsArr);

  /* ===========
   *  Add cells.
   * =========== */

  // Loop over the BVH nodes
  for ( asiAlgo_BVHIterator it(m_bvh); it.More(); it.Next() )
  {
    const BVH_Vec4i& nodeData = it.Current();

    if ( m_iLevel != -1 && it.CurrentLevel() != m_iLevel )
      continue;

    if ( !it.IsLeaf() )
    {
      const BVH_Vec3d& minCorner_Left  = m_bvh->MinPoint( nodeData.y() );
      const BVH_Vec3d& maxCorner_Left  = m_bvh->MaxPoint( nodeData.y() );
      const BVH_Vec3d& minCorner_Right = m_bvh->MinPoint( nodeData.z() );
      const BVH_Vec3d& maxCorner_Right = m_bvh->MaxPoint( nodeData.z() );

      // Left box
      {
        gp_Pnt P0( minCorner_Left.x(), minCorner_Left.y(), minCorner_Left.z() );
        gp_Pnt P7( maxCorner_Left.x(), maxCorner_Left.y(), maxCorner_Left.z() );
        gp_Pnt Pmin = P0;
        gp_Pnt Pmax = P7;

        gp_Pnt P1( Pmax.X(), Pmin.Y(), Pmin.Z() );
        gp_Pnt P2( Pmin.X(), Pmax.Y(), Pmin.Z() );
        gp_Pnt P3( Pmax.X(), Pmax.Y(), Pmin.Z() );
        gp_Pnt P4( Pmin.X(), Pmin.Y(), Pmax.Z() );
        gp_Pnt P5( Pmax.X(), Pmin.Y(), Pmax.Z() );
        gp_Pnt P6( Pmin.X(), Pmax.Y(), Pmax.Z() );

        this->registerVoxel(P0, P1, P2, P3, P4, P5, P6, P7, true, pOutputGrid);
      }

      // Right box
      {
        gp_Pnt P0( minCorner_Right.x(), minCorner_Right.y(), minCorner_Right.z() );
        gp_Pnt P7( maxCorner_Right.x(), maxCorner_Right.y(), maxCorner_Right.z() );
        gp_Pnt Pmin = P0;
        gp_Pnt Pmax = P7;

        gp_Pnt P1( Pmax.X(), Pmin.Y(), Pmin.Z() );
        gp_Pnt P2( Pmin.X(), Pmax.Y(), Pmin.Z() );
        gp_Pnt P3( Pmax.X(), Pmax.Y(), Pmin.Z() );
        gp_Pnt P4( Pmin.X(), Pmin.Y(), Pmax.Z() );
        gp_Pnt P5( Pmax.X(), Pmin.Y(), Pmax.Z() );
        gp_Pnt P6( Pmin.X(), Pmax.Y(), Pmax.Z() );

        this->registerVoxel(P0, P1, P2, P3, P4, P5, P6, P7, false, pOutputGrid);
      }
    }
  }

  return 1;
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_BVHSource::registerVoxel(const gp_Pnt&        node0,
                                   const gp_Pnt&        node1,
                                   const gp_Pnt&        node2,
                                   const gp_Pnt&        node3,
                                   const gp_Pnt&        node4,
                                   const gp_Pnt&        node5,
                                   const gp_Pnt&        node6,
                                   const gp_Pnt&        node7,
                                   const bool           isLeft,
                                   vtkUnstructuredGrid* pData)
{
  std::vector<vtkIdType> pids =
  {
    this->addPoint(node0, pData),
    this->addPoint(node1, pData),
    this->addPoint(node2, pData),
    this->addPoint(node3, pData),
    //
    this->addPoint(node4, pData),
    this->addPoint(node5, pData),
    this->addPoint(node6, pData),
    this->addPoint(node7, pData)
  };

  // Set scalars.
  vtkDoubleArray*
    pScalarsArr = vtkDoubleArray::SafeDownCast( pData->GetPointData()->GetArray(ARRNAME_BVH_N_SCALARS) );
  //
  double sc[1] = { isLeft ? 0. : 1. };
  //
  pScalarsArr->InsertTypedTuple(pids[0], sc);
  pScalarsArr->InsertTypedTuple(pids[1], sc);
  pScalarsArr->InsertTypedTuple(pids[2], sc);
  pScalarsArr->InsertTypedTuple(pids[3], sc);
  pScalarsArr->InsertTypedTuple(pids[4], sc);
  pScalarsArr->InsertTypedTuple(pids[5], sc);
  pScalarsArr->InsertTypedTuple(pids[6], sc);
  pScalarsArr->InsertTypedTuple(pids[7], sc);

  // Register voxel cell.
  vtkIdType cellID = pData->InsertNextCell(VTK_VOXEL, 8, &pids[0]);

  return cellID;
}

//-----------------------------------------------------------------------------

vtkIdType
  asiVisu_BVHSource::addPoint(const gp_Pnt&        coords,
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
