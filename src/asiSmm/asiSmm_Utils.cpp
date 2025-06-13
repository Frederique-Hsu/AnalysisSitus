//-----------------------------------------------------------------------------
// Created on: 05 June 2025
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

// Own include
#include "asiSmm_Utils.h"

// OpenCascade includes
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>

using namespace asiSmm;

//-----------------------------------------------------------------------------

TopoDS_Solid Utils::BuildBaseBlock(const double dx,
                                   const double dy,
                                   const double dz)
{
  gp_XYZ O = gp::Origin().XYZ();
  gp_XYZ D(dx, dy, dz);

  TopoDS_Shape
    box = BRepPrimAPI_MakeBox(O, O + D);

  return TopoDS::Solid(box);
}

//-----------------------------------------------------------------------------

TopoDS_Solid
  Utils::BuildRevolvedBlock(const TopoDS_Face& profile,
                            const gp_Ax1&      axis,
                            const double       angleDeg,
                            TopoDS_Face&       lastFace)
{
  if ( angleDeg < Precision::Angular()*180./M_PI )
    return TopoDS_Solid();

  BRepPrimAPI_MakeRevol mkRevol(profile, axis, angleDeg*M_PI/180., true);

  lastFace = TopoDS::Face( mkRevol.LastShape() );
  TopoDS_Solid S = TopoDS::Solid( mkRevol.Shape() );

  // Correct the orientation of the last face.
  for ( TopExp_Explorer exp(S, TopAbs_FACE); exp.More(); exp.Next() )
  {
    const TopoDS_Shape& currentFace = exp.Current();
    //
    if ( currentFace.IsPartner(lastFace) )
    {
      lastFace.Orientation( currentFace.Orientation() );
      break;
    }
  }

  return S;
}

//-----------------------------------------------------------------------------

TopoDS_Solid
  Utils::BuildExtrudedBlock(const TopoDS_Face& base,
                            const gp_Vec&      V)
{
  return TopoDS::Solid( BRepPrimAPI_MakePrism(base, V) );
}
