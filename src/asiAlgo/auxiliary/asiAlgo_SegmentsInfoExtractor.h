//-----------------------------------------------------------------------------
// Created on: 24 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Kiselev
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

#ifndef asiAlgo_SegmentsInfoExtractor_h
#define asiAlgo_SegmentsInfoExtractor_h

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// asiAlgo include
#include <asiAlgo_SegmentsInfo.h>

// OCCT includes
#include <Geom_Curve.hxx>
#include <gp_Pln.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>

// STL includes
#include <vector>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Class to extract segments information from the passed wire.
class asiAlgo_SegmentsInfoExtractor : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_SegmentsInfoExtractor, ActAPI_IAlgorithm)

public:

  //! Constructor.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_SegmentsInfoExtractor(ActAPI_ProgressEntry progress = nullptr,
                                  ActAPI_PlotterEntry  plotter  = nullptr);

public:

  //! Extracts information about segments (edges) from the passed wire.
  //!
  //! \param[in]  wire            the wire to extract segments information from.
  //! \param[out] segmentsInfoVec the resulting vector of segments infomation data structures.
  //!
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT
    bool Perform(const TopoDS_Wire&       wire,
                 asiAlgo_SegmentsInfoVec& segmentsInfoVec);

  //! Extracts information about segments (edges) from the passed face.
  //!
  //! \param[in]  face            the face to extract segments information from.
  //! \param[out] segmentsInfoVec the resulting vector of segments infomation data structures.
  //!
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT
    bool Perform(const TopoDS_Face&       face,
                 asiAlgo_SegmentsInfoVec& segmentsInfoVec);

};

#endif
