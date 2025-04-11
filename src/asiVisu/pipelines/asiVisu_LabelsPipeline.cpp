//-----------------------------------------------------------------------------
// Created on: 05 February 2025
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
#include <asiVisu_LabelsPipeline.h>

// asiVisu includes
#include <asiVisu_LabelsDataProvider.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

// VTK includes
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGrid.h>

//-----------------------------------------------------------------------------

asiVisu_LabelsPipeline::asiVisu_LabelsPipeline()
//
: asiVisu_Pipeline( vtkSmartPointer<vtkDataSetMapper>::New(),
                    vtkSmartPointer<vtkActor>::New() )
{
  m_origins      = vtkSmartPointer< asiVisu_PointsSource<double> > ::New();
  m_lblHierarchy = vtkSmartPointer<vtkPointSetToLabelHierarchy>    ::New();
  m_lblMapper    = vtkSmartPointer<vtkLabelPlacementMapper>        ::New();
  m_lblActor     = vtkSmartPointer<vtkActor2D>                     ::New();

  m_lblActor->SetMapper(m_lblMapper);
}

//-----------------------------------------------------------------------------

void asiVisu_LabelsPipeline::SetInput(const Handle(asiVisu_DataProvider)& DP)
{
  Handle(asiVisu_LabelsDataProvider)
    provider = Handle(asiVisu_LabelsDataProvider)::DownCast(DP);

  /* ===========================
   *  Validate input Parameters.
   * =========================== */

  if ( !provider->HasAnchorPoints() )
  {
    // Pass empty data set in order to have valid pipeline.
    vtkSmartPointer<vtkUnstructuredGrid> dummyDS = vtkSmartPointer<vtkUnstructuredGrid>::New();
    this->SetInputData(dummyDS);
    this->Modified(); // Update modification timestamp.
    return; // Do nothing.
  }

  this->Actor()->GetProperty()->SetPointSize(1.);

  // Get color.
  double r, g, b;
  provider->GetColor(r, g, b);

  /* ============================
   *  Prepare polygonal data set.
   * ============================ */

  if ( provider->MustExecute( this->GetMTime() ) )
  {
    Handle(asiAlgo_BaseCloud<double>)
      anchors = provider->GetAnchorPoints();
    //
    Handle(HStringArray) hLabels = provider->GetLabels();

    m_origins->SetInputPoints(anchors);
    m_origins->Update();

    // Add label array.
    vtkNew<vtkStringArray> labels;
    labels->SetNumberOfValues( anchors->GetNumberOfElements() );
    labels->SetName("labels");
    //
    for ( int i = 0; i < anchors->GetNumberOfElements(); ++i )
    {
      t_asciiString lbl = hLabels->Value(i);
      labels->SetValue( i, lbl.ToCString() );
    }
    //
    m_origins->GetOutput()->GetPointData()->AddArray(labels);

    // Add priority array.
    vtkNew<vtkIntArray> sizes;
    sizes->SetNumberOfValues( anchors->GetNumberOfElements() );
    sizes->SetName("sizes");
    //
    for ( int i = 0; i < anchors->GetNumberOfElements(); ++i )
    {
      sizes->SetValue(i, 1);
    }
    //
    m_origins->GetOutput()->GetPointData()->AddArray(sizes);

    // Generate the label hierarchy.
    m_lblHierarchy->SetInputConnection( m_origins->GetOutputPort() );
    m_lblHierarchy->SetLabelArrayName("labels");
    m_lblHierarchy->SetPriorityArrayName("sizes");
    m_lblHierarchy->GetTextProperty()->SetFontSize( provider->GetSize() );
    m_lblHierarchy->GetTextProperty()->SetColor(r, g, b);
    m_lblHierarchy->Update();

    // Create a mapper and actor for the labels.
    m_lblMapper->SetInputConnection( m_lblHierarchy->GetOutputPort() );

    // Initialize pipeline.
    this->SetInputConnection( m_origins->GetOutputPort() );
  }

  // Update modification timestamp.
  this->Modified();
}

//-----------------------------------------------------------------------------

void asiVisu_LabelsPipeline::callback_add_to_renderer(vtkRenderer* renderer)
{
  renderer->AddActor(m_lblActor);
}

//-----------------------------------------------------------------------------

void asiVisu_LabelsPipeline::callback_remove_from_renderer(vtkRenderer* renderer)
{
  renderer->RemoveActor(m_lblActor);
}

//-----------------------------------------------------------------------------

void asiVisu_LabelsPipeline::callback_update()
{}
