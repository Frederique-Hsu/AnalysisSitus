//-----------------------------------------------------------------------------
// Created on: 09 December 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC
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

#ifndef asiAlgo_MeshOrient_h
#define asiAlgo_MeshOrient_h

// asiAlgo includes
#include "asiAlgo_Optional.h"

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OpenCascade includes
#include <gp_Ax3.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! This class attempts to find a better orientation for the passed mesh piece.
class asiAlgo_MeshOrient : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_MeshOrient, ActAPI_IAlgorithm)

public:

  //! Constructs the reorientation tool.
  //! \param[in] mesh     the mesh to orient.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_MeshOrient(const Handle(Poly_Triangulation)& mesh,
                       ActAPI_ProgressEntry              progress = nullptr,
                       ActAPI_PlotterEntry               plotter  = nullptr);

public:

  //! Performs reorientation.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

public:

  //! \return the computed transformation to apply to the shape.
  const gp_Trsf& GetTrsf() const
  {
    return m_T;
  }

  //! \return the detected local axes.
  const tl::optional<gp_Ax3>& GetAxes() const
  {
    return m_ax;
  }

  //! \return linear characteristic part's extents.
  double GetExtents() const
  {
    return m_fExtents;
  }

protected:

  Handle(Poly_Triangulation) m_mesh;     //!< The mesh to orient.
  gp_Trsf                    m_T;        //!< The computed transformation.
  tl::optional<gp_Ax3>       m_ax;       //!< The computed local axes.
  double                     m_fExtents; //!< The characteristic dimension.

};

#endif
