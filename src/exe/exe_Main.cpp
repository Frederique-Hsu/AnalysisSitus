//-----------------------------------------------------------------------------
// Created on: 07 November 2015
//-----------------------------------------------------------------------------
// Copyright (c) 2015-present, Sergey Slyadnev
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

#if defined _WIN32
  #define RuntimePathVar "PATH"
#else
  #define RuntimePathVar "LD_LIBRARY_PATH"
#endif

#undef VTK_TEST
#ifndef VTK_TEST

// exe includes
#include <exe_CommonFacilities.h>
#include <exe_GenerateDocs.h>
#include <exe_InjectLinks.h>
#include <exe_Keywords.h>
#include <exe_MainWindow.h>

// asiTcl includes
#include <asiTcl_Plugin.h>

// asiVisu includes
#include <asiVisu_CalculusLawPrs.h>
#include <asiVisu_ClearancePrs.h>
#include <asiVisu_CurvatureCombsPrs.h>
#include <asiVisu_DeviationPrs.h>
#include <asiVisu_DiscrFacePrs.h>
#include <asiVisu_FaceDomainPrs.h>
#include <asiVisu_GeomBoundaryEdgesPrs.h>
#include <asiVisu_GeomCurvePrs.h>
#include <asiVisu_GeomEdgePrs.h>
#include <asiVisu_GeomFaceContourPrs.h>
#include <asiVisu_GeomFaceNormsPrs.h>
#include <asiVisu_GeomSurfPrs.h>
#include <asiVisu_Grid2dPrs.h>
#include <asiVisu_HatchingPrs.h>
#include <asiVisu_IVAxesPrs.h>
#include <asiVisu_IVCurve2dPrs.h>
#include <asiVisu_IVCurvePrs.h>
#include <asiVisu_IVLabelFieldPrs.h>
#include <asiVisu_IVPointSet2dPrs.h>
#include <asiVisu_IVPointSetPrs.h>
#include <asiVisu_IVSurfacePrs.h>
#include <asiVisu_IVTessItemPrs.h>
#include <asiVisu_IVTextItemPrs.h>
#include <asiVisu_IVTopoItemPrs.h>
#include <asiVisu_IVVectorFieldPrs.h>
#include <asiVisu_OctreePrs.h>
#include <asiVisu_PartPrs.h>
#include <asiVisu_ReCoedgePrs.h>
#include <asiVisu_ReEdgePrs.h>
#include <asiVisu_RePatchPrs.h>
#include <asiVisu_ReVertexPrs.h>
#include <asiVisu_SurfDeviationPrs.h>
#include <asiVisu_TessellationPrs.h>
#include <asiVisu_TessellationNormsPrs.h>
#include <asiVisu_ThicknessPrs.h>
#include <asiVisu_TolerantRangePrs.h>
#include <asiVisu_TriangulationPrs.h>

// asiAlgo includes
#include <asiAlgo_Dictionary.h>
#include <asiAlgo_FileFormat.h>

// asiUI includes
#include <dialogs/asiUI_DialogDump.h>

// Qt includes
#pragma warning(push, 0)
#include <QApplication>
#include <QDesktopWidget>
#pragma warning(pop)

// VTK includes
#pragma warning(push, 0)
#include <vtkCamera.h>
#include <vtkOpenGLRenderWindow.h>
#pragma warning(pop)

// VTK init
#include <vtkAutoInit.h>

// OCCT includes
#include <OSD.hxx>
#include <OSD_Environment.hxx>
#include <OSD_Process.hxx>

// Qt includes
#pragma warning(push, 0)
#include <QDir>
#include <QSplashScreen>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QTimer>
#pragma warning(pop)

#include <QVTKOpenGLNativeWidget.h>

// Activate object factories
VTK_MODULE_INIT(vtkRenderingContextOpenGL2)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkIOExportOpenGL2)
VTK_MODULE_INIT(vtkRenderingGL2PSOpenGL2)

#define EXE_LOAD_MODULE(__cf, name) \
{ \
  asiTcl_Plugin::Status status = asiTcl_Plugin::Load(__cf->Interp, __cf, name); \
  if ( status == asiTcl_Plugin::Status_Failed ) \
    __cf->Progress.SendLogMessage(LogErr(Normal) << "Cannot load %1 commands." << name); \
  else if ( status == asiTcl_Plugin::Status_OK ) \
    __cf->Progress.SendLogMessage(LogInfo(Normal) << "Loaded %1 commands." << name); \
}

int runJsonView(int argc,
                char** argv,
                const std::string& scriptArg)
{
  if (scriptArg.empty())
    return 1;

  QApplication app(argc, argv);
  exe_MainWindow::setApplicationStyle(":qdarkstyle/style.qss");

  std::string fileName = scriptArg;
  QFile file(fileName.c_str());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return 0;

  QString fileBuff = file.readAll().constData();

  asiUI_DialogDump* dlg = new asiUI_DialogDump("Json text");
  dlg->Populate(fileBuff.toStdString());
  dlg->show();

  app.exec();
  return 0;
}

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

//! main().
int main(int argc, char** argv)
{
  QLocale::setDefault(QLocale::c());

  // Check whether batch mode is requested.
  std::string scriptArg;
  const bool
    isRunScript = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runscript, scriptArg);
  const bool
    isRunCommand = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runcommand, scriptArg);
  const bool
    isRunJsonView = asiExe::GetKeyValue(argc, argv, ASITUS_KW_runjsonview, scriptArg);
  const bool
    isGenDoc = asiExe::HasKeyword(argc, argv, ASITUS_KW_gendoc);
  const bool
    isBatch = isRunScript || isRunCommand || isGenDoc;

  if ( isRunJsonView )
  {
    return runJsonView(argc, argv, scriptArg);
  }

  OSD::SetSignal(true);
  OSD::SetFloatingSignal(false);

  std::cout << "Batch mode: " << (isBatch ? "true" : "false") << std::endl;

  // Get command line arguments to process in a batch mode.
  for ( int i = 0; i < argc; ++i )
    std::cout << "Passed arg[" << i << "]: " << argv[i] << std::endl;

  // Register Presentations.
  REGISTER_PRESENTATION(asiVisu_PartPrs)
  REGISTER_PRESENTATION(asiVisu_DeviationPrs)
  REGISTER_PRESENTATION(asiVisu_DiscrFacePrs)
  REGISTER_PRESENTATION(asiVisu_OctreePrs)
  REGISTER_PRESENTATION(asiVisu_GeomBoundaryEdgesPrs)
  REGISTER_PRESENTATION(asiVisu_GeomCurvePrs)
  REGISTER_PRESENTATION(asiVisu_GeomEdgePrs)
  REGISTER_PRESENTATION(asiVisu_FaceDomainPrs)
  REGISTER_PRESENTATION(asiVisu_GeomFaceNormsPrs)
  REGISTER_PRESENTATION(asiVisu_GeomFaceContourPrs)
  REGISTER_PRESENTATION(asiVisu_GeomSurfPrs)
  REGISTER_PRESENTATION(asiVisu_Grid2dPrs)
  REGISTER_PRESENTATION(asiVisu_HatchingPrs)
  REGISTER_PRESENTATION(asiVisu_CalculusLawPrs)
  REGISTER_PRESENTATION(asiVisu_CurvatureCombsPrs)
  REGISTER_PRESENTATION(asiVisu_SurfDeviationPrs)
  REGISTER_PRESENTATION(asiVisu_TessellationPrs)
  REGISTER_PRESENTATION(asiVisu_TessellationNormsPrs)
  REGISTER_PRESENTATION(asiVisu_TolerantRangePrs)
  REGISTER_PRESENTATION(asiVisu_TriangulationPrs)
  REGISTER_PRESENTATION(asiVisu_ReCoedgePrs)
  REGISTER_PRESENTATION(asiVisu_ReEdgePrs)
  REGISTER_PRESENTATION(asiVisu_RePatchPrs)
  REGISTER_PRESENTATION(asiVisu_ReVertexPrs)
  REGISTER_PRESENTATION(asiVisu_ThicknessPrs)
  REGISTER_PRESENTATION(asiVisu_ClearancePrs)

  // Imperative viewer.
  REGISTER_PRESENTATION(asiVisu_IVAxesPrs)
  REGISTER_PRESENTATION(asiVisu_IVPointSet2dPrs)
  REGISTER_PRESENTATION(asiVisu_IVPointSetPrs)
  REGISTER_PRESENTATION(asiVisu_IVCurve2dPrs)
  REGISTER_PRESENTATION(asiVisu_IVCurvePrs)
  REGISTER_PRESENTATION(asiVisu_IVLabelFieldPrs)
  REGISTER_PRESENTATION(asiVisu_IVSurfacePrs)
  REGISTER_PRESENTATION(asiVisu_IVTessItemPrs)
  REGISTER_PRESENTATION(asiVisu_IVTextItemPrs)
  REGISTER_PRESENTATION(asiVisu_IVTopoItemPrs)
  REGISTER_PRESENTATION(asiVisu_IVVectorFieldPrs)

  //---------------------------------------------------------------------------
  // Environment
  //---------------------------------------------------------------------------

  std::string workdir = OSD_Process::ExecutableFolder().ToCString();
  //
  asiAlgo_Utils::Str::ReplaceAll(workdir, "\\", "/");

  // Adjust PATH/LD_LIBRARY_PATH for loading the plugins.
  std::string
    pluginsDir = asiAlgo_Utils::Str::Slashed(workdir) + "asi-plugins";
  //
  qputenv(RuntimePathVar, qgetenv(RuntimePathVar) + ";" + pluginsDir.c_str());
  //
  std::cout << RuntimePathVar
            << " = "
            << QStr2AsciiStr( QString::fromLatin1( qgetenv(RuntimePathVar).data() ) ).ToCString()
            << std::endl;

  // Set extra environment variables for resources.
  std::string
    resDir = asiAlgo_Utils::Str::Slashed(workdir) + "resources";
  //
  if ( QDir( resDir.c_str() ).exists() )
  {
    qputenv( "CSF_PluginDefaults",    resDir.c_str() );
    qputenv( "CSF_ResourcesDefaults", resDir.c_str() );

    TCollection_AsciiString resDirStr = QStr2AsciiStr( QString::fromLatin1( resDir.data() ) );
    //
    std::cout << "CSF_PluginDefaults: " << resDirStr.ToCString() << std::endl;
    std::cout << "CSF_ResourcesDefaults: " << resDirStr.ToCString() << std::endl;

    // Load data dictionary.
    std::string dictFilename    = resDir + "/asiExeDictionary.xml";
    QString     dictFilenameStr = QString::fromLatin1( dictFilename.data() );
    //
    if ( !asiAlgo_Dictionary::Load( QStr2AsciiStr(dictFilenameStr) ) )
    {
      std::cout << "Cannot load data dictionary from "
                << QStr2AsciiStr(dictFilenameStr).ToCString() << std::endl;
    }
  }

  //---------------------------------------------------------------------------
  // Batch vs UI initialization
  //---------------------------------------------------------------------------

  if ( !isBatch )
  {
    // Needed to ensure appropriate OpenGL context is created for VTK rendering.
    QSurfaceFormat::setDefaultFormat( QVTKOpenGLNativeWidget::defaultFormat() );

    // Prepare application.
    QApplication app(argc, argv);
    //
#ifdef _DEBUG
    QApplication::setWindowIcon( QIcon(":icons/asitus/asitus-debug_icon_16x16.png") );
#else
    QApplication::setWindowIcon( QIcon(":icons/asitus/asitus_icon_16x16.png") );
#endif

    // Splash screen.
    QSplashScreen* pSplash = nullptr;
    //
    if ( !isBatch )
    {
      pSplash = new QSplashScreen( QPixmap(":img/asitus/splash.png"), Qt::WindowStaysOnTopHint );
      pSplash->show();
    }

    // Construct main window but do not show it to allow off-screen batch.
    exe_MainWindow* pMainWindow = new exe_MainWindow(isBatch);

    // Give splash screen some seconds, no matter how fast the main window appears.
    if ( pSplash )
    {
      QTimer::singleShot( 3000, pSplash, SLOT( close() ) );
      QTimer::singleShot( 3000, pMainWindow, SLOT( slInit() ) );
    }

    // Let Qt do whatever it wants to do before showing UI. This helps
    // to avoid some sort of blinking on launch.
    QApplication::processEvents(QEventLoop::AllEvents, 10000);

    // Move to a handy position.
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    const int center_x   = ( screenGeometry.width() - pMainWindow->width() ) / 2;
    const int center_y   = ( screenGeometry.height() - pMainWindow->height() ) / 2;
    //
    pMainWindow->move(center_x/8, center_y/4);

    // Show main window.
    pMainWindow->show();

    // Set focus on Tcl console.
    pMainWindow->Widgets.wConsole->setFocus();

    //---------------------------------------------------------------------------
    // Check the autoread log
    //---------------------------------------------------------------------------

    // Absolute filename.
    QString logFilename = QDir::currentPath() + "/" + asiTcl_AutoLogFilename;

    std::cout << "Searching for autolog at " << QStr2StdStr(logFilename) << "...";

    QFile qFile(logFilename);
    //
    if ( qFile.exists() )
    {
      std::cout << " found." << std::endl;
      if ( qFile.open(QIODevice::ReadOnly | QFile::Text) )
      {
        QTextStream in(&qFile);
        pMainWindow->Widgets.wConsole->setText( in.readAll() );
      }
    }
    else
    {
      std::cout << " not found." << std::endl;
    }

    //---------------------------------------------------------------------------
    // Process the second argument to open the passed file
    //---------------------------------------------------------------------------

    if ( argc == 2 )
    {
      QStringList qtArgs = QApplication::arguments();
      //
      TCollection_AsciiString
        arg1Str = QStr2AsciiStr( QDir::fromNativeSeparators( qtArgs.at(1) ) );

      // Check format.
      TCollection_AsciiString ext = asiAlgo_FileFormatTool::GetFileExtension(arg1Str);

      // Prepare Tcl command.
      TCollection_AsciiString cmd;
      //
      if ( ext == ACTBinExt )
      {
        cmd = "load"; cmd += " \""; cmd += arg1Str; cmd += "\"";
      }
      else
      {
        cmd = "load-part"; cmd += " \""; cmd += arg1Str; cmd += "\"";
      }

      // Execute command.
      if ( !cmd.IsEmpty() )
      {
        QApplication::processEvents(QEventLoop::AllEvents, 10000);

        // Get Tcl interpeter.
        const Handle(asiTcl_Interp)&
          interp = pMainWindow->Widgets.wConsole->GetInterp();

        if ( interp->Eval(cmd) != TCL_OK )
          std::cout << "Tcl finished with error." << std::endl;

        QApplication::processEvents(QEventLoop::AllEvents, 10000);

        if ( interp->Eval("fit") != TCL_OK )
          std::cout << "Tcl finished with error." << std::endl;
      }
    }

    // Run event loop.
    return app.exec();
  }

  else /* Batch mode */
  {
    std::cout << "Running Analysis Situs in batch mode..." << std::endl;

    // Prepare common facilities for batch mode.
    Handle(asiUI_BatchFacilities) cf = asiUI_BatchFacilities::Instance();

    // Load default commands.
    EXE_LOAD_MODULE(cf, "cmdMisc")
    EXE_LOAD_MODULE(cf, "cmdEngine")
    EXE_LOAD_MODULE(cf, "cmdRE")
    EXE_LOAD_MODULE(cf, "cmdDDF")
    EXE_LOAD_MODULE(cf, "cmdAsm")
    EXE_LOAD_MODULE(cf, "cmdTest")
    //
#ifdef USE_MOBIUS
    EXE_LOAD_MODULE(cf, "cmdMobius")
#endif

    // Lookup for custom plugins and try to load them.
    QDir pluginDir( QDir::currentPath() + "/asi-plugins" );
    TCollection_AsciiString pluginDirStr = pluginDir.absolutePath().toLatin1().data();
    //
    std::cout << "Looking for plugins at "
              << pluginDirStr.ToCString() << "..." << std::endl;
    //
    QStringList cmdLibs = pluginDir.entryList(QStringList() << "*.dll", QDir::Files);
    //
    foreach ( QString cmdLib, cmdLibs )
    {
      TCollection_AsciiString cmdLibName = cmdLib.section(".", 0, 0).toLatin1().data();
      //
      cf->Progress.SendLogMessage(LogNotice(Normal) << "Detected %1 as a custom plugin's library."
                                                    << cmdLibName);

      EXE_LOAD_MODULE(cf, cmdLibName);
    }

    if ( isGenDoc )
    {
      std::string
        docsDir = asiAlgo_Utils::Str::Slashed( asiAlgo_Utils::Env::AsiDocs() );

      cf->Progress.SendLogMessage(LogNotice(Normal) << "Generating commands list in '%1'..."
                                                    << docsDir);

      std::string docFnIn  (docsDir + "commands_template.html");
      std::string docFnOut (docsDir + "commands.html");

      /* Generate documentation page with all Tcl commands listed */
      exe_GenerateDocs::Perform(cf->Interp, docFnIn, docFnOut);

      /* Replace all variable placeholders with links */
      exe_InjectLinks injLinks(asiAlgo_Utils::Env::AsiDocs(), cf->Progress);
      //
      if ( !injLinks.Perform() )
        return 1;
    }
    else
    {
      /* Execute batch job */

      const int
        ret = cf->Interp->Eval( isRunScript ? asiTcl_SourceCmd( scriptArg.c_str() )
                   /* run single command */ : scriptArg.c_str() );

      // Check result.
      if ( ret != TCL_OK )
        std::cout << "Batch mode finished with error code " << ret << "." << std::endl;
      else
        std::cout << "Batch mode finished successfully (error code " << ret << ")." << std::endl;

      return ret;
    }
  }
}

#else

#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>

#include <QApplication>
#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

#include <cmath>
#include <cstdlib>
#include <random>

namespace {
/**
 * Deform the sphere source using a random amplitude and modes and render it in
 * the window
 *
 * @param sphere the original sphere source
 * @param mapper the mapper for the scene
 * @param window the window to render to
 * @param randEng the random number generator engine
 */
void Randomize(vtkSphereSource* sphere, vtkDataSetMapper* mapper,
               vtkGenericOpenGLRenderWindow* window, std::mt19937& randEng);
} // namespace

int main(int argc, char* argv[])
{
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

  QApplication app(argc, argv);

  // Main window.
  QMainWindow mainWindow;
  mainWindow.resize(1200, 900);

  // Control area.
  QDockWidget controlDock;
  mainWindow.addDockWidget(Qt::LeftDockWidgetArea, &controlDock);

  QLabel controlDockTitle("Control Dock");
  controlDockTitle.setMargin(20);
  controlDock.setTitleBarWidget(&controlDockTitle);

  QPointer<QVBoxLayout> dockLayout = new QVBoxLayout();
  QWidget layoutContainer;
  layoutContainer.setLayout(dockLayout);
  controlDock.setWidget(&layoutContainer);

  QPushButton randomizeButton;
  randomizeButton.setText("Randomize");
  dockLayout->addWidget(&randomizeButton);

  // Render area.
  QPointer<QVTKOpenGLNativeWidget> vtkRenderWidget =
      new QVTKOpenGLNativeWidget();
  mainWindow.setCentralWidget(vtkRenderWidget);

  // VTK part.
  vtkNew<vtkGenericOpenGLRenderWindow> window;
  vtkRenderWidget->setRenderWindow(window.Get());

  vtkNew<vtkSphereSource> sphere;
  sphere->SetRadius(1.0);
  sphere->SetThetaResolution(100);
  sphere->SetPhiResolution(100);

  vtkNew<vtkDataSetMapper> mapper;
  mapper->SetInputConnection(sphere->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetEdgeVisibility(true);
  actor->GetProperty()->SetRepresentationToSurface();

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);

  window->AddRenderer(renderer);

  // Setup initial status.
  std::mt19937 randEng(0);
  ::Randomize(sphere, mapper, window, randEng);

  // connect the buttons
  QObject::connect(&randomizeButton, &QPushButton::released,
                   [&]() { ::Randomize(sphere, mapper, window, randEng); });

  mainWindow.show();

  return app.exec();
}

namespace {
void Randomize(vtkSphereSource* sphere, vtkDataSetMapper* mapper,
               vtkGenericOpenGLRenderWindow* window, std::mt19937& randEng)
{
  // Generate randomness.
  double randAmp = 0.2 + ((randEng() % 1000) / 1000.0) * 0.2;
  double randThetaFreq = 1.0 + (randEng() % 9);
  double randPhiFreq = 1.0 + (randEng() % 9);

  // Extract and prepare data.
  sphere->Update();
  vtkSmartPointer<vtkPolyData> newSphere;
  newSphere.TakeReference(sphere->GetOutput()->NewInstance());
  newSphere->DeepCopy(sphere->GetOutput());
  vtkNew<vtkDoubleArray> height;
  height->SetName("Height");
  height->SetNumberOfComponents(1);
  height->SetNumberOfTuples(newSphere->GetNumberOfPoints());
  newSphere->GetPointData()->AddArray(height);

  // Deform the sphere.
  for (int iP = 0; iP < newSphere->GetNumberOfPoints(); iP++)
  {
    double pt[3] = {0.0};
    newSphere->GetPoint(iP, pt);
    double theta = std::atan2(pt[1], pt[0]);
    double phi =
        std::atan2(pt[2], std::sqrt(std::pow(pt[0], 2) + std::pow(pt[1], 2)));
    double thisAmp =
        randAmp * std::cos(randThetaFreq * theta) * std::sin(randPhiFreq * phi);
    height->SetValue(iP, thisAmp);
    pt[0] += thisAmp * std::cos(theta) * std::cos(phi);
    pt[1] += thisAmp * std::sin(theta) * std::cos(phi);
    pt[2] += thisAmp * std::sin(phi);
    newSphere->GetPoints()->SetPoint(iP, pt);
  }
  newSphere->GetPointData()->SetScalars(height);

  // Reconfigure the pipeline to take the new deformed sphere.
  mapper->SetInputDataObject(newSphere);
  mapper->SetScalarModeToUsePointData();
  mapper->ColorByArrayComponent("Height", 0);
  window->Render();
}
} // namespace

#endif
