//-----------------------------------------------------------------------------
// Created on: 14 November 2024
// Created by: Sergey KISELEV
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

#ifndef asiAsm_XdeFindDirtyParts_HeaderFile
#define asiAsm_XdeFindDirtyParts_HeaderFile

// asiAsm includes
#include <asiAsm_XdeDoc.h>

// Active Data includes
#include <ActAPI_IAlgorithm.h>

//-----------------------------------------------------------------------------

namespace asiAsm {
namespace xde {

//! \ingroup ASIASM
//!
//! Finds 'dirty' parts (isolated groups of vertices, construction lines, etc.)
//! in the given Assembly Document.
//!
//! \attention Be careful with `checkFacets` argument. Passing `true` means that
//!            a part without visualization facets is recognized as 'dirty'.
//!            However, if you have never generated visualization facets,
//!            all parts will contain no mesh and such reasoning will not be valid.
class FindDirtyParts : public ActAPI_IAlgorithm
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(FindDirtyParts, ActAPI_IAlgorithm)

public:

  //! Ctor.
  //!
  //! \param[in] asmDoc        the document to perform search in.
  //! \param[in] checkVertices the Boolean flag indicating whether to check for dangling vertices.
  //! \param[in] checkEdges    the Boolean flag indicating whether to check for dangling edges.
  //! \param[in] checkFacets   the Boolean flag indicating whether to check if facets exist.
  //! \param[in] checkGeometry the Boolean flag indicating whether to check if empty topology exist.
  //! \param[in] progress      the progress notifier.
  asiAsm_EXPORT
    FindDirtyParts(const Handle(Doc)&   doc,
                   const bool           checkVertices = true,
                   const bool           checkEdges    = true,
                   const bool           checkFacets   = false,
                   const bool           checkGeometry = true,
                   ActAPI_ProgressEntry progress      = nullptr);

public:

  //! Searches for parts which contain 'dirty' geometry.
  //! \param[out] dirtyParts the identified collection of 'dirty' parts.
  //! \return true in the case of success, false -- otherwise.
  asiAsm_EXPORT bool
    Perform(PartIds& dirtyParts);

private:

  Handle(Doc) m_doc;            //!< XDE document with the CAD assembly in question.
  bool        m_bCheckVertices; //!< Whether to check dangling vertices.
  bool        m_bCheckEdges;    //!< Whether to check dangling edges.
  bool        m_bCheckFacets;   //!< Whether to check if facets exist.
  bool        m_bCheckGeometry; //!< Whether to check if empty topology exist.
};

} // xde
} // asiAsm

#endif // asiAsm_FindDirtyParts_HeaderFile
