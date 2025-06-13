//-----------------------------------------------------------------------------
// Created on: 13 June 2025
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

// Own include
#include "asiUI_DialogMakeFlange.h"

// asiUI includes
#include "asiUI_IV.h"

// asiAlgo includes
#include <asiAlgo_Timer.h>

// asiEngine includes
#include <asiEngine_Part.h>

// OpenCascade includes
#include <BRep_Builder.hxx>

// VTK includes
#pragma warning(push, 0)
#include <vtkCallbackCommand.h>
#include <vtkNamedColors.h>
#include <vtkProperty2D.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkTextProperty.h>
#pragma warning(pop)

// Qt includes
#pragma warning(push, 0)
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#pragma warning(pop)

//-----------------------------------------------------------------------------

using namespace asiSmm;

//-----------------------------------------------------------------------------

#define CONTROL_EDIT_WIDTH 100
#define CONTROL_BTN_WIDTH 250
#define CONTROL_TBL_WIDTH 350

#define DefaultAngleDeg 90.
#define DefaultFlangeLength 50.

//-----------------------------------------------------------------------------

static const char* const image0_data[] = {
"48 48 10 1",
". c None",
"a c #000000",
"g c #008200",
"h c #008284",
"e c #840000",
"b c #0f9edb",
"c c #848284",
"# c #c6c3c6",
"d c #52cbff",
"f c #ffffff",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
".......................#aa......................",
".......................abb...............#aaac..",
".......................abb.............cbdbabda.",
"..ccccccccc#####abdddddddddbbeeaa#.caabbbb...#dc",
"..ddddddddddddddddddddd#####ddddbbbaaaebba....ba",
"..bbbbbbbbdddddddddddd#ffff#dddbbbeaaegbba....ba",
"..fcccaaabbbbbddddddd##ffff#dcbbeeaaaeabbb...#d#",
"..........caaebbbbdddd#fff##dcbbaeaeabaebbbaeda.",
"............#aegbbbdddd####dcbbeaaeebegaaaaaa#..",
"..............aaaabbbbdd###bbbeeaaaac...........",
"...............#aeaeebbbbbbbbbebaa..............",
"..................aeeaeebbbbbeac................",
"...................#aaeebeeaac..................",
"......................cbbea.....................",
".......................dba#.....................",
"......................#bbag.....................",
"......................adbaa.....................",
"......................bdbaa.....................",
".....................cddbbea....................",
".....................dddbbaec...................",
"...................cdddddbaaae..................",
"................#bddddddbbeaaaaaf...............",
"................cccccccccbbhb.e.c...............",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................",
"................................................"};

//-----------------------------------------------------------------------------

namespace
{
  // Callback for the interaction with the bend angle.
  class GizmoCallbackAngle : public vtkCallbackCommand
  {
  public:
    static GizmoCallbackAngle* New()
    {
      return new GizmoCallbackAngle;
    }
    virtual void Execute(vtkObject* caller, unsigned long, void*)
    {
      if ( Blocked )
        return;

      vtkSliderWidget*
        sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
      vtkSliderRepresentation*
        sliderRep = static_cast<vtkSliderRepresentation*>( sliderWidget->GetRepresentation() );

      // Take value from the slider.
      const double bendAngleDeg = sliderRep->GetValue();
      //
      this->Dialog->SetBendAngleDeg(bendAngleDeg);
    }
    GizmoCallbackAngle() : Dialog(nullptr), Blocked(false)
    {
    }
    asiUI_DialogMakeFlange* Dialog;
    bool                    Blocked;
  };
}

//-----------------------------------------------------------------------------

asiUI_DialogMakeFlange::asiUI_DialogMakeFlange(const Handle(asiUI_CommonFacilities)& cf,
                                               const Handle(asiEngine_Model)&        model,
                                               ActAPI_ProgressEntry                  progress,
                                               ActAPI_PlotterEntry                   plotter,
                                               QWidget*                              parent)
: QDialog    (parent),
  m_cf       (cf),
  m_model    (model),
  m_progress (progress),
  m_plotter  (plotter)
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();

  // Frames for controls.
  QFrame*    pEdgesFrame      = new QFrame;
  QGroupBox* pGroupParameters = new QGroupBox("Parameters");
  QFrame*    pButtonsFrame    = new QFrame;

  // Selected edges.
  m_widgets.pEdges = new asiUI_LineEdit();

  // Datums for flange parameters.
  m_widgets.pAngleDeg     = cf->WidgetFactory->CreateEditor("SMM_BendAngle",    this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);
  m_widgets.pFlangeLength = cf->WidgetFactory->CreateEditor("SMM_FlangeLength", this, asiUI_Datum::All | asiUI_Datum::UseMinMaxRange);
  //
  m_widgets.pAngleDeg    ->SetValue(DefaultAngleDeg);
  m_widgets.pFlangeLength->SetValue(DefaultFlangeLength);

  //---------------------------------------------------------------------------
  // Interactive sliders
  //---------------------------------------------------------------------------

  vtkNew<vtkNamedColors> colors;

  // Bend angle gizmo.
  {
    vtkNew<vtkSliderRepresentation2D> sliderRep;

    sliderRep->SetMinimumValue(0.0);
    sliderRep->SetMaximumValue(360.0);
    sliderRep->SetValue(DefaultAngleDeg);
    sliderRep->SetTitleText("Bend angle");

    // Set color properties:
    //   // Set color properties:
    // Change the color of the knob that slides
    sliderRep->GetSliderProperty()->SetColor(
        colors->GetColor3d("Green").GetData());
    // Change the color of the text indicating what the slider controls
    sliderRep->GetTitleProperty()->SetColor(
        colors->GetColor3d("AliceBlue").GetData());
    // Change the color of the text displaying the value
    sliderRep->GetLabelProperty()->SetColor(
        colors->GetColor3d("AliceBlue").GetData());
    // Change the color of the knob when the mouse is held on it
    sliderRep->GetSelectedProperty()->SetColor(
        colors->GetColor3d("DeepPink").GetData());
    // Change the color of the bar
    sliderRep->GetTubeProperty()->SetColor(
        colors->GetColor3d("MistyRose").GetData());
    // Change the color of the ends of the bar
    sliderRep->GetCapProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
    sliderRep->SetSliderLength(0.05);
    sliderRep->SetSliderWidth(0.025);
    sliderRep->SetEndCapLength(0.02);

    // Display pixel values (640 X 480)
    // sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    // sliderRep->GetPoint1Coordinate()->SetValue(128, 48);
    // sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    // sliderRep->GetPoint2Coordinate()->SetValue(512, 48);
    // Or use this - better because it scales to the window size:
    sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    sliderRep->GetPoint1Coordinate()->SetValue(0.2, 0.1);
    sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
    sliderRep->GetPoint2Coordinate()->SetValue(0.8, 0.1);

    m_widgets.sliderAngle = vtkSmartPointer<vtkSliderWidget>::New();
    //
    m_widgets.sliderAngle->SetInteractor( m_cf->ViewerPart->PrsMgr()->GetRenderWindow()->GetInteractor() );
    m_widgets.sliderAngle->SetRepresentation(sliderRep);
    m_widgets.sliderAngle->SetAnimationModeToAnimate();

    // Set interaction callback.
    vtkNew<::GizmoCallbackAngle> callback;
    callback->Dialog = this;
    //
    m_widgets.sliderAngle->AddObserver(vtkCommand::InteractionEvent/*vtkCommand::EndInteractionEvent*/, callback);
  }

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Apply");
  m_widgets.pApply->setFocusPolicy( Qt::NoFocus );

  m_widgets.pClose = new QPushButton("Close");
  m_widgets.pClose->setFocusPolicy( Qt::NoFocus );

  // Sizing.
  m_widgets.pApply->setMaximumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pClose->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reactions.
  connect( m_widgets.pApply,        SIGNAL( clicked() ),
           this,                    SLOT  ( onApply() ) );
  connect( m_widgets.pClose,        SIGNAL( clicked() ),
           this,                    SLOT  ( onClose() ) );
  connect( m_cf->ViewerPart,        SIGNAL( edgePicked(asiVisu_PickerResult*) ),
           this,                    SLOT  ( onEdgePicked() ) );
  connect( m_widgets.pAngleDeg,     SIGNAL( ValueChanged() ),
           this,                    SLOT  ( onPreview() ) );
  connect( m_widgets.pFlangeLength, SIGNAL( ValueChanged() ),
           this,                    SLOT  ( onPreview() ) );

  //---------------------------------------------------------------------------
  // Layout
  //---------------------------------------------------------------------------

  // Layout for edges.
  {
    QHBoxLayout* pLayout = new QHBoxLayout(pEdgesFrame);
    //
    pLayout->setSpacing(10);
    pLayout->setContentsMargins(5, 0, 5, 0);
    //
    pLayout->addWidget( new QLabel("Edge indices") );
    pLayout->addWidget( m_widgets.pEdges );
    //
    pLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  }

  // Layout for parameters.
  {
    QVBoxLayout* pLayout    = new QVBoxLayout(pGroupParameters);
    QFrame*      pGridFrame = new QFrame;

    pLayout->setAlignment(Qt::AlignTop);

    // Grid.
    QGridLayout* pGridLayout = new QGridLayout(pGridFrame);
    pGridLayout->setSpacing(10);
    pGridLayout->setContentsMargins(10, 0, 10, 0);
    //
    m_widgets.pAngleDeg    ->AddTo(pGridLayout, 0, 0);
    m_widgets.pFlangeLength->AddTo(pGridLayout, 1, 0);
    //
    pGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    //
    pLayout->addSpacing(10);
    pLayout->addWidget(pGridFrame);
  }

  // Layout for buttons.
  {
    QHBoxLayout* pButtonsLayout = new QHBoxLayout(pButtonsFrame);
    pButtonsLayout->setSpacing(10);
    //
    pButtonsLayout->addWidget(m_widgets.pApply);
    pButtonsLayout->addWidget(m_widgets.pClose);
  }

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout.
  m_pMainLayout->addWidget(pEdgesFrame);
  m_pMainLayout->addWidget(pGroupParameters);
  m_pMainLayout->addWidget(pButtonsFrame);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  this->setLayout(m_pMainLayout);
  this->setWindowModality(Qt::NonModal);
  this->setWindowTitle("Define sheet metal flange");
  this->setWindowIcon( QIcon( QPixmap( (const char**) image0_data ) ) );

  // Make sure to kill the algorithm on close.
  this->setAttribute(Qt::WA_DeleteOnClose);


  //---------------------------------------------------------------------------
  // Prepare the tool
  //---------------------------------------------------------------------------

  asiEngine_Part partApi( m_model, m_cf->ViewerPart->PrsMgr() );

  // Get AAG of the loaded part.
  Handle(asiAlgo_AAG) aag = partApi.GetAAG();

  // Construct the algorithm.
  m_algo = new MakeEdgeFlange(aag, nullptr, nullptr);
}

//-----------------------------------------------------------------------------

asiUI_DialogMakeFlange::~asiUI_DialogMakeFlange()
{
  this->cleanUpPreviews();
}

//-----------------------------------------------------------------------------

void asiUI_DialogMakeFlange::SetBendAngleDeg(const double angleDeg)
{
  m_widgets.pAngleDeg->blockSignals(true);
  m_widgets.pAngleDeg->SetValue(angleDeg);
  m_widgets.pAngleDeg->blockSignals(false);

  this->onPreview();
}

//-----------------------------------------------------------------------------

void asiUI_DialogMakeFlange::onEdgePicked()
{
  if ( m_algo.IsNull() )
    return;

  /* Read edge IDs from the viewer. */

  asiEngine_Part partApi( m_model, m_cf->ViewerPart->PrsMgr() );

  // Get selected edges.
  TColStd_PackedMapOfInteger eids;
  partApi.GetHighlightedEdges(eids);
  //
  Handle(asiAlgo_AAG) aag = partApi.GetAAG();
  //
  if ( aag.IsNull() )
  {
    m_progress.SendLogMessage(LogWarn(Normal) << "Active part is null. Please, load a model before selecting edges.");
    return;
  }

  const TopTools_IndexedMapOfShape& allEdges = aag->RequestMapOfEdges();

  // Collect edges.
  BRep_Builder bbuilder;
  TopoDS_Compound edgeComp;
  bbuilder.MakeCompound(edgeComp);
  //
  QStringList edgeIds;
  for ( TColStd_PackedMapOfInteger::Iterator eit(eids); eit.More(); eit.Next() )
  {
    const int edgeId = eit.Key();

    if ( edgeId >= 1 && edgeId <= allEdges.Extent() )
    {
      bbuilder.Add( edgeComp, allEdges(edgeId) );
      edgeIds.append( QString::number(edgeId) );
    }
    else
    {
      m_progress.SendLogMessage(LogWarn(Normal) << "Edge with ID %1 does not exist in the model."
                                                << edgeId);
    }
  }

  m_widgets.pEdges->setText(edgeIds.join(" "));

  /* Preview. */

  this->cleanUpPreviews();
  this->onPreview();
}

//-----------------------------------------------------------------------------

void asiUI_DialogMakeFlange::onApply()
{
  // Read edge IDs.
  TColStd_PackedMapOfInteger eids;
  QStringList eidList = m_widgets.pEdges->text().split(QRegExp("[\\D]+"), QString::SkipEmptyParts);
  //
  for ( const auto& eidStr : eidList )
  {
    const int eid = eidStr.toInt();
    eids.Add(eid);
  }

  // Read parameters.
  const double bendAngleDeg = m_widgets.pAngleDeg->GetDouble();
  const double flangeLength = m_widgets.pFlangeLength->GetDouble();

  // Let the algorithm talk.
  m_algo->SetProgress(m_progress);

  // Do modeling.
  for ( TColStd_PackedMapOfInteger::Iterator eit(eids); eit.More(); eit.Next() )
  {
    const int eid = eit.Key();

    if ( !m_algo->Build(eid, bendAngleDeg, flangeLength) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Failed to build feature solids for the edge %1."
                                               << eid);
    }

    const TopoDS_Shape& result = m_algo->GetResult();

    // Update part.
    m_model->OpenCommand();
    {
      asiEngine_Part(m_model).Update(result);
    }
    m_model->CommitCommand();

    // Reinitialize the algorithm.
    m_algo->Initialize(result);
  }

  this->cleanUpPreviews();

  // Update part.
  m_cf->ViewerPart->PrsMgr()->Actualize( asiEngine_Part(m_model).GetPart() );

  // Reset edge IDs.
  m_widgets.pEdges->setText("");
}

//-----------------------------------------------------------------------------

void asiUI_DialogMakeFlange::onClose()
{
  this->close();
}

//-----------------------------------------------------------------------------

void asiUI_DialogMakeFlange::onPreview()
{
  if ( m_algo.IsNull() )
    return;

  // Shut up the algorithm.
  m_algo->SetProgress(nullptr);

  /* =================
   *  Read parameters.
   * ================= */

  // Read edge IDs.
  TColStd_PackedMapOfInteger eids;
  QStringList eidList = m_widgets.pEdges->text().split(QRegExp("[\\D]+"), QString::SkipEmptyParts);
  //
  for ( const auto& eidStr : eidList )
  {
    const int eid = eidStr.toInt();
    eids.Add(eid);
  }

  if ( eids.IsEmpty() )
    return;

  // Enable gizmos.
  m_widgets.sliderAngle->EnabledOn();

  // Read flange parameters.
  const double bendAngleDeg = m_widgets.pAngleDeg->GetDouble();
  const double flangeLength = m_widgets.pFlangeLength->GetDouble();

  /* ========================
   *  Preview feature solids.
   * ======================== */

  Handle(asiUI_IV)
    IV = Handle(asiUI_IV)::DownCast( m_plotter.Access() );

  // Shut down Qt events.
  bool pe = IV->IsProcessEventsOn();
  IV->SetProcessEvents(false);

  // Iterate over the feature edges.
  for ( TColStd_PackedMapOfInteger::Iterator eit(eids); eit.More(); eit.Next() )
  {
    const int eid = eit.Key();

    TopoDS_Solid bendSolid, wallSolid;
    //
    if ( !m_algo->BuildFeatureSolids(eid, bendAngleDeg, flangeLength, bendSolid, wallSolid) )
    {
      m_progress.SendLogMessage(LogErr(Normal) << "Failed to build feature solids for the edge %1."
                                               << eid);
    }

    // Make previews.
    t_asciiString bendPreviewName = t_asciiString("bendSolid_eid_") + eid;
    t_asciiString wallPreviewName = t_asciiString("wallSolid_eid_") + eid;
    //
    m_plotter.REDRAW_SHAPE( bendPreviewName, bendSolid, Color_Red, 1. );
    m_plotter.REDRAW_SHAPE( wallPreviewName, wallSolid, Color_Green, 1. );
    //
    m_previewNames.push_back(bendPreviewName);
    m_previewNames.push_back(wallPreviewName);
  }

  // Restore the previous Qt events processing state.
  IV->SetProcessEvents(pe);
}

//-----------------------------------------------------------------------------

void asiUI_DialogMakeFlange::cleanUpPreviews()
{
  // Hide gizmos.
  m_widgets.sliderAngle->EnabledOff();

  // Delete preview objects.
  for ( const auto& name : m_previewNames )
  {
    Handle(ActAPI_INode) N = m_model->FindNodeByName(name);
    //
    if ( !N.IsNull() )
    {
      m_cf->ViewerPart->PrsMgr()->DeletePresentation(N);

      // Delete the persistent OCAF object.
      m_model->OpenCommand();
      {
        m_model->DeleteNode(N);
      }
      m_model->CommitCommand();
    }
  }

  // Update UI state.
  m_cf->ObjectBrowser->Populate();
  m_cf->ViewerPart->Repaint();
}
