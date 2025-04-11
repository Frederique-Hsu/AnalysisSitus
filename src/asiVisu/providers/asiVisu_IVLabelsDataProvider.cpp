//-----------------------------------------------------------------------------
// Created on: 06 February 2025
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
#include <asiVisu_IVLabelsDataProvider.h>

// asiAlgo includes
#include <asiAlgo_PointCloudUtils.h>

// Active Data includes
#include <ActAPI_IPlotter.h>

//-----------------------------------------------------------------------------

asiVisu_IVLabelsDataProvider::asiVisu_IVLabelsDataProvider(const Handle(asiData_IVLabelFieldNode)& node)
: asiVisu_LabelsDataProvider(node)
{}

//-----------------------------------------------------------------------------

bool asiVisu_IVLabelsDataProvider::HasAnchorPoints() const
{
  return true;
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_BaseCloud<double>)
  asiVisu_IVLabelsDataProvider::GetAnchorPoints() const
{
  return asiAlgo_PointCloudUtils::AsCloudd( Handle(asiData_IVLabelFieldNode)::DownCast(m_source)->GetPoints() );
}

//-----------------------------------------------------------------------------

Handle(HStringArray) asiVisu_IVLabelsDataProvider::GetLabels() const
{
  return Handle(asiData_IVLabelFieldNode)::DownCast(m_source)->GetLabels();
}

//-----------------------------------------------------------------------------

int asiVisu_IVLabelsDataProvider::GetSize() const
{
  return Handle(asiData_IVLabelFieldNode)::DownCast(m_source)->GetSize();
}

//-----------------------------------------------------------------------------

void asiVisu_IVLabelsDataProvider::GetColor(double& r, double& g, double& b) const
{
  const int color = Handle(asiData_IVLabelFieldNode)::DownCast(m_source)->GetColor();

  ActAPI_Color rgb = ActAPI_Color::IntToColor(color);

  r = rgb.Red();
  g = rgb.Green();
  b = rgb.Blue();
}

//-----------------------------------------------------------------------------

Handle(ActAPI_HParameterList)
  asiVisu_IVLabelsDataProvider::translationSources() const
{
  ActParamStream out;

  out << m_source->Parameter(asiData_IVLabelFieldNode::PID_Points)
      << m_source->Parameter(asiData_IVLabelFieldNode::PID_Labels)
      << m_source->Parameter(asiData_IVLabelFieldNode::PID_Size)
      << m_source->Parameter(asiData_IVLabelFieldNode::PID_Color);

  return out;
}
