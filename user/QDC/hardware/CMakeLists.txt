include_directories(include)
# library
if(USER_QDC_BUILD)
  list(APPEND USER_HARDWARE_SRC src/QDCControl.cc)
endif()

find_library(CAENFOUND NAMES CAENVME HINTS "/usr/lib/libCAENVME")
if(NOT CAENFOUND)
  message(FATAL_ERROR "CAEN library not found")
else()
    message("Found CAEN Libs controllers")
endif()

if(USER_QDC_BUILD)
  add_library(${USER_HARDWARE_LIBRARY} STATIC ${USER_HARDWARE_SRC})
  
  list(APPEND INSTALL_TARGETS ${USER_HARDWARE_LIBRARY})
endif()
# end library

install(TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
