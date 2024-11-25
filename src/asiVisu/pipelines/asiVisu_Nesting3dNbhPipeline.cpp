//-----------------------------------------------------------------------------
// Created on: 12 October 2024
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
#include <asiVisu_Nesting3dNbhPipeline.h>

// asiVisu includes
#include <asiVisu_Nesting3dNbhDataProvider.h>
#include <asiVisu_Utils.h>

// VTK includes
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>
#include <vtkUnstructuredGrid.h>

using namespace asiAlgo::nesting;

//-----------------------------------------------------------------------------

asiVisu_Nesting3dNbhPipeline::asiVisu_Nesting3dNbhPipeline()
//
: asiVisu_Pipeline( vtkSmartPointer<vtkDataSetMapper>::New(),
                    vtkSmartPointer<vtkActor>::New() )
{
  m_source = vtkSmartPointer<asiVisu_Nesting3dNbhSource>::New();
}

//-----------------------------------------------------------------------------

void asiVisu_Nesting3dNbhPipeline::SetInput(const Handle(asiVisu_DataProvider)& DP)
{
  Handle(asiVisu_Nesting3dNbhDataProvider)
    provider = Handle(asiVisu_Nesting3dNbhDataProvider)::DownCast(DP);

  /* ===========================
   *  Validate input Parameters.
   * =========================== */

  Nbh3d nbh = provider->GetNeighborhood();
  //
  if ( nbh.IsEmpty() )
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
    m_source->SetInputNbh(nbh);

    // Initialize pipeline.
    this->SetInputConnection( m_source->GetOutputPort() );
  }

  // Update modification timestamp.
  this->Modified();
}

//-----------------------------------------------------------------------------

void asiVisu_Nesting3dNbhPipeline::callback_add_to_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

void asiVisu_Nesting3dNbhPipeline::callback_remove_from_renderer(vtkRenderer*)
{}

//-----------------------------------------------------------------------------

void asiVisu_Nesting3dNbhPipeline::callback_update()
{}
