//-----------------------------------------------------------------------------
// Created on: 07 March 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Julia Slyadneva
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

#pragma once

// asiUI includes
#include <asiUI_ProgressNotifier.h>
#include <asiUI_SelectFile.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QCheckBox>
#include <QDialog>
#include <QProgressBar>
#include <QPushButton>
//
#include <Standard_WarningsRestore.hxx>

//-----------------------------------------------------------------------------

//! Dialog for breaking down assemblies into parts.
class asiUI_DialogBreakDownAssembly : public QDialog
{
  Q_OBJECT

public:

  //! Ctor.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  //! \param[in] parent   the parent widget.
  asiUI_EXPORT
    asiUI_DialogBreakDownAssembly(ActAPI_ProgressEntry progress,
                                  ActAPI_PlotterEntry  plotter,
                                  QWidget*             parent = nullptr);

  //! Dtor.
  asiUI_EXPORT
    virtual ~asiUI_DialogBreakDownAssembly();

public slots:

  //! Expand compounds on/off.
  void onExpandCompounds();

  //! Use prefix filename on/off.
  void onUsePrefix();

  //! Assembly's filename is selected.
  void onInitialFileSelected();

  //! Output directory is selected.
  void onOutputDirectorySelected();

  //! Reaction on clicking "Apply" button.
  void onApply();

  //! Updates UI progress bar according to an algorithm progress.
  void onProgressUpdated();

  //! Updates UI progress bar visibity according to an algorithm status.
  void onStatusChanged(const int);

  //! Prevents algorithm for further execution.
  void onCancel();

private:

  //! Widgets.
  struct t_widgets
  {
    QPushButton*      pApply;                //!< Apply surface fitting method.
    QPushButton*      pClose;                //!< Closes the dialog.
    QLabel*           pSelectInputFileLabel; //!< STEP file selector label.
    asiUI_SelectFile* pSelectInputFile;      //!< STEP file selector.
    QLabel*           pSelectOutputDirLabel; //!< Output directory selector label.
    asiUI_SelectFile* pSelectOutputDir;      //!< Output directory selector.
    QWidget*          pProgressFrame;        //!< Progress widget.
    QProgressBar*     pProgressBar;          //!< Progress bar.
    QCheckBox*        pExpandCompounds;      //!< Expand compounds on/off.
    QCheckBox*        pPrefixFilename;       //!< Prefix the original filename on/off.

    //! Default ctor.
    t_widgets() : pApply                (nullptr),
                  pClose                (nullptr),
                  pSelectInputFileLabel (nullptr),
                  pSelectInputFile      (nullptr),
                  pSelectOutputDirLabel (nullptr),
                  pSelectOutputDir      (nullptr),
                  pProgressFrame        (nullptr),
                  pProgressBar          (nullptr),
                  pExpandCompounds      (nullptr),
                  pPrefixFilename       (nullptr)
    {}

    void Release()
    {
      delete pApply;                 pApply                = nullptr;
      delete pClose;                 pClose                = nullptr;
      delete pSelectInputFileLabel;  pSelectInputFileLabel = nullptr;
      delete pSelectInputFile;       pSelectInputFile      = nullptr;
      delete pSelectOutputDirLabel;  pSelectOutputDirLabel = nullptr;
      delete pSelectOutputDir;       pSelectOutputDir      = nullptr;
      delete pProgressFrame;         pProgressFrame        = nullptr;
      delete pProgressBar;           pProgressBar          = nullptr;
      delete pExpandCompounds;       pExpandCompounds      = nullptr;
      delete pPrefixFilename;        pPrefixFilename       = nullptr;
    }
  };

  t_widgets            m_widgets;     //!< UI controls.
  QVBoxLayout*         m_pMainLayout; //!< Layout of the widget.

  /* User's data */

  bool                 m_bExpand;    //!< Expand compounds on/off.
  bool                 m_bUsePrefix; //!< Whether to use the original filename as a prefix.
  QString              m_fileName;   //!< The target filename defined by an user.
  QString              m_outputDir;  //!< The target output directory defined by an user.

  /* Diagnostics */

  ActAPI_ProgressEntry m_progress; //!< Progress notifier.
  ActAPI_PlotterEntry  m_plotter;  //!< Imperative plotter.

  Handle(asiUI_ProgressNotifier) m_localProgress; //!< Local progress notifier.
};
