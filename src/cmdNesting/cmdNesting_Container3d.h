//-----------------------------------------------------------------------------
// Created on: 17 September 2024
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Sergey Slyadnev
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

#ifndef cmdNesting_Container3d_h
#define cmdNesting_Container3d_h

// cmdNesting includes
#include <cmdNesting.h>

// asiAlgo includes
#include <asiAlgo_NestingContainer3d.h>

// asiTcl includes
#include <asiTcl_Variable.h>

//-----------------------------------------------------------------------------

//! 3D nesting container in a Tcl session.
class cmdNesting_Container3d : public asiTcl_Variable
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(cmdNesting_Container3d, asiTcl_Variable)

public:

  //! Ctor.
  //! \param[in] container the nesting container to set.
  cmdNesting_EXPORT
    cmdNesting_Container3d(const Handle(asiAlgo::nesting::Container3d)& container = nullptr);

  //! Dtor.
  cmdNesting_EXPORT virtual
    ~cmdNesting_Container3d();

public:

  //! Sets the container into the Tcl variable.
  //! \param[in] container the container to set.
  cmdNesting_EXPORT void
    SetContainer(const Handle(asiAlgo::nesting::Container3d)& container);

  //! \return the referenced 3D nesting container.
  cmdNesting_EXPORT const Handle(asiAlgo::nesting::Container3d)&
    GetContainer() const;

public:

  //! \return brief description "what is" this object.
  cmdNesting_EXPORT virtual std::string
    WhatIs() const;

  //! Dumps this variable to the passed output stream.
  //! \param[in,out] out the output stream.
  cmdNesting_EXPORT virtual void
    Dump(std::ostream& out) const;

protected:

  Handle(asiAlgo::nesting::Container3d) m_container; //!< Owned nesting container.

};

#endif
