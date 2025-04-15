//-----------------------------------------------------------------------------
// Created on: 12 May 2025
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

// OCCT inludes
#include <TopoDS_Wire.hxx>

// STL includes
#include <vector>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Module namespace.
namespace asiAlgo {

//! \ingroup ASI_MODELING
//!
//! Algorithms.
namespace algo {

//! Outline.
class Outline : public Standard_Transient
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(Outline, Standard_Transient)

  public:

    //! Ctor.
    asiAlgo_EXPORT
      Outline(const TopoDS_Wire& wire);

    //! Destructor.
    asiAlgo_EXPORT
      virtual ~Outline();

    //! Ctor.
    asiAlgo_EXPORT explicit
      Outline(const Outline& outline);

  public:

    //! Returns the length of the outline.
    double Length() const
    {
      return m_length;
    }

    //! Constructs a wire for the outline from the saved vector of edges.
    const TopoDS_Wire& Wire() const
    {
      return m_wire;
    }

    //! Returns the boolean flag if the outline is closed or not.
    bool IsClosed() const
    {
      return m_isClosed;
    }

  public:

    //! Checks if the passed outline is equal to this one.
    //!
    //! \param[in] other       the other outline data structure.
    //! \param[in] linToler    the linear tolerance.
    //! \param[in] angTolerDeg the angular tolerance in degrees.
    //!
    //! \return true in case of success, false -- otherwise.
    asiAlgo_EXPORT bool
      IsEqual(const Handle(Outline)& other,
              const double           linToler,
              const double           angTolerDeg) const;

    //! Constructs the outline structure from a JSON object.
    //!
    //! \param[in]  pJsonGenericObj the JSON object to construct the data structure from.
    //! \param[out] outlinee        the outcome feature.
    asiAlgo_EXPORT static void
      FromJSON(void*            pJsonGenericObj,
               Handle(Outline)& outline);

    //! Converts the passed feature to JSON (the passed `out` stream).
    //!
    //! \param[in]     outline the outline to serialize.
    //! \param[in]     indent  the pretty indentation shift.
    //! \param[in,out] out     the output JSON string stream.
    asiAlgo_EXPORT static void
      ToJSON(const Handle(Outline)& outline,
             const int              indent,
             std::ostream&          out);

  private:

    //! Ctor.
    asiAlgo_EXPORT
      Outline();

  private:

    int    m_nbEdges;
    bool   m_isClosed;
    double m_length;

    TopoDS_Wire m_wire;

};

}
}
