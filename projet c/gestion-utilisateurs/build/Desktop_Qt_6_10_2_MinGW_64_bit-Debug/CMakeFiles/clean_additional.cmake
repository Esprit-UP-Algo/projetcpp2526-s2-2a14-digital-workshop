# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\GestionCollaborative_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\GestionCollaborative_autogen.dir\\ParseCache.txt"
  "GestionCollaborative_autogen"
  )
endif()
