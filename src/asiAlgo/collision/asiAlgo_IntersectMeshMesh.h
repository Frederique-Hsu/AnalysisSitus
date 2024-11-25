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
#include <asiAlgo_BVHFacets.h>

// OpenCascade includes
#include <BVH_Types.hxx>
#include <Bnd_Box.hxx>

//-----------------------------------------------------------------------------

namespace asiAlgo {

//! \ingroup ASI_MODELING
//!
//! Collision checker.
namespace collide
{

//! Class intended for detection of intersections between two sets of shapes
//! (triangle set and triangle set). The algorithm returns all pairs
//! of intersecting triangles. When two equal sets are passed, the algorithm
//! avoids adding triangles with equal indexes to the result.
class IntersectMeshMesh : public Standard_Transient
{
public:

  //! Creates new BVH-based intersection checker.
  //! \param [in] set1 the first set.
  //! \param [in] set2 the second set.
  asiAlgo_EXPORT
    IntersectMeshMesh(const Handle(asiAlgo_BVHFacets)& set1,
                      const Handle(asiAlgo_BVHFacets)& set2);

public:

  //! Performs internal intersection of a mesh with a mesh.
  //! \return true if the calculation is successful, false -- otherwise.
  asiAlgo_EXPORT bool
    Perform();

public:

  //! \return Vector of intersections. The number of indexes is always a factor
  //! of two. The first index belongs to the first set, the second index belongs
  //! to the second set.
  const std::vector<int>& GetIntersectionIndexes() const
  { return m_int; }

protected:

  //! Internal method to intersect leaf with triangle.
  //! \param [in]  leaf1 BVH-leaf object from the first tree.
  //! \param [in]  leaf2 BVH-leaf object from the second tree.
  //! \param [out] ints  pairs of intersecting indexes.
  void intersectLeaves(const BVH_Vec4i&  leaf1,
                       const BVH_Vec4i&  leaf2,
                       std::vector<int>& ints);

private:

  //! Copy-assignment operator is forbidden.
  IntersectMeshMesh& operator=(const IntersectMeshMesh&) = delete;

private:

  //! Triangles set.
  Handle(asiAlgo_BVHFacets) m_triSet1;

  //! Triangles set.
  Handle(asiAlgo_BVHFacets) m_triSet2;

  //! Intersections.
  std::vector<int> m_int;

  //! Internal variable representing situation when m_triSet1 and m_triSet2 are the same.
  const bool m_areSetsSame;

};

} // collide
} // asiAlgo
