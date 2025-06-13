//-----------------------------------------------------------------------------
// Created on: 05 June 2025
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2025-present, Sergey Slyadnev
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

#ifndef asiSmm_MakeEdgeFlange_h
#define asiSmm_MakeEdgeFlange_h

// asiSmm includes
#include "asiSmm.h"

// asiAlgo includes
#include <asiAlgo_AAG.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

namespace asiSmm {

//! \ingroup ASI_SMM
//!
//! Creates a flange out of an edge of a base shape.
class MakeEdgeFlange : public ActAPI_IAlgorithm
{
  // OpenCascade RTTI
  DEFINE_STANDARD_RTTI_INLINE(MakeEdgeFlange, ActAPI_IAlgorithm)

public:

  //! Ctor with the base shape.
  asiSmm_EXPORT
    MakeEdgeFlange(const TopoDS_Shape&  baseShape,
                   ActAPI_ProgressEntry progress = nullptr,
                   ActAPI_PlotterEntry  plotter  = nullptr);

  //! Ctor with the AAG of the base shape.
  asiSmm_EXPORT
    MakeEdgeFlange(const Handle(asiAlgo_AAG)& G,
                   ActAPI_ProgressEntry       progress = nullptr,
                   ActAPI_PlotterEntry        plotter  = nullptr);

public:

  //! Initializes the algorithm.
  asiSmm_EXPORT void
    Initialize(const TopoDS_Shape& baseShape);

public:

  //! Constructs feature shapes for the bend and the flange wall.
  //! This function is exposed publicly to be used for a preview.
  //! \param[in]  eid      the numerical 1-based ID of the seed edge.
  //! \param[in]  alphaDeg the desired bend angle (in degrees).
  //! \param[in]  l        the desired flange length.
  //! \param[out] bend     the bend solid.
  //! \param[out] wall     the flange wall solid.
  //! \return true in the case of success, false -- otherwise.
  asiSmm_EXPORT bool
    BuildFeatureSolids(const int     eid,
                       const double  alphaDeg,
                       const double  l,
                       TopoDS_Solid& bend,
                       TopoDS_Solid& wall) const;

  //! Creates a flange starting from the given `edge` on the base shape.
  //! \param[in] eid      the numerical 1-based ID of the seed edge.
  //! \param[in] alphaDeg the desired bend angle (in degrees).
  //! \param[in] l        the desired flange length.
  //! \return true in the case of success, false -- otherwise.
  asiSmm_EXPORT bool
    Build(const int    eid,
          const double alphaDeg,
          const double l);

public:

  //! \return the result shape.
  const TopoDS_Shape& GetResult() const
  {
    return m_result;
  }

protected:

  //! Selects a face that would be used to generate a bend feature.
  //! \param[in]  eid       the 1-based ID of the seed edge.
  //! \param[out] thickness the computed material thickness.
  //! \return the 1-based ID of the selected thickness face.
  asiSmm_EXPORT int
    findThicknessFace(const int eid,
                      double&   thickness) const;

  //! Probes thickness values at the passed face.
  //! \param[in]  fid the ID of the face in question.
  //! \param[in]  E   the seed edge.
  //! \param[out] t   the measured thickness value.
  //! \return false if thickness cannot be figured out.
  asiSmm_EXPORT bool
    probeThickness(const int          fid,
                   const TopoDS_Edge& E,
                   double&            t) const;

protected:

  TopoDS_Shape        m_baseShape; //!< Base shape before a flange is constructed.
  Handle(asiAlgo_AAG) m_baseG;     //!< AAG of the base shape.
  TopoDS_Shape        m_result;    //!< The result of modeling.

};

} // `asiSmm` namespace.

#endif
