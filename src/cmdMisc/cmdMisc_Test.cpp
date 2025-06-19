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

#include <Geo.h>
#include <GCS.h>

  using namespace GCS;

//-----------------------------------------------------------------------------

int MISC_Test(const Handle(asiTcl_Interp)& interp,
              int                          argc,
              const char**                 argv)
{
  Handle(Geom_Plane) XOY = new Geom_Plane( gp::XOY() );

  int iter = 1;
  interp->GetKeyValue(argc, argv, "iter", iter);

  const double angleStep = 360. / (double)(iter);

  for ( int i = 1; i <= iter; ++i )
  {
    const double angleRad = i*angleStep*M_PI / 180.;

    // Prepare geometric objects.
    Point p1;
    p1.x = new double(0);
    p1.y = new double(0);
    Point p2;
    p2.x = new double( Cos(angleRad) );
    p2.y = new double( Sin(angleRad) );

    // Draw calls.
    interp->GetPlotter().REDRAW_POINT("p1",
                                       XOY->Value(*p1.x, *p1.y),
                                       Color_Red);
    //
    interp->GetPlotter().REDRAW_POINT("p2",
                                       XOY->Value(*p2.x, *p2.y),
                                       Color_Red);

    // Prepare constraints.
    double* distance = new double(5);
    double* posX     = new double(0);
    double* posY     = new double(0);

    // Construct and populate the system of constraints.
    System* sys = new System();
    //
    sys->addConstraintCoordinateX(p1, posX);
    sys->addConstraintCoordinateX(p1, posY);
    sys->addConstraintDifference(p1.x, p2.x, distance);
    //
    std::vector<double*> p_parameters;
    p_parameters.push_back(p1.x);
    p_parameters.push_back(p1.y);
    p_parameters.push_back(p2.x);
    p_parameters.push_back(p2.y);

    // Solve and apply.
    sys->solve(p_parameters);
    sys->applySolution();

    // Draw calls.
    interp->GetPlotter().REDRAW_POINT("p1_sol",
                                       XOY->Value(*p1.x, *p1.y),
                                       Color_Green);
    //
    interp->GetPlotter().REDRAW_POINT("p2_sol",
                                       XOY->Value(*p2.x, *p2.y),
                                       Color_Green);
    //
    interp->GetPlotter().REDRAW_LINK("p1_p2",
                                      XOY->Value(*p1.x, *p1.y),
                                      XOY->Value(*p2.x, *p2.y),
                                      Color_Green);

    t_asciiString itername = "Iteration ";
    itername += i;
    //
    interp->GetPlotter().REDRAW_TEXT("caption", itername);

    delete sys;
    delete p1.x;
    delete p1.y;
    delete p2.x;
    delete p2.y;
    delete distance;
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
