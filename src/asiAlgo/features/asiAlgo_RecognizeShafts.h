//-----------------------------------------------------------------------------
// Created on: 28 September 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC (www.quaoar-studio.com)
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

#ifndef asiAlgo_RecognizeShafts_h
#define asiAlgo_RecognizeShafts_h

// asiAlgo includes
#include <asiAlgo_Recognizer.h>
#include <asiAlgo_Shaft.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// OpenCascade includes
#include <Precision.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_AFR
//!
//! Utility to recognize shaft features ("holes inverted").
class asiAlgo_RecognizeShafts : public asiAlgo_Recognizer
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_RecognizeShafts, asiAlgo_Recognizer)

public:

  //! Ctor.
  //! \param[in] aag      the attributed adjacency graph.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiAlgo_EXPORT
    asiAlgo_RecognizeShafts(const Handle(asiAlgo_AAG)& aag,
                            ActAPI_ProgressEntry       progress = nullptr,
                            ActAPI_PlotterEntry        plotter  = nullptr);

public:

  //! Sets the linear tolerance to use. If not set, the tolerance will be
  //! taken from the model.
  //! \param[in] tol the tolerance to set.
  asiAlgo_EXPORT void
    SetLinearTolerance(const double tol);

  //! Sets the universum of faces to recognize.
  //! \param[in] domain the faces to recognize.
  asiAlgo_EXPORT void
    SetDomain(const asiAlgo_Feature& domain);

  //! Sets the collection of face IDs to exclude from the consideration.
  //! Use this method to narrow down the candidate list of the seed faces.
  //! \param[in] fids the face IDs to exclude from the collection of seeds.
  asiAlgo_EXPORT void
    SetExcludedFaces(const asiAlgo_Feature& fids);

public:

  //! Performs recognition.
  //! \param[in] radius the max radius (infinite by default).
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform(const double radius = Precision::Infinite());

public:

  //! \return the stored AAG.
  const Handle(asiAlgo_AAG)& GetAAG() const
  {
    return m_aag;
  }

  //! \return the recognized shaft features.
  const std::vector<Handle(asiAlgo_Shaft)>& GetShafts() const
  {
    return m_shafts;
  }

protected:

  //! Performs recognition.
  //! \param[in] radius the max radius.
  //! \return true in the case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    performInternal(const double radius);

  //! Computes shaft properties for the given collection of indices.
  //! \param[in]  fids  the face IDs of an isolated shaft feature.
  //! \param[out] shaft the populated shaft feature.
  asiAlgo_EXPORT void
    populateShaft(const asiAlgo_Feature& fids,
                  Handle(asiAlgo_Shaft)& shaft) const;

protected:

  double                             m_fLinToler; //!< Linear tolerance to use.
  asiAlgo_Feature                    m_domain;    //!< Universum.
  asiAlgo_Feature                    m_xSeeds;    //!< IDs of the excluded faces.
  std::vector<Handle(asiAlgo_Shaft)> m_shafts;    //!< Extracted shafts with props.

};

#endif
