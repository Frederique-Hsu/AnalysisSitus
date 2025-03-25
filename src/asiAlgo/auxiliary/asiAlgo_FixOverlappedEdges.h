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

//! Algorithm to find fully overlapped edges (duplicates) and
//! keep only a single instance of such edges.
class asiAlgo_FixOverlappedEdges : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_FixOverlappedEdges, ActAPI_IAlgorithm)

  public:

    //! Constructor.
    //!
    //! \param[in] progress the progress entry.
    //! \param[in] plotter  the plotter entry.
    asiAlgo_FixOverlappedEdges(ActAPI_ProgressEntry progress = nullptr,
                               ActAPI_PlotterEntry  plotter  = nullptr)
      : ActAPI_IAlgorithm( progress, plotter ),
        m_maxAllowedDistance( Precision::Confusion() )
    {}

    //! Destructor.
    virtual ~asiAlgo_FixOverlappedEdges() {}

  public:

    //! Perform.
    //!
    //! \param[in/out] edges the initial set to proceed.
    //!
    //! \return true in case of success, false -- otherwise.
    asiAlgo_EXPORT
      bool Perform(std::vector< TopoDS_Edge >& edges);


    //! Sets the maximum allowed distance between edges to mark them as overlapped.
    void SetMaxAllowedDistance(const double& maxAllowedDistance)
    {
      m_maxAllowedDistance = maxAllowedDistance;
    }

  private:

    double m_maxAllowedDistance; //!< Max allowed distance between edges.
};
