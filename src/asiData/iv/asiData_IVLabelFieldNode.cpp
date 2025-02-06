//-----------------------------------------------------------------------------
// Created on: 05 February 2025
//-----------------------------------------------------------------------------
// Copyright (c) 2025-present, Quaoar Studio LLC (http://analysissitus.org)
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
#include <asiData_IVLabelFieldNode.h>

// asiAlgo includes
#include <asiAlgo_PointCloudUtils.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_IVLabelFieldNode::asiData_IVLabelFieldNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,        PID_Name);
  REGISTER_PARAMETER(RealArray,   PID_Points);
  REGISTER_PARAMETER(StringArray, PID_Labels);
  REGISTER_PARAMETER(Group,       PID_GroupPrs);
  REGISTER_PARAMETER(Int,         PID_Color);
  REGISTER_PARAMETER(Int,         PID_Size);
}

//! Returns new DETACHED instance of the Node ensuring its correct
//! allocation in a heap.
//! \return new instance of the Node.
Handle(ActAPI_INode) asiData_IVLabelFieldNode::Instance()
{
  return new asiData_IVLabelFieldNode();
}

//! Performs initial actions required to make Node WELL-FORMED.
void asiData_IVLabelFieldNode::Init()
{
  // Initialize properties.
  this->InitParameter (PID_Name,     "Name",         "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_GroupPrs, "Presentation", "",               ParameterFlag_IsVisible, true);
  this->InitParameter (PID_Color,    "Color",        "PrsCustomColor", ParameterFlag_IsVisible, true);
  this->InitParameter (PID_Size,     "Size",         "",               ParameterFlag_IsVisible, true);
  //
  this->SetPoints   ( nullptr );
  this->SetLabels   ( nullptr );
  this->SetColor    ( 16777215 );
  this->SetSize     ( 22 );
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_IVLabelFieldNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param[in] name the name to set.
void asiData_IVLabelFieldNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------
// Handy API
//-----------------------------------------------------------------------------

//! \return stored point cloud representing the positions of labels.
Handle(HRealArray) asiData_IVLabelFieldNode::GetPoints() const
{
  return P_RealArray(this, PID_Points)->GetArray();
}

//-----------------------------------------------------------------------------

//! Sets point cloud to store.
//! \param[in] points the points to store.
void asiData_IVLabelFieldNode::SetPoints(const Handle(HRealArray)& points)
{
  P_RealArray(this, PID_Points)->SetArray(points);
}

//-----------------------------------------------------------------------------

Handle(HStringArray) asiData_IVLabelFieldNode::GetLabels() const
{
  return P_StringArray(this, PID_Labels)->GetArray();
}

//-----------------------------------------------------------------------------

void asiData_IVLabelFieldNode::SetLabels(const Handle(HStringArray)& labels)
{
  P_StringArray(this, PID_Labels)->SetArray(labels);
}

//-----------------------------------------------------------------------------

void asiData_IVLabelFieldNode::SetColor(const int color)
{
  ActParamTool::AsInt( this->Parameter(PID_Color) )->SetValue(color);
}

//-----------------------------------------------------------------------------

int asiData_IVLabelFieldNode::GetColor() const
{
  return ActParamTool::AsInt( this->Parameter(PID_Color) )->GetValue();
}

//-----------------------------------------------------------------------------

void asiData_IVLabelFieldNode::SetSize(const int size)
{
  ActParamTool::AsInt( this->Parameter(PID_Size) )->SetValue(size);
}

//-----------------------------------------------------------------------------

int asiData_IVLabelFieldNode::GetSize() const
{
  return ActParamTool::AsInt( this->Parameter(PID_Size) )->GetValue();
}
