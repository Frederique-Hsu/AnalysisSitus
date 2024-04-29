//-----------------------------------------------------------------------------
// Created on: 27 April 2024
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
#include <asiVisu_AdjGraphSource.h>

// asiAlgo includes
#include <asiAlgo_FeatureAttrAngle.h>
#include <asiAlgo_Utils.h>

// OpenCascade includes
#include <BRepAdaptor_Surface.hxx>

// VTK includes
#include <vtkCellData.h>
#include <vtkDataObject.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

//-----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_AdjGraphSource)

//-----------------------------------------------------------------------------

asiVisu_AdjGraphSource::asiVisu_AdjGraphSource() : vtkPolyDataAlgorithm()
{
  this->SetNumberOfInputPorts(0); // Connected directly to our own Data Provider
                                  // which has nothing to do with VTK pipeline.
}

//-----------------------------------------------------------------------------

asiVisu_AdjGraphSource::~asiVisu_AdjGraphSource()
{}

//-----------------------------------------------------------------------------

bool asiVisu_AdjGraphSource::SetInputGraph(const Handle(asiAlgo_AAG)& aag)
{
  m_aag = aag;

  // Update modification time for the source.
  this->Modified();
  return true;
}

//-----------------------------------------------------------------------------

int asiVisu_AdjGraphSource::RequestData(vtkInformation*        request,
                                        vtkInformationVector** inputVector,
                                        vtkInformationVector*  outputVector)
{
  if ( m_aag.IsNull() )
  {
    vtkErrorMacro( << "Invalid domain: nullptr AAG" );
    return 0;
  }

  /* ==============================
   *  Prepare involved collections
   * ============================== */

  vtkPolyData* polyOutput = vtkPolyData::GetData(outputVector);
  polyOutput->Allocate();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  polyOutput->SetPoints(points);

  // Access cell data.
  vtkCellData* cellData = polyOutput->GetCellData();

  // Array for element types.
  vtkSmartPointer<vtkIntArray> typeArr = asiVisu_Utils::InitIntArray(ARRNAME_VEXITY_SCALARS);
  cellData->AddArray(typeArr);

  /* ======================
   *  Build polygonal data
   * ====================== */

  // Map of face IDs vs point IDs.
  NCollection_DataMap<int, vtkIdType> facePointMap;

  // Add points.
  const TopTools_IndexedMapOfShape& faces = m_aag->GetMapOfFaces();
  //
  for ( int fid = 1; fid <= faces.Extent(); ++fid )
  {
    const TopoDS_Face& face = TopoDS::Face( faces(fid) );

    double umin, umax, vmin, vmax;
    asiAlgo_Utils::CacheFaceUVBounds(fid, m_aag, umin, umax, vmin, vmax);

    BRepAdaptor_Surface bas(face, false);
    gp_Pnt P = bas.Value( (umin + umax)*0.5, (vmin + vmax)*0.5 );

    vtkIdType pid = this->registerGridPoint(P, polyOutput);

    facePointMap.Bind(fid, pid);
  }

  // Add lines.
  for ( NCollection_DataMap<int, vtkIdType>::Iterator it(facePointMap); it.More(); it.Next() )
  {
    const int       fid  = it.Key();
    const vtkIdType pid0 = it.Value();

    const asiAlgo_Feature& nids = m_aag->GetNeighbors(fid);

    for ( asiAlgo_Feature::Iterator nit(nids); nit.More(); nit.Next() )
    {
      const int       nid  = nit.Key();
      const vtkIdType pid1 = facePointMap(nid);

      asiAlgo_AAG::t_arc arc(fid, nid);

      // Get angle to check for vexity.
      Handle(asiAlgo_FeatureAttrAngle)
        attrAngle = m_aag->ATTR_ARC<asiAlgo_FeatureAttrAngle>(arc);
      //
      if ( attrAngle.IsNull() )
        continue;

      // Check vexity.
      asiVisu_Vexity vexity = VisuVexity_Undefined;
      //
      if ( asiAlgo_FeatureAngle::IsConvex( attrAngle->GetAngleType() ) )
      {
        vexity = VisuVexity_Convex;
      }
      else if ( asiAlgo_FeatureAngle::IsConcave( attrAngle->GetAngleType() ) )
      {
        vexity = VisuVexity_Concave;
      }

      this->registerLine(pid0, pid1, vexity, polyOutput);
    }
  }

  return Superclass::RequestData(request, inputVector, outputVector);
}

//-----------------------------------------------------------------------------

vtkIdType asiVisu_AdjGraphSource::registerGridPoint(const gp_Pnt& point,
                                                    vtkPolyData*  polyData)
{
  // Access necessary arrays
  vtkPoints* points = polyData->GetPoints();

  // Push the point into VTK data set
  vtkIdType pid = points->InsertNextPoint( point.X(),
                                           point.Y(),
                                           point.Z() );

  return pid;
}

//-----------------------------------------------------------------------------

vtkIdType asiVisu_AdjGraphSource::registerLine(const vtkIdType      ptStart,
                                               const vtkIdType      ptEnd,
                                               const asiVisu_Vexity type,
                                               vtkPolyData*         polyData)
{
  std::vector<vtkIdType> pids = {ptStart, ptEnd};

  vtkIdType cellID =
    polyData->InsertNextCell( VTK_LINE, (int) pids.size(), &pids[0] );

  // Set element type.
  vtkIntArray*
    typeArr = vtkIntArray::SafeDownCast( polyData->GetCellData()->GetArray(ARRNAME_VEXITY_SCALARS) );
  //
  typeArr->InsertNextValue(type);

  return cellID;
}
