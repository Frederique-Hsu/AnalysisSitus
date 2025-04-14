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

// OCCT includes
#include <Precision.hxx>
#include <TopoDS_Edge.hxx>

//-----------------------------------------------------------------------------

//! Algorithm which finds intersections of edges by their first and last vertices
//! and splits them at these vertices.
class asiAlgo_SplitEdgesByStartEndPoints : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_SplitEdgesByStartEndPoints, ActAPI_IAlgorithm)

  public:

    //! Constructor.
    //!
    //! \param[in] progress the progress entry.
    //! \param[in] plotter  the plotter entry.
    asiAlgo_SplitEdgesByStartEndPoints(ActAPI_ProgressEntry progress = nullptr,
                                       ActAPI_PlotterEntry  plotter  = nullptr)
      : ActAPI_IAlgorithm( progress, plotter ),
        m_minAllowedDistFromEnds(  Precision::Confusion() ),
        m_maxDistFromPointToCurve( Precision::Confusion() )
    {}

    //! Destructor.
    virtual ~asiAlgo_SplitEdgesByStartEndPoints() {}

  public:

    //! Perform.
    //!
    //! \param[in/out] edges the initial set to proceed.
    //!
    //! \return true in case of success, false -- otherwise.
    asiAlgo_EXPORT
      bool Perform(std::vector< TopoDS_Edge >& edges);

    //! Sets the tolerance which is used to check if the intersection point is too
    //! close to some edge's end.
    void SetMinAllowedDistFromEnds(const double& minAllowedDistFromEnds)
    {
      m_minAllowedDistFromEnds = minAllowedDistFromEnds;
    }

    //! Sets the maximum distance between the point and the curve.
    void SetMaxDistFromPointToCurve(const double& maxDistFromPointToCurve)
    {
      m_maxDistFromPointToCurve = maxDistFromPointToCurve;
    }

  protected:

    double m_minAllowedDistFromEnds;  //!< Tolerance which is used to check if the intersection
                                      //!< point is too close to some edge's end.

    double m_maxDistFromPointToCurve; //!< Maximum distance between the point and the curve.

};
