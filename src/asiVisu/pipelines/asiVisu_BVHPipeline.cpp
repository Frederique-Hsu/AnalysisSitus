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
#include <asiVisu_BVHPipeline.h>

// asiVisu includes
#include <asiVisu_MeshResultUtils.h>
#include <asiVisu_BVHDataProvider.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>
#include <vtkUnstructuredGrid.h>

#define LEFT_BRANCH_COLOR_R  250./255.
#define LEFT_BRANCH_COLOR_G  60./255.
#define LEFT_BRANCH_COLOR_B  15./255.
#define RIGHT_BRANCH_COLOR_R 140./255.
#define RIGHT_BRANCH_COLOR_G 220./255.
#define RIGHT_BRANCH_COLOR_B 40./255.
#define ULTIMATE_COLOR_R     240./255.
#define ULTIMATE_COLOR_G     240./255.
#define ULTIMATE_COLOR_B     240./255.

//-----------------------------------------------------------------------------

asiVisu_BVHPipeline::asiVisu_BVHPipeline()
//
: asiVisu_Pipeline   ( vtkSmartPointer<vtkDataSetMapper>::New(),
                       vtkSmartPointer<vtkActor>::New() ),
  m_bMapperColorsSet ( false )
{
  m_source = vtkSmartPointer<asiVisu_BVHSource>::New();
}

//-----------------------------------------------------------------------------

void asiVisu_BVHPipeline::SetInput(const Handle(asiVisu_DataProvider)& DP)
{
  Handle(asiVisu_BVHDataProvider)
    provider = Handle(asiVisu_BVHDataProvider)::DownCast(DP);

  /* ===========================
   *  Validate input Parameters.
   * =========================== */

  opencascade::handle<BVH_Tree<double, 3>> bvh      = provider->GetBVH();
  const bool                               renderOn = provider->IsRenderBVH();
  //
  if ( bvh.IsNull() || !renderOn )
  {
    // Pass empty data set in order to have valid pipeline.
    vtkSmartPointer<vtkUnstructuredGrid> dummyDS = vtkSmartPointer<vtkUnstructuredGrid>::New();
    this->SetInputData(dummyDS);
    this->Modified(); // Update modification timestamp.
    return; // Do nothing.
  }

  /* ============================
   *  Prepare polygonal data set.
   * ============================ */

  if ( provider->MustExecute( this->GetMTime() ) )
  {
    m_source->SetInputBVH       ( bvh );
    m_source->SetInputLevel     ( provider->GetLevel() );
    m_source->SetWireframeMode  ( provider->IsWireframe() );
    m_source->SetLeavesOnlyMode ( provider->IsLeavesOnly() );
    m_source->SetPrimitiveSet   ( provider->GetPrimitiveSet() );

    // Initialize pipeline.
    this->SetInputConnection( m_source->GetOutputPort() );
  }

  // Update modification timestamp.
  this->Modified();
}

//-----------------------------------------------------------------------------

bool asiVisu_BVHPipeline::initLookupTable()
{
  if ( !m_source.Get() )
    return false;

  // Get scalar range.
  const double minScalar = BVHSource_Scalar_Left;
  const double maxScalar = BVHSource_Scalar_Ultimate;

  // Build and initialize lookup table.
  m_lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  //
  m_lookupTable->SetTableRange(minScalar, maxScalar);
  m_lookupTable->SetScaleToLinear();
  m_lookupTable->SetNumberOfTableValues(3);

  m_lookupTable->SetTableValue(BVHSource_Scalar_Left,     LEFT_BRANCH_COLOR_R,  LEFT_BRANCH_COLOR_G,  LEFT_BRANCH_COLOR_B);
  m_lookupTable->SetTableValue(BVHSource_Scalar_Right,    RIGHT_BRANCH_COLOR_R, RIGHT_BRANCH_COLOR_G, RIGHT_BRANCH_COLOR_B);
  m_lookupTable->SetTableValue(BVHSource_Scalar_Ultimate, ULTIMATE_COLOR_R,     ULTIMATE_COLOR_G,     ULTIMATE_COLOR_B);

  return true;
}

//-----------------------------------------------------------------------------

void asiVisu_BVHPipeline::callback_add_to_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

void asiVisu_BVHPipeline::callback_remove_from_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

void asiVisu_BVHPipeline::callback_update()
{
  if ( !m_bMapperColorsSet )
  {
    // Initialize lookup table.
    this->initLookupTable();

    // Bind lookup table to mapper.
    m_mapper->ScalarVisibilityOn();
    m_mapper->SetScalarModeToUsePointData();
    m_mapper->SelectColorArray(ARRNAME_BVH_N_SCALARS);
    m_mapper->SetColorModeToMapScalars();
    m_mapper->SetScalarRange( m_lookupTable->GetRange() );
    m_mapper->SetLookupTable( m_lookupTable );
    m_mapper->Update();

    m_bMapperColorsSet = true;
  }
}
