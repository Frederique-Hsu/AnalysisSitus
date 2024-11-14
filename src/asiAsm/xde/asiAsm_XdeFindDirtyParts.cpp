//-----------------------------------------------------------------------------
// Created on: 14 November 2024
// Created by: Sergey KISELEV
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
#include <asiAsm_XdeFindDirtyParts.h>

// asiAlgo includes
#include <asiAlgo_Timer.h>
#include <asiAlgo_Utils.h>

// OpenCascade includes
#include <TopoDS.hxx>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

FindDirtyParts::FindDirtyParts(const Handle(Doc)&   doc,
                               const bool           checkVertices,
                               const bool           checkEdges,
                               const bool           checkFacets,
                               const bool           checkGeometry,
                               ActAPI_ProgressEntry progress)
//
: ActAPI_IAlgorithm (progress, nullptr),
  m_doc             (doc),
  m_bCheckVertices  (checkVertices),
  m_bCheckEdges     (checkEdges),
  m_bCheckFacets    (checkFacets),
  m_bCheckGeometry  (checkGeometry)
{
}

//-----------------------------------------------------------------------------

bool FindDirtyParts::Perform(PartIds& dirtyParts)
{
#if defined COUT_DEBUG
  TIMER_NEW
  TIMER_GO
#endif

  PartIds parts;
  m_doc->GetParts(parts);

  // Initialize progress notifier with the number of parts.
  const int numParts = parts.Length();

  m_progress.Reset();
  m_progress.Init(numParts);

  // Process each part individually.
  for ( int i = 0; i < numParts; ++i )
  {
    m_progress.StepProgress(1);
    //
    if ( m_progress.IsCancelling() )
    {
      m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
      return false;
    }

    // Get next part.
    const PartId& pid       = parts[i];
    TopoDS_Shape  partShape = m_doc->GetShape(pid);
    TDF_Label     partLabel = m_doc->GetLabel(pid);
    //
    if ( partShape.IsNull() )
      continue;

    // Check existence of surface geometries.
    TopTools_IndexedMapOfShape faceMap, edgeMap, vertexMap;
    //
    TopExp::MapShapes( partShape, TopAbs_FACE,   faceMap );
    TopExp::MapShapes( partShape, TopAbs_EDGE,   edgeMap );
    TopExp::MapShapes( partShape, TopAbs_VERTEX, vertexMap );
    //
    bool isDirty = false;

    /* Vertices but no edges between them */
    if ( m_bCheckVertices && !vertexMap.IsEmpty() && edgeMap.IsEmpty() )
    {
      isDirty = true;
    }

    /* Edges but no faces trimmed by them */
    else if ( m_bCheckEdges && !edgeMap.IsEmpty() && faceMap.IsEmpty() )
    {
      isDirty = true;
    }

    /* Faces without facets */
    else if ( m_bCheckFacets )
    {
      bool hasFacets = false;
      for ( int f = 1; f <= faceMap.Extent(); ++f )
      {
        TopLoc_Location            L;
        const TopoDS_Face&         face = TopoDS::Face( faceMap(f) );
        Handle(Poly_Triangulation) tris = BRep_Tool::Triangulation(face, L);
        //
        if ( !tris.IsNull() )
        {
          hasFacets = true;
          break;
        }
      }
      isDirty = !hasFacets;
    }

    /* Untrimmed faces */
    else if ( m_bCheckGeometry && ( vertexMap.IsEmpty() && edgeMap.IsEmpty() && !faceMap.IsEmpty() ) )
    {
      isDirty = true;
    }

    if ( isDirty )
    {
      dirtyParts.Append(pid);
    }
  }

#if defined COUT_DEBUG
  TIMER_FINISH
  TIMER_COUT_RESULT_MSG("FindDirtyParts::Perform")
#endif

  if ( m_progress.IsCancelling() )
  {
    m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Canceled);
    return false;
  }

  m_progress.SetProgressStatus(ActAPI_ProgressStatus::Progress_Succeeded);
  return true;
}
