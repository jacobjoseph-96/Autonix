/**
 * @file test_scenarios.cpp
 * @brief Scenario-based integration tests using XML parser
 */

#include "safe_distance_calculator.hpp"
#include "xml_scenario_parser.hpp"
#include <fstream>
#include <gtest/gtest.h>


using namespace adas::core;
using namespace adas::tests;
using namespace adas::perception;

namespace {

class ScenarioTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Load scenarios from XML file
    try {
      scenarios_ = XmlScenarioParser::parseFile("test_scenarios.xml");
    } catch (const std::exception &e) {
      FAIL() << "Failed to load test scenarios: " << e.what();
    }
  }

  std::vector<TestScenario> scenarios_;
};

TEST_F(ScenarioTest, AllScenariosLoaded) {
  EXPECT_GT(scenarios_.size(), 0U) << "No scenarios loaded from XML";
}

TEST_F(ScenarioTest, DistanceCalculationsMatchExpected) {
  for (const auto &scenario : scenarios_) {
    if (!scenario.expected_distance.has_value()) {
      continue; // Skip scenarios without expected distance
    }

    if (scenario.signs.empty()) {
      ADD_FAILURE() << "Scenario '" << scenario.name
                    << "' has expected distance but no signs";
      continue;
    }

    // Calculate distance to first sign
    const auto &first_sign = scenario.signs[0];
    const double calculated_distance = SafeDistanceCalculator::calculate(
        scenario.car_position, first_sign.getPosition());

    EXPECT_NEAR(calculated_distance, scenario.expected_distance.value(),
                scenario.tolerance)
        << "Scenario: " << scenario.name << "\n  Car: ("
        << scenario.car_position.x << ", " << scenario.car_position.y << ")"
        << "\n  Sign: (" << first_sign.getPosition().x << ", "
        << first_sign.getPosition().y << ")"
        << "\n  Expected: " << scenario.expected_distance.value()
        << "\n  Calculated: " << calculated_distance;
  }
}

// =============================================================================
// Individual Scenario Tests (for detailed failure reporting)
// =============================================================================

class ParameterizedScenarioTest : public ::testing::TestWithParam<std::string> {
protected:
  void SetUp() override {
    try {
      scenarios_ = XmlScenarioParser::parseFile("test_scenarios.xml");
    } catch (const std::exception &) {
      // Handle in test
    }
  }

  std::optional<TestScenario> findScenario(const std::string &name) {
    for (const auto &s : scenarios_) {
      if (s.name == name) {
        return s;
      }
    }
    return std::nullopt;
  }

  std::vector<TestScenario> scenarios_;
};

TEST_P(ParameterizedScenarioTest, ScenarioDistance) {
  const std::string scenario_name = GetParam();
  const auto scenario = findScenario(scenario_name);

  if (!scenario.has_value()) {
    GTEST_SKIP() << "Scenario '" << scenario_name << "' not found";
  }

  if (!scenario->expected_distance.has_value()) {
    GTEST_SKIP() << "Scenario has no expected distance";
  }

  ASSERT_FALSE(scenario->signs.empty()) << "Scenario has no signs";

  const auto &first_sign = scenario->signs[0];
  const double calculated = SafeDistanceCalculator::calculate(
      scenario->car_position, first_sign.getPosition());

  EXPECT_NEAR(calculated, scenario->expected_distance.value(),
              scenario->tolerance);
}

INSTANTIATE_TEST_SUITE_P(XmlScenarios, ParameterizedScenarioTest,
                         ::testing::Values("BasicDistance",
                                           "PythagoreanTriangle",
                                           "HorizontalDistance",
                                           "NegativeCoordinates",
                                           "ZeroDistance", "MultipleSigns",
                                           "LargeDistance", "HighwayApproach"));

// =============================================================================
// Inline XML String Tests
// =============================================================================

TEST(InlineScenarioTest, ParseAndCalculate) {
  const std::string xml = R"(
        <Scenario>
            <Car x="0" y="0"/>
            <Sign type="Stop" x="0" y="50"/>
            <ExpectedDistance>50.0</ExpectedDistance>
        </Scenario>
    )";

  const auto scenarios = XmlScenarioParser::parseString(xml);

  ASSERT_EQ(scenarios.size(), 1U);
  ASSERT_TRUE(scenarios[0].expected_distance.has_value());
  ASSERT_FALSE(scenarios[0].signs.empty());

  const double calculated = SafeDistanceCalculator::calculate(
      scenarios[0].car_position, scenarios[0].signs[0].getPosition());

  EXPECT_NEAR(calculated, 50.0, 0.001);
}

TEST(InlineScenarioTest, RequirementExample) {
  // Test case from requirements:
  // <Scenario><Car x="0" y="0"/><Sign type="Stop" x="0" y="50"/>
  // <ExpectedDistance>50.0</ExpectedDistance></Scenario>

  const Coordinate car_pos{0.0, 0.0};
  const Position sign_pos{0.0, 50.0};

  const double distance = SafeDistanceCalculator::calculate(car_pos, sign_pos);

  EXPECT_NEAR(distance, 50.0, 0.001)
      << "Distance calculation must match expected within 0.001 tolerance";
}

} // namespace
