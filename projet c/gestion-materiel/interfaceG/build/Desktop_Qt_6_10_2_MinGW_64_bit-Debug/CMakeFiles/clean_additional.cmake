# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\interfaceG_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\interfaceG_autogen.dir\\ParseCache.txt"
  "interfaceG_autogen"
  )
endif()
