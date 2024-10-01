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

#ifndef asiAlgo_Shaft_h
#define asiAlgo_Shaft_h

// asiAlgo includes
#include <asiAlgo_FeatureFaces.h>

// OpenCascade includes
#include <gp_Ax1.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_AFR
//!
//! Descriptor for a shaft feature.
struct asiAlgo_Shaft : public Standard_Transient
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_Shaft, Standard_Transient)

  asiAlgo_Feature fids;     //!< Face IDs of a shaft.
  double          diameter; //!< Diameter.
  double          length;   //!< Length.
  gp_Ax1          axis;     //!< Shaft axis.

  //! Default ctor.
  asiAlgo_EXPORT
    asiAlgo_Shaft();

  //! Checks if the passed structure equals this one.
  //! \param[in] other     the shaft structure to compare this one with.
  //! \param[in] linToler  the linear tolerance.
  //! \param[in] angTolDeg the angular tolerance in degrees.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT bool
    IsEqual(const Handle(asiAlgo_Shaft)& other,
            const double                 linToler,
            const double                 angTolDeg) const;

  //! Constructs shaft data structure from a JSON object.
  //! \param[in]  pJsonGenericObj the JSON object to construct the shaft structure from.
  //! \param[out] shaft           the outcome shaft.
  asiAlgo_EXPORT static void
    FromJSON(void*                  pJsonGenericObj,
             Handle(asiAlgo_Shaft)& shaft);

  //! Serializes the passed shaft structure to JSON (the passed `out` stream).
  //! \param[in]     shaft  the shaft structure to serialize.
  //! \param[in]     indent the pretty indentation shift.
  //! \param[in,out] out    the output JSON string stream.
  asiAlgo_EXPORT static void
    ToJSON(const Handle(asiAlgo_Shaft)& shaft,
           const int                    indent,
           std::ostream&                out);
};

#endif // asiAlgo_Shaft_h
