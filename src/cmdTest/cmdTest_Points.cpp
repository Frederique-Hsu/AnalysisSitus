//-----------------------------------------------------------------------------
// Created on: 25 June 2022
// Created by: Andrey Voevodin
//-----------------------------------------------------------------------------
// Copyright (c) 2022-present, Andrey Voevodin
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

// cmdTest includes
#include <cmdTest.h>

// asiEngine includes
#include <asiEngine_Model.h>

// asiAlgo includes
#include <asiAlgo_ClassifyPointSolid.h>
#include <asiAlgo_MeshGen.h>
#include <asiAlgo_PointWithAttr.h>
#include <asiAlgo_PurifyCloud.h>
#include <asiAlgo_QuickHull2d.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// OCCT includes
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <gp_XY.hxx>
#include <gp_XYZ.hxx>
#include <Poly_CoherentTriangulation.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Wire.hxx>

#define PmcOnTol 0.25

//-----------------------------------------------------------------------------

int TEST_BuildQuickHull(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if ( argc != 8 )
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  gp_Pnt loc(std::atof(argv[2]), std::atof(argv[3]), std::atof(argv[4]));
  gp_Dir dir(std::atof(argv[5]), std::atof(argv[6]), std::atof(argv[7]));

  Handle(Geom_Plane) plane = new Geom_Plane(loc, dir);
  ShapeAnalysis_Surface sas(plane);

  // Find Points Node by name.
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast(interp->GetModel()->FindNodeByName(argv[1]));
  //
  if ( pointsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point cloud."
                                                        << argv[2]);
    return TCL_ERROR;
  }

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = pointsNode->GetPoints();
  if ( pts.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No points.");
    return TCL_ERROR;
  }

  // Get output points.
  Handle(asiAlgo_PointWithAttrCloud<gp_XY>) cloud = new asiAlgo_PointWithAttrCloud<gp_XY>;
  //
  for ( int ipt = 0; ipt < pts->GetNumberOfElements(); ++ipt )
  {
    gp_Pnt pnt = pts->GetElement(ipt);

    gp_Pnt proj;
    if ( !asiAlgo_Utils::ProjectPointOnPlane(plane,
                                             dir, pnt, proj) )
    {
      continue;
    }

    gp_XY pnt2d = sas.ValueOfUV(proj, 1.0e-4).XY();

    cloud->AddElement(asiAlgo_PointWithAttr<gp_XY>(pnt2d, 0, ipt));
  }

  ///
  Handle(asiAlgo_PointWithAttrCloud<gp_XY>) sparsedCloud;

  // Sparse cloud using the appropriate type of inspector.
  asiAlgo_PurifyCloud sparser(interp->GetProgress(), interp->GetPlotter());
  //
  sparser.PerformCommon< asiAlgo_PointWithAttrCloud<gp_XY>,
    asiAlgo_PointWithAttrInspector2d<gp_XY> >(0.01, cloud, sparsedCloud);

  ///
  asiAlgo_QuickHull2d<gp_XY> qHull(sparsedCloud, interp->GetProgress(), interp->GetPlotter());
  //
  if ( !qHull.Perform() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Quick hull failed.");
    return TCL_ERROR;
  }

  // Get hull indices.
  const Handle(TColStd_HSequenceOfInteger)& hull = qHull.GetHull();

  // Make polygon
  std::vector<gp_XYZ> chPoints;
  BRepBuilderAPI_MakePolygon mkPolygon;
  for ( int hidx = hull->Lower(); hidx <= hull->Upper(); ++hidx )
  {
    gp_Pnt pnt = plane->Value(sparsedCloud->GetElement(hull->Value(hidx)).Coord.X(),
                              sparsedCloud->GetElement(hull->Value(hidx)).Coord.Y());

    mkPolygon.Add(pnt);

    chPoints.push_back(pnt.XYZ());
  }
  //
  mkPolygon.Close(); // Polygon should be closed
  mkPolygon.Build();
  //
  const TopoDS_Wire& W = mkPolygon.Wire();

  interp->GetPlotter().REDRAW_SHAPE("hull", W, Color_Green, 1., true);
  interp->GetPlotter().REDRAW_POINTS("hull-points", chPoints, Color_Green);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int TEST_BuildAABBPoints(const Handle(asiTcl_Interp)& interp,
                         int                          argc,
                         const char**                 argv)
{
  if (argc != 5)
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  // Get shape to analyze.
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if (M.IsNull())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model is NULL.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape partShape = M->GetPartNode()->GetShape();
  if (partShape.IsNull())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is NULL.");
    return TCL_ERROR;
  }

  Bnd_Box boxOfShape;
  BRepBndLib::AddOptimal(partShape, boxOfShape, false);

  double minX = 0.0;
  double minY = 0.0;
  double minZ = 0.0;
  double maxX = 0.0;
  double maxY = 0.0;
  double maxZ = 0.0;
  boxOfShape.Get(minX, minY, minZ, maxX, maxY, maxZ);

  int xNbPntsLoc = std::atoi(argv[2]);
  int yNbPntsLoc = std::atoi(argv[3]);
  int zNbPntsLoc = std::atoi(argv[4]);

  double xStep = abs(maxX - minX) / (double)(xNbPntsLoc);
  double yStep = abs(maxY - minY) / (double)(yNbPntsLoc);
  double zStep = abs(maxZ - minZ) / (double)(zNbPntsLoc);

  Handle(asiAlgo_BaseCloud<double>) pts = new asiAlgo_BaseCloud<double>;

  if (xStep >= Precision::Confusion() &&
      yStep >= Precision::Confusion() &&
      zStep >= Precision::Confusion())
  {
    int nx = xNbPntsLoc;
    int ny = yNbPntsLoc;
    int nz = zNbPntsLoc;

    double x, y, z;
    for (int xx = 0; xx <= nx; ++xx)
    {
      x = minX + xStep * xx;
      for (int yy = 0; yy <= ny; ++yy)
      {
        y = minY + yStep * yy;
        for (int zz = 0; zz <= nz; ++zz)
        {
          z = minZ + zStep * zz;

          pts->AddElement(x, y, z);
        }
      }
    }
  }

  interp->GetPlotter().REDRAW_POINTS(argv[1], pts->GetCoordsArray(), Color_Green);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int TEST_ClassifyPointSolid(const Handle(asiTcl_Interp)& interp,
                            int                          argc,
                            const char**                 argv)
{
  if (argc != 5)
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const int status = std::atoi(argv[3]);
  if (status != 0 && status != 1 && status != -1)
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Wrong status.");
    return TCL_ERROR;
  }

  const bool isUsedDist = (bool)(std::atoi(argv[4]));

  // Find Points Node by name.
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast(interp->GetModel()->FindNodeByName(argv[1]));
  //
  if ( pointsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point cloud."
                                                        << argv[1]);
    return TCL_ERROR;
  }

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = pointsNode->GetPoints();
  if ( pts.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No points.");
    return TCL_ERROR;
  }

  // Get shape to analyze.
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast(interp->GetModel());
  if (M.IsNull())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Model is NULL.");
    return TCL_ERROR;
  }
  //
  TopoDS_Shape partShape = M->GetPartNode()->GetShape();
  if (partShape.IsNull())
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Shape is NULL.");
    return TCL_ERROR;
  }

  asiAlgo_MeshInfo meshInfo = asiAlgo_MeshInfo::Extract(partShape);
  if (!meshInfo.nFacets)
  {
    const double linDefl = asiAlgo_MeshGen::AutoSelectLinearDeflection(partShape);
    const double angDefl = asiAlgo_MeshGen::AutoSelectAngularDeflection(partShape);

    if (!asiAlgo_MeshGen::DoNative(partShape,
                                   linDefl,
                                   angDefl,
                                   meshInfo))
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Tessellation failed.");
      return TCL_ERROR;
    }
  }

  Handle(Poly_CoherentTriangulation) shapeTris = new Poly_CoherentTriangulation();
  for (TopExp_Explorer fexp(partShape, TopAbs_FACE); fexp.More(); fexp.Next())
  {
    const TopoDS_Face& face = TopoDS::Face(fexp.Current());

    // Poly_MeshPurpose was introduced in OpenCascade 7.6 and remained
    // undocumented.
    TopLoc_Location L;
    const Handle(Poly_Triangulation)& poly = BRep_Tool::Triangulation(face, L);
    if (poly.IsNull())
    {
      continue;
    }

    // Add nodes.
    std::unordered_map<int, int> nodesMap;
    for (int iNode = 1; iNode <= poly->NbNodes(); ++iNode)
    {
      // Make sure to apply location, e.g., see the effect in /cad/ANC101.brep
      const int n = shapeTris->SetNode(poly->Node(iNode).Transformed(L).XYZ());

      // Local to global node index mapping.
      nodesMap.insert({ iNode, n });
    }

    // Add triangles.
    for (int iTri = 1; iTri <= poly->NbTriangles(); ++iTri)
    {
      const Poly_Triangle& tri = poly->Triangle(iTri);

      int iNodes[3];
      tri.Get(iNodes[0], iNodes[1], iNodes[2]);

      // Try disabling this and check how mesh is visualized.
      if (face.Orientation() == TopAbs_REVERSED)
      {
        std::swap(iNodes[1], iNodes[2]);
      }

      shapeTris->AddTriangle(nodesMap[iNodes[0]], nodesMap[iNodes[1]], nodesMap[iNodes[2]]);
    }
  }

  ShapeAnalysis_ShapeTolerance tolerChecker;
  const double maxTol = tolerChecker.Tolerance(partShape, 1);  // 1 means max.
  const double onToler = Max(Max(maxTol, Precision::Confusion()), PmcOnTol);

  asiAlgo_ClassifyPointSolid SC(shapeTris->GetTriangulation());

  // Get output points.
  Handle(asiAlgo_BaseCloud<double>) ptsOut = new asiAlgo_BaseCloud<double>;

  for ( int ipt = 0; ipt < pts->GetNumberOfElements(); ++ipt )
  {
    gp_Pnt pnt = pts->GetElement(ipt);

    bool isAdd = false;

    if (status == -1)
    {
      if (SC.IsOut(pnt.XYZ(), onToler, isUsedDist))
      {
        isAdd = true;
      }
    }
    else if (status == 0)
    {
      if (SC.IsOn(pnt.XYZ(), onToler))
      {
        isAdd = true;
      }
    }
    else if (status == 1)
    {
      if (SC.IsIn(pnt.XYZ(), onToler, isUsedDist))
      {
        isAdd = true;
      }
    }

    if (isAdd)
    {
      ptsOut->AddElement(pnt.X(), pnt.Y(), pnt.Z());
    }
  }

  interp->GetPlotter().REDRAW_POINTS(argv[2], ptsOut->GetCoordsArray(), Color_Green);

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int TEST_NumberOfPoints(const Handle(asiTcl_Interp)& interp,
                        int                          argc,
                        const char**                 argv)
{
  if (argc != 3)
  {
    return interp->ErrorOnWrongArgs(argv[0]);
  }

  const int nbPoints = std::atoi(argv[2]);

  // Find Points Node by name.
  Handle(asiData_IVPointSetNode)
    pointsNode = Handle(asiData_IVPointSetNode)::DownCast(interp->GetModel()->FindNodeByName(argv[1]));
  //
  if ( pointsNode.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Node '%1' is not a point cloud."
                                                        << argv[1]);
    return TCL_ERROR;
  }

  // Get point cloud.
  Handle(asiAlgo_BaseCloud<double>) pts = pointsNode->GetPoints();
  if ( pts.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "No points.");
    return TCL_ERROR;
  }

  if (pts->GetNumberOfElements() != nbPoints)
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Incorrect number of points. Actual value = '%1'." << pts->GetNumberOfElements());
    return TCL_ERROR;
  }

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdTest::Commands_Points(const Handle(asiTcl_Interp)&      interp,
                              const Handle(Standard_Transient)& cmdTest_NotUsed(data))
{
  static const char* group = "cmdTest";

  interp->AddCommand("test-build-quick-hull",
    //
    "test-build-quick-hull <pointsName> <loc_x> <loc_y> <loc_z> <dir_x> <dir_y> <dir_z>\n"
    "\t Build quick hull.",
    //
    __FILE__, group, TEST_BuildQuickHull);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-build-aabb-points",
    //
    "test-build-aabb-points <pointsName> <nbPntsX> <nbPntsY> <nbPntsZ>\n"
    "\t Builds AABB in point cloud representation (number of samples is specified for each direction).",
    //
    __FILE__, group, TEST_BuildAABBPoints);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-classify-point-solid",
    //
    "test-classify-point-solid <pointsName> <pointsOutName> <status(out '-1', on '0', in '1')> <useDist(0, 1)>\n"
    "\t PMC.",
    //
    __FILE__, group, TEST_ClassifyPointSolid);

  //-------------------------------------------------------------------------//
  interp->AddCommand("test-number-of-points",
    //
    "test-number-of-points <pointsName> <nbPoinst>\n"
    "\t Check number of points",
    //
    __FILE__, group, TEST_NumberOfPoints);
}
