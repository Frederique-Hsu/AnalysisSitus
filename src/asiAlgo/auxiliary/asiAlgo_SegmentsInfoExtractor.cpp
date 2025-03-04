//-----------------------------------------------------------------------------
// Created on: 24 June 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Kiselev
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
#include <asiAlgo_SegmentsInfoExtractor.h>

// asiAlgo includes
#include <asiAlgo_AAG.h>
#include <asiAlgo_CheckVertexVexity.h>
#include <asiAlgo_FeatureAngleType.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BOPAlgo_Tools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <Geom_Curve.hxx>
#include <GProp_GProps.hxx>
#include <ShapeExtend_WireData.hxx>
#include <TopExp_Explorer.hxx>

//-----------------------------------------------------------------------------

asiAlgo_SegmentsInfoExtractor::asiAlgo_SegmentsInfoExtractor(ActAPI_ProgressEntry progress,
                                                             ActAPI_PlotterEntry  plotter)
  : ActAPI_IAlgorithm ( progress, plotter )
{}

//-----------------------------------------------------------------------------

bool asiAlgo_SegmentsInfoExtractor::Perform(const TopoDS_Wire&       wire,
                                            asiAlgo_SegmentsInfoVec& segmentsInfoVec)
{
  // Build a temporary face for the input wire.
  TopoDS_Shape face;

  if ( !BOPAlgo_Tools::WiresToFaces( wire, face, Precision::Confusion() ) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to build the face for the passed wire to extract it's segments information.");

    return false;
  }

  TopTools_IndexedMapOfShape faces;
  TopExp::MapShapes( face, TopAbs_FACE, faces );

  if ( faces.Extent() != 1 )
  {
    return false;
  }

  return Perform( TopoDS::Face( faces.FindKey(1) ), segmentsInfoVec );
}

//-----------------------------------------------------------------------------

bool asiAlgo_SegmentsInfoExtractor::Perform(const TopoDS_Face&       face,
                                            asiAlgo_SegmentsInfoVec& segmentsInfoVec)
{
  // Prepare AAG.
  Handle(asiAlgo_AAG) G = new asiAlgo_AAG( face, false );

  // Extract vertices vexity.
  asiAlgo_CheckVertexVexity::t_vexityMap vexity;

  asiAlgo_CheckVertexVexity algo( G, m_progress, nullptr );

  algo.CheckContours( 1, vexity );

  // Transfer results to segments information data structures.
  for ( TopExp_Explorer wexp( face, TopAbs_WIRE ); wexp.More(); wexp.Next() )
  {
    const TopoDS_Wire&                 W  = TopoDS::Wire( wexp.Current() );
    const Handle(ShapeExtend_WireData) WD = new ShapeExtend_WireData(W);
    const int                          ne = WD->NbEdges();

    // Edge by edge.
    for ( int eid = 1; eid <= ne; ++eid )
    {
      // Get consecutive edges.
      const int nextSegmentId = eid == ne ? 1 : eid + 1;

      const TopoDS_Edge& E1 = WD->Edge(eid);
      const TopoDS_Edge& E2 = WD->Edge(nextSegmentId);

      bool isSingleSegment = WD->NbEdges() == 1;

      // Get curve.
      double U1 = 0., U2 = 0.;
      Handle(Geom_Curve) curve = BRep_Tool::Curve( E1, U1, U2 );

      if ( curve.IsNull() )
        continue;

      // Get edge length.
      GProp_GProps nvsProps;
      BRepGProp::LinearProperties( E1, nvsProps );

      const double length = nvsProps.Mass();

      asiAlgo_SegmentsInfo info( eid, asiAlgo_Utils::CurveName( curve ), length < Precision::Confusion() ? 0. : length );

      // Get middle point.
      curve->D0( ( U1 + U2 ) * 0.5, info.midPnt );

      // Handle circular curves.
      {
        gp_Circ circ;

        if ( asiAlgo_Utils::IsCircular( curve, circ ) )
        {
          info.radius = circ.Radius();
          info.angle  = ( U2 - U1 ) / M_PI * 180.0;
        }
      }

      // Get angle to the next segment if any.
      if ( !isSingleSegment )
      {
        info.nextSegment = nextSegmentId;

        TopoDS_Vertex commonVertex = asiAlgo_Utils::GetCommonVertex( E1, E2 );

        if ( vexity.IsBound( commonVertex ) )
        {
          info.turningAngleToNextSegment    = ( vexity.Find( commonVertex ).angRad / M_PI ) * 180.0;
          info.connectionPointToNextSegment = BRep_Tool::Pnt( commonVertex );
        }
        else
        {
          m_progress.SendLogMessage( LogWarn(Normal) << "Could not find a common vertex for the pair of connected segments '%1' and '%2'."
                                                     << eid << nextSegmentId );
        }
      }

      segmentsInfoVec.push_back( info );
    }
  }

  return true;
}
