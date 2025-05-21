if (WIN32)
  ASITUS_THIRDPARTY_PRODUCT("vtk" "" "vtkAlgorithm.h" "vtkCommonCore-${3RDPARTY_VTK_VERSION}")
else()
  find_package(VTK)

  message (STATUS "... Package finder returns VTK_DIR=${VTK_DIR}")

  set(3RDPARTY_vtk_INCLUDE_DIR ${vtkCommonCore_INCLUDE_DIRS} CACHE PATH "The directory containing VTK includes")
  set(3RDPARTY_vtk_LIBRARY_DIR ${vtkCommonCore_RUNTIME_LIBRARY_DIRS} CACHE PATH "The directory containing VTK libraries")
endif()

add_definitions (-DUSE_VTK)

if (3RDPARTY_vtk_INCLUDE_DIR STREQUAL "")
  message (STATUS "... VTK Include dir is not conventional")
  list (REMOVE_ITEM 3RDPARTY_NOT_INCLUDED 3RDPARTY_vtk_INCLUDE_DIR)
  set (3RDPARTY_vtk_INCLUDE_DIR ${3RDPARTY_vtk_DIR}/include/vtk-${3RDPARTY_VTK_VERSION} CACHE FILEPATH "Non-conventional inc dir" FORCE)
endif()

message (STATUS "... VTK Include dirs: ${3RDPARTY_vtk_INCLUDE_DIR}")
message (STATUS "... VTK Library dirs: ${3RDPARTY_vtk_LIBRARY_DIR}")
message (STATUS "... VTK Binary  dirs: ${3RDPARTY_vtk_DLL_DIR}")

string (REPLACE lib libd 3RDPARTY_vtk_LIBRARY_DIR_DEBUG ${3RDPARTY_vtk_LIBRARY_DIR})
if (3RDPARTY_vtk_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}")
  if (WIN32)
    if (NOT EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/vtkCommonCore-${3RDPARTY_VTK_VERSION}.lib")
      set (3RDPARTY_vtk_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  else()
    if (NOT EXISTS "${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}/libvtkCommonCore-${3RDPARTY_VTK_VERSION}.so")
      set (3RDPARTY_vtk_LIBRARY_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

if (WIN32)
  string (REPLACE bin bind 3RDPARTY_vtk_DLL_DIR_DEBUG ${3RDPARTY_vtk_DLL_DIR})
  if (3RDPARTY_vtk_DLL_DIR_DEBUG AND EXISTS "${3RDPARTY_vtk_DLL_DIR_DEBUG}")
    if (NOT EXISTS "${3RDPARTY_vtk_DLL_DIR_DEBUG}/vtkCommonCore-${3RDPARTY_VTK_VERSION}.dll")
      set (3RDPARTY_vtk_DLL_DIR_DEBUG "" CACHE INTERNAL "" FORCE)
    endif()
  endif()
endif()

message (STATUS "... VTK Debug Library dirs: ${3RDPARTY_vtk_LIBRARY_DIR_DEBUG}")
message (STATUS "... VTK Debug Binary  dirs: ${3RDPARTY_vtk_DLL_DIR_DEBUG}")

#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

set (LIBS
  vtkChartsCore-${3RDPARTY_VTK_VERSION}
  vtkCommonComputationalGeometry-${3RDPARTY_VTK_VERSION}
  vtkCommonColor-${3RDPARTY_VTK_VERSION}
  vtkCommonCore-${3RDPARTY_VTK_VERSION}
  vtkCommonDataModel-${3RDPARTY_VTK_VERSION}
  vtkCommonExecutionModel-${3RDPARTY_VTK_VERSION}
  vtkCommonMath-${3RDPARTY_VTK_VERSION}
  vtkCommonMisc-${3RDPARTY_VTK_VERSION}
  vtkCommonSystem-${3RDPARTY_VTK_VERSION}
  vtkCommonTransforms-${3RDPARTY_VTK_VERSION}
  vtkFiltersCore-${3RDPARTY_VTK_VERSION}
  vtkFiltersExtraction-${3RDPARTY_VTK_VERSION}
  vtkFiltersGeneral-${3RDPARTY_VTK_VERSION}
  vtkFiltersGeometry-${3RDPARTY_VTK_VERSION}
  vtkFiltersHybrid-${3RDPARTY_VTK_VERSION}
  vtkFiltersImaging-${3RDPARTY_VTK_VERSION}
  vtkFiltersModeling-${3RDPARTY_VTK_VERSION}
  vtkFiltersParallel-${3RDPARTY_VTK_VERSION}
  vtkFiltersSources-${3RDPARTY_VTK_VERSION}
  vtkFiltersStatistics-${3RDPARTY_VTK_VERSION}
  vtkfreetype-${3RDPARTY_VTK_VERSION}
  vtkGUISupportQt-${3RDPARTY_VTK_VERSION}
  vtkglew-${3RDPARTY_VTK_VERSION}
  vtkImagingColor-${3RDPARTY_VTK_VERSION}
  vtkImagingCore-${3RDPARTY_VTK_VERSION}
  vtkImagingFourier-${3RDPARTY_VTK_VERSION}
  vtkImagingGeneral-${3RDPARTY_VTK_VERSION}
  vtkImagingHybrid-${3RDPARTY_VTK_VERSION}
  vtkImagingSources-${3RDPARTY_VTK_VERSION}
  vtkInteractionWidgets-${3RDPARTY_VTK_VERSION}
  vtkInteractionStyle-${3RDPARTY_VTK_VERSION}
  vtkInfovisCore-${3RDPARTY_VTK_VERSION}
  vtkInfovisLayout-${3RDPARTY_VTK_VERSION}
  vtkIOCore-${3RDPARTY_VTK_VERSION}
  vtkIOImage-${3RDPARTY_VTK_VERSION}
  vtkIOLegacy-${3RDPARTY_VTK_VERSION}
  vtkIOExport-${3RDPARTY_VTK_VERSION}
  vtkIOXML-${3RDPARTY_VTK_VERSION}
  vtkIOXMLParser-${3RDPARTY_VTK_VERSION}
  vtkexpat-${3RDPARTY_VTK_VERSION}
  vtkParallelCore-${3RDPARTY_VTK_VERSION}
  vtkRenderingAnnotation-${3RDPARTY_VTK_VERSION}
  vtkRenderingContext2D-${3RDPARTY_VTK_VERSION}
  vtkRenderingContextOpenGL2-${3RDPARTY_VTK_VERSION}
  vtkRenderingCore-${3RDPARTY_VTK_VERSION}
  vtkRenderingGL2PSOpenGL2-${3RDPARTY_VTK_VERSION}
  vtkRenderingFreeType-${3RDPARTY_VTK_VERSION}
  vtkRenderingLabel-${3RDPARTY_VTK_VERSION}
  vtkRenderingOpenGL2-${3RDPARTY_VTK_VERSION}
  vtkRenderingVolume-${3RDPARTY_VTK_VERSION}
  vtksys-${3RDPARTY_VTK_VERSION}
  vtkViewsContext2D-${3RDPARTY_VTK_VERSION}
  vtkViewsCore-${3RDPARTY_VTK_VERSION}
  vtkViewsInfovis-${3RDPARTY_VTK_VERSION}
  vtkzlib-${3RDPARTY_VTK_VERSION}
  vtklz4-${3RDPARTY_VTK_VERSION}
  vtkgl2ps-${3RDPARTY_VTK_VERSION}
  vtkpng-${3RDPARTY_VTK_VERSION}
  vtklibharu-${3RDPARTY_VTK_VERSION}
  vtkDICOMParser-${3RDPARTY_VTK_VERSION}
  vtkmetaio-${3RDPARTY_VTK_VERSION}
  vtktiff-${3RDPARTY_VTK_VERSION}
  vtkjpeg-${3RDPARTY_VTK_VERSION}
  vtklzma-${3RDPARTY_VTK_VERSION}
  vtkdoubleconversion-${3RDPARTY_VTK_VERSION}
)

ASITUS_INSTALL_3RDPARTY (LIBS "vtk" "" "1")
