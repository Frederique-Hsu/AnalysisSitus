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
#include "asiAlgo_Utils.h"
#include "asiAlgo_BVHFacets.h"
#include "asiAlgo_FeatureFaces.h"
#include "asiAlgo_HitFacet.h"
#include "asiAlgo_HlrPreciseAlgo.h"
#include "asiAlgo_HlrToShape.h"
#include "asiAlgo_Timer.h"

// OCCT includes
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Tools.hxx>
#include <BOPDS_DS.hxx>
#include <BOPDS_PDS.hxx>
#include <BRep_Builder.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepLib.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Ellipse.hxx>
#include <HLRBRep_Data.hxx>
#include <HLRBRep_PolyAlgo.hxx>
#include <HLRBRep_PolyHLRToShape.hxx>
#include <NCollection_UBTreeFiller.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeFix_Wire.hxx>
#include <TopExp_Explorer.hxx>

#define DRAW_DEBUG
#undef DRAW_DEBUG

#ifdef DRAW_DEBUG
  #define dirCoords -0.123775, 1.36516e-17, -0.99231
  static std::set< int > toDraw = { -1 }; // '-1' to draw all vectors;
                                          // 'empty' to skip drawing;
                                          // 'custom indices' to draw vectors for specific edges.

  #define DRAW_INDEX toDraw.count( edgeData->index )
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

#define InterCurveCurveTol   Precision::Confusion()
#define OverlappingTol       Precision::Confusion()
#define BBGap                1.0
#define MinDistForRaycasting 0.1
#define EdgesToWiresTol      m_linearTolerance

//-----------------------------------------------------------------------------

using namespace asiAlgo;
using namespace asiAlgo::algo;

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

  bool makeProjection(const Handle(asiAlgo_AAG)& aag,
                      const asiAlgo_Feature&     shapeFaces,
                      const gp_Ax1&              ax,
                      const asiAlgo_Feature&     featureFaces,
                      TopoDS_Shape&              hlrResult,
                      TopoDS_Shape&              featureFacesResult,
                      Handle(BRepTools_History)& history)
  {
    gp_Ax2 transform( gp::Origin(), ax.Direction() );
    HLRAlgo_Projector projector( transform );

    TopoDS_Compound visible;
    BRep_Builder().MakeCompound(visible);

    TopoDS_Compound FC;
    BRep_Builder().MakeCompound(FC);
    TopTools_IndexedMapOfShape fcMap;

    for (asiAlgo_Feature::Iterator itSF(shapeFaces); itSF.More(); itSF.Next())
    {
      const int fid = itSF.Key();

      const TopoDS_Face& face = aag->GetFace(fid);

      Handle(hlr::PreciseAlgo) brep_hlr = new hlr::PreciseAlgo();
      brep_hlr->Add(face);

      try
      {
        brep_hlr->Projector(projector);
        brep_hlr->Update();
        brep_hlr->Hide();
      }
      catch (...)
      {
        return false;
      }

      hlr::HlrToShape shapes(brep_hlr);

      TopoDS_Shape V, V1, VN, VO, VI;

#ifdef INCLUDE_HIDDEN_LINES
      TopoDS_Shape H, H1, HN, HO, HI;
#endif

      try
      {
        V = build3dCurves(shapes.VCompound()); // "hard edges" visible
        V1 = build3dCurves(shapes.Rg1LineVCompound()); // "smooth edges" visible
        VN = build3dCurves(shapes.RgNLineVCompound()); // "contour edges" visible
        VO = build3dCurves(shapes.OutLineVCompound()); // "outline" visible
        VI = build3dCurves(shapes.IsoLineVCompound()); // "isolines" visible (precise HLR only)

#ifdef INCLUDE_HIDDEN_LINES
        H = build3dCurves(shapes.HCompound()); // "hard edges" hidden
        H1 = build3dCurves(shapes.Rg1LineHCompound()); // "smooth edges" hidden
        HN = build3dCurves(shapes.RgNLineHCompound()); // "contour edges" hidden
        HO = build3dCurves(shapes.OutLineHCompound()); // "outline" hidden
        HI = build3dCurves(shapes.IsoLineHCompound()); // "isolines" hidden (precise HLR only)
#endif
      }
      catch (...)
      {
        return false;
      }

      if (!V.IsNull())
        BRep_Builder().Add(visible, V);

      if (!V1.IsNull())
        BRep_Builder().Add(visible, V1);

      if (!VN.IsNull())
        BRep_Builder().Add(visible, VN);

      if (!VO.IsNull())
        BRep_Builder().Add(visible, VO);

      if (!VI.IsNull())
        BRep_Builder().Add(visible, VI);

#ifdef INCLUDE_HIDDEN_LINES
      if (!H.IsNull())
        BRep_Builder().Add(visible, H);

      if (!H1.IsNull())
        BRep_Builder().Add(visible, H1);

      if (!HN.IsNull())
        BRep_Builder().Add(visible, HN);

      if (!HO.IsNull())
        BRep_Builder().Add(visible, HO);

      if (!HI.IsNull())
        BRep_Builder().Add(visible, HI);
#endif

      for (TopExp_Explorer expE(V, TopAbs_EDGE); expE.More(); expE.Next())
      {
        const TopoDS_Edge& edge = TopoDS::Edge(expE.Value());
        history->AddGenerated(face, edge);
      }

      if ( featureFaces.Contains(fid) )
      {
        Handle(HLRBRep_Data) ds = brep_hlr->DataStructure();
        TopTools_IndexedMapOfShape& dsFaces = ds->FaceMap();

        TopoDS_Compound facesComp;
        BRep_Builder().MakeCompound( facesComp );
        BRep_Builder().Add( facesComp, dsFaces(1));

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

    gp_Trsf T;
    T.SetTransformation(gp_Ax3(transform));
    T.Invert();
    T.SetTranslationPart(ax.Location().XYZ());

    {
      Handle(BRepTools_History) historyOfTrsf = new BRepTools_History();
      bool isDoneTransf = true;
      TopoDS_Compound comp;
      BRep_Builder().MakeCompound(comp);
      for (TopExp_Explorer exp(visible, TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());

        TopoDS_Shape shapeWithTrsf = edge.Moved(T);
        BRepBuilderAPI_Transform transf(shapeWithTrsf.Location().Transformation());
        transf.Perform(shapeWithTrsf.Located(TopLoc_Location()), true);
        if (!transf.IsDone())
        {
          isDoneTransf = false;
          break;
        }

        shapeWithTrsf = transf.Shape();
        for (TopExp_Explorer expSWT(shapeWithTrsf, TopAbs_EDGE); expSWT.More(); expSWT.Next())
        {
          const TopoDS_Edge& edgeWithTrsf = TopoDS::Edge(expSWT.Value());
          BRep_Builder().Add(comp, edgeWithTrsf);
          historyOfTrsf->AddModified(edge, edgeWithTrsf);
        }
      }

      if (!isDoneTransf)
      {
        historyOfTrsf = new BRepTools_History();
        comp = TopoDS_Compound();
        BRep_Builder().MakeCompound(comp);

        for (TopExp_Explorer exp(visible, TopAbs_EDGE); exp.More(); exp.Next())
        {
          const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());
          TopoDS_Edge edgeTrsf = TopoDS::Edge(edge.Moved(T));

          BRep_Builder().Add(comp, edgeTrsf);
          historyOfTrsf->AddModified(edge, edgeTrsf);
        }
      }

      history->Merge(historyOfTrsf);
      hlrResult = comp;
    }


    {
      bool isDoneTransf = true;
      TopoDS_Compound comp;
      BRep_Builder().MakeCompound(comp);
      for (TopExp_Explorer exp(FC, TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());

        TopoDS_Shape shapeWithTrsf = edge.Moved(T);
        BRepBuilderAPI_Transform transf(shapeWithTrsf.Location().Transformation());
        transf.Perform(shapeWithTrsf.Located(TopLoc_Location()), true);
        if (!transf.IsDone())
        {
          isDoneTransf = false;
          break;
        }

        shapeWithTrsf = transf.Shape();
        for (TopExp_Explorer expSWT(shapeWithTrsf, TopAbs_EDGE); expSWT.More(); expSWT.Next())
        {
          const TopoDS_Edge& edgeWithTrsf = TopoDS::Edge(expSWT.Value());
          BRep_Builder().Add(comp, edgeWithTrsf);
        }
      }

      if (!isDoneTransf)
      {
        comp = TopoDS_Compound();
        BRep_Builder().MakeCompound(comp);

        for (TopExp_Explorer exp(FC, TopAbs_EDGE); exp.More(); exp.Next())
        {
          const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());
          TopoDS_Edge edgeTrsf = TopoDS::Edge(edge.Moved(T));

          BRep_Builder().Add(comp, edgeTrsf);
        }
      }

      featureFacesResult = comp;
    }

    return true;
  }

  //-----------------------------------------------------------------------------

  bool makeProjectionDiscr(const Handle(asiAlgo_AAG)& aag,
                           const asiAlgo_Feature&     shapeFaces,
                           const gp_Ax1&              ax,
                           const asiAlgo_Feature&     featureFaces,
                           TopoDS_Shape&              hlrResult,
                           TopoDS_Shape&              featureFacesResult,
                           Handle(BRepTools_History)& history)
  {
    gp_Ax2 transform( gp::Origin(), ax.Direction() );
    HLRAlgo_Projector projector( transform );

    TopoDS_Compound visible;
    BRep_Builder().MakeCompound(visible);

    TopoDS_Compound FC;
    BRep_Builder().MakeCompound(FC);
    TopTools_IndexedMapOfShape fcMap;

    for (asiAlgo_Feature::Iterator itSF(shapeFaces); itSF.More(); itSF.Next())
    {
      const int fid = itSF.Key();

      const TopoDS_Face& face = aag->GetFace(fid);

      Handle(HLRBRep_PolyAlgo) polyAlgo = new HLRBRep_PolyAlgo;

      try
      {
        polyAlgo->Projector(projector);
        polyAlgo->Load(face);
        polyAlgo->Update();
      }
      catch (...)
      {
        // Sometimes crashes.
        return false;
      }

      HLRBRep_PolyHLRToShape shapes;

      try
      {
        shapes.Update(polyAlgo);
      }
      catch (...)
      {
        // Sometimes crashes.
        return false;
      }

      TopoDS_Shape V, V1, VN, VO;

#ifdef INCLUDE_HIDDEN_LINES
      TopoDS_Shape H, H1, HN, HO;
#endif

      V  = build3dCurves(shapes.VCompound());        // "hard edges" visible
      V1 = build3dCurves(shapes.Rg1LineVCompound()); // "smooth edges" visible
      VN = build3dCurves(shapes.RgNLineVCompound()); // "contour edges" visible
      VO = build3dCurves(shapes.OutLineVCompound()); // "outline" visible

#ifdef INCLUDE_HIDDEN_LINES
      H  = build3dCurves(shapes.HCompound());        // "hard edges" hidden
      H1 = build3dCurves(shapes.Rg1LineHCompound()); // "smooth edges" hidden
      HN = build3dCurves(shapes.RgNLineHCompound()); // "contour edges" hidden
      HO = build3dCurves(shapes.OutLineHCompound()); // "outline" hidden
#endif

      if (!V.IsNull())
        BRep_Builder().Add(visible, V);

      if (!V1.IsNull())
        BRep_Builder().Add(visible, V1);

      if (!VN.IsNull())
        BRep_Builder().Add(visible, VN);

      if (!VO.IsNull())
        BRep_Builder().Add(visible, VO);


#ifdef INCLUDE_HIDDEN_LINES
      if (!H.IsNull())
        BRep_Builder().Add(visible, H);

      if (!H1.IsNull())
        BRep_Builder().Add(visible, H1);

      if (!HN.IsNull())
        BRep_Builder().Add(visible, HN);

      if (!HO.IsNull())
        BRep_Builder().Add(visible, HO);
#endif

      for (TopExp_Explorer expE(V, TopAbs_EDGE); expE.More(); expE.Next())
      {
        const TopoDS_Edge& edge = TopoDS::Edge(expE.Value());
        history->AddGenerated(face, edge);
      }

      if ( featureFaces.Contains(fid) )
      {
        TopTools_IndexedMapOfShape fMap;
        TopExp::MapShapes(face, TopAbs_FACE, fMap);

        TopoDS_Compound facesComp;
        BRep_Builder().MakeCompound(facesComp);

        asiAlgo_Feature::Iterator fiter(featureFaces);
        for (; fiter.More(); fiter.Next())
        {
          BRep_Builder().Add(facesComp, fMap(1));
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

    gp_Trsf T;
    T.SetTransformation(gp_Ax3(transform));
    T.Invert();
    T.SetTranslationPart(ax.Location().XYZ());

    {
      Handle(BRepTools_History) historyOfTrsf = new BRepTools_History();
      bool isDoneTransf = true;
      TopoDS_Compound comp;
      BRep_Builder().MakeCompound(comp);
      for (TopExp_Explorer exp(visible, TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());

        TopoDS_Shape shapeWithTrsf = edge.Moved(T);
        BRepBuilderAPI_Transform transf(shapeWithTrsf.Location().Transformation());
        transf.Perform(shapeWithTrsf.Located(TopLoc_Location()), true);
        if (!transf.IsDone())
        {
          isDoneTransf = false;
          break;
        }

        shapeWithTrsf = transf.Shape();
        for (TopExp_Explorer expSWT(shapeWithTrsf, TopAbs_EDGE); expSWT.More(); expSWT.Next())
        {
          const TopoDS_Edge& edgeWithTrsf = TopoDS::Edge(expSWT.Value());
          BRep_Builder().Add(comp, edgeWithTrsf);
          historyOfTrsf->AddModified(edge, edgeWithTrsf);
        }
      }

      if (!isDoneTransf)
      {
        historyOfTrsf = new BRepTools_History();
        comp = TopoDS_Compound();
        BRep_Builder().MakeCompound(comp);

        for (TopExp_Explorer exp(visible, TopAbs_EDGE); exp.More(); exp.Next())
        {
          const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());
          TopoDS_Edge edgeTrsf = TopoDS::Edge(edge.Moved(T));

          BRep_Builder().Add(comp, edgeTrsf);
          historyOfTrsf->AddModified(edge, edgeTrsf);
        }
      }

      history->Merge(historyOfTrsf);
      hlrResult = comp;
    }


    {
      bool isDoneTransf = true;
      TopoDS_Compound comp;
      BRep_Builder().MakeCompound(comp);
      for (TopExp_Explorer exp(FC, TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());

        TopoDS_Shape shapeWithTrsf = edge.Moved(T);
        BRepBuilderAPI_Transform transf(shapeWithTrsf.Location().Transformation());
        transf.Perform(shapeWithTrsf.Located(TopLoc_Location()), true);
        if (!transf.IsDone())
        {
          isDoneTransf = false;
          break;
        }

        shapeWithTrsf = transf.Shape();
        for (TopExp_Explorer expSWT(shapeWithTrsf, TopAbs_EDGE); expSWT.More(); expSWT.Next())
        {
          const TopoDS_Edge& edgeWithTrsf = TopoDS::Edge(expSWT.Value());
          BRep_Builder().Add(comp, edgeWithTrsf);
        }
      }

      if (!isDoneTransf)
      {
        comp = TopoDS_Compound();
        BRep_Builder().MakeCompound(comp);

        for (TopExp_Explorer exp(FC, TopAbs_EDGE); exp.More(); exp.Next())
        {
          const TopoDS_Edge& edge = TopoDS::Edge(exp.Value());
          TopoDS_Edge edgeTrsf = TopoDS::Edge(edge.Moved(T));

          BRep_Builder().Add(comp, edgeTrsf);
        }
      }

      featureFacesResult = comp;
    }

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

  struct intersectionInfo
  {
    tl::optional< double > distance;
    tl::optional< double > param;

    tl::optional< int    > otherIndex;
    tl::optional< double > otherParam;

    intersectionInfo()
    {
    }

    intersectionInfo(const int     index,
                     const double& thisParam,
                     const double& otherParam)
      : param( thisParam ),
        otherIndex( index ),
        otherParam( otherParam )
    {
    }
  };

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

    gp_Pnt shiftP;
    gp_Vec shiftV;

    bool isIOk;
    intersectionInfo iInfo;

    sideInfo()
      : hasMaterial( HasMaterial_Undefined ),
        isIOk( true )
    {
    }
  };

  //-----------------------------------------------------------------------------

  class edgeInfo : public Standard_Transient
  {
    public:

      enum Status
      {
        Status_Undefined = 0,
        Status_Border,
        Status_Body,
        Status_Dangling
      };

    public:

      int index;

      TopoDS_Edge E;

      bool isShiftEdge;

      sideInfo sideInfo_Left;
      sideInfo sideInfo_Right;

      Bnd_Box box;

      // 3D parameters.
      double f, l;
      Handle(Geom_Curve) C3d;

      // 2D parameters.
      double f2d, l2d;
      Handle(Geom2d_Curve) C2d;

      bool isReversed2d;

      // Information about intersections.
      std::vector< intersectionInfo > intersected2DParams;


    public:

      // Constructor.
      edgeInfo(const TopoDS_Edge&        e,
               const Handle(Geom_Plane)& plane,
               const int                 i)
        : Standard_Transient (),
          index( i ),
          isShiftEdge( false ),
          E( e )
      {
        // Get bounding boxes.
        {
          BRepBndLib::AddOptimal( E, box, false, false );

          box.SetGap( BBGap );
        }

        // Get edges's parameters in 3D.
        {
          C3d = BRep_Tool::Curve( E, f, l );

          // Default points and vectors.
          gp_Pnt midP;
          gp_Vec midV;

          C3d->D1( ( f + l ) * 0.5, midP, midV );

          if ( midV.Magnitude() < Precision::Confusion() )
          {
            // Try to move along curve to find another place for ray casting.
            GeomAdaptor_Curve adaptor( C3d );

            GCPnts_UniformAbscissa discretizer( adaptor, 21 );

            if ( !discretizer.IsDone() ||
                  discretizer.NbPoints() == 0 )
            {
              return;
            }

            std::vector< double > params;

            const int nbPoints = discretizer.NbPoints();
            //
            for ( int j = 2; j < nbPoints; ++j )
            {
              const double param = discretizer.Parameter(j);

              C3d->D1( param, midP, midV );

              if ( midV.Magnitude() < Precision::Confusion() )
              {
                continue;
              }

              break;
            }
          }

          sideInfo_Left.shiftP  = midP;
          sideInfo_Right.shiftP = midP;

          sideInfo_Left.shiftV  = midV.Rotated( gp_Ax1( midP, plane->Axis().Direction() ), M_PI / 2. ).Normalized();
          sideInfo_Right.shiftV = sideInfo_Left.shiftV.Reversed();
        }

        // Get edges's parameters in 2D.
        {
          Handle(Geom2d_Curve) curve2d = BRep_Tool::CurveOnPlane( E, plane, TopLoc_Location(), f2d, l2d );
          C2d = new Geom2d_TrimmedCurve( curve2d, f2d, l2d );

          isReversed2d = f2d > l2d;
        }
      }

      //-----------------------------------------------------------------------------

      Status Status()
      {
        if ( sideInfo_Left.hasMaterial  == sideInfo::HasMaterial_Undefined ||
             sideInfo_Right.hasMaterial == sideInfo::HasMaterial_Undefined )
        {
          return Status_Undefined;
        }

        if ( sideInfo_Left.hasMaterial  == sideInfo::HasMaterial_NoMaterial &&
             sideInfo_Right.hasMaterial == sideInfo::HasMaterial_NoMaterial )
        {
          return Status_Dangling;
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

  void drawEdges(const std::vector< Handle(edgeInfo) >& edges,
                 const std::string&                     name,
                 ActAPI_PlotterEntry                    plotter,
                 ActAPI_Color                           color = Color_Pink,
                 const bool                             toSeparate = false)
  {
    if ( edges.empty() )
    {
      return;
    }

    TopoDS_Compound C;

    BRep_Builder bb;
    bb.MakeCompound(C);

    for ( const Handle(edgeInfo)& edge : edges )
    {
      bb.Add( C, edge->E );

      if ( toSeparate )
      {
        std::string nameSep = name;
        nameSep += "_";
        nameSep += std::to_string( edge->index );

        plotter.REDRAW_SHAPE( nameSep.c_str(), edge->E, color );
      }
    }

    if ( !toSeparate )
    {
      plotter.REDRAW_SHAPE( name.c_str(), C, color );
    }
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

  class Selector_FindIntersections : public boxBndTree::Selector
  {
    public:

      Selector_FindIntersections(std::vector< Handle(edgeInfo) >& edges,
                                 ActAPI_PlotterEntry              plotter)
        : m_targetEdgeInfo ( nullptr ),
          m_data ( edges ),
          m_plotter ( plotter )
      {
      }

      void Define(edgeInfo* info)
      {
        m_targetEdgeInfo = info;
      }

      bool Reject(const Bnd_Box& box) const
      {
        return m_targetEdgeInfo->box.IsOut( box );
      }

      bool Accept(const int& index)
      {
#ifdef DRAW_DEBUG
        const int id = m_targetEdgeInfo->index;
#endif
        // Skip intersections check with the edge which we test.
        if ( m_targetEdgeInfo->index == index - 1 )
        {
          return false;
        }

        return checkIntersection( m_targetEdgeInfo, m_data[ index - 1 ] );
      }

    private:

      bool checkIntersection(Handle(edgeInfo)& edge,
                             Handle(edgeInfo)& otherEdge)
      {
        if ( edge->C2d.IsNull() ||
             otherEdge->C2d.IsNull() )
        {
          return false;
        }

        bool hasIntersections = false;

        try
        {
          OCC_CATCH_SIGNALS

          Geom2dAPI_InterCurveCurve icc( edge->C2d, otherEdge->C2d, InterCurveCurveTol );

          int nbPnts     = icc.NbPoints();
          int nbSegments = icc.NbSegments();

          // They are not intersected with given tolerance.
          if ( nbPnts == 0 && nbSegments == 0 )
          {
            return false;
          }

          // Get the list of intersection parameters of curves.
          const Geom2dInt_GInter& intersector = icc.Intersector();

          // Check point intersections.
          for ( int i = 1; i <= icc.NbPoints(); ++i )
          {
            const IntRes2d_IntersectionPoint& p =
              intersector.Point( i );

            if ( checkParam( p.ParamOnFirst(), p.ParamOnSecond(), edge, otherEdge ) )
            {
              hasIntersections = true;
            }
          }

          // Check segment intersections.
          for ( int i = 1; i <= icc.NbSegments(); ++i )
          {
            const IntRes2d_IntersectionSegment& seg =
              intersector.Segment( i );

            if ( seg.HasFirstPoint() )
            {
              if ( checkParam( seg.FirstPoint().ParamOnFirst(), seg.FirstPoint().ParamOnSecond(), edge, otherEdge ) )
              {
                hasIntersections = true;
              }
            }

            if ( seg.HasLastPoint() )
            {
              if ( checkParam( seg.LastPoint().ParamOnFirst(), seg.LastPoint().ParamOnSecond(), edge, otherEdge ) )
              {
                hasIntersections = true;
              }
            }
          }
        }
        catch ( ... )
        {
          return false;
        }

        return hasIntersections;
      }

      //-----------------------------------------------------------------------------

      bool checkParam(const double      T,
                      const double      otherT,
                      Handle(edgeInfo)& tOwnerEdge,
                      Handle(edgeInfo)& otherEdge)
      {
        // Check that intersection point on the edge.
        const bool isOnEdge = tOwnerEdge->isReversed2d ? ( T > tOwnerEdge->l2d - Precision::Confusion() && T < tOwnerEdge->f2d + Precision::Confusion() )
                                                       : ( T > tOwnerEdge->f2d - Precision::Confusion() && T < tOwnerEdge->l2d + Precision::Confusion() );

        if ( isOnEdge )
        {
          tOwnerEdge->intersected2DParams.push_back( { otherEdge->index, T, otherT } );

          return true;
        }

        return false;
      }

    public:

      std::vector< Handle(edgeInfo) >& m_data;
      Handle(edgeInfo)                 m_targetEdgeInfo;
      ActAPI_PlotterEntry              m_plotter;

  };
}

//-----------------------------------------------------------------------------

asiAlgo_ComputeOutline::asiAlgo_ComputeOutline(const Handle(asiAlgo_AAG)& aag,
                                               ActAPI_ProgressEntry       progress,
                                               ActAPI_PlotterEntry        plotter)
  : ActAPI_IAlgorithm( progress, plotter ),
    m_aag( aag ),
    m_linearTolerance( Precision::Confusion() ),
    m_mode( Mode::Mode_Precise ),
    m_history( new BRepTools_History ),
    m_includeDanglingEdges( true ),
    m_fuzzyValue(0.01)
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

//-----------------------------------------------------------------------------

void checkSide(asiAlgo_HitFacet&           hitFacets,
               Selector_FindIntersections& treeSelector,
               Handle(edgeInfo)&           edgeData,
               const bool                  isLeft,
               const Handle(Geom_Plane)&   plane,
               bool&                       isSomethingDone,
               const checkingModes         checkingMode,
               ActAPI_ProgressEntry        progress,
               ActAPI_PlotterEntry         plotter)
{
  sideInfo& thisSideInfo  = isLeft ? edgeData->sideInfo_Left
                                   : edgeData->sideInfo_Right;

  if ( thisSideInfo.hasMaterial != sideInfo::HasMaterial_Undefined )
  {
    return;
  }

  if ( thisSideInfo.isIOk != true )
  {
    thisSideInfo.hasMaterial = sideInfo::HasMaterial_HasMaterial;

    isSomethingDone = true;

    return;
  }

  // Try to get information from the intersected edge.
  tl::optional< double > raycastDist;

  const bool hasIntersection = thisSideInfo.iInfo.distance.has_value();

  if ( hasIntersection )
  {
    const Handle(edgeInfo)& intersectedEdge = treeSelector.m_data[ *thisSideInfo.iInfo.otherIndex ];

    // Convert intersection 2D point to 3D space.
    gp_Pnt2d P2d;

    intersectedEdge->C2d->D0( *thisSideInfo.iInfo.otherParam, P2d );

    gp_Pnt P = plane->Value( P2d.X(), P2d.Y() );

    // Get the point's parameter on a curve.
    ShapeAnalysis_Curve sac;

    double param = 0.;
    sac.Project( intersectedEdge->C3d, P, Precision::Confusion(), P, param );

    gp_Pnt iP;
    gp_Vec iV;

    intersectedEdge->C3d->D1( param, iP, iV );

#ifdef DRAW_DEBUG
    if ( DRAW_INDEX || DRAW_ALL )
    {
      gp_Pnt tP;
      gp_Vec tV;

      edgeData->C3d->D1( *thisSideInfo.iInfo.param, tP, tV );

      debugInfoVec_Target.push_back( { tP, tV.Normalized() } );
      debugInfoVec_Intersected.push_back( { iP, iV.Normalized() } );
    }
#endif

    gp_Pnt vecP1 = iP;
    gp_Pnt vecP2 = thisSideInfo.shiftP;

    gp_Vec V1( vecP1, vecP2 );

    const bool atLeftSide = iV.AngleWithRef( V1, plane->Axis().Direction() ) > 0.;

    sideInfo::HasMaterial intersectedEdgeMaterial = atLeftSide ? intersectedEdge->sideInfo_Left.hasMaterial
                                                               : intersectedEdge->sideInfo_Right.hasMaterial;

#ifdef DRAW_DEBUG
    if ( DRAW_INDEX || DRAW_ALL )
    {
      debugInfoVec_FromTargetToIntersected.push_back( { vecP1, V1.Reversed().Normalized() } );
    }
#endif

    if ( intersectedEdgeMaterial != sideInfo::HasMaterial_Undefined )
    {
      thisSideInfo.hasMaterial = intersectedEdgeMaterial;

      isSomethingDone = true;

      return;
    }
  }
  else
  {
    raycastDist = 0.5;
  }

  if ( checkingMode != checkingModes::checkingModes_NoRayTracingForIntersectedCases &&
       thisSideInfo.hasMaterial == sideInfo::HasMaterial_Undefined &&
       thisSideInfo.iInfo.otherIndex.has_value() )
  {
    raycastDist = *thisSideInfo.iInfo.distance * 0.5;
  }

  if ( raycastDist.has_value() )
  {
    if ( *raycastDist < MinDistForRaycasting )
    {
#ifdef DRAW_DEBUG
      progress.SendLogMessage( LogInfo(Normal) << "Too small half dist for %1."
                                               << edgeData->index );
#endif

      {
        thisSideInfo.hasMaterial = sideInfo::HasMaterial_HasMaterial;

        isSomethingDone = true;
      }

      return;
    }
    else
    {
      gp_Pnt rayP = thisSideInfo.shiftP.Translated( thisSideInfo.shiftV.Scaled( *raycastDist ) );

      bool _hasHit = hasHit( hitFacets, rayP, plane->Axis().Direction() ) ||
                     hasHit( hitFacets, rayP, plane->Axis().Direction().Reversed() );

      sideInfo::HasMaterial hasMetarial = _hasHit ? sideInfo::HasMaterial_HasMaterial
                                                  : sideInfo::HasMaterial_NoMaterial;

      if ( ( checkingMode == checkingModes::checkingModes_NoRayTracingForIntersectedCases && !hasIntersection ) ||
           ( checkingMode == checkingModes::checkingModes_RayTracingForIntersectedCases_NoMaterialOnly && hasMetarial == sideInfo::HasMaterial_NoMaterial ) ||
           ( checkingMode == checkingModes::checkingModes_RayTracingForIntersectedCases_All ) )
      {
        thisSideInfo.hasMaterial = hasMetarial;

        isSomethingDone = true;
      }

#ifdef DRAW_DEBUG
      if ( DRAW_INDEX || DRAW_ALL )
      {
        if ( _hasHit )
        {
          debugInfoVec_Rays_Red.push_back( { rayP, plane->Axis().Direction() } );
        }
        else
        {
          debugInfoVec_Rays_Green.push_back( { rayP, plane->Axis().Direction() } );
        }

        debugInfoVec_Shift.push_back( { thisSideInfo.shiftP, thisSideInfo.shiftV.Scaled( *raycastDist ) } );
      }
#endif
    }
  }
}

//-----------------------------------------------------------------------------

bool findRayPositionAtParam(boxBndTree&                 bbTree,
                            Selector_FindIntersections& treeSelector,
                            Handle(edgeInfo)&           edgeData,
                            const gp_Pnt&               shiftPnt,
                            const gp_Vec&               shiftVec,
                            const Handle(Geom_Plane)&   plane,
                            intersectionInfo&           iInfo)
{
  // Check if we intersect any other edge.
  TopoDS_Edge shiftEdge;

  try
  {
    shiftEdge = BRepBuilderAPI_MakeEdge( shiftPnt, shiftPnt.Translated( shiftVec ) );
  }
  catch( ... )
  {
    return false;
  }

  double f, l;
  Handle(Geom_Curve) C3d = BRep_Tool::Curve( shiftEdge, f, l );

  if ( C3d.IsNull() )
  {
    return false;
  }

  edgeInfo rayShiftEdge( shiftEdge, plane, edgeData->index );

  rayShiftEdge.isShiftEdge = true;

  treeSelector.Define( &rayShiftEdge );

  bbTree.Select( treeSelector );

  // Get closest intersected edges info if any.
  for ( const auto& iInfoRes : rayShiftEdge.intersected2DParams )
  {
    gp_Pnt2d iP;
    rayShiftEdge.C2d->D0( *iInfoRes.param, iP );

    const double dist = shiftPnt.Distance( plane->Value( iP.X(), iP.Y() ) );

    if ( !iInfo.distance.has_value() ||
          dist < *iInfo.distance )
    {
      iInfo.distance   = dist;
      iInfo.otherIndex = iInfoRes.otherIndex;
      iInfo.otherParam = iInfoRes.otherParam;
    }
  }

  const bool hasIntersection = iInfo.distance.has_value();

  if ( hasIntersection )
  {
    const Handle(edgeInfo)& intersectedEdge = treeSelector.m_data[ *iInfo.otherIndex ];

    // Convert intersection 2D point to 3D space.
    gp_Pnt2d P2d;

    intersectedEdge->C2d->D0( *iInfo.otherParam, P2d );

    gp_Pnt P3d = plane->Value( P2d.X(), P2d.Y() );

    // Get the point's parameter on a curve.
    ShapeAnalysis_Curve sac;

    double t = 0.;
    sac.Project( intersectedEdge->C3d, P3d, Precision::Confusion(), P3d, t );

    gp_Pnt iP;
    gp_Vec iV;

    intersectedEdge->C3d->D1( t, iP, iV );

    // Try to avoid cases with zero length vector.
    if ( iP.IsEqual( shiftPnt, Precision::Confusion() ) )
    {
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

void findRayPosition(boxBndTree&                 bbTree,
                     Selector_FindIntersections& treeSelector,
                     Handle(edgeInfo)&           edgeData,
                     const bool                  isLeft,
                     const Handle(Geom_Plane)&   plane,
                     ActAPI_ProgressEntry        progress,
                     ActAPI_PlotterEntry         plotter)
{
  sideInfo& thisSideInfo = isLeft ? edgeData->sideInfo_Left
                                  : edgeData->sideInfo_Right;

  // 1. Try to check middle point of the edge.
  {
    intersectionInfo iInfo;

    const bool isOk = findRayPositionAtParam( bbTree,
                                              treeSelector,
                                              edgeData,
                                              thisSideInfo.shiftP,
                                              thisSideInfo.shiftV,
                                              plane,
                                              iInfo );

    if ( isOk )
    {
      thisSideInfo.iInfo = iInfo;

      return;
    }
  }

#ifdef DRAW_DEBUG
  const int id = edgeData->index;
#endif

  thisSideInfo.isIOk = false;

  // 2. Try to move along curve to find another place for ray casting.
  GeomAdaptor_Curve adaptor( edgeData->C3d );

  GCPnts_UniformAbscissa discretizer( adaptor, 21 );

  if ( !discretizer.IsDone() ||
        discretizer.NbPoints() == 0 )
  {
    return;
  }

  std::vector< double > params;

  const int nbPoints = discretizer.NbPoints();
  //
  for ( int i = 2; i < nbPoints; ++i )
  {
    const double param = discretizer.Parameter(i);

    gp_Pnt P;
    gp_Vec V;

    edgeData->C3d->D1( param, P, V );

    if ( V.Magnitude() < Precision::Confusion() )
    {
      continue;
    }

    V = V.Rotated( gp_Ax1( P, plane->Axis().Direction() ), M_PI / 2. ).Normalized();

    if ( !isLeft )
    {
      V = V.Reversed();
    }

    intersectionInfo iInfo;

    const bool isOk = findRayPositionAtParam( bbTree,
                                              treeSelector,
                                              edgeData,
                                              P,
                                              V,
                                              plane,
                                              iInfo );

    if ( isOk )
    {
      thisSideInfo.isIOk = true;
      thisSideInfo.iInfo = iInfo;

      thisSideInfo.shiftP = P;
      thisSideInfo.shiftV = V;

      return;
    }
  }

  return;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ComputeOutline::Perform(const gp_Ax1&                                ax,
                                     std::vector<Handle(asiAlgo::algo::Outline)>& outlines,
                                     const Mode                                   mode)
{
  TopoDS_Compound outlineWires;

  if ( !Perform( ax, outlineWires, mode ) )
  {
    return false;
  }

  for ( TopExp_Explorer it( outlineWires, TopAbs_WIRE ); it.More(); it.Next() )
  {
    const TopoDS_Wire& W = TopoDS::Wire( it.Current() );

    Handle(Outline) outline = new Outline( W );

    outlines.push_back( outline );
  }

  return true;
}

//-----------------------------------------------------------------------------

bool asiAlgo_ComputeOutline::Perform(const gp_Ax1&    _ax,
                                     TopoDS_Compound& outlineWires,
                                     const Mode       mode)
{
  m_history->Clear();

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
  gp_Pnt location;
#else
  gp_Dir dir(_ax.Direction());
  gp_Pnt location(_ax.Location());
#endif

  gp_Ax1 ax(location, dir);

  Handle(Geom_Plane) plane = new Geom_Plane( location, dir );

  m_progress.SendLogMessage(LogInfo(Normal) << "Direction of projection for HLR Outline is: %1."
                                            << asiAlgo_Utils::Json::FromDirAsTuple( dir ) );

  //-----------------------------------------------------------------------------
  // Get faces to project.
  //-----------------------------------------------------------------------------

  TopoDS_Shape targetShape;

  if ( m_domain.IsEmpty() )
  {
    targetShape = m_aag->GetMasterShape();
  }
  else
  {
    TopoDS_Compound comp;

    BRep_Builder bbuilder;
    bbuilder.MakeCompound( comp );

    asiAlgo_Feature::Iterator fiter( m_domain );
    for ( ; fiter.More(); fiter.Next() )
    {
      bbuilder.Add( comp, m_aag->GetFace( fiter.Key() ) );
    }

    targetShape = comp;
  }

  //-----------------------------------------------------------------------------
  // Prepare BVH data structure.
  //-----------------------------------------------------------------------------

  Handle(asiAlgo_BVHFacets) bvh = new asiAlgo_BVHFacets( targetShape );

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
        if ( !makeProjection( m_aag, m_domain, ax, m_facesToExclude, hlrResult, features, m_history ) )
        {
          return false;
        }

        break;
      }
      case Mode_Discrete:
      {
        if ( !makeProjectionDiscr( m_aag, m_domain, ax, m_facesToExclude, hlrResult, features, m_history ) )
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
  m_plotter.REDRAW_SHAPE( "features",  features  );
#endif

  //-----------------------------------------------------------------------------
  // Exclude recognized features.
  //-----------------------------------------------------------------------------

  if ( !features.IsNull() )
  {
    BRepAlgoAPI_Cut API;
    asiAlgo_Utils::BooleanCut( hlrResult, features, false, m_fuzzyValue, API );

    hlrResult = API.Shape();
    m_history->Merge(API.History());
  }

#ifdef DRAW_DEBUG
  m_plotter.REDRAW_SHAPE( "hlrResult_cut", hlrResult );
#endif

  //-----------------------------------------------------------------------------
  // Extract edges data.
  //-----------------------------------------------------------------------------

  TopTools_ListOfShape shapes;

  int i = 0;

  Handle(BRepTools_History) historyLoc = new BRepTools_History();

  for ( TopExp_Explorer exp( hlrResult, TopAbs_EDGE ); exp.More(); exp.Next() )
  {
    i++;

    const TopoDS_Edge& E = TopoDS::Edge( exp.Value() );

    // Replace collapsed ellipses by line.
    Handle(Geom_Ellipse) baseEllipse;

    if ( asiAlgo_Utils::IsTypeOf< Geom_Ellipse >( E, baseEllipse ) )
    {
      const double r1 = baseEllipse->MajorRadius();
      const double r2 = baseEllipse->MinorRadius();

      if ( r1 * 2. < m_linearTolerance ||
           r2 * 2. < m_linearTolerance )
      {
        double f, l;
        Handle(Geom_Curve) curve = BRep_Tool::Curve( E, f, l );

        gp_Pnt fP, lP;

        baseEllipse->D0( f, fP );
        baseEllipse->D0( l, lP );

        if ( fP.Distance( lP ) < m_linearTolerance )
        {
          gp_Pnt mP;
          baseEllipse->D0( ( f + l ) * 0.5, mP );

          if ( fP.Distance( mP ) > Precision::Confusion() )
          {
            TopoDS_Edge newEdge = BRepBuilderAPI_MakeEdge(fP, mP);

            shapes.Append(newEdge);

            historyLoc->AddModified(E, newEdge);
          }

          continue;
        }
      }
    }

    shapes.Append( E );
  }

  m_history->Merge(historyLoc);

  if (shapes.IsEmpty())
  {
    return false;
  }

  //TIMER_NEW
  //TIMER_GO

  BOPAlgo_PaveFiller filler;

  filler.SetArguments( shapes );

  filler.Perform();

  double t1 = 0., t2 = 0.;

  BOPDS_MapOfPaveBlock doneBlocks;

  const BOPDS_PDS& pDS = filler.PDS();

  std::vector< TopoDS_Edge > topoEdges;

  const int nb = pDS->NbSourceShapes();

  historyLoc = new BRepTools_History();

  for ( int n = 0; n < nb; ++n )
  {
    const BOPDS_ShapeInfo& si = pDS->ShapeInfo(n);

    if ( si.ShapeType() != TopAbs_EDGE )
    {
      continue;
    }

    if ( pDS->IsNewShape(n) )
      continue;

    if ( !pDS->HasPaveBlocks(n) )
    {
      // Take the initial edge.
      topoEdges.push_back( TopoDS::Edge( pDS->Shape(n) ) );

      historyLoc->AddGenerated(TopoDS::Edge(pDS->Shape(n)), TopoDS::Edge(pDS->Shape(n)));

      continue;
    }

    const BOPDS_ListOfPaveBlock& blocks = pDS->PaveBlocks( n );

    BOPDS_ListOfPaveBlock::Iterator bIter( blocks );
    for ( ; bIter.More(); bIter.Next() )
    {
      Handle(BOPDS_PaveBlock)& block = bIter.Value();

      if ( doneBlocks.Contains( block ) )
      {
        continue;
      }

      block->Range( t1, t2 );

      // To keep only a single instance for a common block.
      if ( pDS->IsCommonBlock( block ) )
      {
        Handle(BOPDS_CommonBlock) common = pDS->CommonBlock( block );

        BOPDS_ListOfPaveBlock::Iterator cIter( common->PaveBlocks() );
        for ( ; cIter.More(); cIter.Next() )
        {
          Handle(BOPDS_PaveBlock)& otherBlock = cIter.Value();

          doneBlocks.Add( otherBlock );
        }
      }

      // Add edge from the block.
      const TopoDS_Edge& originalEdge = TopoDS::Edge(pDS->Shape(block->OriginalEdge()));

      double f, l;
      Handle(Geom_Curve) c3d = BRep_Tool::Curve( originalEdge, f, l );

      TopoDS_Edge newEdge = BRepBuilderAPI_MakeEdge(c3d, t1, t2);

      historyLoc->AddGenerated(originalEdge, newEdge);

      topoEdges.push_back(newEdge);
    }
  }

  for (TopTools_ListOfShape::Iterator itSh(shapes); itSh.More(); itSh.Next())
  {
    const TopoDS_Edge& edge = TopoDS::Edge(itSh.Value());

    const TopTools_ListOfShape& genList = historyLoc->Generated(edge);
    if (genList.IsEmpty())
    {
      if (!m_history->IsRemoved(edge))
      {
        m_history->Remove(edge);
      }
    }
    else
    {
      const TopoDS_Edge& fEdgeGen = TopoDS::Edge(genList.First());

      if (genList.Size() == 1 && edge.IsEqual(fEdgeGen))
      {
        continue;
      }

      bool isFirst = true;
      for (TopTools_ListOfShape::Iterator itG(genList); itG.More(); itG.Next())
      {
        if (itG.Value().ShapeType() != TopAbs_EDGE)
        {
          continue;
        }

        const TopoDS_Edge& edgeGen = TopoDS::Edge(itG.Value());

        if (m_history->IsRemoved(edge))
        {
          continue;
        }

        if (isFirst)
        {
          m_history->AddModified(edge, edgeGen);
          isFirst = false;
        }
        else
        {
          m_history->AddGenerated(edge, edgeGen);
        }
      }
    }
  }

  if ( topoEdges.empty() )
  {
    return false;
  }

  std::vector< Handle(edgeInfo) > edges;

  i = 0;
  for ( auto& edge : topoEdges )
  {
    edges.push_back( new edgeInfo( edge, plane, i++ ) );
  }

  //TIMER_FINISH
  //TIMER_COUT_RESULT_NOTIFIER( m_progress, "BOPAlgo_PaveFiller" )

#ifdef DRAW_DEBUG
  Handle(HRealArray)   coords = new HRealArray  ( 0, (int) edges.size() * 3 - 1, 0. );
  Handle(HStringArray) labels = new HStringArray( 0, (int) edges.size() - 1 );

  int coordIdx = 0;
  int lblIdx = 0;

  for ( Handle(edgeInfo)& edge : edges )
  {
    gp_Pnt midP;
    edge->C3d->D0( ( edge->f + edge->l ) * 0.5, midP );

    coords->ChangeValue(coordIdx)     = midP.X();
    coords->ChangeValue(coordIdx + 1) = midP.Y();
    coords->ChangeValue(coordIdx + 2) = midP.Z();

    labels->ChangeValue(lblIdx++)     = edge->index;

    coordIdx += 3;
  }

  m_plotter.REDRAW_LABELS( "initial_indices", coords, labels, Color_White );
  drawEdges( edges, "initial_edges", m_plotter );
#endif

  //-----------------------------------------------------------------------------
  // Resolve rays positions.
  //-----------------------------------------------------------------------------

  boxBndTree   bbTree;
  boxBndFiller treeFiller( bbTree );

  Selector_FindIntersections treeSelector( edges, m_plotter );

  for ( Handle(edgeInfo)& edgeData : edges )
  {
    treeFiller.Add( edgeData->index + 1, edgeData->box );
  }

  treeFiller.Fill();

  for ( Handle(edgeInfo)& edgeData : edges )
  {
#ifdef DRAW_DEBUG
    const int id = edgeData->index;
#endif

    // Left side.
    findRayPosition( bbTree, treeSelector, edgeData, true, plane, m_progress, m_plotter );

    // Right side.
    findRayPosition( bbTree, treeSelector, edgeData, false, plane, m_progress, m_plotter );
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

      for ( Handle(edgeInfo)& edgeData : edges )
      {
#ifdef DRAW_DEBUG
        const int id = edgeData->index;
#endif

        if ( edgeData->Status() != edgeInfo::Status_Undefined )
        {
          continue;
        }

        // Check left side.
        checkSide( hitFacets, treeSelector, edgeData, true, plane, isSomethingDone, checkingMode, m_progress, m_plotter );

        // Check right side.
        checkSide( hitFacets, treeSelector, edgeData, false, plane, isSomethingDone, checkingMode, m_progress, m_plotter );
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

  std::vector< Handle(edgeInfo) > totallyGoodEdges;
  std::vector< Handle(edgeInfo) > totallyBadEdges;
  std::vector< Handle(edgeInfo) > danglingEdges;
  std::vector< Handle(edgeInfo) > otherEdges;

  {
    for ( Handle(edgeInfo)& edgeData : edges )
    {
      if ( edgeData->Status() == edgeInfo::Status_Body )
      {
        totallyBadEdges.push_back( edgeData );
      }
      else if ( edgeData->Status() == edgeInfo::Status_Border )
      {
        totallyGoodEdges.push_back( edgeData );
      }
      else if ( edgeData->Status() == edgeInfo::Status_Dangling )
      {
        danglingEdges.push_back( edgeData );
      }
      else
      {
        otherEdges.push_back( edgeData );
      }
    }
  }

  drawEdges( danglingEdges,    "dangling",         m_plotter, Color_Maroon );
  drawEdges( otherEdges,       "other",            m_plotter, Color_Yellow );
  drawEdges( totallyBadEdges,  "totallyBadEdges",  m_plotter, Color_Red    );
  drawEdges( totallyGoodEdges, "totallyGoodEdges", m_plotter, Color_Green  );
#endif

  // Do we have anything?
  std::vector< TopoDS_Edge > filteredEdges;

  {
    for ( Handle(edgeInfo)& edgeData : edges )
    {
      if ( edgeData->Status() == edgeInfo::Status_Border )
      {
        filteredEdges.push_back( edgeData->E );
      }
      else if ( m_includeDanglingEdges && edgeData->Status() == edgeInfo::Status_Dangling )
      {
        filteredEdges.push_back( edgeData->E );
      }
      else
      {
        m_history->Remove(edgeData->E);
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
                                           EdgesToWiresTol ) )
  {
    if ( outlineWiresVec.empty() )
    {
      return false;
    }

    //// Fix wires.
    //for ( auto& W : outlineWiresVec )
    //{
    //  ShapeFix_Wire fixWire;
    //  fixWire.Load( W );
    //
    //  fixWire.FixIntersectingEdgesMode() = 1;
    //  fixWire.FixSelfIntersectingEdgeMode() = 1;
    //  fixWire.FixSelfIntersectionMode() = 1;
    //  fixWire.FixSmallMode() = 1;
    //
    //  if ( fixWire.Perform() )
    //  {
    //    W = fixWire.WireAPIMake();
    //  }
    //}

    // Extract outlines.
    TopoDS_Compound WC;

    BRep_Builder bb;
    bb.MakeCompound(WC);

    ShapeAnalysis_Edge edgeAnalysis;

    for ( auto& W : outlineWiresVec )
    {
      for (TopExp_Explorer exp(W, TopAbs_EDGE); exp.More(); exp.Next())
      {
        const TopoDS_Edge& imageEdge = TopoDS::Edge(exp.Value());
        const TopoDS_Vertex& firstVertex = edgeAnalysis.FirstVertex(imageEdge);
        const TopoDS_Vertex& lastVertex = edgeAnalysis.LastVertex(imageEdge);
        gp_Pnt fPnt = BRep_Tool::Pnt(firstVertex);
        gp_Pnt lPnt = BRep_Tool::Pnt(lastVertex);

        std::vector<TopoDS_Edge>::const_iterator itFEs = filteredEdges.cbegin();
        for (; itFEs != filteredEdges.cend(); ++itFEs)
        {
          const TopoDS_Edge& originEdge = *itFEs;
          if (m_history->IsRemoved(originEdge))
          {
            continue;
          }

          const TopoDS_Vertex& firstVertexC = edgeAnalysis.FirstVertex(originEdge);
          const TopoDS_Vertex& lastVertexC = edgeAnalysis.LastVertex(originEdge);
          gp_Pnt fPntC = BRep_Tool::Pnt(firstVertexC);
          gp_Pnt lPntC = BRep_Tool::Pnt(lastVertexC);

          if (fPnt.IsEqual(fPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(firstVertexC), BRep_Tool::Tolerance(firstVertex)))) &&
              lPnt.IsEqual(lPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(lastVertexC), BRep_Tool::Tolerance(lastVertex)))) ||
              fPnt.IsEqual(lPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(firstVertex), BRep_Tool::Tolerance(lastVertexC)))) &&
              lPnt.IsEqual(fPntC, std::max(Precision::Confusion(), std::max(BRep_Tool::Tolerance(lastVertex), BRep_Tool::Tolerance(firstVertexC)))))
          {
            m_history->AddModified(originEdge, imageEdge);
            break;
          }

        }
      }

      bb.Add( WC, W );
    }

#ifdef DRAW_DEBUG
    //m_plotter.REDRAW_SHAPE( "wires", WC );
#endif

    outlineWires = WC;

#ifdef DRAW_DEBUG
    for (asiAlgo_Feature::Iterator itD(m_domain); itD.More(); itD.Next())
    {
      const int fid = itD.Key();

      const TopoDS_Face& face = m_aag->GetFace(fid);

      m_plotter.DRAW_SHAPE(face, Color_Red, "originalFace_");

      const TopTools_ListOfShape& genList = m_history->Generated(face);
      for (TopTools_ListOfShape::Iterator itG(genList); itG.More(); itG.Next())
      {
        if (itG.Value().ShapeType() != TopAbs_EDGE)
        {
          continue;
        }

        const TopoDS_Edge& edge = TopoDS::Edge(itG.Value());

        if (m_history->IsRemoved(edge))
        {
          continue;
        }

        m_plotter.DRAW_SHAPE(edge, Color_Red, "imageEdge_");
      }

      const TopTools_ListOfShape& modList = m_history->Modified(face);
      for (TopTools_ListOfShape::Iterator itM(modList); itM.More(); itM.Next())
      {
        if (itM.Value().ShapeType() != TopAbs_EDGE)
        {
          continue;
        }

        const TopoDS_Edge& edge = TopoDS::Edge(itM.Value());

        if (m_history->IsRemoved(edge))
        {
          continue;
        }

        m_plotter.DRAW_SHAPE(edge, Color_Red, "imageEdge_");
      }
    }
#endif

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

//-----------------------------------------------------------------------------

void asiAlgo_ComputeOutline::SetDomain(const asiAlgo_Feature& domain)
{
  m_domain = domain;
}

//-----------------------------------------------------------------------------

void asiAlgo_ComputeOutline::SetDomain(const TopoDS_Shape& faces)
{
  for ( TopExp_Explorer it( faces, TopAbs_FACE ); it.More(); it.Next() )
  {
    m_domain.Add( m_aag->GetFaceId( TopoDS::Face( it.Current() ) ) );
  }
}
