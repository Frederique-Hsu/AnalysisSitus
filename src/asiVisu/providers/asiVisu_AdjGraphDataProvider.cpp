//-----------------------------------------------------------------------------
// Created on: 27 April 2024
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

// Own include
#include <asiVisu_AdjGraphDataProvider.h>

// asiData includes
#include <asiData_RootNode.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// OpenCascade includes
#include <gp_Quaternion.hxx>

//-----------------------------------------------------------------------------

asiVisu_AdjGraphDataProvider::asiVisu_AdjGraphDataProvider()
: asiVisu_DataProvider()
{}

//-----------------------------------------------------------------------------

asiVisu_AdjGraphDataProvider::asiVisu_AdjGraphDataProvider(const Handle(asiData_PartNode)& partNode)
: asiVisu_DataProvider(), m_node(partNode)
{}

//-----------------------------------------------------------------------------

ActAPI_DataObjectId asiVisu_AdjGraphDataProvider::GetNodeID() const
{
  return m_node->GetId();
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_AAG) asiVisu_AdjGraphDataProvider::GetAAG() const
{
  return m_node->GetAAG();
}

//-----------------------------------------------------------------------------

bool asiVisu_AdjGraphDataProvider::IsRenderAAG() const
{
  return m_node->GetRenderAAG();
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HParameterList) asiVisu_AdjGraphDataProvider::translationSources() const
{
  ActParamStream params;

  Handle(asiData_RootNode) rootNode =
    Handle(asiData_RootNode)::DownCast(m_node->GetParentNode());

  params << m_node->Parameter(asiData_PartNode::PID_Geometry)
         << m_node->Parameter(asiData_PartNode::PID_AAG)
         << m_node->Parameter(asiData_PartNode::PID_RenderAAG)
         ;

  return params.List;
}
