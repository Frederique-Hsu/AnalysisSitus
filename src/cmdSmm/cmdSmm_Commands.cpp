//-----------------------------------------------------------------------------
// Created on: 11 June 2025
//-----------------------------------------------------------------------------
// Copyright (c) 2025-present, Sergey Slyadnev
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

// cmdSmm includes
#include "cmdSmm.h"

// asiSmm includes
#include <asiSmm_MakeEdgeFlange.h>
#include <asiSmm_Utils.h>

// asiEngine includes
#include <asiEngine_Part.h>

// asiUI includes
#include <asiUI_CommonFacilities.h>
#include <asiUI_DialogMakeFlange.h>

// Qt includes
#pragma warning(push, 0)
#include <QMainWindow>
#pragma warning(pop)

#define IsGui \
  !cmdSmm::cf.IsNull()

#define UpdateUi \
  if ( IsGui ) \
    cmdSmm::cf->ViewerPart->PrsMgr()->Actualize( asiEngine_Part(cmdSmm::model).GetPart() );

using namespace asiSmm;

//-----------------------------------------------------------------------------

int SMM_MakeBlock(const Handle(asiTcl_Interp)& interp,
                  int                          argc,
                  const char**                 argv)
{
  // Dimensions.
  double dx = 100.0, dy = 100.0, dz = 10.0;
  //
  if ( argc >= 3 )
  {
    dx = atof(argv[0]);
    dy = atof(argv[1]);
    dz = atof(argv[2]);
  }

  // Create and draw the box primitive.
  TopoDS_Solid box = Utils::BuildBaseBlock(dx, dy, dz);

  // Update part.
  cmdSmm::model->OpenCommand();
  {
    asiEngine_Part(cmdSmm::model).Update(box);
  }
  cmdSmm::model->CommitCommand();

  // Update UI.
  UpdateUi

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int SMM_MakeEdgeFlange(const Handle(asiTcl_Interp)& interp,
                       int                          argc,
                       const char**                 argv)
{
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  if ( argc == 1 )
  {
    asiUI_DialogMakeFlange*
      pFlangeDlg = new asiUI_DialogMakeFlange( cmdSmm::cf,
                                               M,
                                               interp->GetProgress(),
                                               interp->GetPlotter(),
                                               cmdSmm::cf->MainWindow );
    //
    pFlangeDlg->show();

    return TCL_OK;
  }

  // Get part.
  Handle(asiData_PartNode) partNode = M->GetPartNode();

  // Attempt to get the highlighted sub-shapes.
  TColStd_PackedMapOfInteger edgeIds;
  //
  if ( IsGui )
  {
    asiEngine_Part PartAPI( M,
                            cmdSmm::cf->ViewerPart->PrsMgr(),
                            interp->GetProgress(),
                            interp->GetPlotter() );
    //
    PartAPI.GetHighlightedEdges(edgeIds);
  }

  // Get the ID of the seed edge.
  int eid = 0;
  //
  if ( edgeIds.IsEmpty() )
  {
    interp->GetKeyValue(argc, argv, "eid", eid);
  }
  else
  {
    eid = edgeIds.GetMinimalMapped();
  }

  // Verify that we have an edge to proceed.
  if ( !eid )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Seed edge is not provided.");
    return TCL_ERROR;
  }
  //
  interp->GetProgress().SendLogMessage(LogInfo(Normal) << "Seed edge ID: %1."
                                                       << eid);

  // Get the bend angle input.
  double bendAngleDeg = 90.; // Default.
  interp->GetKeyValue(argc, argv, "angle", bendAngleDeg);

  // Get the flange length input.
  double flangeLength = 10.; // Default.
  interp->GetKeyValue(argc, argv, "length", flangeLength);

  // Initialize the flange builder tool.
  MakeEdgeFlange maker( partNode->GetAAG(),
                        interp->GetProgress(),
                        interp->HasKeyword(argc, argv, "draw") ? interp->GetPlotter() : nullptr );

  // Construct a flange.
  if ( !maker.Build(eid, bendAngleDeg, flangeLength) )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Failed to construct a flange of %1 mm length "
                                                           "under %2 degrees."
                                                        << flangeLength << bendAngleDeg);
    return TCL_ERROR;
  }

  // Update part.
  cmdSmm::model->OpenCommand();
  {
    asiEngine_Part(cmdSmm::model).Update( maker.GetResult() );
  }
  cmdSmm::model->CommitCommand();

  // Update UI.
  UpdateUi

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdSmm::Commands(const Handle(asiTcl_Interp)&      interp,
                      const Handle(Standard_Transient)& cmdSmm_NotUsed(data))
{
  static const char* group = "cmdSmm";

  //-------------------------------------------------------------------------//
  interp->AddCommand("smm-make-block",
    //
    "smm-make-block [<dx> <dy> <dz>]\n"
    "\t Builds a base block to start modeling from.",
    //
    __FILE__, group, SMM_MakeBlock);

  //-------------------------------------------------------------------------//
  interp->AddCommand("smm-make-edge-flange",
    //
    "smm-make-edge-flange [-eid <eid>] [-length <h>] [-angle <angDeg>] [-draw]\n"
    "\t Makes a flange starting from the given edge. The provided length\n"
    "\t includes bend deduction contribution.",
    //
    __FILE__, group, SMM_MakeEdgeFlange);
}
