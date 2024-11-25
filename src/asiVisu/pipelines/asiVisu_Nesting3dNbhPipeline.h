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

#ifndef asiVisu_Nesting3dNbhPipeline_h
#define asiVisu_Nesting3dNbhPipeline_h

// asiVisu includes
#include <asiVisu_DataProvider.h>
#include <asiVisu_Nesting3dNbhSource.h>
#include <asiVisu_Pipeline.h>

// VTK includes
#include <vtkLookupTable.h>

//-----------------------------------------------------------------------------

//! Visualization pipeline for 3d nesting neighborhood.
class asiVisu_Nesting3dNbhPipeline : public asiVisu_Pipeline
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_Nesting3dNbhPipeline, asiVisu_Pipeline)

public:

  asiVisu_EXPORT
    asiVisu_Nesting3dNbhPipeline();

public:

  asiVisu_EXPORT virtual void
    SetInput(const Handle(asiVisu_DataProvider)& DP);

private:

  virtual void callback_add_to_renderer      (vtkRenderer* pRenderer);
  virtual void callback_remove_from_renderer (vtkRenderer* pRenderer);
  virtual void callback_update               ();

private:

  //! Copying prohibited.
  asiVisu_Nesting3dNbhPipeline(const asiVisu_Nesting3dNbhPipeline&) = delete;

  //! Assignment prohibited.
  asiVisu_Nesting3dNbhPipeline& operator=(const asiVisu_Nesting3dNbhPipeline&) = delete;

protected:

  //! Neighborhood source.
  vtkSmartPointer<asiVisu_Nesting3dNbhSource> m_source;

};

#endif
