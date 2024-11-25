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
#include <asiVisu_Nesting3dCursorPrs.h>

// asiVisu includes
#include <asiVisu_Nesting3dNbhDataProvider.h>
#include <asiVisu_Nesting3dNbhPipeline.h>

// VTK includes
#include <vtkMapper.h>
#include <vtkProperty.h>

using namespace asiAlgo::nesting;

//-----------------------------------------------------------------------------

asiVisu_Nesting3dCursorPrs::asiVisu_Nesting3dCursorPrs(const Handle(ActAPI_INode)& N)
//
: asiVisu_DefaultPrs(N)
{
  Handle(asiData_Nesting3dCursorNode)
    cursorNode = Handle(asiData_Nesting3dCursorNode)::DownCast(N);

  /* Axes */
  {
    // Create data provider.
    Handle(asiVisu_Nesting3dNbhDataProvider)
      axesProvider = new asiVisu_Nesting3dNbhDataProvider(cursorNode);

    // Create pipeline for the axes.
    Handle(asiVisu_Nesting3dNbhPipeline)
      pipeline = new asiVisu_Nesting3dNbhPipeline;
    //
    this->addPipeline        ( Pipeline_Axes, pipeline );
    this->assignDataProvider ( Pipeline_Axes, axesProvider );
  }
}

//-----------------------------------------------------------------------------

Handle(asiVisu_Prs)
  asiVisu_Nesting3dCursorPrs::Instance(const Handle(ActAPI_INode)& N)
{
  return new asiVisu_Nesting3dCursorPrs(N);
}

//-----------------------------------------------------------------------------

void asiVisu_Nesting3dCursorPrs::afterUpdatePipelines() const
{
  Handle(asiData_Nesting3dCursorNode)
    N = Handle(asiData_Nesting3dCursorNode)::DownCast( this->GetNode() );

  Nbh3d nbv = N->GetNeighborhood();

  vtkActor* pActor = this->GetPipeline(Pipeline_Axes)->Actor();

  if ( nbv.isRoomy )
    pActor->GetProperty()->SetColor(YAXIS_R, YAXIS_G, YAXIS_B);
  else
    pActor->GetProperty()->SetColor(XAXIS_R, XAXIS_G, XAXIS_B);

  pActor->GetProperty()->SetLineWidth(5.f);
}
