# ADAS Project Compiler Settings
# MISRA C++:2008 and AUTOSAR Adaptive C++ compliance

function(set_adas_compiler_settings target)
  target_compile_options(${target} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    -Wsign-conversion
    -Wcast-qual
    -Wcast-align
    -Wshadow
    -Wnon-virtual-dtor
  )
endfunction()
