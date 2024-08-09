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
#include <asiVisu_ShapeBVHDataProvider.h>

// asiData includes
#include <asiData_IVTopoItemNode.h>
#include <asiData_PartNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

void asiVisu_BVHFacets::GetBbox(const int  primFirstId,
                                const int  primLastId,
                                BVH_Vec3d& minCorner,
                                BVH_Vec3d& maxCorner) const
{
  BVH_Box<double, 3> bbox;

  for ( int i = primFirstId; i <= primLastId; ++i )
  {
    BVH_Box<double, 3> bbox_i = m_facets->Box(i);

    BVH_Vec3d cornerMin = bbox_i.CornerMin();
    BVH_Vec3d cornerMax = bbox_i.CornerMax();

    bbox.Add(cornerMin);
    bbox.Add(cornerMax);
  }

  minCorner = bbox.CornerMin();
  maxCorner = bbox.CornerMax();
}

//-----------------------------------------------------------------------------

asiVisu_ShapeBVHDataProvider::asiVisu_ShapeBVHDataProvider(const ActAPI_DataObjectId&           nodeId,
                                                           const Handle(ActAPI_HParameterList)& paramList)
: asiVisu_BVHDataProvider (),
  m_nodeID                (nodeId),
  m_params                (paramList)
{
}

//-----------------------------------------------------------------------------

opencascade::handle<BVH_Tree<double, 3>> asiVisu_ShapeBVHDataProvider::GetBVH() const
{
  Handle(asiData_BVHParameter)
    bvhParam = Handle(asiData_BVHParameter)::DownCast( m_params->Value(1) );

  Handle(asiAlgo_BVHFacets) facets = bvhParam->GetBVH();
  //
  if ( facets.IsNull() )
    return nullptr;

  return facets->BVH();
}

//-----------------------------------------------------------------------------

bool asiVisu_ShapeBVHDataProvider::IsRenderBVH() const
{
  return ActParamTool::AsBool( m_params->Value(2) )->GetValue();
}

//-----------------------------------------------------------------------------

int asiVisu_ShapeBVHDataProvider::GetLevel() const
{
  return ActParamTool::AsInt( m_params->Value(3) )->GetValue();
}

//-----------------------------------------------------------------------------

bool asiVisu_ShapeBVHDataProvider::IsWireframe() const
{
  return ActParamTool::AsBool( m_params->Value(4) )->GetValue();
}

//-----------------------------------------------------------------------------

bool asiVisu_ShapeBVHDataProvider::IsLeavesOnly() const
{
  return ActParamTool::AsBool( m_params->Value(5) )->GetValue();
}

//-----------------------------------------------------------------------------

Handle(asiVisu_BVHPrimitiveSet)
  asiVisu_ShapeBVHDataProvider::GetPrimitiveSet()
{
  Handle(asiData_BVHParameter)
    bvhParam = Handle(asiData_BVHParameter)::DownCast( m_params->Value(1) );

  Handle(asiAlgo_BVHFacets) facets = bvhParam->GetBVH();
  //
  if ( !facets.IsNull() )
  {
    m_primSet = new asiVisu_BVHFacets(facets);
  }

  return m_primSet;
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HParameterList)
  asiVisu_ShapeBVHDataProvider::translationSources() const
{
  return m_params;
}
