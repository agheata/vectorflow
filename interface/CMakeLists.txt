#----------------------------------------------------------------------------------------------
# Add source files & include directories
#
include_directories("${PROJECT_SOURCE_DIR}/interface/inc")

file(GLOB headers inc/vectorFlow/*)

install(FILES ${headers} DESTINATION inc/vectorFlow)

#----------------------------------------------------------------------------------------------
# Add this directory to Doxygen
#
if(DOXYGEN_FOUND)
  set(DOXYFILE_SOURCE_DIRS "${DOXYFILE_SOURCE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
endif()
