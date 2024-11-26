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
#include <asiAlgo_IntersectMeshMesh.h>

// Includes from omp.
#include <omp.h>

// asiAlgo includes
#include <asiAlgo_BVHIterator2.h>
#include <asiAlgo_CollideUtils.h>
//#include <collide_GeomCG.h>
//#include <collide_GeomTriangleSet.h>
//#include <collide_Utils.h>

// Standard includes
#include <thread>

using namespace asiAlgo::collide;

namespace
{
  gp_Pnt P3D(const BVH_Vec3d& P)
  {
    return gp_Pnt( P.x(), P.y(), P.z() );
  }

  struct LeafLeaf
  {
    BVH_Vec4i m_leaf1;
    BVH_Vec4i m_leaf2;
  };

  struct IdId
  {
    IdId(int id1,
         int id2)
    : m_id1(id1),
      m_id2(id2)
    {}

    bool operator<(const IdId& other) const
    {
      if ( m_id1  < other.m_id1 ||
          (m_id1 == other.m_id1 && m_id2 < other.m_id2) )
      {
        return true;
      }

      return false;
    }

    int m_id1;
    int m_id2;
  };
}

//-----------------------------------------------------------------------------

IntersectMeshMesh::IntersectMeshMesh(const Handle(asiAlgo_BVHFacets)& triSet1,
                                     const Handle(asiAlgo_BVHFacets)& triSet2)
: m_triSet1     (triSet1),
  m_triSet2     (triSet2),
  m_areSetsSame (triSet1 == triSet2)
{
}

//-----------------------------------------------------------------------------

void IntersectMeshMesh::intersectLeaves(const BVH_Vec4i&  leaf1,
                                        const BVH_Vec4i&  leaf2,
                                        std::vector<int>& ints)
{
  ints.clear();

  // node.y() is the index of the first primitive
  // node.z() is the index of the last primitive
  for ( int idx1 = leaf1.y(); idx1 <= leaf1.z(); ++idx1 )
  {
    const asiAlgo_BVHFacets::t_facet&
      td1 = m_triSet1->GetFacet(idx1);

    gp_Pnt tri1[3] = { P3D(td1.P0), P3D(td1.P1), P3D(td1.P2) };

    Bnd_Box b1;
    b1.Add(tri1[0]);
    b1.Add(tri1[1]);
    b1.Add(tri1[2]);

    for ( int idx2 = leaf2.y(); idx2 <= leaf2.z(); ++idx2 )
    {
      const asiAlgo_BVHFacets::t_facet&
        td2 = m_triSet2->GetFacet(idx2);

      gp_Pnt tri2[3] = { P3D(td2.P0), P3D(td2.P1), P3D(td2.P2) };

      if ( m_areSetsSame && idx1 >= idx2 )
        continue;

      Bnd_Box b2;
      b2.Add(tri2[0]);
      b2.Add(tri2[1]);
      b2.Add(tri2[2]);

      if ( b1.IsOut(b2) )
        continue;

      if ( Utils::IntersectTriTri(tri1, td1.N, tri2, td2.N) )
      {
        ints.push_back(td1.FaceIndex);
        ints.push_back(td2.FaceIndex);
      }
    }
  }
}

//-----------------------------------------------------------------------------

bool IntersectMeshMesh::Perform()
{
  /* =================
   *  Contract checks.
   * ================= */

  const opencascade::handle< BVH_Tree<double, 3> >& bvh1 = m_triSet1->BVH();
  //
  if ( bvh1.IsNull() )
  {
    return false;
  }

  const opencascade::handle< BVH_Tree<double, 3> >& bvh2 = m_triSet2->BVH();
  //
  if ( bvh2.IsNull() )
  {
    return false;
  }

  /* =========================================
   *  Do pair-wise traversal of the BVH trees.
   * ========================================= */

  m_int.clear();

  std::vector<LeafLeaf> candidates;
  candidates.reserve(std::max(m_triSet1->Size(), m_triSet1->Size()));

  for ( asiAlgo_BVHIterator2 it(bvh1, bvh2); it.More(); it.Next() )
  {
    const BVH_Vec4i& node1 = it.Current1();
    const BVH_Vec4i& node2 = it.Current2();

    if ( it.IsLeaf1() && it.IsLeaf2() )
    {
      // Perform precise test.
      LeafLeaf ll;
      ll.m_leaf1 = node1;
      ll.m_leaf2 = node2;
      candidates.push_back(ll);
    }
    else if ( it.IsLeaf1() && !it.IsLeaf2() )
    {
      // The first  tree is leaf.
      // The second tree is not leaf.
      const BVH_Vec3d& minPnt1    = bvh1->MinPoint( node1.w() );
      const BVH_Vec3d& maxPnt1    = bvh1->MaxPoint( node1.w() );
      const BVH_Vec3d& minPntLft2 = bvh2->MinPoint( node2.y() );
      const BVH_Vec3d& maxPntLft2 = bvh2->MaxPoint( node2.y() );
      const BVH_Vec3d& minPntRgh2 = bvh2->MinPoint( node2.z() );
      const BVH_Vec3d& maxPntRgh2 = bvh2->MaxPoint( node2.z() );

      const bool out1 = Utils::IsOutBBox(minPnt1, maxPnt1, minPntLft2, maxPntLft2);
      const bool out2 = Utils::IsOutBBox(minPnt1, maxPnt1, minPntRgh2, maxPntRgh2);

      if ( out1 )
        it.BlockLeafLeft();

      if ( out2 )
        it.BlockLeafRight();
    }
    else if ( !it.IsLeaf1() && it.IsLeaf2() )
    {
      // The first  tree is not leaf.
      // The second tree is leaf.
      const BVH_Vec3d& minPntLft1 = bvh1->MinPoint(node1.y());
      const BVH_Vec3d& maxPntLft1 = bvh1->MaxPoint(node1.y());
      const BVH_Vec3d& minPntRgh1 = bvh1->MinPoint(node1.z());
      const BVH_Vec3d& maxPntRgh1 = bvh1->MaxPoint(node1.z());
      const BVH_Vec3d& minPnt2    = bvh2->MinPoint(node2.w());
      const BVH_Vec3d& maxPnt2    = bvh2->MaxPoint(node2.w());

      const bool out1 = Utils::IsOutBBox(minPntLft1, maxPntLft1, minPnt2, maxPnt2);
      const bool out2 = Utils::IsOutBBox(minPntRgh1, maxPntRgh1, minPnt2, maxPnt2);

      if ( out1 )
        it.BlockLeftLeaf();

      if ( out2 )
        it.BlockRightLeaf();
    }
    else if ( !it.IsLeaf1() && !it.IsLeaf2() )
    {
      // The first  tree is not leaf.
      // The second tree is leaf.
      const BVH_Vec3d& minPntLft1 = bvh1->MinPoint(node1.y());
      const BVH_Vec3d& maxPntLft1 = bvh1->MaxPoint(node1.y());
      const BVH_Vec3d& minPntRgh1 = bvh1->MinPoint(node1.z());
      const BVH_Vec3d& maxPntRgh1 = bvh1->MaxPoint(node1.z());
      const BVH_Vec3d& minPntLft2 = bvh2->MinPoint(node2.y());
      const BVH_Vec3d& maxPntLft2 = bvh2->MaxPoint(node2.y());
      const BVH_Vec3d& minPntRgh2 = bvh2->MinPoint(node2.z());
      const BVH_Vec3d& maxPntRgh2 = bvh2->MaxPoint(node2.z());

      const bool out11 = Utils::IsOutBBox(minPntLft1, maxPntLft1, minPntLft2, maxPntLft2);
      const bool out12 = Utils::IsOutBBox(minPntLft1, maxPntLft1, minPntRgh2, maxPntRgh2);
      const bool out21 = Utils::IsOutBBox(minPntRgh1, maxPntRgh1, minPntLft2, maxPntLft2);
      const bool out22 = Utils::IsOutBBox(minPntRgh1, maxPntRgh1, minPntRgh2, maxPntRgh2);

      if ( out11 )
        it.BlockLeftLeft();

      if ( out12 )
        it.BlockLeftRight();

      if ( out21 )
        it.BlockRightLeft();

      if ( out22 )
        it.BlockRightRight();
    }
  }

  // Perform actual intersection for detected candidates in parallel mode.
  const int nbThreads = std::thread::hardware_concurrency();

  // Partial solutions detected within threads.
  std::vector<std::vector<IdId>> partialSols(nbThreads);

  // Auxiliary variable storing detected intersections.
  std::vector<std::vector<int>> internalSols(nbThreads, std::vector<int>(16));

  // OpenMP works only with integral indexes (size_t is not allowed).
  const int size = (int) candidates.size();
  #pragma omp parallel for schedule(static, 250)
  for(int i = 0; i < size; ++i)
  {
    const int threadId = omp_get_thread_num();
    std::vector<int>& internalSol = internalSols[threadId];

    // Perform intersection test.
    intersectLeaves(candidates[i].m_leaf1, candidates[i].m_leaf2, internalSol);

    // Parse detected solutions and put them in a special structure.
    for(size_t j = 0; j < internalSol.size(); j += 2)
      partialSols[threadId].push_back(IdId(internalSol[j], internalSol[j + 1]));
  }

  // Merge partial solutions.
  size_t mergeSize = 0;
  std::vector<IdId> mergedSols;
  for(int i = 0; i < nbThreads; ++i)
    mergeSize += partialSols[i].size();
  mergedSols.reserve(mergeSize);

  for(int i = 0; i < nbThreads; ++i)
    mergedSols.insert(mergedSols.end(), partialSols[i].begin(), partialSols[i].end());

  // Sort solutions to avoid arbitrary order after detection in parallel mode.
  std::sort(mergedSols.begin(), mergedSols.end());

  m_int.reserve(2 * mergedSols.size());
  for(size_t i = 0; i < mergedSols.size(); ++i)
  {
    m_int.push_back(mergedSols[i].m_id1);
    m_int.push_back(mergedSols[i].m_id2);
  }

  return m_int.size() != 0;
}
