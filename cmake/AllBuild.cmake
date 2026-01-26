# All Build CMake Module
# Creates all_build target to build all project components

function(add_all_build)
  add_custom_target(all_build
    DEPENDS adas_core adas_perception adas_ui adas_simulation
    COMMENT "Building all project components..."
  )
endfunction()
