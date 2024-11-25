//-----------------------------------------------------------------------------
// Created on: 12 October 2024
// Created by: Sergey SLYADNEV
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

// Own include
#include <asiData_Nesting3dNbhAttr.h>

// OCCT includes
#include <Standard_GUID.hxx>

using namespace asiAlgo::nesting;

//-----------------------------------------------------------------------------
// Construction & settling-down routines
//-----------------------------------------------------------------------------

//! Default constructor.
asiData_Nesting3dNbhAttr::asiData_Nesting3dNbhAttr() : TDF_Attribute()
{}

//! Settles down new Attribute to the given OCAF Label.
//! \param[in] label TDF Label to settle down the new Attribute to.
//! \return newly created Attribute settled down onto the target Label.
Handle(asiData_Nesting3dNbhAttr) asiData_Nesting3dNbhAttr::Set(const TDF_Label& label)
{
  Handle(asiData_Nesting3dNbhAttr) A;
  //
  if ( !label.FindAttribute(GUID(), A) )
  {
    A = new asiData_Nesting3dNbhAttr();
    label.AddAttribute(A);
  }
  return A;
}

//-----------------------------------------------------------------------------
// Accessors for Attribute's GUID
//-----------------------------------------------------------------------------

//! Returns statically defined GUID for this Attribute type.
//! \return statically defined GUID.
const Standard_GUID& asiData_Nesting3dNbhAttr::GUID()
{
  static Standard_GUID AttrGUID("0982F77B-2873-488D-876A-CFF4A36D7771");
  return AttrGUID;
}

//! Accessor for GUID associated with this kind of OCAF Attribute.
//! \return GUID of the OCAF Attribute.
const Standard_GUID& asiData_Nesting3dNbhAttr::ID() const
{
  return GUID();
}

//-----------------------------------------------------------------------------
// Attribute's kernel methods:
//-----------------------------------------------------------------------------

//! Creates new instance of the Attribute which is not initially populated
//! with any data structures.
//! \return new instance of AAG Attribute.
Handle(TDF_Attribute) asiData_Nesting3dNbhAttr::NewEmpty() const
{
  return new asiData_Nesting3dNbhAttr();
}

//! Performs data transferring from the given OCAF Attribute to this one.
//! This method is mainly used by OCAF Undo/Redo kernel as a part of
//! backup functionality.
//! \param[in] mainAttr OCAF Attribute to copy data from.
void asiData_Nesting3dNbhAttr::Restore(const Handle(TDF_Attribute)& mainAttr)
{
  Handle(asiData_Nesting3dNbhAttr)
    fromCasted = Handle(asiData_Nesting3dNbhAttr)::DownCast(mainAttr);
  //
  m_nbh = fromCasted->GetNeighborhood();
}

//! Supporting method for Copy/Paste functionality. Performs full copying of
//! the underlying data.
//! \param[in] into       where to paste.
//! \param[in] relocTable relocation table.
void asiData_Nesting3dNbhAttr::Paste(const Handle(TDF_Attribute)&       into,
                                     const Handle(TDF_RelocationTable)& asiData_NotUsed(relocTable)) const
{
  Handle(asiData_Nesting3dNbhAttr)
    intoCasted = Handle(asiData_Nesting3dNbhAttr)::DownCast(into);
  //
  intoCasted->SetNeighborhood(m_nbh);
}

//-----------------------------------------------------------------------------
// Accessors for domain-specific data
//-----------------------------------------------------------------------------

//! Sets the neighborhood to store.
//! \param[in] nbh the neighborhood to store.
void asiData_Nesting3dNbhAttr::SetNeighborhood(const Nbh3d& nbh)
{
  this->Backup();

  m_nbh = nbh;
}

//! \return the stored neighborhood.
const Nbh3d& asiData_Nesting3dNbhAttr::GetNeighborhood() const
{
  return m_nbh;
}
