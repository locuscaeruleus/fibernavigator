CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


PROJECT( FiberNavigator )

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
############ !!!! Visual Studio users should set the type in CMAKE !!!! ################
# This is ugly but forces Visual Studio to be equal to CMAKE_BUILD_TYPE
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)
MARK_AS_ADVANCED( CMAKE_CONFIGURATION_TYPES )


# Include local CMake modules
LIST( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../tools/CMakeModules ${PROJECT_SOURCE_DIR}/../tools/rpavlik_cmakemodules )

# Version configuration
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version/VersionString.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/version/VersionString.cpp" @ONLY)
SET( VERSION_HDR "${CMAKE_CURRENT_SOURCE_DIR}/version/VersionString.h")
SET( VERSION_SRC "${CMAKE_CURRENT_BINARY_DIR}/version/VersionString.cpp")

# First of all find out which platform we are on and set our own platform variable
SET( FIBERNAVIGATOR_PLATFORM_LINUX "Linux" )
SET( FIBERNAVIGATOR_PLATFORM_WINDOWS "Windows" )
IF( CMAKE_HOST_SYSTEM MATCHES Linux )
  SET( FIBERNAVIGATOR_PLATFORM
       ${FIBERNAVIGATOR_PLATFORM_LINUX}
       CACHE STRING "The platform we are building on." )
ENDIF( CMAKE_HOST_SYSTEM MATCHES Linux )
IF( CMAKE_HOST_SYSTEM MATCHES Windows )
  SET( FIBERNAVIGATOR_PLATFORM
    ${FIBERNAVIGATOR_PLATFORM_WINDOWS}
    CACHE STRING "The platform we are building on." )
ENDIF( CMAKE_HOST_SYSTEM MATCHES Windows )

# Platform dependencies
IF( FIBERNAVIGATOR_PLATFORM MATCHES ${FIBERNAVIGATOR_PLATFORM_WINDOWS} )
  SET( LIBRARY_OUTPUT_PATH
       ${PROJECT_BINARY_DIR}/bin
       CACHE PATH "The libraries will be put into this directory.")
ELSE( FIBERNAVIGATOR_PLATFORM MATCHES ${FIBERNAVIGATOR_PLATFORM_WINDOWS} )
  SET( LIBRARY_OUTPUT_PATH
       ${PROJECT_BINARY_DIR}/lib
       CACHE PATH "The libraries will be put into this directory.")
ENDIF( FIBERNAVIGATOR_PLATFORM MATCHES ${FIBERNAVIGATOR_PLATFORM_WINDOWS} )
MARK_AS_ADVANCED( LIBRARY_OUTPUT_PATH )
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "The executables will be put into this directory.")
MARK_AS_ADVANCED( EXECUTABLE_OUTPUT_PATH )

# Set default build type
IF ( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE Debug Release CACHE STRING "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release" FORCE )
    MARK_AS_ADVANCED( CMAKE_BUILD_TYPE )
ENDIF ( NOT CMAKE_BUILD_TYPE )

# Package dependencies:
FIND_PACKAGE( GLEW REQUIRED )

IF (APPLE)
    FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS base core gl net xml adv )
    FIND_PACKAGE( OPENGL REQUIRED )
    SET( WXWIDGETS_USE_GL TRUE )
ELSE (APPLE)
   IF (WIN32)
     FIND_PACKAGE( wxWidgets REQUIRED COMPONENTS adv base core gl net xml )
     FIND_PACKAGE( OPENGL REQUIRED )
   ELSE (WIN32)
     FIND_PACKAGE( wxWidgets REQUIRED )
   ENDIF(WIN32)
ENDIF(APPLE)

# Includes:
INCLUDE_DIRECTORIES( ${GLEW_INCLUDE_PATH} )
INCLUDE( ${wxWidgets_USE_FILE} )
INCLUDE_DIRECTORIES( . ${wxWidgets_INCLUDE_DIRS} )

IF (UNIX AND NOT "${CMAKE_GENERATOR}" MATCHES Xcode)
ADD_DEFINITIONS( `${wxWidgets_CONFIG_EXECUTABLE} --cppflags` )
ENDIF (UNIX AND NOT "${CMAKE_GENERATOR}" MATCHES Xcode)

# This is needed because Xcode does not understand the "`" syntax to execute the wxconfig command.
# Therefore, when the projet was built, it failed because the command was not executed and only
# considered as a compiler flag, which was not understood.
IF("${CMAKE_GENERATOR}" MATCHES Xcode)
  EXEC_PROGRAM( ${wxWidgets_CONFIG_EXECUTABLE} ARGS --cxxflags OUTPUT_VARIABLE CMAKE_WXWINDOWS_CXX_FLAGS )
  ADD_DEFINITIONS(${CMAKE_WXWINDOWS_CXX_FLAGS})
ENDIF("${CMAKE_GENERATOR}" MATCHES Xcode)

# To shut up a few warnings on Visual Studio 9 MS deprecated C. This removes all 1.276E1265 security warnings. Copied for ITK CMakeList
if (WIN32)
 ADD_DEFINITIONS(
            -D_CRT_FAR_MAPPINGS_NO_DEPRECATE
            -D_CRT_IS_WCTYPE_NO_DEPRECATE
            -D_CRT_MANAGED_FP_NO_DEPRECATE
            -D_CRT_NONSTDC_NO_DEPRECATE
            -D_CRT_SECURE_NO_DEPRECATE
            -D_CRT_SECURE_NO_DEPRECATE_GLOBALS
            -D_CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
            -D_CRT_TIME_FUNCTIONS_NO_DEPRECATE
            -D_CRT_VCCLRIT_NO_DEPRECATE
            -D_SCL_SECURE_NO_DEPRECATE
            )
ENDIF (WIN32)

# Disables all geometry shader support from code when OpenGL version does not support it
# Presented as an advanced option to the user.

OPTION(USE_GEOMETRY_SHADERS "Compile and use geometry shaders" ON)

IF(USE_GEOMETRY_SHADERS)
    ADD_DEFINITIONS( -D_COMPILE_GEO_SHADERS=1 )
ELSE(USE_GEOMETRY_SHADERS)
    ADD_DEFINITIONS( -D_COMPILE_GEO_SHADERS=0 )
ENDIF(USE_GEOMETRY_SHADERS)

MARK_AS_ADVANCED( USE_GEOMETRY_SHADERS )

# Option to enable building a lightweight version of the
# GUI, without features that are not used by lots
# of users.

OPTION(BUILD_LIGHTWEIGHT_GUI "Build lightweight GUI" ON)

IF(BUILD_LIGHTWEIGHT_GUI)
    ADD_DEFINITIONS( -D_USE_LIGHT_GUI=1 )
ELSE(BUILD_LIGHTWEIGHT_GUI)
    ADD_DEFINITIONS( -D_USE_LIGHT_GUI=0 )
ENDIF(BUILD_LIGHTWEIGHT_GUI)

MARK_AS_ADVANCED( BUILD_LIGHTWEIGHT_GUI )

# Option to enable HD (e.g. 4k resolution) GUI

OPTION(BUILD_HD_GUI_ZOOM "Build HD GUI for 4k" ON)

IF(BUILD_HD_GUI_ZOOM)
    ADD_DEFINITIONS( -D_USE_ZOOM_GUI=1 )
ELSE(BUILD_HD_GUI_ZOOM)
    ADD_DEFINITIONS( -D_USE_ZOOM_GUI=0 )
ENDIF(BUILD_HD_GUI_ZOOM)

MARK_AS_ADVANCED( BUILD_HD_GUI_ZOOM )

#hiding this for now
MARK_AS_ADVANCED( wxWidgets_wxrc_EXECUTABLE )
MARK_AS_ADVANCED( wxWidgets_USE_DEBUG )

IF(UNIX)
# Global compiler flags for LINUX:
#-----------------------
# -Wno-long-long    since on Ubuntu 8.10 it won't compile without it
# -ansi             force ISO-C++98 compliance (not GNU++98)
SET( CMAKE_CXX_FLAGS "-Wall -ansi -Wno-long-long" )
SET( CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -O0" )
SET( CMAKE_CXX_FLAGS_RELEASE "-O3" )
SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -DDEBUG -O2" )
ENDIF(UNIX)

# Add this define for every platform. Used by nifti_io to try to read .nii.gz files.
ADD_DEFINITIONS(
    -DHAVE_ZLIB
)

SET( target "fibernavigator")


# Should do the same for header files so that they are included in the Visual Studio solution
file( GLOB GUI_HDR "gui/*.h" )
file( GLOB GFX_HDR "gfx/*.h" )
file( GLOB DS_HDR "dataset/*.h" )
file( GLOB MISC_HDR "misc/*.h" )
file( GLOB ALGORITHMS_HDR "misc/Algorithms/*.h" )
file( GLOB FANTOM_HDR "misc/Fantom/*.h" )
file( GLOB ISO_HDR "misc/IsoSurface/*.h" )
file( GLOB LIC_HDR "misc/lic/*.h" )
file( GLOB NIFTI_HDR "misc/nifti/*.h" )

file( GLOB GUI_SRC "gui/*.cpp" )
file( GLOB GFX_SRC "gfx/*.cpp" )
file( GLOB DS_SRC "dataset/*.cpp" )
file( GLOB MISC_SRC "misc/*.cpp" )
file( GLOB ALGORITHMS_SRC "misc/Algorithms/*.cpp" )
file( GLOB FANTOM_SRC "misc/Fantom/*.cpp" )
file( GLOB ISO_SRC "misc/IsoSurface/*.cpp" )
file( GLOB LIC_SRC "misc/lic/*.cpp" )
file( GLOB NIFTI_SRC "misc/nifti/*.c" )

IF( WIN32 )
 ADD_EXECUTABLE( ${target} WIN32 main.h main.cpp Logger.h Logger.cpp
                 ${ALGORITHMS_SRC} ${DS_SRC} ${GFX_SRC} ${GUI_SRC} ${FANTOM_SRC} ${ISO_SRC} ${LIC_SRC} ${MISC_SRC} ${NIFTI_SRC} ${VERSION_SRC}
                 ${ALGORITHMS_HDR} ${DS_HDR} ${GFX_HDR} ${GUI_HDR} ${FANTOM_HDR} ${ISO_HDR} ${LIC_HDR} ${MISC_HDR} ${NIFTI_HDR} ${VERSION_HDR} )
ELSE( WIN32 )
 ADD_EXECUTABLE( ${target} main.h main.cpp Logger.h Logger.cpp
                 ${ALGORITHMS_SRC} ${DS_SRC} ${GFX_SRC} ${GUI_SRC} ${FANTOM_SRC} ${ISO_SRC} ${LIC_SRC} ${MISC_SRC} ${NIFTI_SRC} ${VERSION_SRC}
                 ${ALGORITHMS_HDR} ${DS_HDR} ${GFX_HDR} ${GUI_HDR} ${FANTOM_HDR} ${ISO_HDR} ${LIC_HDR} ${MISC_HDR} ${NIFTI_HDR} ${VERSION_HDR} )
ENDIF( WIN32 )

IF( WIN32 )
  FIND_FILE( GLEW_DLL_FOUND glew32.dll REQUIRED )
  find_file( wxmsw28ud_adv_vc_custom_dll_found wxmsw28ud_adv_vc_custom.dll REQUIRED )
  find_file( wxmsw28ud_core_vc_custom_dll_found wxmsw28ud_core_vc_custom.dll REQUIRED )
  find_file( wxmsw28ud_gl_vc_custom_dll_found wxmsw28ud_gl_vc_custom.dll REQUIRED )
  find_file( wxbase28ud_xml_vc_custom_dll_found wxbase28ud_xml_vc_custom.dll REQUIRED )
  find_file( wxbase28ud_vc_custom_dll_found wxbase28ud_vc_custom.dll REQUIRED )
  find_file( opengl_dll_found opengl32.dll REQUIRED )
  find_file( glu_dll_found glu32.dll REQUIRED )
ENDIF( WIN32 )

IF (APPLE)
   TARGET_LINK_LIBRARIES( ${target} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} ${wxWidgets_LIBRARIES} z)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icons ${CMAKE_BINARY_DIR}/bin/Contents/Resources/icons)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/GLSL ${CMAKE_BINARY_DIR}/bin/Contents/Resources/GLSL)
ELSE(APPLE)
  IF(WIN32)
    TARGET_LINK_LIBRARIES( ${target} ${GLEW_LIBRARY} ${OPENGL_LIBRARY} ${wxWidgets_LIBRARIES})
    FOREACH(BUILD_TYPE ${CMAKE_BUILD_TYPE})
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/GLSL ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/GLSL )
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icons ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/icons )
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${GLEW_DLL_FOUND} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${wxmsw28ud_adv_vc_custom_dll_found} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${wxmsw28ud_core_vc_custom_dll_found} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${wxmsw28ud_gl_vc_custom_dll_found} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${wxbase28ud_xml_vc_custom_dll_found} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${wxbase28ud_vc_custom_dll_found} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${opengl_dll_found} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${glu_dll_found} ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/)
      # For the VisualStudio solution, copy the shaders at the root of the project so that the shaders are found when 'F5' is ran
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/GLSL ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/GLSL)
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE}/icons)
    ENDFOREACH(BUILD_TYPE ${CMAKE_BUILD_TYPE})
  ELSE(WIN32)
    TARGET_LINK_LIBRARIES( ${target} ${GLEW_LIBRARY} ${wxWidgets_LIBRARIES} wx_gtk2u_gl-3.0 libGL.so libz.so libGLU.so )
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/GLSL ${CMAKE_BINARY_DIR}/bin/GLSL)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icons ${CMAKE_BINARY_DIR}/bin/icons)
  ENDIF(WIN32)
ENDIF(APPLE)
