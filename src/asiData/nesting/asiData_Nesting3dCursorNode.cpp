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

// Own include
#include <asiData_Nesting3dCursorNode.h>

// asiData includes
#include <asiData_Nesting3dNbhParameter.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

using namespace asiAlgo::nesting;

//-----------------------------------------------------------------------------

asiData_Nesting3dCursorNode::asiData_Nesting3dCursorNode() : ActData_BaseNode()
{
  // Register standard Active Data Parameters.
  REGISTER_PARAMETER(Name, PID_Name);

  // Register custom Parameters specific to Analysis Situs.
  this->registerParameter(PID_Nbh, asiData_Nesting3dNbhParameter::Instance(), false);
}

//-----------------------------------------------------------------------------

Handle(ActAPI_INode) asiData_Nesting3dCursorNode::Instance()
{
  return new asiData_Nesting3dCursorNode();
}

//-----------------------------------------------------------------------------

void asiData_Nesting3dCursorNode::Init()
{
  this->InitParameter(PID_Name, "Name");

  // Set defaults.
  this->SetNeighborhood( Nbh3d() );
}

//-----------------------------------------------------------------------------

TCollection_ExtendedString asiData_Nesting3dCursorNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_Nesting3dCursorNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------

void asiData_Nesting3dCursorNode::SetNeighborhood(const Nbh3d& nbh)
{
  Handle(asiData_Nesting3dNbhParameter)::DownCast( this->Parameter(PID_Nbh) )->SetNeighborhood(nbh);
}

//-----------------------------------------------------------------------------

Nbh3d asiData_Nesting3dCursorNode::GetNeighborhood() const
{
  return Handle(asiData_Nesting3dNbhParameter)::DownCast( this->Parameter(PID_Nbh) )->GetNeighborhood();
}
