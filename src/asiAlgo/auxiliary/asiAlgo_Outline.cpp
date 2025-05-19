//-----------------------------------------------------------------------------
// Created on: 12 May 2025
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
#include "asiAlgo_Outline.h"

// pgsAlgo includes
#include "asiAlgo_JsonDict.h"

// asiAlgo includes
#include <asiAlgo_AnalyzeWire.h>
#include <asiAlgo_Utils.h>

// OCCT includes
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>

// Rapidjson includes
#include <rapidjson/document.h>

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;
typedef rapidjson::Document::Object    t_jsonObject;

using namespace asiAlgo::algo;

//-----------------------------------------------------------------------------

namespace
{
  bool isClosed(const std::vector< TopoDS_Edge >& edges)
  {
    const int nbEdges = (int) edges.size();

    if ( nbEdges == 0 )
    {
      return false;
    }

    TopoDS_Vertex V1, V2;
    TopExp::Vertices( edges.front(), V1, V2 );

    const gp_Pnt P1 = BRep_Tool::Pnt( V1 );
    const gp_Pnt P2 = BRep_Tool::Pnt( V2 );

    double maxToler = Max( BRep_Tool::Tolerance( V1 ),
                           BRep_Tool::Tolerance( V2 ) );

    // Check if the single edge is closed or not.
    if ( nbEdges == 1 )
    {
      return P1.IsEqual( P2, maxToler );
    }

    TopoDS_Vertex V3, V4;
    TopExp::Vertices( edges.back(), V3, V4 );

    const gp_Pnt P3 = BRep_Tool::Pnt( V3 );
    const gp_Pnt P4 = BRep_Tool::Pnt( V4);

    maxToler = Max( Max( BRep_Tool::Tolerance( V3 ),
                         BRep_Tool::Tolerance( V4 ) ),
                    maxToler );

    if ( nbEdges == 2 )
    {
      return ( P1.IsEqual( P3, maxToler ) && P2.IsEqual( P4, maxToler ) ) ||
             ( P1.IsEqual( P4, maxToler ) && P2.IsEqual( P3, maxToler ) );
    }

    return ( P1.IsEqual( P3, maxToler ) || P1.IsEqual( P4, maxToler ) ) ||
           ( P2.IsEqual( P3, maxToler ) || P2.IsEqual( P4, maxToler ) );
  }
}

//-----------------------------------------------------------------------------

Outline::Outline()
  : m_nbEdges( 0 ),
    m_isClosed( false ),
    m_length( 0. )
{
}

//-----------------------------------------------------------------------------

Outline::Outline(const TopoDS_Wire& wire)
  : Outline()
{
  m_wire = wire;

  std::vector< TopoDS_Edge > edges;

  for ( TopExp_Explorer expE( wire, TopAbs_EDGE ); expE.More(); expE.Next() )
  {
    const TopoDS_Edge& E = TopoDS::Edge( expE.Value() );

    edges.push_back( E );
  }

  m_nbEdges  = (int) edges.size();

  m_isClosed = isClosed( edges );

  GProp_GProps lProps;
  BRepGProp::LinearProperties( wire, lProps );

  m_length = lProps.Mass();
}

//-----------------------------------------------------------------------------

Outline::~Outline()
{
}

//-----------------------------------------------------------------------------

Outline::Outline(const Outline& outline)
{
  m_nbEdges  = outline.m_nbEdges;
  m_isClosed = outline.m_isClosed;
  m_length   = outline.m_length;
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

bool Outline::IsEqual(const Handle(Outline)& other,
                      const double           linToler,
                      const double           /*angTolerDeg*/) const
{
  if ( m_nbEdges != other->m_nbEdges )
    return false;

  if ( m_isClosed != other->m_isClosed )
    return false;

  if ( Abs( m_length - other->m_length ) > linToler )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

void Outline::FromJSON(void*            pJsonGenericObj,
                       Handle(Outline)& outline)
{
  if ( outline.IsNull() )
    outline = new Outline();

  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonGenericObj);

  // Iterate members of the object.
  t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();

  for ( ; mit != pJsonObj->MemberEnd(); mit++ )
  {
    std::string prop(mit->name.GetString());

    // m_NbEdges.
    if ( prop == PropName_OutlineNbEdges )
    {
      outline->m_nbEdges = mit->value.GetInt();
    }

    // m_length.
    if ( prop == PropName_Length )
    {
      outline->m_length = mit->value.GetDouble();
    }

    // m_isClosed.
    if ( prop == PropName_OutlineIsClosed )
    {
      outline->m_isClosed = mit->value.GetBool();
    }

  }
}

//-----------------------------------------------------------------------------

void Outline::ToJSON(const Handle(Outline)& outline,
                     const int              indent,
                     std::ostream&          out)
{
  std::string ws(indent, ' ');
  std::string nl = "\n" + ws;
  std::string subWS(2, ' ');

  /* Dump props */

  out << nl << "\"" << PropName_OutlineNbEdges << "\": " << outline->m_nbEdges;

  out << "," << nl << "\"" << PropName_Length << "\": " << outline->m_length;

  out << "," << nl << "\"" << PropName_OutlineIsClosed << "\": " << ( outline->m_isClosed ? "true" : "false" );
}
