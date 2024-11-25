//-----------------------------------------------------------------------------
// Created on: 11 September 2024
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
#include <asiAlgo_CollideUtils.h>

// OCCT includes.
#include <Bnd_Box.hxx>
#include <BVH_Box.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <IntAna_QuadQuadGeo.hxx>

using namespace asiAlgo::collide;

//-----------------------------------------------------------------------------

bool Utils::IsOutBBox(const BVH_Vec3d& boxMin1,
                      const BVH_Vec3d& boxMax1,
                      const BVH_Vec3d& boxMin2,
                      const BVH_Vec3d& boxMax2,
                      const double     prec)
{
  BVH_Box<double, 3> box1(boxMin1 - BVH_Vec3d(prec), boxMax1 + BVH_Vec3d(prec));
  BVH_Box<double, 3> box2(boxMin2 - BVH_Vec3d(prec), boxMax2 + BVH_Vec3d(prec));

  if ( box1.IsOut(box2) )
    return true;

  return false;
}

//-----------------------------------------------------------------------------

bool Utils::IsOutBBox(const BVH_Vec3d& boxMin,
                      const BVH_Vec3d& boxMax,
                      const Bnd_Box&   box,
                      const double     prec)
{
  Bnd_Box localBox;
  localBox.Add( gp_Pnt(boxMin[0], boxMin[1], boxMin[2]) );
  localBox.Add( gp_Pnt(boxMax[0], boxMax[1], boxMax[2]) );
  localBox.Enlarge(prec);

  if ( !localBox.IsOut(box) )
    return false; // Boxes intersect.

  return true;
}

//-----------------------------------------------------------------------------

double Utils::SqDistPntTri(const gp_Pnt& p,
                           const gp_Pnt& p1,
                           const gp_Pnt& p2,
                           const gp_Pnt& p3,
                           gp_Pnt&       sol)
{
  const gp_Vec p1p2 (p1, p2);
  const gp_Vec p1p3 (p1, p3);
  const gp_Vec p1p  (p1, p);

  const double p1p2dotp1p = p1p2.Dot(p1p);
  const double p1p3dotp1p = p1p3.Dot(p1p);

  if (p1p2dotp1p <= 0.0 && p1p3dotp1p <= 0.0)
  {
    sol = p1;
    return sol.SquareDistance(p);
  }

  const gp_Vec p2p3(p2, p3);
  const gp_Vec p2p(p2, p);

  const double p2p1dotp2p = -p1p2.Dot(p2p);
  const double p2p3dotp2p = p2p3.Dot(p2p);

  if (p2p1dotp2p <= 0.0 && p2p3dotp2p <= 0.0)
  {
    sol = p2;
    return sol.SquareDistance(p);
  }

  const gp_Vec p3p(p3, p);

  const double p3p2dotp3p = -p2p3.Dot(p3p);
  const double p3p1dotp3p = -p1p3.Dot(p3p);

  if (p3p1dotp3p <= 0.0 && p3p2dotp3p <= 0.0)
  {
    sol = p3;
    return sol.SquareDistance(p);
  }

  const double p1p3dotp2p = p1p3.Dot(p2p);
  const double v3 = p1p2dotp1p * p1p3dotp2p + p2p1dotp2p * p1p3dotp1p;
  if (v3 <= 0.0 && p1p2dotp1p >= 0.0 && p2p1dotp2p >= 0.0)
  {
    sol = p.XYZ() - (p1p - p1p2 * (p1p2dotp1p / (p1p2dotp1p + p2p1dotp2p))).XYZ();
    return sol.SquareDistance(p);
  }

  const double p1p2dotp3p = p1p2.Dot(p3p);
  const double v1 = p2p1dotp2p * p3p1dotp3p - p1p2dotp3p * p1p3dotp2p;
  if (v1 <= 0.0 && p2p3dotp2p >= 0.0 && p3p2dotp3p >= 0.0)
  {
    sol = p.XYZ() - (p2p - p2p3 * (p2p3dotp2p / (p2p3dotp2p + p3p2dotp3p))).XYZ();
    return sol.SquareDistance(p);
  }

  const double v2 = p1p2dotp3p * p1p3dotp1p + p1p2dotp1p * p3p1dotp3p;
  if (v2 <= 0.0 && p1p3dotp1p >= 0.0 && p3p1dotp3p >= 0.0)
  {
    sol = p.XYZ() - (p1p - p1p3 * (p1p3dotp1p / (p1p3dotp1p + p3p1dotp3p))).XYZ();
    return sol.SquareDistance(p);
  }

  const double norm = v1 + v2 + v3;
  sol = (p1.XYZ() * v1 + p2.XYZ() * v2 + p3.XYZ() * v3) / norm;
  return sol.SquareDistance(p);
}

//-----------------------------------------------------------------------------

double Utils::SqDistPntTri(const gp_Pnt& pnt,
                           const gp_Pnt& p1,
                           const gp_Pnt& p2,
                           const gp_Pnt& p3)
{
  gp_Pnt P;
  return SqDistPntTri(pnt, p1, p2, p3, P);
}

//-----------------------------------------------------------------------------

double Utils::SqDistLineLine(const gp_Lin& l1,
                             const gp_Lin& l2,
                             double&       par1,
                             double&       par2,
                             const double  prec)
{
  const double angle = l1.Direction().Angle( l2.Direction() );
  //
  if ( Abs(angle) < prec || Abs(M_PI - angle) < prec )
  {
    par1 = 0.0;
    par2 = 0.0;
    return Precision::Infinite();
  }

  const gp_Pnt& p1 = l1.Location();
  const gp_Pnt& p2 = p1.XYZ() + l1.Direction().XYZ();
  const gp_Pnt& p3 = l2.Location();
  const gp_Pnt& p4 = p3.XYZ() + l2.Direction().XYZ();

  // ( p1 - p3 + par1 * d1 - par2 * d2 ) dot (d1) = 0
  // ( p1 - p3 + par1 * d1 - par2 * d2 ) dot (d2) = 0
  //
  //  d_1321 + par1 * d_2121 - par2 * d_4321 = 0
  //  d_1343 + par1 * d_4321 - par2 * d_4343 = 0
  //
  // d_mnop = (x_m - x_n)(x_o - x_p) + (y_m - y_n)(y_o - y_p) + (z_m - z_n)(z_o - z_p)

  gp_Vec v13(p3, p1);
  gp_Vec v21(p1, p2);
  gp_Vec v43(p3, p4);
  const double d_1321 = v13.Dot(v21);
  const double d_2121 = v21.Dot(v21);
  const double d_4321 = v43.Dot(v21);
  const double d_1343 = v13.Dot(v43);
  const double d_4343 = v43.Dot(v43);

  const double det = -(d_2121 * d_4343 - d_4321 * d_4321);
  par1 = d_1321 * d_4343 - d_1343 * d_4321;
  par1 /= det;

  par2 = -d_2121 * d_1343 + d_4321 * d_1321;
  par2 /= det;

  const gp_Dir& d1 = l1.Direction();
  const gp_Dir& d2 = l2.Direction();
  const gp_Pnt pSol1 = p1.XYZ() + par1 * d1.XYZ();
  const gp_Pnt pSol2 = p3.XYZ() + par2 * d2.XYZ();

  return pSol1.SquareDistance(pSol2);
}

//-----------------------------------------------------------------------------

void Utils::IntersectLineTri(const gp_Lin& line,
                             const gp_Pnt  nodes[3],
                             double        tol,
                             double        interParams[2])
{
  interParams[0] =  Precision::Infinite();
  interParams[1] = -Precision::Infinite();

  for ( int i = 0; i < 3; ++i )
  {
    const gp_Pnt& start = nodes[i];
    const gp_Pnt& end   = nodes[(i + 1) % 3];

    gp_Vec vec(start, end);
    double triEdgeLen = start.Distance(end);
    
    gp_Lin triEdge(start, vec);

    double p1, p2;
    const double sqDist = SqDistLineLine(line, triEdge, p1, p2, 0.01);
    //
    if ( sqDist > tol * tol )
      continue;

    if ( p2 > -tol && p2 < triEdgeLen + tol )
    {
      interParams[0] = Min(p1, interParams[0]);
      interParams[1] = Max(p1, interParams[1]);
    }
  }
}

//-----------------------------------------------------------------------------

bool Utils::IntersectTriTri(const gp_Pnt  nodes1[3],
                            const gp_Dir& norm1,
                            const gp_Pnt  nodes2[3],
                            const gp_Dir& norm2,
                            const double  prec)
{
  const double l1 = std::min(std::min(nodes1[0].SquareDistance(nodes1[1]),
                                      nodes1[0].SquareDistance(nodes1[2])),
                                      nodes1[1].SquareDistance(nodes1[2]));
  const double l2 = std::min(std::min(nodes2[0].SquareDistance(nodes2[1]),
                                      nodes2[0].SquareDistance(nodes2[2])),
                                      nodes2[1].SquareDistance(nodes2[2]));
  const double tol = std::max(Precision::Confusion() * std::min(l1, l2), 1.0e-13);

  // Get for coincident nodes.
  int nbEqual = 0;
  int equalNodes[3] = { -1, -1, -1 };
  //
  for ( int i = 0; i < 3; ++i )
  {
    for ( int j = 0; j < 3; ++j )
    {
      if ( nodes1[i].SquareDistance(nodes2[j]) < tol * tol )
      {
        equalNodes[i] = j;
        nbEqual += 1;
      }
    }
  }
  // Triangles are fully coincident -> report intersection.
  if ( nbEqual == 3 )
    return true;

  // Build planes for triangles.
  gp_Pln plane1(nodes1[0], norm1);
  gp_Pln plane2(nodes2[0], norm2);

  // Check intersection.
  double dot = norm1.Dot(norm2);
  if ( Abs(dot) < 1.0 - tol )
  {
    if ( nbEqual == 2 )
      return false; // Triangles are intersecting along shared edge.

    // Get intersection line.
    IntAna_QuadQuadGeo inter(plane1, plane2, Precision::Angular(), tol);
    //
    if ( inter.IsDone() )
    {
      gp_Lin interLine = inter.Line(1);

      double intPar1[2], intPar2[2];
      //
      IntersectLineTri(interLine, nodes1, tol, intPar1);
      IntersectLineTri(interLine, nodes2, tol, intPar2);

      if ( intPar1[1] > intPar1[0] + prec && // Parameters range is bigger than 1.0e-7.
           intPar2[1] > intPar2[0] + prec && // Parameters range is bigger than 1.0e-7.
           intPar1[1] > intPar2[0] + prec && // There is inner point of intersection.
           intPar2[1] > intPar1[0] + prec)   // There is inner point of intersection.
      {
        return true;
      }
    }
  }
  else
  {
    // Parallel or equal planes.
    double distance = norm1.XYZ().Dot(nodes2[0].XYZ() - nodes1[0].XYZ());
    if ( distance > tol )
      return false; // Parallel planes.

    // Check nodes projection.
    for ( int i = 0; i < 3; ++i )
    {
      // Skip node if it is shared between triangles.
      if ( equalNodes[i] >= 0 )
        continue;

      const double sqDist = SqDistPntTri(nodes1[i], nodes2[0], nodes2[1], nodes2[2]);
      //
      if ( sqDist < tol * tol )
        return true;
    }

    // Check nodes projection.
    for ( int j = 0; j < 3; ++j )
    {
      // Skip node if it is shared between triangles.
      if ( j == equalNodes[0] || j == equalNodes[1] || j == equalNodes[2] )
        continue;

      const double sqDist = SqDistPntTri(nodes2[j], nodes1[0], nodes1[1], nodes1[2]);
      //
      if ( sqDist < tol * tol )
        return true;
    }
  }
  return false;
}
