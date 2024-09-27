//-----------------------------------------------------------------------------
// Created on: 7 March 2024
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

// Own include
#include "asiUI_DialogBreakDownAssembly.h"

// asiAsm includes
#include <asiAsm_XdeBreakDown.h>

// Qt includes
#include <Standard_WarningsDisable.hxx>
//
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QCoreApplication>
//
#include <Standard_WarningsRestore.hxx>

using namespace asiAsm::xde;

//-----------------------------------------------------------------------------

#define CONTROL_BTN_WIDTH 150

//-----------------------------------------------------------------------------

asiUI_DialogBreakDownAssembly::asiUI_DialogBreakDownAssembly(ActAPI_ProgressEntry progress,
                                                             ActAPI_PlotterEntry  plotter,
                                                             QWidget*             parent)
: QDialog      ( parent, Qt::WindowFlags(Qt::WindowModal) ),
  m_progress   ( progress ),
  m_plotter    ( plotter ),
  m_bExpand    ( true ),
  m_bUsePrefix ( true )
{
  // Main layout.
  m_pMainLayout = new QVBoxLayout();
  m_pMainLayout->setSpacing(12);

  // Filename for STEP data.
  m_widgets.pSelectInputFileLabel = new QLabel("CAD assembly filename", this);
  m_widgets.pSelectInputFile      = new asiUI_SelectFile("CAD file (*.step *.stp)",
                                                         "Open CAD file",
                                                          QString(),
                                                          QImage(":icons/asitus/select_xyz.svg"),
                                                          asiUI_Common::OpenSaveAction::OpenSaveAction_Open,
                                                          this);

  // Output directory.
  m_widgets.pSelectOutputDirLabel = new QLabel("Output directory", this);
  m_widgets.pSelectOutputDir      = new asiUI_SelectFile("",
                                                         "Set output directory",
                                                          QString(),
                                                          QImage(":icons/asitus/select_xyz.svg"),
                                                          asiUI_Common::OpenSaveAction::OpenSaveAction_OpenDirectory,
                                                          this);

  // Expand compounds.
  QFrame* pExpandFrame = new QFrame;
  //
  {
    m_widgets.pExpandCompounds = new QCheckBox;

    QLabel*      pExpandLbl    = new QLabel("Expand compound parts");
    QHBoxLayout* pExpandLayout = new QHBoxLayout(pExpandFrame);
    //
    pExpandLayout->addWidget(pExpandLbl);
    pExpandLayout->addWidget(m_widgets.pExpandCompounds);
    pExpandLayout->addStretch(1);
    pExpandLayout->setSpacing(10);
    pExpandLayout->setContentsMargins(10, 10, 10, 0);
  }

  // Prefix filename.
  QFrame* pPrefixFrame = new QFrame;
  //
  {
    m_widgets.pPrefixFilename = new QCheckBox;

    QLabel*      pPrefixLbl    = new QLabel("Use filename prefix");
    QHBoxLayout* pPrefixLayout = new QHBoxLayout(pPrefixFrame);
    //
    pPrefixLayout->addWidget(pPrefixLbl);
    pPrefixLayout->addWidget(m_widgets.pPrefixFilename);
    pPrefixLayout->addStretch(1);
    pPrefixLayout->setSpacing(10);
    pPrefixLayout->setContentsMargins(10, 10, 10, 0);
  }

  //---------------------------------------------------------------------------
  // Buttons
  //---------------------------------------------------------------------------

  m_widgets.pApply = new QPushButton("Convert");
  m_widgets.pApply->setFocusPolicy( Qt::NoFocus );

  m_widgets.pClose = new QPushButton("Close");
  m_widgets.pClose->setFocusPolicy( Qt::NoFocus );

  // Sizing.
  m_widgets.pApply->setMaximumWidth(CONTROL_BTN_WIDTH);
  m_widgets.pClose->setMaximumWidth(CONTROL_BTN_WIDTH);

  // Reaction.
  connect( m_widgets.pSelectInputFile, SIGNAL( textChanged               (const QString&) ),
           this,                       SLOT  ( onInitialFileSelected     ()               ) );
  connect( m_widgets.pSelectOutputDir, SIGNAL( textChanged               (const QString&) ),
           this,                       SLOT  ( onOutputDirectorySelected ()               ) );
  connect( m_widgets.pApply,           SIGNAL( clicked                   ()               ),
           this,                       SLOT  ( onApply                   ()               ) );
  connect( m_widgets.pClose,           SIGNAL( clicked                   ()               ),
           this,                       SLOT  ( close                     ()               ) );
  connect( m_widgets.pExpandCompounds, SIGNAL( clicked                   ()               ),
           this,                       SLOT  ( onExpandCompounds         ()               ) );
  connect( m_widgets.pPrefixFilename,  SIGNAL( clicked                   ()               ),
           this,                       SLOT  ( onUsePrefix               ()               ) );

  //---------------------------------------------------------------------------
  // Layout
  //---------------------------------------------------------------------------

  // Layout for buttons.
  QFrame* bButtonsFrame = new QFrame;
  {
    QHBoxLayout* pButtonsLayout = new QHBoxLayout(bButtonsFrame);
    pButtonsLayout->setSpacing(10);
    //
    pButtonsLayout->addWidget(m_widgets.pApply);
    pButtonsLayout->addWidget(m_widgets.pClose);
  }

  //---------------------------------------------------------------------------
  // Progress indication
  //---------------------------------------------------------------------------

  m_widgets.pProgressFrame = new QWidget(this);

  // Progress indicator.
  m_widgets.pProgressBar = new QProgressBar;
  m_widgets.pProgressBar->setRange(0, 0);
  m_widgets.pProgressBar->setTextVisible(false);
  m_widgets.pProgressBar->setAlignment(Qt::AlignHCenter);
  m_widgets.pProgressBar->setMinimumWidth(200);
  m_widgets.pProgressFrame->hide();

  // Cancel button
  QIcon cancelIcon(":icons/asitus/asitus_cancel_icon_16x16.png");
  //
  QToolButton* pCancelButton = new QToolButton(m_widgets.pProgressFrame);
  pCancelButton->setIcon(cancelIcon);
  pCancelButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  pCancelButton->setSizePolicy( QSizePolicy(QSizePolicy::Preferred,
                                            QSizePolicy::Expanding) );
  //
  connect( pCancelButton, SIGNAL( clicked() ), this, SLOT( onCancel() ) );

  // Configure layout.
  QHBoxLayout* progressLayout = new QHBoxLayout();
  progressLayout->setMargin(2);
  progressLayout->setSpacing(3);
  progressLayout->addWidget(m_widgets.pProgressBar, 10, Qt::AlignVCenter);
  progressLayout->addWidget(pCancelButton, 0, Qt::AlignVCenter);
  //
  m_widgets.pProgressFrame->setLayout(progressLayout);

  // Local progress notifier.
  m_localProgress = new asiUI_ProgressNotifier;
  //
  QObject* pSignaler = m_localProgress->SignalNotifier();
  //
  connect( pSignaler, SIGNAL ( ProgressUpdated() ),
           this,      SLOT   ( onProgressUpdated() ),
           Qt::AutoConnection );
  //
  connect( pSignaler, SIGNAL ( StatusChanged(const int) ),
           this,      SLOT   ( onStatusChanged(const int) ),
           Qt::AutoConnection );

  //---------------------------------------------------------------------------
  // Main layout
  //---------------------------------------------------------------------------

  // Configure main layout.
  m_pMainLayout->addWidget(m_widgets.pSelectInputFileLabel);
  m_pMainLayout->addWidget(m_widgets.pSelectInputFile);
  m_pMainLayout->addWidget(m_widgets.pSelectOutputDirLabel);
  m_pMainLayout->addWidget(m_widgets.pSelectOutputDir);
  m_pMainLayout->addWidget(pExpandFrame);
  m_pMainLayout->addWidget(pPrefixFrame);
  m_pMainLayout->addWidget(bButtonsFrame);
  m_pMainLayout->addWidget(m_widgets.pProgressFrame);
  //
  m_pMainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  m_pMainLayout->setContentsMargins(10, 10, 10, 10);

  // Set good initial state.
  this->setMinimumSize ( QSize(650, 320) );
  this->setLayout      ( m_pMainLayout );
  this->setWindowTitle ( "Break down assembly" );
  //
  m_widgets.pExpandCompounds->setChecked(true);
  m_widgets.pPrefixFilename->setChecked(true);
}

//-----------------------------------------------------------------------------

asiUI_DialogBreakDownAssembly::~asiUI_DialogBreakDownAssembly()
{}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onExpandCompounds()
{
  m_bExpand = m_widgets.pExpandCompounds->isChecked();
}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onUsePrefix()
{
  m_bUsePrefix = m_widgets.pPrefixFilename->isChecked();
}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onInitialFileSelected()
{
  QString fileName = m_widgets.pSelectInputFile->text();

  if ( fileName.isEmpty() )
    return;

  QFile file(fileName);
  //
  if ( !file.exists() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "File '%1' does not exist."
                                             << QStr2StdStr(fileName) );
    return;
  }

  m_fileName = fileName;
}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onOutputDirectorySelected()
{
  QString path = m_widgets.pSelectOutputDir->text();

  if ( path.isEmpty() )
    return;

  QDir dir(path);
  //
  if ( !dir.exists() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Directory '%1' does not exist."
                                             << QStr2StdStr(path) );
    return;
  }

  m_outputDir = path;
}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onApply()
{
  ActAPI_ProgressEntry localProgress(m_localProgress);

  t_asciiString fn     = QStr2AsciiStr(m_fileName);
  t_asciiString prefix = asiAlgo_Utils::Str::BaseFilename(fn, false);

  // Load assembly.
  Handle(Doc) xdeDoc = new Doc(localProgress);
  //
  if ( !xdeDoc->Load(fn) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Cannot load passed *.step file into XDE document.");
    return;
  }

  // Break down assembly and export the elementary parts to files.
  BreakDown breakAssembly(localProgress, m_plotter);
  //
  breakAssembly.SetExpandCompounds(m_bExpand);
  //
  if ( m_bUsePrefix )
    breakAssembly.SetFilenamePrefix(prefix);
  //
  if ( !breakAssembly.Perform( xdeDoc,
                               QStr2AsciiStr(m_outputDir) ) )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Breaking assembly onto parts failed.");
  }

  m_progress.SendLogMessage(LogInfo(Normal) << "Breaking assembly onto parts is done.");
}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onProgressUpdated()
{
  if ( m_localProgress->ProgressStatus() == Progress_Undefined )
    return;

  if ( !m_localProgress->IsInfinite() )
  {
    const int capacity = m_localProgress->Capacity();

    if ( capacity )
    {
      double progress = (double) m_localProgress->CurrentProgress() / capacity;
      m_widgets.pProgressBar->setValue(int(progress * 100));
    }
    else
      m_widgets.pProgressBar->setValue(0);
  }
  else
  {
    m_widgets.pProgressBar->setValue(100);
  }
}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onStatusChanged(const int status)
{
  switch ( status )
  {
    case Progress_Running:
    {
      m_widgets.pProgressFrame->setVisible(true);
      m_widgets.pProgressBar->setRange(0, 100);
    }
    break;

    case Progress_Canceled:
    case Progress_Succeeded:
    case Progress_Failed:
    {
      m_widgets.pProgressFrame->setVisible(false);
    }
    break;

    default:
      break;
  }

  this->adjustSize();
  QCoreApplication::processEvents(); // keep it to have enough time for dialog's background update
}

//-----------------------------------------------------------------------------

void asiUI_DialogBreakDownAssembly::onCancel()
{
  if ( m_localProgress->ProgressStatus() == Progress_Running )
    m_localProgress->Cancel();
}
