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

#pragma once

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <BVH_Types.hxx>
#include <Precision.hxx>

// Forward declarations
class Bnd_Box;
class gp_Pnt;

//-----------------------------------------------------------------------------

namespace asiAlgo {

//! \ingroup ASI_MODELING
//!
//! Collision checker.
namespace collide {

//! \ingroup ASI_MODELING
//!
//! Miscellaneous methods that facilitate collision detection.
namespace Utils
{
  //! Checks if two axis-aligned bounding boxes (AABB) intersect
  //! each other.
  //! \param[in] boxMin1 the lower corner of the first AABB.
  //! \param[in] boxMax1 the upper corner of the first AABB.
  //! \param[in] boxMin2 the lower corner of the second AABB.
  //! \param[in] boxMax2 the upper corner of the second AABB.
  //! \param[in] prec    the tolerance value.
  //! \return true if the passed bounding boxes intersect each other,
  //!         false -- otherwise.
  asiAlgo_EXPORT bool
    IsOutBBox(const BVH_Vec3d& boxMin1,
              const BVH_Vec3d& boxMax1,
              const BVH_Vec3d& boxMin2,
              const BVH_Vec3d& boxMax2,
              const double     prec = Precision::Confusion());

  //! Checks if two axis-aligned bounding boxes (AABB) intersect
  //! each other. The second box is passed in OpenCascade-ish way.
  //! \param[in] boxMin the lower corner of the AABB.
  //! \param[in] boxMin the upper corner of the AABB.
  //! \param[in] box    the other AABB to intersect with.
  //! \param[in] prec   the tolerance value.
  //! \return true if the passed bounding boxes intersect each other,
  //!         false -- otherwise.
  asiAlgo_EXPORT bool
    IsOutBBox(const BVH_Vec3d& boxMin,
              const BVH_Vec3d& boxMax,
              const Bnd_Box&   box,
              const double     prec = Precision::Confusion());

  //! Computes squared distance from the passed point to the
  //! triangle defined with its nodes.
  //! \param[in]  pnt the point in question.
  //! \param[in]  p1  the 1-st node of the triangle.
  //! \param[in]  p2  the 2-nd node of the triangle.
  //! \param[in]  p3  the 3-rd node of the triangle.
  //! \param[out] sol the computed extremum point on the triangle.
  //! \return the computed squared distance.
  asiAlgo_EXPORT double
    SqDistPntTri(const gp_Pnt& p,
                 const gp_Pnt& p1,
                 const gp_Pnt& p2,
                 const gp_Pnt& p3,
                 gp_Pnt&       sol);

  //! Computes squared distance from the passed point to the
  //! triangle defined with its nodes.
  //! \param[in] pnt the point in question.
  //! \param[in] p1  the 1-st node of the triangle.
  //! \param[in] p2  the 2-nd node of the triangle.
  //! \param[in] p3  the 3-rd node of the triangle.
  //! \return the computed squared distance.
  asiAlgo_EXPORT double
    SqDistPntTri(const gp_Pnt& pnt,
                 const gp_Pnt& p1,
                 const gp_Pnt& p2,
                 const gp_Pnt& p3);

  //! Computes squared distance between the passed lines.
  //! \param[in]  l1   the first line to test.
  //! \param[in]  l2   the second line to test.
  //! \param[out] par1 the extremum argument over the 1-st line.
  //! \param[out] par2 the extremum argument over the 2-nd line.
  //! \param[in]  prec the computation precision.
  //! \return the computed squared distance.
  asiAlgo_EXPORT double
    SqDistLineLine(const gp_Lin& l1,
                   const gp_Lin& l2,
                   double&       par1,
                   double&       par2,
                   const double  prec = Precision::Confusion());

  //! Intersects the passed line with a triangle defined with the
  //! array of its nodes.
  //! \param[in]  line        the line to test for intersection with a triangle.
  //! \param[in]  nodes       the triangle nodes.
  //! \param[in]  tol         the intersection tolerance.
  //! \param[out] interParams the computed intersection parameters.
  asiAlgo_EXPORT void
    IntersectLineTri(const gp_Lin& line,
                     const gp_Pnt  nodes[3],
                     double        tol,
                     double        interParams[2]);

  //! Checks the passed pair of triangles for intersection.
  //! \param[in] nodes1 the nodes of the first triangle.
  //! \param[in] norm1  the normal of the first triangle.
  //! \param[in] nodes2 the nodes of the second triangle.
  //! \param[in] norm2  the normal of the second triangle.
  //! \param[in] prec   the intersection precision.
  //! \return true if the passed triangles intersect,
  //!         false -- otherwise.
  asiAlgo_EXPORT bool
    IntersectTriTri(const gp_Pnt  nodes1[3],
                    const gp_Dir& norm1,
                    const gp_Pnt  nodes2[3],
                    const gp_Dir& norm2,
                    const double  prec = Precision::Confusion());
} // Utils
} // collide
} // asiAlgo
