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

#ifndef asiVisu_AdjGraphDataProvider_h
#define asiVisu_AdjGraphDataProvider_h

// asiVisu includes
#include <asiVisu_DataProvider.h>

// asiData includes
#include <asiData_PartNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

//! Data provider for AAG from the Part Node.
class asiVisu_AdjGraphDataProvider : public asiVisu_DataProvider
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_AdjGraphDataProvider, asiVisu_DataProvider)

public:

  //! Ctor.
  asiVisu_EXPORT
    asiVisu_AdjGraphDataProvider(const Handle(asiData_PartNode)& partNode);

protected:

  //! Protected ctor.
  asiVisu_EXPORT
    asiVisu_AdjGraphDataProvider();

public:

  //! Returns ID of the Data Node which is being sourced by the visualization
  //! pipeline.
  //! \return Node ID.
  asiVisu_EXPORT virtual ActAPI_DataObjectId
    GetNodeID() const;

  //! \return the active Part's AAG.
  asiVisu_EXPORT virtual Handle(asiAlgo_AAG)
    GetAAG() const;

  //! \return the Boolean flag activating/deactivating the AAG rendering mode.
  asiVisu_EXPORT virtual bool
    IsRenderAAG() const;

protected:

  //! Enumerates Data Parameters playing as sources for DOMAIN -> VTK
  //! translation process.
  //! \return source Parameters.
  virtual Handle(ActAPI_HParameterList)
    translationSources() const;

protected:

  //! Source Node.
  Handle(asiData_PartNode) m_node;

};

#endif
