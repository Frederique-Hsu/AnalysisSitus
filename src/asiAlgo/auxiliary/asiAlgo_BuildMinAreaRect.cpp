//-----------------------------------------------------------------------------
// Created on: September 2023
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
#include "asiAlgo_BuildMinAreaRect.h"

// OpenCascade includes
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <TopoDS_Wire.hxx>

// STD include
#include <algorithm>

namespace
{
  gp_XY P2d(const gp_Pnt& P)
  {
    return gp_XY( P.X(), P.Y() );
  }

  gp_Pnt P3d(const gp_XY& P)
  {
    return gp_Pnt( P.X(), P.Y(), 0. );
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildMinAreaRect::Calculate(const std::vector<gp_Pnt>& points,
                                         BoundingBox&               bbox,
                                         const double               alignmentTolerance,
                                         ActAPI_ProgressEntry       progress,
                                         ActAPI_PlotterEntry        plotter)
{
  if ( points.empty() )
  {
    return false;
  }

  // Calculate the convex hull.
  std::vector<gp_XY> hullPoints;
  if ( !convexHull( points, hullPoints ) )
  {
    return false;
  }

  const int nbHPnts = (int) hullPoints.size();

  // Check if no bounding box available
  if ( nbHPnts <= 1 )
  {
    return false;
  }

  Rect minBox;
  double minAngle = 0;

  std::vector<TopoDS_Wire> polyWires, bbRects;
  int bestWireIdx = -1;

  for ( int i = 0; i < nbHPnts; ++i )
  {
    auto nextIndex = i + 1;

    auto current = hullPoints[i];

    auto next = hullPoints[ nextIndex % nbHPnts ];

    Segment segment = Segment( current, next );

    plotter.DRAW_LINK(::P3d(current), ::P3d(next), Color_White, "caliper");

    // Min/Max points.
    double top    = DBL_MAX * -1;
    double bottom = DBL_MAX;
    double left   = DBL_MAX;
    double right  = DBL_MAX * -1;

    // Get angle of segment to x axis.
    double angle = angleToXAxis( segment );

    // Create a new polygonal wire.
    BRepBuilderAPI_MakePolygon mkPolygon, mkRect;

    // Rotate every point and get min and max values for each direction.
    for ( auto& p : hullPoints )
    {
      gp_XY rotatedPoint = rotateToXAxis( p, angle );

      if ( !plotter.Access().IsNull() )
        mkPolygon.Add( ::P3d(rotatedPoint) );

      top    = std::max( top,    rotatedPoint.Y() );
      bottom = std::min( bottom, rotatedPoint.Y() );

      left  = std::min( left,  rotatedPoint.X() );
      right = std::max( right, rotatedPoint.X() );
    }
    //
    if ( !plotter.Access().IsNull() )
    {
      if ( !Precision::IsInfinite(left) )
      {
        TopoDS_Wire polyWire = mkPolygon.Wire();
        //
        polyWires.push_back(polyWire);

        plotter.DRAW_SHAPE(polyWire, Color_Yellow, "polyWire");
      }
      else
      {
        polyWires.push_back( TopoDS_Wire() );
      }
    }

    // Create axis aligned bounding box.
    Rect box = Rect( { left, bottom }, { right, top } );
    //
    if ( !plotter.Access().IsNull() )
    {
      if ( !Precision::IsInfinite(left) )
      {
        mkRect.Add( ::P3d( rotateToXAxis( gp_XY(left,  bottom), -angle) ) );
        mkRect.Add( ::P3d( rotateToXAxis( gp_XY(right, bottom), -angle) ) );
        mkRect.Add( ::P3d( rotateToXAxis( gp_XY(right, top), -angle) ) );
        mkRect.Add( ::P3d( rotateToXAxis( gp_XY(left,  top), -angle) ) );
        mkRect.Close();
        //
        TopoDS_Wire polyRect = mkRect.Wire();
        //
        bbRects.push_back(polyRect);

        t_asciiString polyRectName("polyRect ");
        polyRectName += box.GetArea();

        plotter.DRAW_SHAPE(polyRect, Color_Pink, polyRectName);
      }
      else
      {
        bbRects.push_back( TopoDS_Wire() );
      }
    }

    if ( minBox.IsEmpty() || ( minBox.GetArea() > box.GetArea() ) )
    {
      minBox      = box;
      minAngle    = angle;
      bestWireIdx = i;
    }
  }

  /*if ( bestWireIdx != -1 )
  {
    plotter.DRAW_SHAPE(polyWires [bestWireIdx], Color_Green, "polyWireSol");
    plotter.DRAW_SHAPE(bbRects   [bestWireIdx], Color_Pink,  "bbRectSol");
  }*/

  // Get bounding box dimensions using axis aligned box,
  // larger side is considered as height.
  const auto alignedBoxPoints = minBox.getPoints();
  auto rotatedBoxPoints = alignedBoxPoints;

  gp_XY v1 = rotatedBoxPoints[0] - rotatedBoxPoints[1];
  gp_XY v2 = rotatedBoxPoints[1] - rotatedBoxPoints[2];

  double absX = abs( v1.X() );
  double absY = abs( v2.Y() );

  double width  = std::min( absX, absY );
  double height = std::max( absX, absY );

  // Rotate axis aligned box back and get center.
  double sumX = 0.0;
  double sumY = 0.0;

  // Create a new polygonal wire.
  BRepBuilderAPI_MakePolygon mkBestRect;
  //
  for ( gp_XY& point : rotatedBoxPoints )
  {
    point = rotateToXAxis( point, -minAngle );

    sumX += point.X();
    sumY += point.Y();

    if ( !plotter.Access().IsNull() )
    {
      mkBestRect.Add( ::P3d(point) );
    }
  }
  //
  if ( !plotter.Access().IsNull() )
  {
    mkBestRect.Close();
    TopoDS_Wire bestRect = mkBestRect.Wire();
    //
    plotter.DRAW_SHAPE(bestRect, Color_Red, "bestRect");
  }

  auto center = gp_XY( sumX / 4., sumY / 4. );

  // Get angles.
  gp_XY heightPoint1 = ( absX > absY ) ? rotatedBoxPoints[0]
                                       : rotatedBoxPoints[1];

  gp_XY heightPoint2 = ( absX > absY ) ? rotatedBoxPoints[1]
                                       : rotatedBoxPoints[2];

  double heightAngle = angleToXAxis( Segment( heightPoint1, heightPoint2 ) );

  double widthAngle = ( heightAngle > 0.0 ) ? heightAngle - M_PI_2
                                          : heightAngle + M_PI_2;

  // Get alignment.
  double tolerance = alignmentTolerance * ( M_PI / 180.0 );

  bool isAligned = ( widthAngle <=  tolerance &&
                     widthAngle >= -tolerance );

  bbox = {
    rotatedBoxPoints,
    alignedBoxPoints,
    center,
    width,
    height,
    widthAngle,
    heightAngle,
    isAligned
  };

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildMinAreaRect::convexHull(const std::vector<gp_Pnt>& points,
                                          std::vector<gp_XY>&        hullPoints)
{
  // Break if only one point as input.
  if ( points.size() <= 1 )
  {
    return false;
  }

  // Make points copy to sort them.
  std::vector<gp_XY> sortedPoints( points.size() );

  int Pi = 0;
  for ( const auto& P : points )
  {
    sortedPoints[Pi++] = { P.X(), P.Y() };
  }

  // Sort vector.
  std::sort( sortedPoints.begin(), sortedPoints.end(),
    [&](const gp_XY& p1, const gp_XY& p2) -> bool {
      if ( isDoubleEqual( p1.X(), p2.X() ) )
      {
        if ( isDoubleEqual( p1.Y(), p2.Y() ) )
        {
          return false;
        }

        return ( p1.Y() < p2.Y() );
      }

      return ( p1.X() < p2.X() );
    });

  hullPoints = std::vector<gp_XY>( sortedPoints.size() * 2 );

  auto pointLength = sortedPoints.size();
  auto counter = 0;

  // Iterate for lowerHull.
  for ( int i = 0; i < pointLength; ++i )
  {
    while ( counter >= 2 && cross( hullPoints[ counter - 2 ], hullPoints[ counter - 1 ], sortedPoints[i] ) <= 0 )
    {
      counter--;
    }

    hullPoints[ counter++ ] = sortedPoints[i];
  }

  // Iterate for upperHull.
  for ( int i = (int) pointLength - 2, j = counter + 1; i >= 0; i-- )
  {
    while ( counter >= j && cross( hullPoints[ counter - 2 ], hullPoints[ counter - 1 ], sortedPoints[i] ) <= 0 )
    {
      counter--;
    }

    hullPoints[ counter++ ] = sortedPoints[i];
  }

  // Remove duplicate start points.
  hullPoints.erase( hullPoints.begin() + counter, hullPoints.end() );

  return true;
}

//-----------------------------------------------------------------------------

double asiAlgo_BuildMinAreaRect::cross(const gp_XY& o,
                                       const gp_XY& a,
                                       const gp_XY& b)
{
  return ( a.X() - o.X() ) * ( b.Y() - o.Y() ) - ( a.Y() - o.Y() ) * ( b.X() - o.X() );
}

//-----------------------------------------------------------------------------

bool asiAlgo_BuildMinAreaRect::isDoubleEqual(const double v1,
                                             const double v2)
{
  return std::abs( v1 - v2 ) < DBL_EPSILON;
}

//-----------------------------------------------------------------------------

double asiAlgo_BuildMinAreaRect::angleToXAxis(const Segment& s)
{
  gp_XY delta = s.a - s.b;

  return -atan( delta.Y() / delta.X() );
}

//-----------------------------------------------------------------------------

gp_XY asiAlgo_BuildMinAreaRect::rotateToXAxis(const gp_XY& p,
                                              const double angle)
{
  double newX = p.X() * cos( angle ) - p.Y() * sin( angle );
  double newY = p.X() * sin( angle ) + p.Y() * cos( angle );

  return { newX, newY };
}
