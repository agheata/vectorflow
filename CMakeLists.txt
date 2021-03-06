cmake_minimum_required(VERSION 3.6.0)

include(cmake/RecordCmdLine.cmake)

enable_language(CXX)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11 CACHE STRING "C++ ISO Standard")
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

option(GPERFTOOLS "Enable profiling using Google performance tools" OFF)
option(BUILD_TESTING  "Build test binaries and create test target"  OFF)
option(BUILD_EXAMPLES "Build binaries for performance benchmarking" OFF)
option(ROOT "Include ROOT" OFF)
set(VecCore_VERSION 0.5.0)

# Set backend for tests and benchmarks
if (BUILD_TESTING OR BUILD_EXAMPLES)
  message("Vc backend selected for testing")
  set(VecCore_BACKEND Vc)
endif()

project(VectorFlow VERSION 0.1.0)

add_library(VectorFlow INTERFACE)
target_include_directories(VectorFlow INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)

# Added due to VecGeom dependency with ROOT
if ( ROOT )
  find_package(ROOT)
  # Added the following IF statement to NOT force ROOT package to be required
  if ( ROOT_FOUND )
    message(STATUS "ROOT dir found: ${ROOT_INCLUDE_DIRS}")
    message(STATUS "ROOT libraries: ${ROOT_LIBRARIES}")
    include_directories(${ROOT_INCLUDE_DIRS})
    target_link_libraries(VectorFlow INTERFACE ${ROOT_LIBRARIES})
  endif ( ROOT_FOUND )
endif ( ROOT )

#Find VecCore with correct backend
find_package(VecCore ${VecCore_VERSION} REQUIRED COMPONENTS ${VecCore_BACKEND})
target_link_libraries(VectorFlow INTERFACE VecCore::VecCore)

# Find VecMath library with correct backend
find_package(VecMath 0.1.0 REQUIRED COMPONENTS Vc)
#Temporary solution is to use VecMath_INCLUDES/LIBRARIES variables since GV does not support modern cmake targets
include_directories(${VecMath_INCLUDE_DIRS})
add_definitions(${VecMath_DEFINITIONS})

# Find VecGeom geometry headers library
find_package(VecGeom REQUIRED)
message(STATUS "VecGeom libraries ${VECGEOM_LIBRARIES}")
#---ensure that VectorFlow is compiled with same architecture flags as VecGeom
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VECGEOM_CXX_FLAGS}")
include_directories(${VECGEOM_INCLUDE_DIRS})
set ( VECGEOM_ROOT OFF )
if ( VECGEOM_DEFINITIONS MATCHES VECGEOM_ROOT )
  set ( VECGEOM_ROOT ON )
endif()

#----------------------------------------------------------------------------
# Find GPERFTOOLS
#----------------------------------------------------------------------------
if (GPERFTOOLS)
  find_package(Gperftools)
  if (GPERFTOOLS_FOUND)
    add_definitions("-DUSE_GPERFTOOLS")
    message(STATUS "Found Gperftools in " ${GPERFTOOLS_LIBRARIES})
    set(PROFILE_LIBRARIES ${GPERFTOOLS_LIBRARIES})
    include_directories( AFTER SYSTEM ${GPERFTOOLS_INCLUDE_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -gdwarf")
  else()
    message(STATUS "Gperftools libraries not found. Install libgoogle-perftools-dev. Disabling Gperftools.")
    set(GPERFTOOLS OFF)
  endif()
endif()

#installation configuration
#install(DIRECTORY include/ DESTINATION include)

include(CMakePackageConfigHelpers)

#exported targets file
install(TARGETS VectorFlow EXPORT VectorFlowTargets
  INCLUDES DESTINATION include
)
export(EXPORT VectorFlowTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/VectorFlowTargets.cmake"
  NAMESPACE VectorFlow::
)

#package config file
set(VectorFlow_INSTALL_INCLUDEDIR include)
configure_package_config_file(cmake/VectorFlowConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/VectorFlowConfig.cmake
  INSTALL_DESTINATION lib/cmake/VectorFlow
  PATH_VARS VectorFlow_INSTALL_INCLUDEDIR
)

#version file
write_basic_package_version_file("VectorFlowConfigVersion.cmake"
	VERSION ${VectorFlow_VERSION}
	COMPATIBILITY SameMajorVersion
	)

#install package
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VectorFlowConfigVersion.cmake
${CMAKE_CURRENT_BINARY_DIR}/VectorFlowConfig.cmake
${CMAKE_CURRENT_BINARY_DIR}/VectorFlowTargets.cmake
	DESTINATION lib/cmake/VectorFlow
	)

#----------------------------------------------------------------------------------------------
# Use absolute RPATH when linking to avoid needing to export LD_LIBRARY_PATH
#
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

add_subdirectory(base)
add_subdirectory(interface)

#----------------------------------------------------------------------------------------------
# Build tests
#
if (BUILD_TESTING)
  add_subdirectory(magneticfield)
  add_subdirectory(event)
  add_subdirectory(examples)
endif()

#----------------------------------------------------------------------------------------------
# Build benchmarks
#
if (BUILD_EXAMPLES)
  add_subdirectory(test)
  # Benchmark examples dependencies
  if (NOT BUILD_TESTING)
    add_subdirectory(magneticfield)
    add_subdirectory(event)
    add_subdirectory(examples)
  endif()
endif()
