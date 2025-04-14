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
#include "asiAlgo_ComputeOutline.h"

// Active Data includes
#include <ActAPI_IAlgorithm.h>

// asiAlgo includes
#include "asiAlgo_FixOverlappedEdges.h"
#include "asiAlgo_SplitEdgesByStartEndPoints.h"
#include "asiAlgo_Utils.h"
#include <asiAlgo_BVHFacets.h>
#include <asiAlgo_FeatureFaces.h>
#include <asiAlgo_HitFacet.h>
#include <asiAlgo_HlrPreciseAlgo.h>
#include <asiAlgo_HlrToShape.h>
#include <asiAlgo_IntersectCC.h>

// OCCT includes
#include <BOPAlgo_Tools.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepGProp.hxx>
#include <BRepLib.hxx>
#include <GProp_GProps.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <HLRBRep_Data.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <ShapeFix_Wire.hxx>
#include <TopExp_Explorer.hxx>

#define DRAW_DEBUG
#undef DRAW_DEBUG

#ifdef DRAW_DEBUG
  #define dirCoords 1, -1, 1

  static std::set< int > toDraw = { -1 }; // '-1' to draw all vectors;
                                          // 'empty' to skip drawing;
                                          // 'custom indices' to draw vectors for specific edges.

  #define DRAW_INDEX toDraw.count( *edgeData.seqIndex )
  #define DRAW_ALL   toDraw.count(-1)

  // #define INCLUDE_HIDDEN_LINES

  static std::vector< std::pair< gp_Pnt, gp_Vec > > debugInfoVec_Shift = {};
  static std::vector< std::pair< gp_Pnt, gp_Vec > > debugInfoVec_Rays_Red = {};
  static std::vector< std::pair< gp_Pnt, gp_Vec > > debugInfoVec_Rays_Green = {};

  static std::vector< std::pair< gp_Pnt, gp_Vec > > debugInfoVec_Target = {};
  static std::vector< std::pair< gp_Pnt, gp_Vec > > debugInfoVec_Intersected = {};
  static std::vector< std::pair< gp_Pnt, gp_Vec > > debugInfoVec_FromTargetToIntersected = {};

#endif

//-----------------------------------------------------------------------------

using namespace asiAlgo;

typedef NCollection_UBTree       < int, Bnd_Box > boxBndTree;
typedef NCollection_UBTreeFiller < int, Bnd_Box > boxBndFiller;

//-----------------------------------------------------------------------------

namespace
{
  const TopoDS_Shape& build3dCurves(const TopoDS_Shape& shape)
  {
    for ( TopExp_Explorer it( shape, TopAbs_EDGE ); it.More(); it.Next() )
    {
      BRepLib::BuildCurve3d( TopoDS::Edge( it.Current() ) );
    }

    return shape;
  }

  //-----------------------------------------------------------------------------

  bool makeProjection(const TopoDS_Shape&      shape,
                      const gp_Dir&            dir,
                      const asiAlgo_Feature&   featureFaces,
                      TopoDS_Shape&            hlrResult,
                      TopoDS_Shape&            featureFacesResult)
  {
    gp_Ax2 transform( gp::Origin(), dir );
    HLRAlgo_Projector projector( transform );

    Handle(hlr::PreciseAlgo)
      brep_hlr = new hlr::PreciseAlgo();
    //
    brep_hlr->Add( shape );

    try
    {
      brep_hlr->Projector( projector );
      brep_hlr->Update();
      brep_hlr->Hide();
    }
    catch ( ... )
    {
      return false;
    }

    hlr::HlrToShape shapes( brep_hlr );

    TopoDS_Shape V, V1, VN, VO, VI;

#ifdef INCLUDE_HIDDEN_LINES
    TopoDS_Shape H, H1, HN, HO, HI;
#endif

    try
    {
      V  = build3dCurves( shapes.VCompound       () ); // "hard edges" visible
      V1 = build3dCurves( shapes.Rg1LineVCompound() ); // "smooth edges" visible
      VN = build3dCurves( shapes.RgNLineVCompound() ); // "contour edges" visible
      VO = build3dCurves( shapes.OutLineVCompound() ); // "outline" visible
      VI = build3dCurves( shapes.IsoLineVCompound() ); // "isolines" visible (precise HLR only)

#ifdef INCLUDE_HIDDEN_LINES
      H  = build3dCurves( shapes.HCompound       () ); // "hard edges" hidden
      H1 = build3dCurves( shapes.Rg1LineHCompound() ); // "smooth edges" hidden
      HN = build3dCurves( shapes.RgNLineHCompound() ); // "contour edges" hidden
      HO = build3dCurves( shapes.OutLineHCompound() ); // "outline" hidden
      HI = build3dCurves( shapes.IsoLineHCompound() ); // "isolines" hidden (precise HLR only)
#endif
    }
    catch ( ... )
    {
      return false;
    }

    gp_Trsf T;
    T.SetTransformation( gp_Ax3( transform ) );
    T.Invert();

    TopoDS_Compound visible;
    BRep_Builder().MakeCompound( visible );

    if ( !V.IsNull() )
      BRep_Builder().Add( visible, V );

    if ( !V1.IsNull() )
      BRep_Builder().Add( visible, V1 );

    if ( !VN.IsNull() )
      BRep_Builder().Add( visible, VN );

    if ( !VO.IsNull() )
      BRep_Builder().Add( visible, VO );

    if ( !VI.IsNull() )
      BRep_Builder().Add( visible, VI );

#ifdef INCLUDE_HIDDEN_LINES
    if ( !H.IsNull() )
      BRep_Builder().Add( visible, H );

    if ( !H1.IsNull() )
      BRep_Builder().Add( visible, H1 );

    if ( !HN.IsNull() )
      BRep_Builder().Add( visible, HN );

    if ( !HO.IsNull() )
      BRep_Builder().Add( visible, HO );

    if ( !HI.IsNull() )
      BRep_Builder().Add( visible, HI );
#endif

    // Extract additional lines set of edges.
    TopoDS_Compound FC;
    BRep_Builder().MakeCompound( FC );

    {
      if ( !featureFaces.IsEmpty() )
      {
        Handle(HLRBRep_Data) ds = brep_hlr->DataStructure();

        TopTools_IndexedMapOfShape& dsFaces = ds->FaceMap();

        TopoDS_Compound facesComp;
        BRep_Builder().MakeCompound( facesComp );

        asiAlgo_Feature::Iterator fiter( featureFaces );
        for ( ; fiter.More(); fiter.Next() )
        {
          BRep_Builder().Add( facesComp, dsFaces( fiter.Key() ) );
        }

        TopoDS_Shape FV, FV1, FVN, FVO;

#ifdef INCLUDE_HIDDEN_LINES
         TopoDS_Shape FH, FH1, FHN, FHO;
#endif

        try
        {
          FV  = build3dCurves( shapes.VCompound       ( facesComp ) ); // "hard edges" visible
          FV1 = build3dCurves( shapes.Rg1LineVCompound( facesComp ) ); // "smooth edges" visible
          FVN = build3dCurves( shapes.RgNLineVCompound( facesComp ) ); // "contour edges" visible
          FVO = build3dCurves( shapes.OutLineVCompound( facesComp ) ); // "outline" visible

#ifdef INCLUDE_HIDDEN_LINES
          FH  = build3dCurves( shapes.HCompound       ( facesComp ) ); // "hard edges" hidden
          FH1 = build3dCurves( shapes.Rg1LineHCompound( facesComp ) ); // "smooth edges" hidden
          FHN = build3dCurves( shapes.RgNLineHCompound( facesComp ) ); // "contour edges" hidden
          FHO = build3dCurves( shapes.OutLineHCompound( facesComp ) ); // "outline" hidden
#endif
        }
        catch ( ... )
        {
          return false;
        }

        //
        if ( !FV.IsNull() )
          BRep_Builder().Add( FC, FV );
        //
        if ( !FV1.IsNull() )
          BRep_Builder().Add( FC, FV1 );
        //
        if ( !FVN.IsNull() )
          BRep_Builder().Add( FC, FVN );
        //
        if ( !FVO.IsNull() )
          BRep_Builder().Add( FC, FVO );

#ifdef INCLUDE_HIDDEN_LINES
        if ( !FH.IsNull() )
          BRep_Builder().Add( FC, FH );
        //
        if ( !FH1.IsNull() )
          BRep_Builder().Add( FC, FH1 );
        //
        if ( !FHN.IsNull() )
          BRep_Builder().Add( FC, FHN );
        //
        if ( !FHO.IsNull() )
          BRep_Builder().Add( FC, FHO );
#endif
      }
    }

    hlrResult          = visible.Moved(T);
    featureFacesResult = FC.Moved(T);

    return true;
  }

  //-----------------------------------------------------------------------------

  bool makeProjectionDiscr(const TopoDS_Shape&    shape,
                           const gp_Dir&          dir,
                           const asiAlgo_Feature& featureFaces,
                           TopoDS_Shape&          hlrResult,
                           TopoDS_Shape&          featureFacesResult)
  {
    gp_Ax2 transform( gp::Origin(), dir );

    // Prepare projector.
    HLRAlgo_Projector projector( transform );

    // Prepare polygonal HLR algorithm which is known to be more reliable than
    // the "curved" version of HLR.
    Handle(HLRBRep_PolyAlgo) polyAlgo = new HLRBRep_PolyAlgo;
    //
    try
    {
      polyAlgo->Projector( projector );
      polyAlgo->Load( shape );
      polyAlgo->Update();
    }
    catch ( ... )
    {
      // Sometimes crashes.
      return false;
    }

    // Create topological entities.
    HLRBRep_PolyHLRToShape shapes;
    //
    try
    {
      shapes.Update( polyAlgo );
    }
    catch ( ... )
    {
      // Sometimes crashes.
      return false;
    }

    // V -- visible
    TopoDS_Shape V  = build3dCurves( shapes.VCompound       () ); // "hard edges" visible
    TopoDS_Shape V1 = build3dCurves( shapes.Rg1LineVCompound() ); // "smooth edges" visible
    TopoDS_Shape VN = build3dCurves( shapes.RgNLineVCompound() ); // "contour edges" visible
    TopoDS_Shape VO = build3dCurves( shapes.OutLineVCompound() ); // "outline" visible

#ifdef INCLUDE_HIDDEN_LINES
    // H -- hidden
    TopoDS_Shape H  = build3dCurves( shapes.HCompound       () ); // "hard edges" hidden
    TopoDS_Shape H1 = build3dCurves( shapes.Rg1LineHCompound() ); // "smooth edges" hidden
    TopoDS_Shape HN = build3dCurves( shapes.RgNLineHCompound() ); // "contour edges" hidden
    TopoDS_Shape HO = build3dCurves( shapes.OutLineHCompound() ); // "outline" hidden
#endif

    TopoDS_Compound C;
    BRep_Builder().MakeCompound( C );
    //
    if ( !V.IsNull() )
      BRep_Builder().Add( C, V );
    //
    if ( !V1.IsNull() )
      BRep_Builder().Add( C, V1 );
    //
    if ( !VN.IsNull() )
      BRep_Builder().Add( C, VN );
    //
    if ( !VO.IsNull() )
      BRep_Builder().Add( C, VO );

#ifdef INCLUDE_HIDDEN_LINES
    //
    if ( !H.IsNull() )
      BRep_Builder().Add( C, H );
    //
    if ( !H1.IsNull() )
      BRep_Builder().Add( C, H1 );
    //
    if ( !HN.IsNull() )
      BRep_Builder().Add( C, HN );
    //
    if ( !HO.IsNull() )
      BRep_Builder().Add( C, HO );
#endif

    gp_Trsf T;
    T.SetTransformation( gp_Ax3( transform ) );
    T.Invert();

    // Extract additional lines set of edges.
    TopoDS_Compound FC;
    BRep_Builder().MakeCompound( FC );

    {
      if ( !featureFaces.IsEmpty() )
      {
        TopTools_IndexedMapOfShape fMap;
        TopExp::MapShapes( shape, TopAbs_FACE, fMap );

        TopoDS_Compound facesComp;
        BRep_Builder().MakeCompound( facesComp );

        asiAlgo_Feature::Iterator fiter( featureFaces );
        for ( ; fiter.More(); fiter.Next() )
        {
          BRep_Builder().Add( facesComp, fMap( fiter.Key() ) );
        }

        TopoDS_Shape FV, FV1, FVN, FVO;
        // TopoDS_Shape FH, FH1, FHN, FHO;

        try
        {
          FV  = build3dCurves( shapes.VCompound       ( facesComp ) ); // "hard edges" visible
          FV1 = build3dCurves( shapes.Rg1LineVCompound( facesComp ) ); // "smooth edges" visible
          FVN = build3dCurves( shapes.RgNLineVCompound( facesComp ) ); // "contour edges" visible
          FVO = build3dCurves( shapes.OutLineVCompound( facesComp ) ); // "outline" visible

#ifdef INCLUDE_HIDDEN_LINES
          FH  = build3dCurves( shapes.HCompound       ( facesComp ) ); // "hard edges" hidden
          FH1 = build3dCurves( shapes.Rg1LineHCompound( facesComp ) ); // "smooth edges" hidden
          FHN = build3dCurves( shapes.RgNLineHCompound( facesComp ) ); // "contour edges" hidden
          FHO = build3dCurves( shapes.OutLineHCompound( facesComp ) ); // "outline" hidden
#endif
        }
        catch ( ... )
        {
          return false;
        }

        //
        if ( !FV.IsNull() )
          BRep_Builder().Add( FC, FV );
        //
        if ( !FV1.IsNull() )
          BRep_Builder().Add( FC, FV1 );
        //
        if ( !FVN.IsNull() )
          BRep_Builder().Add( FC, FVN );
        //
        if ( !FVO.IsNull() )
          BRep_Builder().Add( FC, FVO );

#ifdef INCLUDE_HIDDEN_LINES
        //
        if ( !FH.IsNull() )
          BRep_Builder().Add( FC, FH );
        //
        if ( !FH1.IsNull() )
          BRep_Builder().Add( FC, FH1 );
        //
        if ( !FHN.IsNull() )
          BRep_Builder().Add( FC, FHN );
        //
        if ( !FHO.IsNull() )
          BRep_Builder().Add( FC, FHO );
#endif
      }
    }

    hlrResult          =  C.Moved(T);
    featureFacesResult = FC.Moved(T);

    return true;
  }

  //-----------------------------------------------------------------------------

  const bool hasHit(asiAlgo_HitFacet& testAxis,
                    const gp_Pnt&     p,
                    const gp_Dir&     d)
  {
    gp_Lin ray( p, d );

    // Do the intersection test.
    gp_XYZ hit;
    int facetIdx = -1;

    testAxis( ray, facetIdx, hit );

    return facetIdx != -1;
  }

  //-----------------------------------------------------------------------------

  void drawEdges(const std::vector< TopoDS_Edge >& edges,
                 const std::string&                name,
                 ActAPI_PlotterEntry               plotter,
                 ActAPI_Color                      color = Color_Pink)
  {
    if ( edges.empty() )
    {
      return;
    }

    TopoDS_Compound C;

    BRep_Builder bb;
    bb.MakeCompound(C);

    for ( auto& edge : edges )
    {
      bb.Add( C, edge );
    }

    plotter.REDRAW_SHAPE( name.c_str(), C, color );
  }

  //-----------------------------------------------------------------------------

  void drawDebugInfoVec(const std::string&                                name,
                        const ActAPI_Color&                               color,
                        const std::vector< std::pair< gp_Pnt, gp_Vec > >& vec,
                        ActAPI_PlotterEntry                               plotter,
                        const bool                                        tips = true,
                        const bool                                        rescale = true)
  {
    if ( vec.empty() )
    {
      return;
    }

    Handle(HRealArray) vecs   = new HRealArray( 0, (int) vec.size() * 3 - 1, 0. );
    Handle(HRealArray) coords = new HRealArray( 0, (int) vec.size() * 3 - 1, 0. );

    int index = 0;

    for ( auto& info : vec )
    {
      coords->ChangeValue(index)     = info.first.X();
      coords->ChangeValue(index + 1) = info.first.Y();
      coords->ChangeValue(index + 2) = info.first.Z();

      vecs->ChangeValue(index)     = info.second.X();
      vecs->ChangeValue(index + 1) = info.second.Y();
      vecs->ChangeValue(index + 2) = info.second.Z();

      index += 3;
    }

    plotter.REDRAW_VECTORS( name.c_str(), coords, vecs, color, tips, rescale );
  }

  //-----------------------------------------------------------------------------

  struct sideInfo
  {
    enum HasMaterial
    {
      HasMaterial_Undefined = 0,
      HasMaterial_HasMaterial,
      HasMaterial_NoMaterial
    };

    HasMaterial hasMaterial;

    gp_Vec sideShiftV;

    // Intersection results.
    bool                   m_iDone;

    tl::optional< double > m_iDistance;
    tl::optional< int    > m_iEdgeIndex;
    tl::optional< double > m_iEdgeParam;

    sideInfo()
      : hasMaterial( HasMaterial_Undefined ),
        m_iDone( false )
    {
    }
  };

  //-----------------------------------------------------------------------------

  struct edgeInfo
  {
    tl::optional< int > seqIndex;

    enum Status
    {
      Status_Undefined = 0,
      Status_Border,
      Status_Body
    };

    sideInfo sideInfo_Left;
    sideInfo sideInfo_Right;

    Bnd_Box box;

    double tolerance;

    // 3D parameters.
    gp_Pnt midP;
    gp_Vec midV;

    double f, l;
    Handle(Geom_Curve) C3d;

    // Constructor.
    edgeInfo(const TopoDS_Edge&        edge,
             const gp_Dir&             norm,
             const double              gap,
             const tl::optional< int > i)
      : tolerance( gap ),
        seqIndex(i)
    {
      // Get bounding boxes.
      {
        asiAlgo_Utils::Bounds( edge, false, true, box );

        box.SetGap( tolerance );
      }

      // Get edges's parameters in 3D.
      {
        C3d = BRep_Tool::Curve( edge, f, l );

        C3d->D1( ( f + l ) * 0.5, midP, midV );

        midV.Normalize();
      }

      sideInfo_Left.sideShiftV  = midV.Rotated( gp_Ax1( midP, norm ),   M_PI / 2. ).Normalized();
      sideInfo_Right.sideShiftV = midV.Rotated( gp_Ax1( midP, norm ), - M_PI / 2. ).Normalized();
    }

    Status status()
    {
      if ( sideInfo_Left.hasMaterial  == sideInfo::HasMaterial_Undefined ||
           sideInfo_Right.hasMaterial == sideInfo::HasMaterial_Undefined )
      {
        return Status_Undefined;
      }

      if ( sideInfo_Left.hasMaterial  == sideInfo::HasMaterial_HasMaterial &&
           sideInfo_Right.hasMaterial == sideInfo::HasMaterial_HasMaterial )
      {
        return Status_Body;
      }

      return Status_Border;
    }
  };

  //-----------------------------------------------------------------------------

  class Selector_FindIntersections : public boxBndTree::Selector
  {
    public:

      Selector_FindIntersections(const std::vector< TopoDS_Edge >& edges,
                                 const gp_Dir&                     norm,
                                 const double                      gap)
        : m_shiftEdgeInfo( nullptr ),
          m_sideInfo( nullptr ),
          m_intersector( { nullptr, nullptr } )
      {
        if ( edges.empty() )
        {
          return;
        }

        // Extract data from the passed edges set.
        int i = 1;

        for ( const TopoDS_Edge& edge : edges )
        {
          m_data.push_back( { edge, norm, gap, i } );

          ++i;
        }
      }

      void Define(edgeInfo* info,
                  sideInfo* sInfo)
      {
        m_shiftEdgeInfo = info;
        m_sideInfo      = sInfo;
      }

      bool Reject(const Bnd_Box& box) const
      {
        return m_shiftEdgeInfo->box.IsOut( box );
      }

      bool Accept(const int& index)
      {
        // Skip intersections check with the edge which we test.
        if ( *m_shiftEdgeInfo->seqIndex == index )
        {
          return false;
        }

        const edgeInfo& indexEdgeInfo = m_data[ index - 1 ];

        bool hasIntersection = false;

        try
        {
          OCC_CATCH_SIGNALS

          asiAlgo_IntersectionPointsCC iPoints;

          if ( !m_intersector( indexEdgeInfo.C3d,    indexEdgeInfo.f,    indexEdgeInfo.l,
                               m_shiftEdgeInfo->C3d, m_shiftEdgeInfo->f, m_shiftEdgeInfo->l,
                               Precision::Confusion(),
                               iPoints ) )
          {
            return false;
          }

          if ( iPoints.Size() == 0 )
          {
            return false;
          }

          for ( int i = 1; i <= iPoints.Size(); ++i )
          {
            const Handle(asiAlgo_IntersectionPointCC)& res = iPoints(i);

            if ( res->W2 < m_shiftEdgeInfo->f ||
                 res->W2 > m_shiftEdgeInfo->l )
            {
              continue;
            }

            // 'seqIndex' contains id of contour edge, while 'm_shiftEdgeInfo' itself
            // is a dump edge created to check intersections on the side of this contour edge.
            const double dist = m_data[ m_shiftEdgeInfo->seqIndex.value() - 1 ].midP.Distance( res->P );

            if ( !m_sideInfo->m_iDistance.has_value() ||
                  dist < *m_sideInfo->m_iDistance )
            {
              hasIntersection = true;

              m_sideInfo->m_iDistance  = dist;
              m_sideInfo->m_iEdgeIndex = index - 1;
              m_sideInfo->m_iEdgeParam = res->W1;
            }
          }
        }
        catch(...)
        {
          return false;
        }

        return hasIntersection;
      }


    public:

      asiAlgo_IntersectCC     m_intersector;
      std::vector< edgeInfo > m_data;
      edgeInfo*               m_shiftEdgeInfo;
      sideInfo*               m_sideInfo;

  };
}

//-----------------------------------------------------------------------------

asiAlgo_ComputeOutline::asiAlgo_ComputeOutline(const Handle(asiAlgo_AAG)& aag,
                                               ActAPI_ProgressEntry       progress,
                                               ActAPI_PlotterEntry        plotter)
  : ActAPI_IAlgorithm( progress, plotter ),
    m_aag( aag ),
    m_linearTolerance( Precision::Confusion() ),
    m_mode( Mode::Mode_Precise )
{}

//-----------------------------------------------------------------------------

asiAlgo_ComputeOutline::asiAlgo_ComputeOutline(const TopoDS_Shape&  shape,
                                               ActAPI_ProgressEntry progress,
                                               ActAPI_PlotterEntry  plotter)
  : asiAlgo_ComputeOutline( new asiAlgo_AAG( shape, false ), progress, plotter )
{}

//-----------------------------------------------------------------------------

enum checkingModes
{
  checkingModes_NoRayTracingForIntersectedCases = 0,
  checkingModes_RayTracingForIntersectedCases_NoMaterialOnly,
  checkingModes_RayTracingForIntersectedCases_All
};

void checkSide(asiAlgo_HitFacet&           hitFacets,
               boxBndTree&                 bbTree,
               Selector_FindIntersections& treeSelector,
               edgeInfo&                   edgeData,
               const bool                  isLeft,
               const gp_Dir&               dir,
               bool&                       isSomethingDone,
               const checkingModes         checkingMode,
               ActAPI_ProgressEntry        progress,
               ActAPI_PlotterEntry         plotter)
{
  sideInfo& thisSideInfo  = isLeft ? edgeData.sideInfo_Left  : edgeData.sideInfo_Right;
  sideInfo& otherSideInfo = isLeft ? edgeData.sideInfo_Right : edgeData.sideInfo_Left;

  if ( thisSideInfo.hasMaterial != sideInfo::HasMaterial_Undefined )
  {
    return;
  }

  tl::optional< double > raycastDist;

  const bool hasIntersection = thisSideInfo.m_iEdgeIndex.has_value();

  if ( thisSideInfo.m_iDone )
  {
    if ( hasIntersection )
    {
      if ( *thisSideInfo.m_iDistance < Precision::Confusion() )
      {
        return;
      }

      const edgeInfo& intersectedEdge = treeSelector.m_data[ *thisSideInfo.m_iEdgeIndex ];

      gp_Pnt iP;
      gp_Vec iV;

      intersectedEdge.C3d->D1( *thisSideInfo.m_iEdgeParam, iP, iV );

      if ( !iP.IsEqual( edgeData.midP, Precision::Confusion() ) )
      {
        gp_Vec V1( iP, edgeData.midP );

        const bool atLeftSide = iV.AngleWithRef( V1, dir ) > 0;

        sideInfo::HasMaterial intersectedEdgeMaterial = atLeftSide ? intersectedEdge.sideInfo_Left.hasMaterial
                                                                   : intersectedEdge.sideInfo_Right.hasMaterial;

#ifdef DRAW_DEBUG
        if ( DRAW_INDEX || DRAW_ALL )
        {
          debugInfoVec_Target.push_back( { edgeData.midP, edgeData.midV } );
          debugInfoVec_Intersected.push_back( { iP, iV.Normalized() } );
          debugInfoVec_FromTargetToIntersected.push_back( { edgeData.midP, V1.Reversed() } );
        }
#endif

        if ( intersectedEdgeMaterial != sideInfo::HasMaterial_Undefined )
        {
          thisSideInfo.hasMaterial = intersectedEdgeMaterial;

          if ( thisSideInfo.hasMaterial == sideInfo::HasMaterial_NoMaterial &&
               otherSideInfo.hasMaterial == sideInfo::HasMaterial_Undefined )
          {
            otherSideInfo.hasMaterial = sideInfo::HasMaterial_HasMaterial;
          }

          isSomethingDone = true;
        }
      }
    }
    else
    {
      raycastDist = 0.5;
    }
  }
  else
  {
    thisSideInfo.m_iDone = true;

    // Check that we do not intersect any other edge.
    TopoDS_Edge shiftEdge = BRepBuilderAPI_MakeEdge( edgeData.midP, edgeData.midP.Translated( thisSideInfo.sideShiftV ) );

    edgeInfo rayShiftEdge( shiftEdge, dir, edgeData.tolerance, edgeData.seqIndex );

    treeSelector.Define( &rayShiftEdge, &thisSideInfo );

    bbTree.Select( treeSelector );

    checkSide( hitFacets, bbTree, treeSelector, edgeData, isLeft, dir, isSomethingDone, checkingMode, progress, plotter );

    return;
  }

  if ( checkingMode != checkingModes::checkingModes_NoRayTracingForIntersectedCases &&
       thisSideInfo.hasMaterial == sideInfo::HasMaterial_Undefined &&
       thisSideInfo.m_iEdgeIndex.has_value() )
  {
    raycastDist = *thisSideInfo.m_iDistance * 0.5;
  }

  if ( raycastDist.has_value() )
  {
    if ( *raycastDist < edgeData.tolerance )
    {
#ifdef DRAW_DEBUG
      progress.SendLogMessage(LogInfo(Normal) << "Too small half dist for %1."
                                              << *edgeData.seqIndex);
#endif

      thisSideInfo.hasMaterial = sideInfo::HasMaterial_HasMaterial;

      isSomethingDone = true;
    }
    else
    {
      gp_Pnt rayP = edgeData.midP.Translated( thisSideInfo.sideShiftV.Scaled( *raycastDist ) );

      bool _hasHit = hasHit( hitFacets, rayP, dir ) ||
                     hasHit( hitFacets, rayP, dir.Reversed() );

      sideInfo::HasMaterial hasMetarial = _hasHit ? sideInfo::HasMaterial_HasMaterial
                                                  : sideInfo::HasMaterial_NoMaterial;

      if ( ( checkingMode == checkingModes::checkingModes_NoRayTracingForIntersectedCases && !hasIntersection ) ||
           ( checkingMode == checkingModes::checkingModes_RayTracingForIntersectedCases_NoMaterialOnly && hasMetarial == sideInfo::HasMaterial_NoMaterial ) ||
           ( checkingMode == checkingModes::checkingModes_RayTracingForIntersectedCases_All ) )
      {
        thisSideInfo.hasMaterial = hasMetarial;

        // Transfer recognition results to the other side if possible.
        if ( thisSideInfo.hasMaterial == sideInfo::HasMaterial_NoMaterial &&
             otherSideInfo.hasMaterial == sideInfo::HasMaterial_Undefined )
        {
          otherSideInfo.hasMaterial = sideInfo::HasMaterial_HasMaterial;
        }

        isSomethingDone = true;
      }

#ifdef DRAW_DEBUG
      if ( DRAW_INDEX || DRAW_ALL )
      {
        std::string rayPV    = "rayPV"    + isLeft ? "_L_" : "_R_" + *edgeData.seqIndex;
        std::string rayShift = "rayShift" + isLeft ? "_L_" : "_R_" + *edgeData.seqIndex;

        if ( _hasHit )
        {
          debugInfoVec_Rays_Red.push_back( { rayP, dir } );
        }
        else
        {
          debugInfoVec_Rays_Green.push_back( { rayP, dir } );
        }

        debugInfoVec_Shift.push_back( { edgeData.midP, thisSideInfo.sideShiftV.Scaled( *raycastDist ) } );
      }
#endif
    }
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_ComputeOutline::Perform(const gp_Dir&    _dir,
                                     TopoDS_Compound& outlineWires,
                                     const Mode       mode)
{
#ifdef DRAW_DEBUG
  debugInfoVec_Shift.clear();
  debugInfoVec_Rays_Red.clear();
  debugInfoVec_Rays_Green.clear();

  debugInfoVec_Target.clear();
  debugInfoVec_Intersected.clear();
  debugInfoVec_FromTargetToIntersected.clear();
#endif

#ifdef dirCoords
  gp_Dir dir( dirCoords );
#else
  gp_Dir dir( _dir );
#endif

  m_progress.SendLogMessage(LogInfo(Normal) << "Direction of projection for HLR Outline is: %1."
                                            << asiAlgo_Utils::Json::FromDirAsTuple( dir ) );

  Handle(asiAlgo_BVHFacets) bvh = new asiAlgo_BVHFacets( m_aag->GetMasterShape() );

  // Avoid execution without facets.
  if ( bvh->Size() == 0 )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to build outline for the shape without facets.");

    return false;
  }

  //-----------------------------------------------------------------------------
  // Build projection.
  //-----------------------------------------------------------------------------

  TopoDS_Shape hlrResult;
  TopoDS_Shape features;

  {
    switch ( mode )
    {
      case Mode_Precise:
      {
        if ( !makeProjection( m_aag->GetMasterShape(), dir, m_facesToExclude, hlrResult, features ) )
        {
          return false;
        }

        break;
      }
      case Mode_Discrete:
      {
        if ( !makeProjectionDiscr( m_aag->GetMasterShape(), dir, m_facesToExclude, hlrResult, features ) )
        {
          return false;
        }

        break;
      }
      default:
        break;
    }
  }

#ifdef DRAW_DEBUG
  m_plotter.REDRAW_SHAPE( "hlrResult", hlrResult );
  //m_plotter.REDRAW_SHAPE( "features",  features  );
#endif

  //-----------------------------------------------------------------------------
  // Exclude recognized features.
  //-----------------------------------------------------------------------------

  if ( !features.IsNull() )
  {
    BRepAlgoAPI_Cut API;
    asiAlgo_Utils::BooleanCut( hlrResult, features, false, 0.0, API );

    hlrResult = API.Shape();
  }

#ifdef DRAW_DEBUG
  //m_plotter.REDRAW_SHAPE( "hlrResult_cut", hlrResult );
#endif

  //-----------------------------------------------------------------------------
  // Split edges by start/end points.
  //-----------------------------------------------------------------------------

  Handle(asiAlgo_SplitEdgesByStartEndPoints) splitAlgo = new asiAlgo_SplitEdgesByStartEndPoints( m_progress, m_plotter );

  std::vector< TopoDS_Edge > edges;

  for ( TopExp_Explorer exp( hlrResult, TopAbs_EDGE ); exp.More(); exp.Next() )
  {
    edges.push_back( TopoDS::Edge( exp.Value() ) );
  }

  splitAlgo->SetMinAllowedDistFromEnds( m_linearTolerance );
  splitAlgo->SetMaxDistFromPointToCurve( m_linearTolerance );

  splitAlgo->Perform( edges );

#ifdef DRAW_DEBUG
  //drawEdges( edges, "edges_after_split", m_plotter );
#endif

  //-----------------------------------------------------------------------------
  // Remove too short edges.
  //-----------------------------------------------------------------------------

  {
    edges.erase( std::remove_if( edges.begin(),
                                 edges.end(),
                                 [&](const TopoDS_Edge& edge)
    {
      GProp_GProps props;
      BRepGProp::LinearProperties( edge, props );

      return Abs( props.Mass() ) < m_linearTolerance;
    } ), edges.end() );
  }

#ifdef DRAW_DEBUG
  //drawEdges( edges, "edges_removed_short", m_plotter );
#endif

  //-----------------------------------------------------------------------------
  // For fully overlapped edges keep only a single instance of such edge.
  //-----------------------------------------------------------------------------

  Handle(asiAlgo_FixOverlappedEdges) overlappedAlgo = new asiAlgo_FixOverlappedEdges( m_progress, m_plotter );

  overlappedAlgo->SetMaxAllowedDistance( m_linearTolerance );

  overlappedAlgo->Perform( edges );

#ifdef DRAW_DEBUG
  //drawEdges( edges, "edges_removed_overlapped", m_plotter );
#endif

  //-----------------------------------------------------------------------------
  // Prepare intersections checker.
  //-----------------------------------------------------------------------------

  if ( edges.empty() )
  {
    return false;
  }

  // Fill the tree by edges prepared for recognition.
  boxBndTree   bbTree;
  boxBndFiller treeFiller( bbTree );

  Selector_FindIntersections treeSelector( edges, dir, m_linearTolerance );

  {
#ifdef DRAW_DEBUG
    Handle(HRealArray)   coords = new HRealArray  ( 0, (int) treeSelector.m_data.size() * 3 - 1, 0. );
    Handle(HStringArray) labels = new HStringArray( 0, (int) treeSelector.m_data.size() - 1 );

    int coordIdx = 0;
    int lblIdx = 0;
#endif

    for ( auto& edgeData : treeSelector.m_data )
    {
      treeFiller.Add( *edgeData.seqIndex, edgeData.box );

#ifdef DRAW_DEBUG
      coords->ChangeValue(coordIdx)     = edgeData.midP.X();
      coords->ChangeValue(coordIdx + 1) = edgeData.midP.Y();
      coords->ChangeValue(coordIdx + 2) = edgeData.midP.Z();

      labels->ChangeValue(lblIdx++)     = *edgeData.seqIndex;

      coordIdx += 3;
#endif
    }

#ifdef DRAW_DEBUG
    m_plotter.REDRAW_LABELS( "indices", coords, labels, Color_White );
#endif

    treeFiller.Fill();
  }

  //-----------------------------------------------------------------------------
  // Filter edges using ray-casting.
  //-----------------------------------------------------------------------------

  asiAlgo_HitFacet hitFacets( bvh );

  // Try to recognize edges status - 'border' or 'body'.
  bool isSomethingDone = false;

  std::vector< checkingModes > modes = {
    checkingModes::checkingModes_NoRayTracingForIntersectedCases,
    checkingModes::checkingModes_RayTracingForIntersectedCases_NoMaterialOnly,
    checkingModes::checkingModes_RayTracingForIntersectedCases_All,
  };

  for ( const auto& checkingMode : modes )
  {
    do
    {
      isSomethingDone = false;

      for ( auto& edgeData : treeSelector.m_data )
      {
        if ( edgeData.status() != edgeInfo::Status_Undefined )
        {
          continue;
        }

        // Check left side.
        checkSide( hitFacets, bbTree, treeSelector, edgeData, true, dir, isSomethingDone, checkingMode, m_progress, m_plotter );

        // Check right side.
        checkSide( hitFacets, bbTree, treeSelector, edgeData, false, dir, isSomethingDone, checkingMode, m_progress, m_plotter );
      }
    }
    while( isSomethingDone );
  }

#ifdef DRAW_DEBUG

  drawDebugInfoVec( "shift",      Color_Magenta, debugInfoVec_Shift,      m_plotter, true, false );
  drawDebugInfoVec( "rays_Hit",   Color_Red,     debugInfoVec_Rays_Red,   m_plotter );
  drawDebugInfoVec( "rays_Empty", Color_Green,   debugInfoVec_Rays_Green, m_plotter );

  drawDebugInfoVec( "target",        Color_Blue,   debugInfoVec_Target,                  m_plotter );
  drawDebugInfoVec( "intersected",   Color_Yellow, debugInfoVec_Intersected,             m_plotter );
  drawDebugInfoVec( "toIntersected", Color_Orange, debugInfoVec_FromTargetToIntersected, m_plotter, true, false );

  std::vector< TopoDS_Edge > totallyGoodEdges;
  std::vector< TopoDS_Edge > totallyBadEdges;
  std::vector< TopoDS_Edge > otherEdges;

  {
    int i = 0;

    for ( auto& edgeData : treeSelector.m_data )
    {
      if ( edgeData.status() == edgeInfo::Status_Body )
      {
        totallyBadEdges.push_back( edges[i] );
      }
      else if ( edgeData.status() == edgeInfo::Status_Border )
      {
        totallyGoodEdges.push_back( edges[i] );
      }
      else
      {
        otherEdges.push_back( edges[i] );
      }

      i++;
    }
  }

  drawEdges( otherEdges,       "other",            m_plotter, Color_Yellow );
  drawEdges( totallyBadEdges,  "totallyBadEdges",  m_plotter, Color_Red    );
  drawEdges( totallyGoodEdges, "totallyGoodEdges", m_plotter, Color_Green  );

#endif

  // Do we have anything?
  std::vector< TopoDS_Edge > filteredEdges;

  {
    for ( auto& edgeData : treeSelector.m_data )
    {
      if ( edgeData.status() == edgeInfo::Status_Border )
      {
        filteredEdges.push_back( edges[ *edgeData.seqIndex - 1 ] );
      }
    }
  }

  if ( filteredEdges.empty() )
  {
    return false;
  }

  //-----------------------------------------------------------------------------
  // Create outlines.
  //-----------------------------------------------------------------------------

  // Connect edges to wires.
  std::vector< TopoDS_Wire > outlineWiresVec;

  if ( asiAlgo_Utils::ConnectEdgesToWires( filteredEdges,
                                           false,
                                           outlineWiresVec,
                                           m_linearTolerance ) )
  {
    if ( outlineWiresVec.empty() )
    {
      return false;
    }

    // Fix wires.
    for ( auto& W : outlineWiresVec )
    {
      ShapeFix_Wire fixWire;
      fixWire.Load( W );

      fixWire.FixIntersectingEdgesMode() = 1;
      fixWire.FixSelfIntersectingEdgeMode() = 1;
      fixWire.FixSelfIntersectionMode() = 1;
      fixWire.FixSmallMode() = 1;

      if ( fixWire.Perform() )
      {
        W = fixWire.WireAPIMake();
      }
    }

    // Extract outlines.
    TopoDS_Compound WC;

    BRep_Builder bb;
    bb.MakeCompound(WC);

    for ( auto& W : outlineWiresVec )
    {
      bb.Add( WC, W );
    }

#ifdef DRAW_DEBUG
    //m_plotter.REDRAW_SHAPE( "wires", WC );
#endif

    outlineWires = WC;

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------

void asiAlgo_ComputeOutline::ExcludeFaces(const asiAlgo_Feature& ids)
{
  m_facesToExclude = ids;
}

//-----------------------------------------------------------------------------

void asiAlgo_ComputeOutline::ExcludeFaces(const TopoDS_Shape& faces)
{
  for ( TopExp_Explorer it( faces, TopAbs_FACE ); it.More(); it.Next() )
  {
    m_facesToExclude.Add( m_aag->GetFaceId( TopoDS::Face( it.Current() ) ) );
  }
}
