# Tcl
if (WIN32)
  ASITUS_THIRDPARTY_PRODUCT("tcl" "" "tcl.h" "tcl86")

  set (TCL_LIBRARY tcl86)
else()
  # use default (CMake) TCL search
  find_package(TCL)

  get_filename_component(TCL_LIBRARY_PATH ${TCL_LIBRARY} DIRECTORY)
  get_filename_component(TCL_LIBRARY_SHORTNAME ${TCL_LIBRARY} NAME_WE)

  set(3RDPARTY_tcl_INCLUDE_DIR ${TCL_INCLUDE_PATH} CACHE PATH "The directory containing TCL includes")
  set(3RDPARTY_tcl_LIBRARY_DIR ${TCL_LIBRARY_PATH} CACHE PATH "The directory containing TCL libraries")
endif()
#
message (STATUS "... TCL_LIBRARY: ${TCL_LIBRARY}")

# TBB
set (INSTALL_TBB OFF CACHE INTERNAL "" FORCE)
if (USE_THREADING OR OpenCASCADE_WITH_TBB)
  set (INSTALL_TBB ON CACHE INTERNAL "" FORCE)

  ASITUS_THIRDPARTY_PRODUCT("tbb" "tbb" "tbb.h" "tbb")
  if (USE_THREADING)
    add_definitions (-DUSE_THREADING)
  endif()
else()
  ASITUS_UNSET_3RDPARTY("tbb")
endif()

# Freetype
if (WIN32)
  ASITUS_THIRDPARTY_PRODUCT("freetype" "" "ft2build.h" "freetype")
else()
  # use default (CMake) TCL search
  find_package(Freetype)

  get_filename_component(FREETYPE_LIBRARY_PATH ${FREETYPE_LIBRARY_RELEASE} DIRECTORY)

  set(3RDPARTY_freetype_INCLUDE_DIR ${FREETYPE_INCLUDE_DIR_freetype2} CACHE PATH "The directory containing Freetype includes")
  set(3RDPARTY_freetype_LIBRARY_DIR ${FREETYPE_LIBRARY_PATH} CACHE PATH "The directory containing Freetype libraries")
endif()

# Freeimage
if (WIN32)
  ASITUS_THIRDPARTY_PRODUCT("freeimage" "" "FreeImage.h" "FreeImage")
else()
  find_path (3RDPARTY_freeimage_LIBRARY_DIR NAMES "libfreeimage.so"
                                            PATHS /usr/lib64
                                            PATH_SUFFIXES "lib lib64"
                                            CMAKE_FIND_ROOT_PATH_BOTH
                                            NO_DEFAULT_PATH)

  message (STATUS "... 3RDPARTY_freeimage_LIBRARY_DIR: ${3RDPARTY_freeimage_LIBRARY_DIR}")
endif()


#--------------------------------------------------------------------------
# Installation
#--------------------------------------------------------------------------

if (NOT BUILD_ALGO_ONLY)
  if (WIN32)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/tcl86.dll CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/tcl86.dll CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/tcl86.dll CONFIGURATIONS Debug          DESTINATION bind)

    install (FILES ${3RDPARTY_tcl_DIR}/bin/zlib1.dll CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/zlib1.dll CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_tcl_DIR}/bin/zlib1.dll CONFIGURATIONS Debug          DESTINATION bind)

    # Freetype
    install (FILES ${3RDPARTY_freetype_DIR}/bin/freetype.dll CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${3RDPARTY_freetype_DIR}/bin/freetype.dll CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${3RDPARTY_freetype_DIR}/bin/freetype.dll CONFIGURATIONS Debug          DESTINATION bind)

    if (USE_THREADING)
      install (FILES ${3RDPARTY_tbb_DLL_DIR}/tbb12.dll           CONFIGURATIONS Release DESTINATION bin)
      install (FILES ${3RDPARTY_tbb_DLL_DIR}/tbbmalloc.dll       CONFIGURATIONS Release DESTINATION bin)
      install (FILES ${3RDPARTY_tbb_DLL_DIR}/tbbmalloc_proxy.dll CONFIGURATIONS Release DESTINATION bin)
    endif()
  else()
    install (FILES ${TCL_LIBRARY} CONFIGURATIONS Release        DESTINATION bin)
    install (FILES ${TCL_LIBRARY} CONFIGURATIONS RelWithDebInfo DESTINATION bini)
    install (FILES ${TCL_LIBRARY} CONFIGURATIONS Debug          DESTINATION bind)
  endif()
endif()
