//-----------------------------------------------------------------------------
// Created on: 11 October 2024
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

#ifndef asiData_Nesting3dCursorNode_h
#define asiData_Nesting3dCursorNode_h

// asiData includes
#include <asiData.h>

// asiAlgo includes
#include <asiAlgo_NestingNbh3d.h>

// Active Data includes
#include <ActData_BaseNode.h>

//! "Cursor" of DBLF filling algorithm for 3D nesting.
class asiData_Nesting3dCursorNode : public ActData_BaseNode
{
public:

  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_Nesting3dCursorNode, ActData_BaseNode)

  // Automatic registration of Node type in global factory
  DEFINE_NODE_FACTORY(asiData_Nesting3dCursorNode, Instance)

public:

  //! IDs for the underlying Parameters.
  enum ParamId
  {
  //----------//
    PID_Name, //!< Name of the Node.
    PID_Nbh,  //!< Neighborhood of a candidate position.
  //----------//
    PID_Last = PID_Name + ActData_BaseNode::RESERVED_PARAM_RANGE
  };

public:

  //! Returns new DETACHED instance of the Node ensuring its correct
  //! allocation in heap.
  //! \return new instance of the Node.
  asiData_EXPORT static Handle(ActAPI_INode)
    Instance();

// Initialization:
public:

  //! Performs initial actions required to make this Node WELL-FORMED.
  asiData_EXPORT void
    Init();

// Generic naming support:
public:

  //! Accessor for the Node's name.
  //! \return name of the Node.
  asiData_EXPORT virtual TCollection_ExtendedString
    GetName();

  //! Sets name for the Node.
  //! \param[in] name name to set.
  asiData_EXPORT virtual void
    SetName(const TCollection_ExtendedString& name);

public:

  //! Sets the neighborhood to store.
  asiData_EXPORT void
    SetNeighborhood(const asiAlgo::nesting::Nbh3d& nbh);

  //! \return the stored neighborhood.
  asiData_EXPORT asiAlgo::nesting::Nbh3d
    GetNeighborhood() const;

protected:

  //! Default ctor. Registers all involved Parameters.
  asiData_EXPORT
    asiData_Nesting3dCursorNode();

};

#endif
