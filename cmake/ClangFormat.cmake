# Clang Format CMake Module
# Adds clang-format target for code formatting

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

if(CLANG_FORMAT_EXECUTABLE)
  message(STATUS "Found clang-format: ${CLANG_FORMAT_EXECUTABLE}")
  
  file(GLOB_RECURSE ALL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/Core_API/include/*.hpp
    ${CMAKE_SOURCE_DIR}/Core_API/src/*.cpp
    ${CMAKE_SOURCE_DIR}/Perception/include/*.hpp
    ${CMAKE_SOURCE_DIR}/Perception/src/*.cpp
    ${CMAKE_SOURCE_DIR}/UI/include/*.hpp
    ${CMAKE_SOURCE_DIR}/UI/src/*.cpp
  )
  
  add_custom_target(format
    COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
    COMMENT "Running clang-format on all source files"
    VERBATIM
  )
else()
  message(WARNING "clang-format not found, format target will not be available")
endif()
