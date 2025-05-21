//-----------------------------------------------------------------------------
// Created on: 19 May 2025
// Created by: Sergey SLYADNEV
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

#ifndef asiTcl_PtrVariable_h
#define asiTcl_PtrVariable_h

// asiTcl includes
#include <asiTcl_Variable.h>

// Standard includes
#include <sstream>

//-----------------------------------------------------------------------------

//! Variable in a Tcl session that stores a raw pointer.
class asiTcl_PtrVariable : public asiTcl_Variable
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiTcl_PtrVariable, asiTcl_Variable)

public:

  //! Ctor.
  asiTcl_PtrVariable(void* ptr) : m_pPtr(ptr)
  {}

public:

  //! \return the stored raw pointer.
  void* GetPtr() const
  {
    return m_pPtr;
  }

public:

  //! \return brief description "what is" this object.
  virtual std::string WhatIs() const
  {
    return "raw pointer";
  }

  //! Dumps this variable to the passed output stream.
  //! \param[in,out] out the output stream.
  virtual void Dump(std::ostream& out) const
  {
    std::string addr_str;
    std::ostringstream ost;
    ost << m_pPtr;
    addr_str = ost.str();

    out << addr_str;
  }

protected:

  void* m_pPtr; //!< Stored raw pointer.

};

#endif
