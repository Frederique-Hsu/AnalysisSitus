//-----------------------------------------------------------------------------
// Created on: 12 October 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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

#ifndef asiAlgo_FindOptimalOrientation_h
#define asiAlgo_FindOptimalOrientation_h

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_History.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OpenCascade includes
#include <gp_Ax1.hxx>
#include <TopoDS_Shape.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Finds the optimal position that gives the smallest stock size for a part.
//!
//! The motivation is to place a part in a way, so that its stock bounding box
//! has quasioptimal dimensions to minimize material waste.
class asiAlgo_FindOptimalOrientation : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_FindOptimalOrientation, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //! \param[in] shape    the input (part) shape.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_FindOptimalOrientation(const TopoDS_Shape&  shape,
                                   ActAPI_ProgressEntry progress = nullptr,
                                   ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Performs orientation algorithm.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

public:

  //! Sets preset axis.
  //! \param[in] isSetPresetAxis indicator showing that preset axis is taken into account.
  //! \param[in] presetAxis      preset axis.
  void SetPresetAxis(const bool isSetPresetAxis,
                     gp_Ax1     presetAxis)
  {
    m_bIsSetPresetAxis = isSetPresetAxis;
    m_presetAxis       = presetAxis;
  }

  //! \return transformation to optimal position.
  const gp_Trsf& GetTransformation() const
  {
    return m_trsf;
  }

  //! \return shape in the optimal position.
  const TopoDS_Shape& GetResult() const
  {
    return m_result;
  }

  //! \return the modification history.
  const Handle(asiAlgo_History)& GetHistory() const
  {
    return m_history;
  }

protected:

  TopoDS_Shape m_shape;            //!< Input shape.
  bool         m_bIsSetPresetAxis; //!< Indicator showing that preset axis is taken into account.
  gp_Ax1       m_presetAxis;       //!< Preset axis. Fixed axis - necessary in case of turning.

  // Result.
  gp_Trsf                 m_trsf;    //!< Transformation to optimal position.
  TopoDS_Shape            m_result;  //!< Result shape.
  Handle(asiAlgo_History) m_history; //!< History of modification.

};

#endif
