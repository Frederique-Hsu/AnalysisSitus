//-----------------------------------------------------------------------------
// Created on: 14 May 2022 (*)
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Sergey Slyadnev
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

// cmdMisc includes
#include <cmdMisc.h>

//-----------------------------------------------------------------------------

#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <asiAlgo_DivideByContinuity.h>

#include <asiAlgo_BaseCloud.h>
#include <asiAlgo_BuildConvexHull.h>
#include <asiAlgo_QuickHull2d.h>

#if defined USE_MOBIUS
  #include <mobius/poly_Mesh.h>
  #include <mobius/core_Polygon.h>
  #include <mobius/cascade.h>
  #include <mobius/nest_Part.h>
  using namespace mobius;
#endif

#include <BRepTools.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>
#include <GeomConvert.hxx>
#include <ShapeFix_Wire.hxx>
#include <BOPAlgo_Builder.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>

#include <IGESControl_Reader.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Builder.hxx>
#include <ShapeAnalysis_Curve.hxx>

#include <GC_MakeArcOfCircle.hxx>
#include <asiAlgo_Timer.h>

//2D implementation of the Ramer-Douglas-Peucker algorithm
//By Tim Sheerman-Chase, 2016
//Released under CC0
//https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm

#include <iostream>
#include <cmath>
#include <utility>
#include <vector>
#include <stdexcept>

//-----------------------------------------------------------------------------

int MISC_Test(const Handle(asiTcl_Interp)& interp,
              int                          /*argc*/,
              const char**                 argv)
{
  t_ptr<nest_Part> part = nest_Part::Import(argv[1]);
  //
  if ( part.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot read a part from '%1'."
                                                        << argv[1]);
    return TCL_ERROR;
  }

  TIMER_NEW
  TIMER_GO

  int    numPolesInit = 0;
  int    numPolesOut  = 0;
  double eps          = 1.0;

  std::vector< t_ptr<t_polygon> > loops, loopsOut;
  //
  part->GetAllLoops(loops);
  //
  for ( const auto& loop : loops )
  {
    // Construct the simplified polygon.
    t_ptr<t_polygon> loopOut = loop->Simplify(eps);
    //
    if ( !loopOut.IsNull() )
    {
      loopsOut.push_back(loopOut);
      //
      numPolesOut += (int) ( loopOut->GetPoles().size() );
    }

    // Count poles.
    numPolesInit += (int) ( loop->GetPoles().size() );
  }

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "Ramer-Douglas-Peucker")

  interp->GetProgress().SendLogMessage( LogNotice(Normal) << "Initial num. of poles: %1."
                                                          << numPolesInit );

  interp->GetProgress().SendLogMessage( LogNotice(Normal) << "Optimized num. of poles: %1."
                                                          << numPolesOut );

  for ( size_t k = 0; k < loops.size(); ++k )
  {
    interp->GetPlotter().DRAW_SHAPE(cascade::GetOpenCascadeFace(loops[k]), Color_White, 1., true, "pgon");
  }
  //
  for ( size_t k = 0; k < loopsOut.size(); ++k )
  {
    interp->GetPlotter().DRAW_SHAPE(cascade::GetOpenCascadeFace(loopsOut[k]), Color_Red, 1., true, "pgonOut");
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdMisc::Commands_Test(const Handle(asiTcl_Interp)&      interp,
                            const Handle(Standard_Transient)& cmdMisc_NotUsed(data))
{
  static const char* group = "cmdMisc";

  interp->AddCommand("test", "Test anything.", __FILE__, group, MISC_Test);
}
