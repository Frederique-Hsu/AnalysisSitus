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

#ifndef asiUI_DialogMakeFlange_h
#define asiUI_DialogMakeFlange_h

// asiUI includes
#include "asiUI_CommonFacilities.h"
#include "asiUI_Datum.h"
#include "asiUI_LineEdit.h"

// asiSmm includes
#include <asiSmm_MakeEdgeFlange.h>

// asiEngine includes
#include <asiEngine_Model.h>

// VTK includes
#include <vtkSliderWidget.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
//
#include <Standard_WarningsRestore.hxx>

//-----------------------------------------------------------------------------

//! Dialog for sheet metal flange definition with a preview.
class asiUI_DialogMakeFlange : public QDialog
{
  Q_OBJECT

public:

  //! Ctor.
  //! \param[in] cf       the common facilities.
  //! \param[in] model    the data model instance.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  //! \param[in] parent   the parent widget.
  asiUI_EXPORT
    asiUI_DialogMakeFlange(const Handle(asiUI_CommonFacilities)& cf,
                           const Handle(asiEngine_Model)&        model,
                           ActAPI_ProgressEntry                  progress,
                           ActAPI_PlotterEntry                   plotter,
                           QWidget*                              parent = nullptr);

  //! Dtor.
  asiUI_EXPORT virtual
    ~asiUI_DialogMakeFlange();

public:

  //! Sets a bend angle in degrees.
  //! \param[in] angleDeg the value to set.
  asiUI_EXPORT void
    SetBendAngleDeg(const double angleDeg);

public slots:

  //! Reaction on edge selection in the viewer.
  void onEdgePicked();

  //! Reaction on clicking "Apply" button.
  void onApply();

  //! Reaction on closing the dialog.
  void onClose();

  //! Reaction on preview.
  void onPreview();

protected:

  void cleanUpPreviews();

protected:

  //! Widgets.
  struct t_widgets
  {
    QPushButton*    pApply;        //!< Perform nesting.
    QPushButton*    pClose;        //!< Closes the dialog.
    asiUI_LineEdit* pEdges;        //!< Indices of edges.
    asiUI_Datum*    pAngleDeg;     //!< Bend angle.
    asiUI_Datum*    pFlangeLength; //!< Flange length.

    //! Bend angle gizmo.
    vtkSmartPointer<vtkSliderWidget> sliderAngle;

    //! Flange length gizmo.
    vtkSmartPointer<vtkSliderWidget> sliderLength;

    //! Default ctor.
    t_widgets() : pApply        (nullptr),
                  pClose        (nullptr),
                  pEdges        (nullptr),
                  pAngleDeg     (nullptr),
                  pFlangeLength (nullptr)
    {}

    void Release()
    {
      delete pApply;        pApply        = nullptr;
      delete pClose;        pClose        = nullptr;
      delete pEdges;        pEdges        = nullptr;
      delete pAngleDeg;     pAngleDeg     = nullptr;
      delete pFlangeLength; pFlangeLength = nullptr;
    }
  };

  Handle(asiUI_CommonFacilities) m_cf;          //!< UI common facilities.
  t_widgets                      m_widgets;     //!< UI controls.
  QVBoxLayout*                   m_pMainLayout; //!< Layout of the widget.
  Handle(asiEngine_Model)        m_model;       //!< Data Model instance.
  Handle(asiSmm::MakeEdgeFlange) m_algo;        //!< Flange construction algorithm.

  //! Names of all used preview shapes.
  std::vector<t_asciiString> m_previewNames;

  /* Diagnostics */

  ActAPI_ProgressEntry m_progress; //!< Progress notifier.
  ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

};

#endif // asiUI_DialogMakeFlange_h
