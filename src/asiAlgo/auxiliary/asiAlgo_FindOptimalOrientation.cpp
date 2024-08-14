//-----------------------------------------------------------------------------
// Created on: 12 October 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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
#include "asiAlgo_FindOptimalOrientation.h"

// asiAlgo includes
#include <asiAlgo_BuildHLR.h>
#include <asiAlgo_BuildMinAreaRect.h>
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_MeshInfo.h>
#include <asiAlgo_OrientCnc.h>

// OCCT includes
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepLib.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

asiAlgo_FindOptimalOrientation::asiAlgo_FindOptimalOrientation(const TopoDS_Shape&  shape,
                                                               ActAPI_ProgressEntry progress,
                                                               ActAPI_PlotterEntry  plotter)
: ActAPI_IAlgorithm  (progress, plotter),
  m_shape            (shape),
  m_bIsSetPresetAxis (false)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_FindOptimalOrientation::Perform()
{
  /* =========
   *  Stage 1.
   * ========= */

  // Orient a part w.r.t.global coordinate axes using basic symmetries of its CAD faces.
  // As a result, a part gets oriented towards OZ axis.

  tl::optional<gp_Ax3> ax;
  gp_Trsf              zTrsf;
  bool                 isOrientedOk = false;
  //
  if ( !m_bIsSetPresetAxis )
  {
    asiAlgo_OrientCnc orient( new asiAlgo_AAG(m_shape, true),
                              m_progress,
                              m_plotter );
    //
    isOrientedOk = orient.Perform();
    //
    if ( !isOrientedOk )
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Reorientation failed.");
      return false;
    }

    ax    = orient.GetAxes();
    zTrsf = orient.GetTrsf();

    if ( !m_plotter.Access().IsNull() )
    {
      m_plotter.REDRAW_AXES( "oriAxes",
                              ax->Location(),
                              ax->XDirection(),
                              ax->YDirection(),
                              ax->Direction(),
                              orient.GetExtents() );
    }
  }
  else
  {
    ax           = gp_Ax3(m_presetAxis.Location(), m_presetAxis.Direction());
    isOrientedOk = true;

    gp_Dir OZ = gp::DZ();
    gp_Ax3 XOY(gp::Origin(), OZ);
    //
    zTrsf = asiAlgo_Utils::GetAlignmentTrsf(XOY, ax.value());
  }

  TopoDS_Shape orientedShape;

  if ( isOrientedOk )
  {
    /*
       We transform here without deep copying to maintain any existing triangulation
       on a part. Re-triangulating a part for the sake of performing DHLR would be a
       computationally expensive procedure, thus we should avoid it. In the end,
       however, we will deep copy the part and reapply all associated transformations,
       resulting in the part having no location in its member fields.
     */

    orientedShape = BRepBuilderAPI_Transform(m_shape, zTrsf, false);

    m_trsf *= zTrsf;
  }
  else
  {
    orientedShape = m_shape;
  }

  /* =========
   *  Stage 2.
   * ========= */

  // Make sure the mesh exists before running DHLR.
  {
    asiAlgo_MeshInfo meshInfo = asiAlgo_MeshInfo::Extract(orientedShape);

    if ( !meshInfo.nFacets )
    {
      const double linDefl = 10*asiAlgo_MeshGen::AutoSelectLinearDeflection  (orientedShape);
      const double angDefl = 10*asiAlgo_MeshGen::AutoSelectAngularDeflection (orientedShape);

      if ( !asiAlgo_MeshGen::DoNative(orientedShape,
                                      linDefl,
                                      angDefl,
                                      meshInfo) )
      {
        return false;
      }
    }
  }

  // Project a part reoriented at the 1-st stage onto XOY plane with discrete HLR algorithm.
  asiAlgo_BuildHLR buildHLR(orientedShape);
  //
  if ( !buildHLR.Perform(gp::DZ(), asiAlgo_BuildHLR::Mode::Mode_Discrete) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot build DHLR presentation.");
    return false;
  }
  //
  TopoDS_Shape hlrResult = buildHLR.GetResult();

  if ( !m_plotter.Access().IsNull() )
  {
    m_plotter.REDRAW_SHAPE("hlrResult", hlrResult, Color_Violet);
  }

  /* =========
   *  Stage 3.
   * ========= */

  // Build convex hull of the discrete projection obtained at the previous stage.
  // Rotate convex hull in 2D using "rotating calipers" method.

  const double linearTol  = 0.001;
  const double angularTol = 1.0 * M_PI / 180.0;

  std::vector<gp_Pnt> points;
  //
  for ( TopExp_Explorer exp(hlrResult, TopAbs_EDGE); exp.More(); exp.Next() )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( exp.Value() );

    BRepAdaptor_Curve bac(edge);
    //
    if ( bac.Curve().Curve().IsNull() )
    {
      BRepLib::BuildCurves3d(edge, Precision::Confusion());
      bac.Initialize(edge);
    }
    //
    const double fParam = bac.FirstParameter();
    const double lParam = bac.LastParameter();

    gp_Trsf ET = edge.Location().Transformation();

    // Discretize.
    GeomAdaptor_Curve gac( Handle(Geom_Curve)::DownCast( bac.Curve().Curve()->Transformed(ET) ) );
    GCPnts_TangentialDeflection pntGen(gac, fParam, lParam, angularTol, linearTol);
    //
    const int nbPnts = pntGen.NbPoints();
    //
    for ( int index = 1; index <= nbPnts; ++index)
    {
      gp_Pnt pnt = gac.Value( pntGen.Parameter(index) );
      bool isFound = false;

      for ( int index1 = 0; index1 < points.size(); ++index1 )
      {
        if ( pnt.Distance(points[index1]) < Precision::Confusion() )
        {
          isFound = true;
          break;
        }
      }
      if ( !isFound )
      {
        points.push_back(pnt);
      }
    }
  }

  // Compute min rectangle.
  asiAlgo_BuildMinAreaRect::BoundingBox bbox;
  //
  if ( !asiAlgo_BuildMinAreaRect::Calculate(points, bbox, Precision::Angular(), m_progress, m_plotter) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot construct minimum area rectangle.");
    return false;
  }

  /* =========
   *  Stage 4.
   * ========= */

  // Apply the obtained angle to the 3D part.
  {
    // Apply self-transformation.
    {
      auto transform = BRepBuilderAPI_Transform( orientedShape.Location().Transformation() );
      transform.Perform( orientedShape.Located(TopLoc_Location() ), true);
      orientedShape = transform.Shape();
    }

    // Apply last transformation.
    gp_Trsf R;
    //
    const double angle = bbox.widthAngle;
    R.SetRotation(gp::OZ(), angle);
    //
    m_result = BRepBuilderAPI_Transform(orientedShape, R, true);
    m_trsf.PreMultiply(R);
  }

  return true;
}
