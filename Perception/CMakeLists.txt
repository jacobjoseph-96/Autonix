# Perception CMakeLists.txt

add_library(adas_perception STATIC
    include/perception_filter.hpp
    include/safe_distance_calculator.hpp
    include/gap_analyzer.hpp
    src/perception_filter.cpp
    src/safe_distance_calculator.cpp
    src/gap_analyzer.cpp
)

target_include_directories(adas_perception PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(adas_perception PUBLIC adas_core)

# Perception Tests
add_executable(test_perception_filter tests/test_perception_filter.cpp)
target_link_libraries(test_perception_filter PRIVATE adas_perception adas_core GTest::gtest GTest::gtest_main)
add_test(NAME PerceptionFilterTests COMMAND test_perception_filter)

add_executable(test_distance_calculator tests/test_distance_calculator.cpp)
target_link_libraries(test_distance_calculator PRIVATE adas_perception adas_core GTest::gtest GTest::gtest_main)
add_test(NAME DistanceCalculatorTests COMMAND test_distance_calculator)

add_executable(test_gap_analyzer tests/test_gap_analyzer.cpp)
target_link_libraries(test_gap_analyzer PRIVATE adas_perception adas_core GTest::gtest GTest::gtest_main)
add_test(NAME GapAnalyzerTests COMMAND test_gap_analyzer)
