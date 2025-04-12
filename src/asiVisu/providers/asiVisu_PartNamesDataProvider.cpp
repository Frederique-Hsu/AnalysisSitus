//-----------------------------------------------------------------------------
// Created on: 11 April 2025
//-----------------------------------------------------------------------------
// Copyright (c) 2025-present, Quaoar Studio LLC (http://analysissitus.org)
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
#include <asiVisu_PartNamesDataProvider.h>

// asiAlgo includes
#include <asiAlgo_PointCloudUtils.h>

// Active Data includes
#include <ActAPI_IPlotter.h>

// OpenCascade includes
#include <BRepAdaptor_Surface.hxx>

//-----------------------------------------------------------------------------

asiVisu_PartNamesDataProvider::asiVisu_PartNamesDataProvider(const Handle(asiData_PartNode)& node)
: asiVisu_LabelsDataProvider(node)
{}

//-----------------------------------------------------------------------------

bool asiVisu_PartNamesDataProvider::HasAnchorPoints() const
{
  Handle(asiData_PartNode)
    partNode = Handle(asiData_PartNode)::DownCast(m_source);

  // Get AAG.
  Handle(asiAlgo_AAG) G = partNode->GetAAG();
  //
  if ( G.IsNull() )
    return false;

  // Get naming.
  Handle(asiAlgo_Naming) naming = partNode->GetNaming();
  //
  if ( naming.IsNull() )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_BaseCloud<double>)
  asiVisu_PartNamesDataProvider::GetAnchorPoints() const
{
  Handle(asiData_PartNode)
    partNode = Handle(asiData_PartNode)::DownCast(m_source);

  // Get AAG.
  Handle(asiAlgo_AAG) G = partNode->GetAAG();
  //
  if ( G.IsNull() )
    return nullptr;

  // Get naming.
  Handle(asiAlgo_Naming) naming = partNode->GetNaming();
  //
  if ( naming.IsNull() )
    return nullptr;

  Handle(asiAlgo_BaseCloud<double>) res = new asiAlgo_BaseCloud<double>;

  // Add points.
  const TopTools_IndexedMapOfShape& faces = G->GetMapOfFaces();
  //
  for ( int fid = 1; fid <= faces.Extent(); ++fid )
  {
    const TopoDS_Face& face = TopoDS::Face( faces(fid) );

    t_asciiString name;
    //
    if ( !naming->FindName(face, name) )
      continue;

    double umin, umax, vmin, vmax;
    asiAlgo_Utils::CacheFaceUVBounds(fid, G, umin, umax, vmin, vmax);

    BRepAdaptor_Surface bas(face, false);
    gp_Pnt P = bas.Value( (umin + umax)*0.5, (vmin + vmax)*0.5 );

    // Add to the point cloud.
    res->AddElement( P.X(), P.Y(), P.Z() );
  }

  return res;
}

//-----------------------------------------------------------------------------

Handle(HStringArray) asiVisu_PartNamesDataProvider::GetLabels() const
{
  Handle(asiData_PartNode)
    partNode = Handle(asiData_PartNode)::DownCast(m_source);

  // Get AAG.
  Handle(asiAlgo_AAG) G = partNode->GetAAG();
  //
  if ( G.IsNull() )
    return nullptr;

  // Get naming.
  Handle(asiAlgo_Naming) naming = partNode->GetNaming();
  //
  if ( naming.IsNull() )
    return nullptr;

  std::vector<t_asciiString> names;

  // Add points.
  const TopTools_IndexedMapOfShape& faces = G->GetMapOfFaces();
  //
  for ( int fid = 1; fid <= faces.Extent(); ++fid )
  {
    const TopoDS_Face& face = TopoDS::Face( faces(fid) );

    t_asciiString name;
    //
    if ( !naming->FindName(face, name) )
      continue;

    names.push_back(name);
  }

  if ( names.empty() )
    return nullptr;

  Handle(HStringArray) res = new HStringArray( 0, (int) (names.size() - 1) );
  //
  for ( int i = 0; i < (int) ( names.size() ); ++i )
  {
    res->ChangeValue(i) = names[i];
  }

  return res;
}

//-----------------------------------------------------------------------------

int asiVisu_PartNamesDataProvider::GetSize() const
{
  return 14;
}

//-----------------------------------------------------------------------------

void asiVisu_PartNamesDataProvider::GetColor(double& r, double& g, double& b) const
{
  ActAPI_Color rgb = Color_White;

  r = rgb.Red();
  g = rgb.Green();
  b = rgb.Blue();
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HParameterList)
  asiVisu_PartNamesDataProvider::translationSources() const
{
  ActParamStream out;

  out << m_source->Parameter(asiData_PartNode::PID_AAG)
      << m_source->Parameter(asiData_PartNode::PID_Geometry)
      << m_source->Parameter(asiData_PartNode::PID_Naming);

  return out;
}
