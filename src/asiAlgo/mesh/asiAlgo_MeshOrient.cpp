//-----------------------------------------------------------------------------
// Created on: 09 December 2024
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
#include "asiAlgo_MeshOrient.h"

// asiAlgo includes
#include "asiAlgo_MeshOBB.h"
#include "asiAlgo_Utils.h"

//-----------------------------------------------------------------------------

asiAlgo_MeshOrient::asiAlgo_MeshOrient(const Handle(Poly_Triangulation)& mesh,
                                       ActAPI_ProgressEntry              progress,
                                       ActAPI_PlotterEntry               plotter)
: ActAPI_IAlgorithm (progress, plotter),
  m_mesh            (mesh),
  m_fExtents        (0.)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_MeshOrient::Perform()
{
  if ( m_mesh.IsNull() )
    return false; // Contract check.

  double xMin, yMin, zMin, xMax, yMax, zMax;
  asiAlgo_Utils::Bounds(m_mesh, xMin, yMin, zMin, xMax, yMax, zMax);

  // Calculate geometric mean.
  m_fExtents = Min( Min(xMax - xMin, yMax - yMin), zMax - zMin );

  asiAlgo_MeshOBB obbAlgo(m_mesh, m_progress, m_plotter);
  //
  if ( !obbAlgo.Perform() )
    return false;

  m_T = obbAlgo.GetResultTrsf().Inverted();

  return true;
}
