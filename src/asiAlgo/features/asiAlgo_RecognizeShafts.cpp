//-----------------------------------------------------------------------------
// Created on: 30 September 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC (www.quaoar-studio.com)
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
#include <asiAlgo_RecognizeShafts.h>

// asiAlgo includes
#include <asiAlgo_RecognitionRuleShaft.h>

// asiAlgo includes
#include <asiAlgo_AAGIterator.h>

// OpenCascade includes
#include <gp_Cylinder.hxx>

//-----------------------------------------------------------------------------

asiAlgo_RecognizeShafts::asiAlgo_RecognizeShafts(const Handle(asiAlgo_AAG)& aag,
                                                 ActAPI_ProgressEntry       progress,
                                                 ActAPI_PlotterEntry        plotter)
//
: asiAlgo_Recognizer (aag, progress, plotter),
  m_fLinToler        (0.)
{}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeShafts::SetLinearTolerance(const double tol)
{
  m_fLinToler = tol;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeShafts::SetDomain(const asiAlgo_Feature& domain)
{
  m_domain = domain;
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeShafts::SetExcludedFaces(const asiAlgo_Feature& fids)
{
  m_xSeeds = fids;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeShafts::Perform(const double radius)
{
  if ( !this->performInternal(radius) )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_RecognizeShafts::performInternal(const double radius)
{
  // Clean up the result.
  m_result.faces.Clear();
  m_result.ids.Clear();

  /* ====================
   *  Stage 1: build AAG.
   * ==================== */

  // Build master AAG.
  if ( m_aag.IsNull() )
  {
    // We do not allow smooth transitions here.
    m_aag = new asiAlgo_AAG(m_master, false);
  }

  /* ===========================
   *  Stage 2: recognition loop.
   * =========================== */

  asiAlgo_Feature traversed;

  // Iterate over the entire AAG in a random manner looking for
  // specific patterns.
  // ...

  Handle(asiAlgo_AAGIterator) seed_it = new asiAlgo_AAGRandomIterator(m_aag);

  // Linear tolerance to use.
  double linToler;
  //
  if ( m_fLinToler < Precision::Confusion() )
    linToler = BRep_Tool::MaxTolerance(m_aag->GetMasterShape(), TopAbs_EDGE);
  else
    linToler = m_fLinToler;

  // Prepare local recognition cursor.
  Handle(asiAlgo_RecognitionRuleShaft)
    rule = new asiAlgo_RecognitionRuleShaft(seed_it,
                                            radius,
                                            m_progress,
                                            m_plotter);
  //
  rule->SetLinearTolerance(linToler);

  // Main recognition loop. Any face is a seed, we let the rule decide.
  for ( ; seed_it->More(); seed_it->Next() )
  {
    const int fid = seed_it->GetFaceId();

    // If domain is set, check that the next face is in the domain.
    if ( !m_domain.IsEmpty() && !m_domain.Contains(fid) )
      continue;

    // Recognizer iterates some faces internally. We don't want to
    // use such faces as seeds, so we skip them here. Alternatively,
    // a face might have been forcible excluded from the consideration.
    if ( traversed.Contains(fid) || m_xSeeds.Contains(fid) )
      continue;

    // Attempt to recognize.
    if ( rule->Recognize(m_result.faces, m_result.ids) )
    {
      // Pick up those faces iterated by the recognizer and exclude them
      // from the list to iterate.
      traversed.Unite( rule->JustTraversed() );
    }

    // Progress.
    if ( m_progress.IsCancelling() )
      return false;
  }

  /* =====================================
   *  Stage 3: compose feature structures.
   * ===================================== */

  // Get the connected components.
  std::vector<asiAlgo_Feature> groups;
  m_aag->GetConnectedComponents(m_result.ids, groups);

  // Iterate over groups.
  for ( auto& group : groups )
  {
    Handle(asiAlgo_Shaft) shaft = new asiAlgo_Shaft;
    //
    this->populateShaft(group, shaft);

    // Add to the result.
    m_shafts.push_back(shaft);
  }

  return true; // Success.
}

//-----------------------------------------------------------------------------

void asiAlgo_RecognizeShafts::populateShaft(const asiAlgo_Feature& fids,
                                            Handle(asiAlgo_Shaft)& shaft) const
{
  // Get all cylindrical faces.
  asiAlgo_Feature           allCyls;
  tl::optional<gp_Cylinder> refCyl;
  //
  for ( asiAlgo_Feature::Iterator fit(fids); fit.More(); fit.Next() )
  {
    const int          fid  = fit.Key();
    const TopoDS_Face& face = m_aag->GetFace(fid);

    gp_Cylinder cyl;
    if ( !asiAlgo_Utils::IsCylindrical(face, cyl) )
      continue;

    if ( !refCyl.has_value() )
      refCyl = cyl;

    allCyls.Add(fid);
  }

  if ( !refCyl.has_value() )
    return;

  gp_Ax1 axis = refCyl->Axis();

  /* ================
   *  Compute length.
   * ================ */

  // Get all vertices of the cylindrical faces.
  std::vector<gp_XYZ> pts;
  //
  for ( asiAlgo_Feature::Iterator fit(allCyls); fit.More(); fit.Next() )
  {
    const int          fid  = fit.Key();
    const TopoDS_Face& face = m_aag->GetFace(fid);

    TopTools_IndexedMapOfShape faceVerts;
    TopExp::MapShapes(face, TopAbs_VERTEX, faceVerts);

    for ( int v = 1; v <= faceVerts.Extent(); ++v )
      pts.push_back( BRep_Tool::Pnt( TopoDS::Vertex( faceVerts(v) ) ).XYZ() );
  }

  // Project to axis to get the range.
  const gp_XYZ axisDir  =  axis.Direction().XYZ();
  double       rangeMin =  DBL_MAX;
  double       rangeMax = -DBL_MAX;
  //
  for ( const auto& pnt : pts )
  {
    const double dot = ( pnt - axis.Location().XYZ() ).Dot(axisDir);

#if defined DRAW_DEBUG
    m_plotter.DRAW_POINT(pnt, Color_White, "rangePt");
#endif

    if ( dot < rangeMin )
      rangeMin = dot;
    if ( dot > rangeMax )
      rangeMax = dot;
  }

  /* =====================
   *  Populate the result.
   * ===================== */

  shaft->axis     = axis;
  shaft->diameter = refCyl->Radius()*2;
  shaft->fids     = fids;
  shaft->length   = Abs(rangeMax - rangeMin);
}
