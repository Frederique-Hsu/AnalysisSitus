//-----------------------------------------------------------------------------
// Created on: 06 February 2025
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

#ifndef asiVisu_IVLabelsDataProvider_h
#define asiVisu_IVLabelsDataProvider_h

// asiVisu includes
#include <asiVisu_LabelsDataProvider.h>

// asiData includes
#include <asiData_IVLabelFieldNode.h>

//! Data provider for IV label fields.
class asiVisu_IVLabelsDataProvider : public asiVisu_LabelsDataProvider
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_IVLabelsDataProvider, asiVisu_LabelsDataProvider)

public:

  asiVisu_EXPORT
    asiVisu_IVLabelsDataProvider(const Handle(asiData_IVLabelFieldNode)& node);

public:

  asiVisu_EXPORT virtual Handle(asiAlgo_BaseCloud<double>)
    GetAnchorPoints() const;

  asiVisu_EXPORT virtual Handle(HStringArray)
    GetLabels() const;

  asiVisu_EXPORT virtual int
    GetSize() const;

  asiVisu_EXPORT virtual void
    GetColor(double& r, double& g, double& b) const;

private:

  virtual Handle(ActAPI_HParameterList)
    translationSources() const;

protected:

  Handle(asiAlgo_BaseCloud<double>) m_points; //!< Cached points.
  Handle(HStringArray)              m_labels; //!< Cached labels.

};

#endif
