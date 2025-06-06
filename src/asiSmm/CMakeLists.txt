project(asiSmm)

#------------------------------------------------------------------------------

set (H_FILES
  asiSmm.h
  asiSmm_MakeEdgeFlange.h
  asiSmm_Utils.h
)
set (CPP_FILES
  asiSmm_MakeEdgeFlange.cpp
  asiSmm_Utils.cpp
)

#------------------------------------------------------------------------------
# OpenCascade libraries
#------------------------------------------------------------------------------

set (OCCT_LIB_FILES
  TKernel
  TKMath
  TKBRep
  TKOffset
  TKTopAlgo
  TKG2d
  TKG3d
  TKGeomBase
  TKGeomAlgo
  TKMesh
  TKShHealing
  TKFeat
  TKBool
  TKBO
  TKPrim
  TKBin
  TKBinL
  TKBinXCAF
  TKLCAF
  TKCDF
  TKCAF
  TKXCAF
  TKService
  TKXSBase
  TKSTEP
  TKIGES
  TKXDESTEP
  TKXDEIGES
)

#------------------------------------------------------------------------------
# Mobius
#------------------------------------------------------------------------------

if (USE_MOBIUS)
  set (MOBIUS_LIB_FILES
    mobiusCore
    mobiusPoly
    mobiusBSpl
    mobiusCascade
    mobiusGeom
  )
endif()

#------------------------------------------------------------------------------
# Add sources
#------------------------------------------------------------------------------

foreach (FILE ${H_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Header Files" FILES "${FILE}")
endforeach (FILE)

foreach (FILE ${CPP_FILES})
  set (src_files ${src_files} ${FILE})
  source_group ("Source Files" FILES "${FILE}")
endforeach (FILE)

#------------------------------------------------------------------------------
# Configure includes
#------------------------------------------------------------------------------

# Create include variable
set (asiSmm_include_dir_loc "${CMAKE_CURRENT_SOURCE_DIR}")
#
set (asiSmm_include_dir ${asiSmm_include_dir_loc} PARENT_SCOPE)

include_directories ( SYSTEM
                      ${asiSmm_include_dir_loc}
                      ${asiActiveData_include_dir}
                      ${asiAlgo_include_dir}
                      ${asiData_include_dir}
                      ${3RDPARTY_OCCT_INCLUDE_DIR}
                      ${3RDPARTY_EIGEN_DIR}
                      ${3RDPARTY_rapidjson_DIR}
                      ${3RDPARTY_tbb_INCLUDE_DIR} )

if (USE_MOBIUS)
  include_directories(SYSTEM ${3RDPARTY_mobius_INCLUDE_DIR})
endif()

#------------------------------------------------------------------------------
# Create library
#------------------------------------------------------------------------------

add_library (asiSmm SHARED
  ${H_FILES} ${CPP_FILES}
)

target_link_libraries(asiSmm asiAlgo)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------

if (3RDPARTY_tbb_LIBRARY_DIR_DEBUG)
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR_DEBUG})
else()
  link_directories(${3RDPARTY_tbb_LIBRARY_DIR})
endif()

foreach (LIB_FILE ${OCCT_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  if (3RDPARTY_OCCT_LIBRARY_DIR_DEBUG AND EXISTS "${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME}")
    target_link_libraries (asiSmm debug ${3RDPARTY_OCCT_LIBRARY_DIR_DEBUG}/${LIB_FILENAME})
    target_link_libraries (asiSmm optimized ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  else()
    target_link_libraries (asiSmm ${3RDPARTY_OCCT_LIBRARY_DIR}/${LIB_FILENAME})
  endif()
endforeach()

foreach (LIB_FILE ${TBB_LIB_FILES})
  if (WIN32)
    set (LIB_FILENAME "${LIB_FILE}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set (LIB_FILENAME_DEBUG "${LIB_FILE}_debug${CMAKE_STATIC_LIBRARY_SUFFIX}")
  else()
    set (LIB_FILENAME "lib${LIB_FILE}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set (LIB_FILENAME_DEBUG "lib${LIB_FILE}_debug${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()

  target_link_libraries (asiSmm debug ${3RDPARTY_tbb_LIBRARY_DIR}/${LIB_FILENAME_DEBUG})
  target_link_libraries (asiSmm optimized ${3RDPARTY_tbb_LIBRARY_DIR}/${LIB_FILENAME})
endforeach()

#------------------------------------------------------------------------------
# Installation
#------------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  install (TARGETS asiSmm CONFIGURATIONS Release        RUNTIME DESTINATION bin  LIBRARY DESTINATION bin  COMPONENT Runtime)
  install (TARGETS asiSmm CONFIGURATIONS RelWithDebInfo RUNTIME DESTINATION bini LIBRARY DESTINATION bini COMPONENT Runtime)
  install (TARGETS asiSmm CONFIGURATIONS Debug          RUNTIME DESTINATION bind LIBRARY DESTINATION bind COMPONENT Runtime)
endif()

#------------------------------------------------------------------------------
# Installation of Analysis Situs as a framework
#------------------------------------------------------------------------------

install (TARGETS asiSmm
         CONFIGURATIONS Release
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bin COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}lib COMPONENT Library)

install (TARGETS asiSmm
         CONFIGURATIONS RelWithDebInfo
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bini COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libi COMPONENT Library)

install (TARGETS asiSmm
         CONFIGURATIONS Debug
         RUNTIME DESTINATION ${SDK_INSTALL_SUBDIR}bind COMPONENT Runtime
         ARCHIVE DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library
         LIBRARY DESTINATION ${SDK_INSTALL_SUBDIR}libd COMPONENT Library)

if (MSVC)
  install (FILES ${PROJECT_BINARY_DIR}/../../${PLATFORM}${COMPILER_BITNESS}/${COMPILER}/bind/asiSmm.pdb DESTINATION ${SDK_INSTALL_SUBDIR}bind CONFIGURATIONS Debug)
endif()

install (FILES ${H_FILES} DESTINATION ${SDK_INSTALL_SUBDIR}include)
