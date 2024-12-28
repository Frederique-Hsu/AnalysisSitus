//-----------------------------------------------------------------------------
// Created on: 28 December 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Sergey Slyadnev
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
#include "asiAlgo_MeshExtractPortion.h"

// Active Data includes
#include <ActData_Mesh_Triangle.h>

// OpenCascade includes
#include <NCollection_IndexedDataMap.hxx>
#include <Poly_Triangulation.hxx>
#include <TColStd_PackedMapOfInteger.hxx>

// Standard includes
#include <vector>

//-----------------------------------------------------------------------------

namespace {

  void ElemNodes(const Handle(ActData_Mesh)&             mesh,
                 const Handle(ActData_Mesh_Element)&     elem,
                 std::vector<Handle(ActData_Mesh_Node)>& nodes)
  {
    nodes.clear();
    int  numNodes = elem->NbNodes();
    int* nodeIDs  = nullptr;

    if ( numNodes == 3 )
    {
      const Handle(ActData_Mesh_Triangle)&
        tri = Handle(ActData_Mesh_Triangle)::DownCast(elem);
      //
      nodeIDs = (int*) tri->GetConnections();

      // Populate output collection.
      for ( int n = 0; n < numNodes; ++n )
        nodes.push_back( mesh->FindNode(nodeIDs[n]) );
    }
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_MeshExtractPortion::Perform(const TColStd_PackedMapOfInteger& elemIds,
                                         Handle(Poly_Triangulation)&       subMesh)
{
  if ( m_tris.IsNull() && m_mesh.IsNull() )
    return false;

  if ( elemIds.IsEmpty() )
    return false;

  /* Triangulation */
  if ( !m_tris.IsNull() )
  {
    // Original mesh node -> sub-mesh node map.
    NCollection_IndexedDataMap<int, int> node2node;

    const int nbTri      = m_tris->NbTriangles();
    int       resNodeIdx = 0;
    //
    for ( TColStd_PackedMapOfInteger::Iterator iter(elemIds); iter.More(); iter.Next() )
    {
      int elemId = iter.Key();
      //
      if ( elemId > nbTri || elemId <= 0 )
        return false;

      const Poly_Triangle& tri = m_tris->Triangle(elemId);

      int id[3];
      tri.Get(id[0], id[1], id[2]);

      for ( int i = 0; i < 3; ++i )
      {
        if ( !node2node.Contains(id[i]) )
          node2node.Add(id[i], ++resNodeIdx);
      }
    }

    bool hasUV = m_tris->HasUVNodes();
    bool hasNormals = m_tris->HasNormals();

    subMesh.Nullify();
    subMesh = new Poly_Triangulation(resNodeIdx, elemIds.Extent(), hasUV);
    if (hasNormals)
    {
      Handle(TShort_HArray1OfShortReal) normals = new TShort_HArray1OfShortReal(1, 3 * resNodeIdx);
      subMesh->SetNormals(normals);
    }

    // Copy mesh nodes & triangles.
    int resTriIdx = 0;
    for( TColStd_PackedMapOfInteger::Iterator iter(elemIds); iter.More(); iter.Next() )
    {
      ++resTriIdx;

      int key = iter.Key();
      const Poly_Triangle& tri = m_tris->Triangle(key);

      int id[3];
      tri.Get(id[0], id[1], id[2]);

      for(int i = 0; i < 3; ++i)
      {
        const int newIdx = node2node.FindFromKey(id[i]);
        subMesh->SetNode(newIdx, m_tris->Node(id[i]));
        subMesh->ChangeTriangle(resTriIdx).Set(i + 1, newIdx);
      }

    }
  }

  /* Mesh */
  if ( !m_mesh.IsNull() )
  {
    subMesh.Nullify();
    subMesh = new Poly_Triangulation(elemIds.Extent()*3, elemIds.Extent(), false);

    int nodeIdx = 0;
    int triIdx  = 0;

    for ( TColStd_PackedMapOfInteger::Iterator iter(elemIds); iter.More(); iter.Next() )
    {
      int elemId = iter.Key();
      //
      Handle(ActData_Mesh_Element) elem = m_mesh->FindElement(elemId);

      if ( elem.IsNull() || !elem->IsKind( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
        return false;

      // Get element's nodes.
      std::vector<Handle(ActData_Mesh_Node)> elemNodes;
      ElemNodes(m_mesh, elem, elemNodes);

      gp_Pnt pnt1 = elemNodes[0]->Pnt();
      gp_Pnt pnt2 = elemNodes[1]->Pnt();
      gp_Pnt pnt3 = elemNodes[2]->Pnt();

      subMesh->SetNode(++nodeIdx, pnt1);
      subMesh->SetNode(++nodeIdx, pnt2);
      subMesh->SetNode(++nodeIdx, pnt3);

      subMesh->SetTriangle( ++triIdx, Poly_Triangle(nodeIdx - 2, nodeIdx - 1, nodeIdx) );
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_MeshExtractPortion::Perform(const int                   triIdx,
                                         Handle(Poly_Triangulation)& subMesh)
{
  TColStd_PackedMapOfInteger idx;
  idx.Add(triIdx);
  return Perform(idx, subMesh);
}
