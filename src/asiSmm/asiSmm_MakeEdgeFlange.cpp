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
#include "asiSmm_MakeEdgeFlange.h"

// asiSmm includes
#include "asiSmm_Utils.h"

// asiAlgo includes
#include <asiAlgo_FeatureAttrAdjacency.h>

// OpenCascade includes
#include <BRepTools.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>

#define MaxPossibleThickness 100.

using namespace asiSmm;

//-----------------------------------------------------------------------------

MakeEdgeFlange::MakeEdgeFlange(const TopoDS_Shape&  baseShape,
                               ActAPI_ProgressEntry progress,
                               ActAPI_PlotterEntry  plotter)
//
: ActAPI_IAlgorithm (progress, plotter),
  m_baseShape       (baseShape)
{
  // Construct AAG for the base shape.
  m_baseG = new asiAlgo_AAG(m_baseShape, true); // `allowSmooth` == true
}

//-----------------------------------------------------------------------------

MakeEdgeFlange::MakeEdgeFlange(const Handle(asiAlgo_AAG)& G,
                               ActAPI_ProgressEntry       progress,
                               ActAPI_PlotterEntry        plotter)
//
: ActAPI_IAlgorithm ( progress, plotter ),
  m_baseShape       ( G->GetMasterShape() ),
  m_baseG           ( G )
{
}

//-----------------------------------------------------------------------------

bool MakeEdgeFlange::Build(const int    eid,
                           const double alphaDeg,
                           const double l)
{
  /* ===========================
   *  Find thickness face `F_t`.
   * =========================== */

  double t     = 0.; // The automatically detected material thickness.
  int    fid_t = this->findThicknessFace(eid, t);
  //
  if ( !fid_t )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot automatically decide on a thickness face.");
    return false;
  }

  m_progress.SendLogMessage(LogInfo(Normal) << "The automatically detected thickness face is %1 "
                                               "with the computed material thickness of %2 mm."
                                            << fid_t << t);

  const TopoDS_Face& F_t = m_baseG->GetFace(fid_t);

  m_plotter.REDRAW_SHAPE( "F_t", F_t, Color_Red, 1. );

  /* ================================
   *  Compute the axis of revolution.
   * ================================ */

  const TopTools_IndexedMapOfShape& allEdges = m_baseG->RequestMapOfEdges();
  //
  const TopoDS_Edge& E = TopoDS::Edge( allEdges.FindKey(eid) );

  // Get a local reference frame at the thickness face.
  asiAlgo_BorderTrihedron btri;
  //
  if ( !asiAlgo_Utils::ComputeBorderTrihedron(F_t, E, btri) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot compute a border trihedron.");
    return false;
  }

  // Compute origin point for the axis.
  gp_XYZ O = btri.V_origin.XYZ() + btri.V_y.Reversed().XYZ()*t;

  // Prepare axis.
  gp_Ax1 ax(O, btri.V_x);

  m_plotter.REDRAW_POINT( "O", O, Color_Red );
  m_plotter.REDRAW_VECTOR_AT( "ax", ax.Location(), ax.Direction().XYZ()*t, Color_Red );

  /* ========================================================
   *  Revolve the thickness face to construct a bend feature.
   * ======================================================== */

  TopoDS_Face F_l;
  //
  TopoDS_Solid bendSolid = Utils::BuildRevolvedBlock(F_t, ax, alphaDeg, F_l);

  m_plotter.REDRAW_SHAPE( "bendSolid", bendSolid, Color_Red, 1. );
  m_plotter.REDRAW_SHAPE( "F_l", F_l, Color_Green, 1. );

  /* ====================
   *  Make a flange wall.
   * ==================== */

  TopoDS_Solid wallSolid;
  //
  {
    Handle(Geom_Surface) surf = BRep_Tool::Surface(F_l);

    double uMin, uMax, vMin, vMax;
    BRepTools::UVBounds(F_l, uMin, uMax, vMin, vMax);

    double uMid = (uMin + uMax)*0.5;
    double vMid = (vMin + vMax)*0.5;

    gp_Pnt P;
    gp_Vec dS_dU, dS_dV;
    surf->D1(uMid, vMid, P, dS_dU, dS_dV);

    gp_Dir norm = dS_dU^dS_dV;

    if ( F_l.Orientation() == TopAbs_REVERSED )
      norm.Reverse();

    gp_Vec offset = norm.XYZ()*l;

    wallSolid = Utils::BuildExtrudedBlock(F_l, offset);
  }

  m_plotter.REDRAW_SHAPE( "wallSolid", wallSolid, Color_Green, 1. );

  // TODO: NYI

  return true;
}

//-----------------------------------------------------------------------------

int MakeEdgeFlange::findThicknessFace(const int eid,
                                      double&   thickness) const
{
  /* ==========================================================
   *  Find an AAG arc that corresponds to the edge in question.
   * ========================================================== */

  const TopTools_IndexedMapOfShape& allEdges = m_baseG->RequestMapOfEdges();
  //
  const TopoDS_Edge& E = TopoDS::Edge( allEdges.FindKey(eid) );

  tl::optional<asiAlgo_AAG::t_arc> arc;
  bool                             isFound = false;

  // Find an arc attribute corresponding to the target edge.
  const asiAlgo_AAG::t_arc_attributes& arcAttrs = m_baseG->GetArcAttributes();
  //
  for ( asiAlgo_AAG::t_arc_attributes::Iterator ait(arcAttrs); ait.More(); ait.Next() )
  {
    const asiAlgo_AAG::t_arc_attr_set& attrSet = ait.Value();

    // We are inspecting the adjacency attribute.
    Handle(asiAlgo_FeatureAttrAdjacency)
      adjAttr = Handle(asiAlgo_FeatureAttrAdjacency)::DownCast(attrSet.AngleAttr);
    //
    if ( adjAttr.IsNull() )
      continue;

    const TColStd_PackedMapOfInteger& eids = adjAttr->GetEdgeIndices();
    //
    if ( eids.Contains(eid) )
    {
      isFound = true;
      arc     = ait.Key();
    }

    if ( isFound )
      break;
  }

  if ( !arc.has_value() )
    return 0; // Cannot find an attribute.

  /* ================================================================
   *  Between the two faces `F1` and `F2` that meet at the specified
   *  edge, we now have to choose the right one.
   * ================================================================ */

  int fid_t = 0;

  // Probe thickness at each face.
  double t[2] = {0., 0.};
  bool   t_ok[2] = { this->probeThickness(arc->F1, E, t[0]),
                     this->probeThickness(arc->F2, E, t[1]) };

  if ( t_ok[0] && !t_ok[1] )
  {
    fid_t     = arc->F1;
    thickness = t[0];
  }
  else if ( !t_ok[0] && t_ok[1] )
  {
    fid_t     = arc->F2;
    thickness = t[1];
  }
  else if ( t_ok[0] && t_ok[1] )
  {
    if ( t[0] < t[1] )
    {
      fid_t     = arc->F1;
      thickness = t[0];
    }
    else
    {
      fid_t     = arc->F2;
      thickness = t[1];
    }
  }
  else
  {
    return 0; // No probe is positive.
  }

  return fid_t;
}

//-----------------------------------------------------------------------------

bool MakeEdgeFlange::probeThickness(const int          fid,
                                    const TopoDS_Edge& E,
                                    double&            t) const
{
  const TopoDS_Face& face = m_baseG->GetFace(fid);
  //
  if ( !asiAlgo_Utils::IsPlanar(face, false) )
    return false; // Limited with planar cases as of now.

  // Find parameters of the edge's pcurve.
  double E_f, E_l;
  Handle(Geom2d_Curve)
    E_c2d = BRep_Tool::CurveOnSurface(E, face, E_f, E_l);

  // We are concerned with straight edges.
  gp_Lin2d E_pcuLin;
  //
  if ( !asiAlgo_Utils::IsStraightPCurve(E_c2d, E_pcuLin, true) )
    return false;

  // Probe a midpoint.
  const double E_m = (E_f + E_l)*0.5;
  //
  gp_Pnt2d E_Pm;
  gp_Vec2d E_Vm;
  E_c2d->D1(E_m, E_Pm, E_Vm);

  // Normal vector is perpendicular to `Vm`, so we can take the coordinates of `Vm` swapped
  // and one component inverted. This could be another "tip of the week".
  gp_Dir2d E_Nm( E_Vm.Y(), -E_Vm.X() );

  // Construct a line to be used for computing intersections.
  gp_Lin2d ray(E_Pm, E_Nm);
  //
  Handle(Geom2d_TrimmedCurve )
    rayCurve = new Geom2d_TrimmedCurve(new Geom2d_Line(E_Pm, E_Nm),
                                      -MaxPossibleThickness,
                                       MaxPossibleThickness);
  //
  /*if ( !m_plotter.Access().IsNull() )
  {
    t_asciiString linkName("rayCurve_");
    linkName += fid;

    m_plotter.REDRAW_CURVE2D( linkName,
                              new Geom2d_TrimmedCurve(new Geom2d_Line(E_Pm, E_Nm), 0., MaxPossibleThickness),
                              Color_White );
  }*/

  // Find intersections.
  std::vector<gp_XY> ipts; // Intersection points.
  //
  for ( TopExp_Explorer eexp(face, TopAbs_EDGE); eexp.More(); eexp.Next() )
  {
    const TopoDS_Edge& edge = TopoDS::Edge( eexp.Current() );
    //
    if ( edge.IsPartner(E) )
      continue; // Skip the seed edge from iteration.

    double f, l;
    Handle(Geom2d_Curve)
      c2d = BRep_Tool::CurveOnSurface(edge, face, f, l);

    Geom2dAPI_InterCurveCurve intCC(rayCurve, c2d);

    for ( int isol = 1; isol <= intCC.NbPoints(); ++isol )
    {
      const gp_XY ipt = intCC.Point(isol).Coord();
      //
      ipts.push_back(ipt);

      //if ( !m_plotter.Access().IsNull() )
      //{
      //  t_asciiString isolName("isol_");
      //  isolName += fid;
      //  //
      //  m_plotter.DRAW_POINT(ipt, Color_Red, isolName);
      //}
    }
  }

  if ( ipts.empty() )
    return false;

  // Choose the minimal distance as a probed thickness to return.
  double minThickness = DBL_MAX;
  //
  for ( const auto& ipt : ipts )
  {
    const double d = ( ipt - E_Pm.XY() ).Modulus();
    //
    if ( d < minThickness )
    {
      minThickness = d;
    }
  }

  // Set the thickness to return.
  t = minThickness;

  return true;
}
