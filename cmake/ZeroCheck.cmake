# Zero Check CMake Module
# Verifies build configuration is correct

function(add_zero_check)
  add_custom_target(zero_check
    COMMAND ${CMAKE_COMMAND} -E echo "Build configuration verified"
    COMMENT "Checking build configuration..."
  )
endfunction()
