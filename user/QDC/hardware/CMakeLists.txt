include_directories(include)
include_directories(/usr/include/caen)
# library

find_library(CAEN_FOUND NAMES CAENVME HINTS "/usr/lib")
if(NOT CAEN_FOUND)
  message("CAEN library not found - please installl CAEN libs, which are provided in external")
else()
    message("Found CAEN Libs controllers")
endif()

if(USER_QDC_BUILD AND CAEN_FOUND)
  list(APPEND USER_HARDWARE_SRC src/QDCControl.cc)
else()
    return()
endif()

if(USER_QDC_BUILD)
  add_library(${USER_HARDWARE_LIBRARY} STATIC ${USER_HARDWARE_SRC})
  target_link_libraries(${USER_HARDWARE_LIBRARY}
                        ${EUDAQ_CORE_LIBRARY}
                        ${USER_HARDWARE_DEP_LIB}
                        ${CAENVME})
  list(APPEND INSTALL_TARGETS ${USER_HARDWARE_LIBRARY})
endif()
# end library

install(TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
