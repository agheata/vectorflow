#----------------------------------------------------------------------------------------------
# Add source files & include directories
#
file(GLOB sources src/*.cxx) # (T*.cxx src/GU*.cxx)
file(GLOB headers inc/Geant/*.h)   # (T*.h inc/GU*.h)

include_directories(
  inc
  ${CMAKE_SOURCE_DIR}/magneticfield/simplifiedCMS/inc
  ${CMAKE_SOURCE_DIR}/base/inc
)  

# - print include directories
# get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
# message(STATUS "Include directories: ${inc_dirs}")

#----------------------------------------------------------------------------------------------
# Add the library
#
add_library(Vmagfield SHARED ${sources} )
target_link_libraries(Vmagfield   ${VECGEOM_LIBRARIES} ${VecCore_LIBRARIES})

#----------------------------------------------------------------------------------------------
# Install the files
#
install(TARGETS Vmagfield LIBRARY DESTINATION lib)
install(FILES ${headers} DESTINATION inc/Geant)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

#----------------------------------------------------------------------------------------------
# Add this directory to Doxygen
#
if (DOXYGEN_FOUND)
  set(DOXYFILE_SOURCE_DIRS "${DOXYFILE_SOURCE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
endif()
