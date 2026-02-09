# UI CMakeLists.txt

add_library(adas_ui STATIC
    include/main_window.hpp
    include/perspective_view.hpp
    include/detection_overlay.hpp
    include/diagnostic_logs.hpp
    include/speed_limit_widget.hpp
    src/main_window.cpp
    src/perspective_view.cpp
    src/detection_overlay.cpp
    src/diagnostic_logs.cpp
    src/speed_limit_widget.cpp
)

target_include_directories(adas_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(adas_ui PUBLIC adas_perception adas_core Qt6::Widgets)

# Main Application
add_executable(adas_simulation src/main.cpp)
target_link_libraries(adas_simulation PRIVATE adas_ui adas_perception adas_core Qt6::Widgets)
