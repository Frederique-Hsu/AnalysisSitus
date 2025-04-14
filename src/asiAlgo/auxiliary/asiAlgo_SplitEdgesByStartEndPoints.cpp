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
#include "asiAlgo_SplitEdgesByStartEndPoints.h"

// asiAlgo includes
#include "asiAlgo_Optional.h"
#include "asiAlgo_Utils.h"

// OCCT includes
#include <BOPTools_AlgoTools.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <NCollection_UBTreeFiller.hxx>

// STL includes
#include <algorithm>

//-----------------------------------------------------------------------------

typedef NCollection_UBTree       < int, Bnd_Box > boxBndTree;
typedef NCollection_UBTreeFiller < int, Bnd_Box > boxBndFiller;

//-----------------------------------------------------------------------------

namespace
{
  struct edgeInfo
  {
    Bnd_Box box;

    gp_Pnt fP, lP;

    Handle(Geom_Curve) C;

    double f, l;

    edgeInfo(const TopoDS_Edge& edge,
             const double       gap)
      : f(0.),
        l(0.)
    {
      // Get bounding boxes.
      {
        asiAlgo_Utils::Bounds( edge, false, true, box );

        box.SetGap( gap );
      }

      // Get edge's first and last points.
      {
        TopoDS_Vertex ev0, ev1;
        TopExp::Vertices( edge, ev0, ev1 );

        fP = BRep_Tool::Pnt( ev0 );
        lP = BRep_Tool::Pnt( ev1 );
      }

      // Get edge's curve parameters.
      {
        C = BRep_Tool::Curve( edge, f, l );
      }
    }
  };

  //-----------------------------------------------------------------------------

  class Selector_SplitEdge : public boxBndTree::Selector
  {
    public:

      Selector_SplitEdge(const double                      minAllowedDistFromEnds,
                         const double                      maxDistFromPointToCurve,
                         const std::vector< TopoDS_Edge >& edges)
        : m_minAllowedDistFromEnds( minAllowedDistFromEnds ),
          m_maxDistFromPointToCurve( maxDistFromPointToCurve ),
          m_edges( edges )
      {
        for ( const TopoDS_Edge& edge : edges )
        {
          m_data.push_back( { edge, m_maxDistFromPointToCurve } );
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

        return findIntersection( index - 1, *m_index - 1 );
      }

    private:

      bool findIntersection(const int& i1,
                            const int& i2)
      {
        try
        {
          OCC_CATCH_SIGNALS

          checkPoint( m_data[i1].fP, i2 );
          checkPoint( m_data[i1].lP, i2 );

          checkPoint( m_data[i2].fP, i1 );
          checkPoint( m_data[i2].lP, i1 );
        }
        catch ( ... )
        {
          return false;
        }

        return true;
      }

      void checkPoint(const gp_Pnt& P,
                      const int&    index)
      {
        const auto& data = m_data[ index ];

        GeomAPI_ProjectPointOnCurve ppc( P, data.C, data.f, data.l );

        const int nbPoints = ppc.NbPoints();

        if ( nbPoints == 0 )
        {
          return;
        }

        const double dist = ppc.LowerDistance();

        // Skip cases when point is too far from the curve.
        if ( dist > m_maxDistFromPointToCurve )
        {
          return;
        }

        const double param = ppc.LowerDistanceParameter();

        gp_Pnt projectedP;
        data.C->D0( param, projectedP );

        // Skip cases when point is located at some edge's end.
        if ( projectedP.Distance( data.fP ) < m_minAllowedDistFromEnds ||
             projectedP.Distance( data.lP ) < m_minAllowedDistFromEnds )
        {
          return;
        }

        if ( m_paramsToSplit.count( index ) )
        {
          m_paramsToSplit[ index ].push_back( param );
        }
        else
        {
          std::vector< double > params;
          params.push_back( param );

          m_paramsToSplit[ index ] = params;
        }
      }

    public:

      tl::optional< int >                    m_index;

      std::vector< edgeInfo >                m_data;

      std::map< int, std::vector< double > > m_paramsToSplit;

      double                                 m_minAllowedDistFromEnds;

      double                                 m_maxDistFromPointToCurve;

      const std::vector< TopoDS_Edge >&      m_edges;
  };
}

//-----------------------------------------------------------------------------

bool asiAlgo_SplitEdgesByStartEndPoints::Perform(std::vector< TopoDS_Edge >& edges)
{
  //-----------------------------------------------------------------------------
  // Intersect edges by their start/end points and
  // find resulting parameters on edges.
  //-----------------------------------------------------------------------------

  boxBndTree   bbTree;
  boxBndFiller treeFiller( bbTree );

  Selector_SplitEdge treeSelector( m_minAllowedDistFromEnds, m_maxDistFromPointToCurve, edges );

  for ( int i = 1; i <= (int) ( edges.size() ); ++i )
  {
    treeFiller.Add( i, treeSelector.m_data[ i - 1 ].box );
  }

  treeFiller.Fill();

  for ( int i = 1; i <= (int) ( edges.size() ); ++i )
  {
    treeSelector.Define( i );

    bbTree.Select( treeSelector );
  }

  //-----------------------------------------------------------------------------
  // Split edges by resulting parameters.
  //-----------------------------------------------------------------------------

  std::vector< TopoDS_Edge > result;

  // Split edges by a set of parameters.
  int i = 0;

  for ( TopoDS_Edge& edge : edges )
  {
    std::vector< double >& paramsDirty = treeSelector.m_paramsToSplit[i];

    if ( paramsDirty.size() == 0 )
    {
      result.push_back( edge );

      i++;

      continue;
    }

    const auto& data = treeSelector.m_data[ i ];

    paramsDirty.push_back( data.f );
    paramsDirty.push_back( data.l );

    std::sort( paramsDirty.begin(), paramsDirty.end() );

    // Perform splitting.
    tl::optional< double >        prev_param;
    tl::optional< TopoDS_Vertex > prev_V;

    for ( const auto& param : paramsDirty )
    {
      // Get vertex for current parameter.
      gp_Pnt P;

      data.C->D0( param, P );

      TopoDS_Vertex V = BRepBuilderAPI_MakeVertex( P );

      // Create a new edge splitting the old one.
      if ( prev_param.has_value() && Abs( param - *prev_param ) > Precision::Confusion() )
      {
        TopoDS_Edge newEdge;
        BOPTools_AlgoTools::MakeSplitEdge( edge, V, param, *prev_V, *prev_param, newEdge );

        result.push_back( newEdge );
      }

      prev_param = param;
      prev_V = V;
    }

    i++;
  }

  if ( result.size() > edges.size() )
  {
    edges = result;
  }

  return true;
}
