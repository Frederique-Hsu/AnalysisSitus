//-----------------------------------------------------------------------------
// Created on: 13 April 2025
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

#ifndef asiAlgo_NamingInfo_h
#define asiAlgo_NamingInfo_h

// asiAlgo includes
#include <asiAlgo_JsonDict.h>

// Active Data includes
#include <ActAPI_Common.h>

// STL includes
#include <ostream>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! \brief Persistent representation of naming registry.
struct asiAlgo_NamingInfo
{
  //! Deserializes naming info from the given JSON stream.
  //! \param[in]  pJsonGenericObj the JSON object to populate the outcome data structure from.
  //! \param[out] info            the outcome naming info.
  asiAlgo_EXPORT static void
    FromJSON(void*               pJsonGenericObj,
             asiAlgo_NamingInfo& info);

  //! Serializes the passed naming info structure to JSON (the passed `out` stream).
  //! \param[in]     info     the data structur to serialize.
  //! \param[in]     indent   the pretty indentation shift.
  //! \param[in,out] out      the output JSON string stream.
  //! \param[in]     pureJSON the flag to convert info to pure JSON (without escaping symbols).
  asiAlgo_EXPORT static void
    ToJSON(const asiAlgo_NamingInfo& info,
           const int                 indent,
           std::ostream&             out,
           const bool                pureJSON = false);

  //! Checks if this data structure equals the passed `other` one.
  //! \param[in] other the other data structure to compare with.
  //! \return true in the case of equality, false -- otherwise.
  asiAlgo_EXPORT bool
    IsEqual(const asiAlgo_NamingInfo& other) const;

  int           fid;  //!< Face ID.
  t_asciiString name; //!< Face name.
};

#endif
