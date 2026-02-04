# Core_API CMakeLists.txt

add_library(adas_core STATIC
    include/types.hpp
    include/traffic_logic.hpp
    include/lane_change_controller.hpp
    include/adaptive_cruise_controller.hpp
    include/npc_vehicle.hpp
    include/pedestrian.hpp
    src/road_segment.cpp
    src/traffic_sign.cpp
    src/traffic_light.cpp
    src/ego_vehicle.cpp
    src/traffic_logic.cpp
    src/lane_change_controller.cpp
    src/adaptive_cruise_controller.cpp
    src/npc_vehicle.cpp
    src/pedestrian.cpp
    src/xml_scenario_parser.cpp
)

target_include_directories(adas_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(adas_core PUBLIC pugixml::pugixml)

# Core API Tests
add_executable(test_traffic_light tests/test_traffic_light.cpp)
target_link_libraries(test_traffic_light PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME TrafficLightTests COMMAND test_traffic_light)

add_executable(test_traffic_logic tests/test_traffic_logic.cpp)
target_link_libraries(test_traffic_logic PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME TrafficLogicTests COMMAND test_traffic_logic)

add_executable(test_pedestrian tests/test_pedestrian.cpp)
target_link_libraries(test_pedestrian PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME PedestrianTests COMMAND test_pedestrian)

add_executable(test_acc tests/test_acc.cpp)
target_link_libraries(test_acc PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME ACCTests COMMAND test_acc)

add_executable(test_structs tests/test_structs.cpp)
target_link_libraries(test_structs PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME StructTests COMMAND test_structs)

add_executable(test_lane_change tests/test_lane_change.cpp)
target_link_libraries(test_lane_change PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME LaneChangeTests COMMAND test_lane_change)

add_executable(test_road_segment tests/test_road_segment.cpp)
target_link_libraries(test_road_segment PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME RoadSegmentTests COMMAND test_road_segment)

add_executable(test_ego_physics tests/test_ego_physics.cpp)
target_link_libraries(test_ego_physics PRIVATE adas_core GTest::gtest GTest::gtest_main)
add_test(NAME EgoPhysicsTests COMMAND test_ego_physics)

