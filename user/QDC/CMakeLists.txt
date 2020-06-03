if(WIN32)
  message(STATUS "QDCProducer is not supported on Windows, you are very
  welcome to implement the windows functionalitites")
  return()
endif()

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/cmake)
get_filename_component(EUDAQ_USERNAME ${CMAKE_CURRENT_LIST_DIR} NAME)

set(EUDAQ_MODULE module_${EUDAQ_USERNAME})
#set(USER_HARDWARE_LIBRARY ${EUDAQ_USERNAME}_static)

option(USER_QDC_BUILD "build user/QDC" ON)

if(NOT USER_QDC_BUILD)
    message(STATUS "user/QDC is NOT to be built (USER_QDC_BUILD=OFF)")
  return()
endif()

message(STATUS "user/QDC is to be built (USER_QDC_BUILD=ON)")

include_directories(${EUDAQ_INCLUDE_DIRS})

include_directories(hardware/include)
add_subdirectory(hardware)
add_subdirectory(module)
