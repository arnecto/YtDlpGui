# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  [[CMakeFiles\YtDlpGui_autogen.dir\AutogenUsed.txt]]
  [[CMakeFiles\YtDlpGui_autogen.dir\ParseCache.txt]]
  "YtDlpGui_autogen"
  )
endif()
