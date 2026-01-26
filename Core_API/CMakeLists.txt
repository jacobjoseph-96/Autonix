# Core_API CMakeLists.txt

add_library(adas_core STATIC
    include/types.hpp
    include/traffic_logic.hpp
    src/road_segment.cpp
    src/traffic_sign.cpp
    src/traffic_light.cpp
    src/ego_vehicle.cpp
    src/traffic_logic.cpp
)

target_include_directories(adas_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Core API Tests
add_executable(test_traffic_light tests/test_traffic_light.cpp)
target_link_libraries(test_traffic_light PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME TrafficLightTests COMMAND test_traffic_light)

add_executable(test_traffic_logic tests/test_traffic_logic.cpp)
target_link_libraries(test_traffic_logic PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME TrafficLogicTests COMMAND test_traffic_logic)
