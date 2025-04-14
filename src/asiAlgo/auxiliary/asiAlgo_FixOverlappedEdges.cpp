//-----------------------------------------------------------------------------
// Created on: 23 March 2025
//-----------------------------------------------------------------------------
// Copyright (c) 2025-present, Sergey Kiselev
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
#include "asiAlgo_FixOverlappedEdges.h"

// asiAlgo includes
#include <asiAlgo_Optional.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BOPTools_AlgoTools.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepGProp.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GProp_GProps.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <ShapeAnalysis_Edge.hxx>

// Standard includes
#include <set>

//-----------------------------------------------------------------------------

typedef NCollection_UBTree       < int, Bnd_Box > boxBndTree;
typedef NCollection_UBTreeFiller < int, Bnd_Box > boxBndFiller;

//-----------------------------------------------------------------------------

namespace
{
  struct edgeInfo
  {
    Bnd_Box box;

    double length;

    edgeInfo(const TopoDS_Edge& edge,
             const double       gap)
    {
      // Get bounding boxes.
      {
        asiAlgo_Utils::Bounds( edge, false, true, box );

        box.SetGap( gap );
      }

      // Gel edge's length.
      {
        GProp_GProps props;
        BRepGProp::LinearProperties( edge, props );

        length = Abs( props.Mass() );
      }
    }
  };

  //-----------------------------------------------------------------------------

  class Selector_FindOverlappedEdges : public boxBndTree::Selector
  {
    public:

      Selector_FindOverlappedEdges(const double                      maxAllowedDistance,
                                   const std::vector< TopoDS_Edge >& edges)
        : m_maxAllowedDistance( maxAllowedDistance ),
          m_edges( edges )
      {
        for ( const TopoDS_Edge& edge : edges )
        {
          m_data.push_back( { edge, m_maxAllowedDistance } );
        }
      }

      void Define(const int index)
      {
        m_index = index;
      }

      bool Reject(const Bnd_Box& box) const
      {
        return m_data[ *m_index - 1 ].box.IsOut( box );
      }

      bool Accept(const int& index)
      {
        if ( *m_index == index )
        {
          return false;
        }

        if ( Abs( m_data[ index - 1 ].length - m_data[ *m_index - 1 ].length ) > m_maxAllowedDistance )
        {
          return false;
        }

        if ( m_overlappingAlgo.CheckOverlapping( m_edges[    index - 1 ],
                                                 m_edges[ *m_index - 1 ],
                                                 m_maxAllowedDistance ) )
        {
          m_duplicates.insert( index );

          return true;
        }

        return false;
      }

    public:

      tl::optional< int >               m_index;

      std::vector< edgeInfo >           m_data;

      std::set< int >                   m_duplicates;

      double                            m_maxAllowedDistance;

      ShapeAnalysis_Edge                m_overlappingAlgo;

      const std::vector< TopoDS_Edge >& m_edges;
  };
}

//-----------------------------------------------------------------------------

bool asiAlgo_FixOverlappedEdges::Perform(std::vector< TopoDS_Edge >& edges)
{
  //-----------------------------------------------------------------------------
  // Find overlapped edges and keep only a single instance of edge for such cases.
  //-----------------------------------------------------------------------------

  boxBndTree   bbTree;
  boxBndFiller treeFiller( bbTree );

  Selector_FindOverlappedEdges treeSelector( m_maxAllowedDistance, edges );

  for ( int i = 1; i <= edges.size(); ++i )
  {
    treeFiller.Add( i, treeSelector.m_data[ i - 1 ].box );
  }

  treeFiller.Fill();

  std::vector< TopoDS_Edge > result;

  for ( int i = 1; i <= edges.size(); ++i )
  {
    if ( treeSelector.m_duplicates.count(i) > 0 )
    {
      continue;
    }

    treeSelector.Define( i );

    bbTree.Select( treeSelector );

    result.push_back( edges[ i - 1 ] );
  }

  // Override 'edges' if we have removed some overlapped edges.
  if ( result.size() != edges.size() )
  {
    edges = result;
  }

  return true;
}
