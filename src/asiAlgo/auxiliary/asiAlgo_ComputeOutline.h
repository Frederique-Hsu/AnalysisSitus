//-----------------------------------------------------------------------------
// Created on: 23 March 2025
//-----------------------------------------------------------------------------
// Copyright (c) 2025-present, Sergey Kiselev
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

#pragma once

// asiAlgo includes
#include "asiAlgo.h"

// asiActiveData
#include <ActAPI_IAlgorithm.h>

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_Optional.h>

// OCCT includes
#include <Precision.hxx>
#include <TopoDS_Shape.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Algorithm to build the outline wires for the passed shape and direction of the view.
class asiAlgo_ComputeOutline : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_ComputeOutline, ActAPI_IAlgorithm)

  public:

    //! HLR mode.
    enum Mode
    {
      Mode_Precise = 0, //!< Precise.
      Mode_Discrete     //!< Discrete.
    };

  public:

    //! Constructor.
    //!
    //! \param[in] aag      the attributed adjacency graph for the input shape.
    //! \param[in] progress the progress entry.
    //! \param[in] plotter  the plotter entry.
    asiAlgo_EXPORT
      asiAlgo_ComputeOutline(const Handle(asiAlgo_AAG)& aag,
                             ActAPI_ProgressEntry       progress = nullptr,
                             ActAPI_PlotterEntry        plotter  = nullptr);

    //! Constructor.
    //!
    //! \param[in] shape    the input shape.
    //! \param[in] progress the progress entry.
    //! \param[in] plotter  the plotter entry.
    asiAlgo_EXPORT
      asiAlgo_ComputeOutline(const TopoDS_Shape&  shape,
                             ActAPI_ProgressEntry progress = nullptr,
                             ActAPI_PlotterEntry  plotter  = nullptr);

    //! Destructor.
    virtual ~asiAlgo_ComputeOutline() {}

  public:

    //! Perform.
    //!
    //! \param[in]  dir          the direction of the view to build the outline.
    //! \param[out] outlineWires the resulting compound of the outline's wires.
    //! \param[in]  mode         the HLR computation mode (precise is the default).
    //!
    //! \return true in case of success, false -- otherwise.
    asiAlgo_EXPORT
      bool Perform(const gp_Dir&    dir,
                   TopoDS_Compound& outlineWires,
                   const Mode       mode = Mode_Precise);

  public:

    void SetLinearTolerance(const double& linTol)
    {
      m_linearTolerance = linTol;
    }

    //! Sets the set of faces IDs to exclude their projections from the outlines.
    //!
    //! \param[in] ids the set of faces ids which projections will be excluded from the outlines.
    asiAlgo_EXPORT
      void ExcludeFaces(const asiAlgo_Feature& ids);

    //! Sets the set of faces to exclude their projections from the outlines.
    //!
    //! \param[in] faces the set of faces which projections will be excluded from the outlines.
    asiAlgo_EXPORT
      void ExcludeFaces(const TopoDS_Shape& faces);

  protected:

    Mode                m_mode;
    Handle(asiAlgo_AAG) m_aag;
    asiAlgo_Feature     m_facesToExclude;  //!< Faces ids to exclude their projections from the outline.
    double              m_linearTolerance;
};
