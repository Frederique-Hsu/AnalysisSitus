//-----------------------------------------------------------------------------
// Created on: 16 September 2024
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

// cmdNesting includes
#include <cmdNesting_Container3d.h>
#include <cmdNesting_Part3d.h>

// asiEngine includes
#include <asiEngine_Nesting3d.h>

// asiAlgo includes
#include <asiAlgo_NestingContainer3d.h>
#include <asiAlgo_NestingDBLF.h>
#include <asiAlgo_NestingPart3d.h>
#include <asiAlgo_Timer.h>

// asiTcl includes
#include <asiTcl_PluginMacro.h>

// asiUI includes
#include <asiUI_IV.h>

using namespace asiAlgo::nesting;

//-----------------------------------------------------------------------------

Handle(asiEngine_Model)        cmdNesting::model = nullptr;
Handle(asiUI_CommonFacilities) cmdNesting::cf    = nullptr;

//-----------------------------------------------------------------------------

//! Drawable DBLF.
class DDBLF : public DBLF
{
public:

  DDBLF(const Handle(asiEngine_Model)& model,
        const Handle(Container3d)&     container,
        ActAPI_ProgressEntry           progress = nullptr,
        ActAPI_PlotterEntry            plotter  = nullptr)
  //
  : DBLF    (container, progress, plotter),
    m_model (model)
  {}

protected:

  virtual void drawCursor(const Nbh3d& nbv)
  {
    if ( m_plotter.Access().IsNull() )
      return;

    asiEngine_Nesting3d api(m_model);

    // Update data model.
    Handle(asiData_Nesting3dCursorNode) cursor;
    //
    m_model->OpenCommand();
    {
      cursor = api.Find_Cursor(true);

      cursor->SetNeighborhood(nbv);
    }
    m_model->CommitCommand();

    // Visualize.
    Handle(asiUI_IV)
      ivPlotter = Handle(asiUI_IV)::DownCast( m_plotter.Access() );
    //
    if ( !ivPlotter.IsNull() )
      ivPlotter->GetPrsMgr3d()->Actualize(cursor);

    m_progress.ProcessEvents();

    ///
    ///if ( nbv.isRoomy )
    //  m_progress.Cancel();
    ///
  }

protected:

  Handle(asiEngine_Model) m_model;

};

//-----------------------------------------------------------------------------

int NESTING_BuildContainer3d(const Handle(asiTcl_Interp)& interp,
                             int                          argc,
                             const char**                 argv)
{
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  const double dx = Atof(argv[1]);
  const double dy = Atof(argv[2]);
  const double dz = Atof(argv[3]);

  double step = 1; // mm
  interp->GetKeyValue(argc, argv, "step", step);

  // Container's name.
  std::string name = "container";
  interp->GetKeyValue(argc, argv, "name", name);

  // Build container.
  Handle(Container3d)     container = new Container3d(dx, dy, dz);
  Handle(Container3dGrid) grid      = container->BuildGrid(step);

  interp->GetPlotter().REDRAW_POINTS( (name + "_vacant").c_str(),
                                      Container3d::GetPoints3d(grid)->GetCoordsArray(),
                                      1, false,
                                      ActAPI_Color(200./255., 200./255., 200./255., Quantity_TOC_RGB) );

  // Set Tcl variable.
  interp->SetVar( name, new cmdNesting_Container3d(container) );

  // Nesting API.
  asiEngine_Nesting3d api(M);
  //
  M->OpenCommand();
  {
    api.Find_Nesting3d(true); // Create if does not exist.
    api.Create_Container( StdStr2ExtStr(name) );
    api.Create_Cursor();
  }
  M->CommitCommand();

  // Update UI.
  if ( !cmdNesting::cf.IsNull() )
    cmdNesting::cf->ObjectBrowser->Populate();

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int NESTING_LoadPart(const Handle(asiTcl_Interp)& interp,
                     int                          argc,
                     const char**                 argv)
{
  // Part's filename.
  std::string filename;
  interp->GetKeyValue(argc, argv, "filename", filename);
  //
  if ( filename.empty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, provide filename "
                                                           "via '-filename' keyword.");
    return TCL_ERROR;
  }

  // Target part's name.
  std::string partName;
  interp->GetKeyValue(argc, argv, "name", partName);
  //
  if ( partName.empty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, provide part name "
                                                           "via '-name' keyword.");
    return TCL_ERROR;
  }

  // Construct a new part.
  Handle(Part3d) part = Part3d::LoadStl( filename, interp->GetProgress() );
  //
  if ( part.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Cannot construct a part.");
    return TCL_ERROR;
  }

  // Set Tcl variable.
  interp->SetVar( partName, new cmdNesting_Part3d(part) );

  // Draw part.
  interp->GetPlotter().REDRAW_TRIANGULATION( partName.c_str(),
                                             part->triangulation,
                                             ActAPI_Color(120./255., 100./255., 100./255., Quantity_TOC_RGB),
                                             1. );

  return TCL_OK;
}

//-----------------------------------------------------------------------------

int NESTING_PlacePart(const Handle(asiTcl_Interp)& interp,
                      int                          argc,
                      const char**                 argv)
{
  Handle(asiEngine_Model)
    M = Handle(asiEngine_Model)::DownCast( interp->GetModel() );

  /* =========================
   *  Prepare working objects.
   * ========================= */

  std::string partName;
  interp->GetKeyValue(argc, argv, "part", partName);
  //
  if ( partName.empty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, provide part name "
                                                           "via '-part' keyword.");
    return TCL_ERROR;
  }

  // Get target part.
  Handle(Part3d) part;
  //
  {
    Handle(asiTcl_Variable)
      var = interp->GetVar(partName);
    //
    Handle(cmdNesting_Part3d)
      varPart = Handle(cmdNesting_Part3d)::DownCast(var);
    //
    if ( varPart.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no Tcl variable for a part "
                                                             "named '%1'."
                                                          << partName);
      return TCL_ERROR;
    }

    part = varPart->GetPart();
    //
    if ( part.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Part named '%1' is not initialized."
                                                          << partName);
      return TCL_ERROR;
    }
  }

  std::string containerName;
  interp->GetKeyValue(argc, argv, "container", containerName);
  //
  if ( containerName.empty() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "Please, provide container name "
                                                            "via '-container' keyword.");
    return TCL_ERROR;
  }

  // Get target container.
  Handle(Container3d) container;
  //
  {
    Handle(asiTcl_Variable)
      var = interp->GetVar(containerName);
    //
    Handle(cmdNesting_Container3d)
      varContainer = Handle(cmdNesting_Container3d)::DownCast(var);
    //
    if ( varContainer.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "There is no Tcl variable for a container "
                                                             "named '%1'."
                                                          << containerName);
      return TCL_ERROR;
    }

    container = varContainer->GetContainer();
    //
    if ( container.IsNull() )
    {
      interp->GetProgress().SendLogMessage(LogErr(Normal) << "Container named '%1' is not initialized."
                                                          << containerName);
      return TCL_ERROR;
    }
  }

  /* ============
   *  Place part.
   * ============ */

  TIMER_NEW
  TIMER_GO

  // Make sure that the part's pattern is available.
  if ( part->pattern.empty() )
  {
    // Store key points of a part w.r.t. the current grid.
    container->BuildPointPattern(part);
  }

  Handle(Part3d) NP = part->DeepCopy();

  // DBLF nesting.
  DDBLF placePart( M,
                   container,
                   interp->GetProgress(),
                   interp->HasKeyword(argc, argv, "draw") ? interp->GetPlotter() : nullptr );
  //
  if ( !placePart.PlacePart(NP) )
  {
    TIMER_FINISH
    TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "DBLF placement")

    interp->GetProgress().SendLogMessage(LogErr(Normal) << "DBLF placement failed.");
    return TCL_ERROR;
  }

  // Set back the last nested position.
  part->lastNestedPos = NP->lastNestedPos;

  TIMER_FINISH
  TIMER_COUT_RESULT_NOTIFIER(interp->GetProgress(), "DBLF placement")

  /* ==========
   *  Finalize.
   * ========== */

  // Draw part.
  interp->GetPlotter().DRAW_TRIANGULATION( NP->triangulation,
                                           ActAPI_Color(120./255., 200./255., 100./255., Quantity_TOC_RGB),
                                           1.,
                                          "nested" );

  interp->GetPlotter().REDRAW_POINTS( (containerName + "_vacant").c_str(),
                                      Container3d::GetPoints3d(container->GetGrid(), Container3d::PointFilter_Vacant)->GetCoordsArray(),
                                      1, false,
                                      ActAPI_Color(200./255., 200./255., 200./255., Quantity_TOC_RGB) );

  interp->GetPlotter().REDRAW_POINTS( (containerName + "_locked").c_str(),
                                      Container3d::GetPoints3d(container->GetGrid(), Container3d::PointFilter_Locked)->GetCoordsArray(),
                                      4, false,
                                      Color_Yellow );

  interp->GetPlotter().REDRAW_POINTS( (containerName + "_occupied").c_str(),
                                      Container3d::GetPoints3d(container->GetGrid(), Container3d::PointFilter_Occupied)->GetCoordsArray(),
                                      4, false,
                                      Color_Red );

  // TODO: NYI

  return TCL_OK;
}

//-----------------------------------------------------------------------------

void cmdNesting::Factory(const Handle(asiTcl_Interp)&      interp,
                         const Handle(Standard_Transient)& data)
{
  static const char* group = "cmdNesting";

  /* ==========================
   *  Initialize UI facilities
   * ========================== */

  // Get common facilities
  Handle(asiUI_CommonFacilities)
    passedCF = Handle(asiUI_CommonFacilities)::DownCast(data);
  //
  if ( passedCF.IsNull() )
    interp->GetProgress().SendLogMessage(LogWarn(Normal) << "[cmdNesting] UI facilities are not available. GUI may not be updated.");
  else
    cf = passedCF;

  /* ================================
   *  Initialize Data Model instance
   * ================================ */

  model = Handle(asiEngine_Model)::DownCast( interp->GetModel() );
  //
  if ( model.IsNull() )
  {
    interp->GetProgress().SendLogMessage(LogErr(Normal) << "[cmdNesting] Data Model instance is null or not of asiEngine_Model kind.");
    return;
  }

  /* ==================
   *  Add Tcl commands
   * ================== */

  //-------------------------------------------------------------------------//
  interp->AddCommand("nesting-build-container3d",
    //
    "nesting-build-container3d <dx> <dy> <dz> [-step <step>]\n"
    "\t Builds a 3D build volume for nesting.",
    //
    __FILE__, group, NESTING_BuildContainer3d);

  //-------------------------------------------------------------------------//
  interp->AddCommand("nesting-load-part",
    //
    "nesting-load-part -filename <filename> -name <name>\n"
    "\t Loads a part from the given file and stores it in a Tcl var named <name>.",
    //
    __FILE__, group, NESTING_LoadPart);

  //-------------------------------------------------------------------------//
  interp->AddCommand("nesting-place-part",
    //
    "nesting-place-part -part <partName> -container <containerName> [-draw]\n"
    "\t Adds a part named <partName> to the container named <name>.",
    //
    __FILE__, group, NESTING_PlacePart);
}

// Declare entry point PLUGINFACTORY
ASIPLUGIN(cmdNesting)
