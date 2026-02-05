/**
 * @file test_parser_errors.cpp
 * @brief Unit tests for XmlScenarioParser error handling and edge cases
 */

#include "xml_scenario_parser.hpp"
#include <gtest/gtest.h>

using namespace adas::tests;
using namespace adas::core;

TEST(XmlParserErrorTest, FileNotFoundThrows)
{
  EXPECT_THROW(XmlScenarioParser::parseFile("non_existent_file.xml"),
               std::runtime_error);
}

TEST(XmlParserErrorTest, InvalidXmlStringThrows)
{
  const std::string bad_xml = "<Scenario><UnclosedTag>";
  EXPECT_THROW(XmlScenarioParser::parseString(bad_xml), std::runtime_error);
}

TEST(XmlParserErrorTest, UnknownSignTypeIgnored)
{
  const std::string xml = R"(
    <Scenario>
      <Car x="0" y="0" />
      <Sign type="UnknownType" x="100" y="0" />
      <Sign type="Stop" x="50" y="0" />
    </Scenario>
  )";

  auto scenarios = XmlScenarioParser::parseString(xml);
  ASSERT_EQ(scenarios.size(), 1U);

  // Should only have the Stop sign, UnknownType should be skipped
  ASSERT_EQ(scenarios[0].signs.size(), 1U);
  EXPECT_EQ(scenarios[0].signs[0].getType(), TrafficSignType::Stop);
}

TEST(XmlParserErrorTest, MissingCarAttributesUseDefaults)
{
  const std::string xml = R"(
    <Scenario>
      <Car /> 
    </Scenario>
  )";

  auto scenarios = XmlScenarioParser::parseString(xml);
  ASSERT_EQ(scenarios.size(), 1U);
  EXPECT_DOUBLE_EQ(scenarios[0].car_position.x, 0.0);
  EXPECT_DOUBLE_EQ(scenarios[0].car_position.y, 0.0);
  EXPECT_DOUBLE_EQ(scenarios[0].car_position.theta, 0.0);
}

TEST(XmlParserErrorTest, MissingExpectedDistanceDoesNotCrash)
{
  const std::string xml = R"(
    <Scenario>
      <Car x="0" y="0" />
      <!-- No ExpectedDistance tag -->
    </Scenario>
  )";

  auto scenarios = XmlScenarioParser::parseString(xml);
  ASSERT_EQ(scenarios.size(), 1U);
  EXPECT_FALSE(scenarios[0].expected_distance.has_value());
}

TEST(XmlParserErrorTest, CaseInsensitiveSignType)
{
  const std::string xml = R"(
    <Scenario>
      <Car x="0" y="0" />
      <Sign type="sToP" x="50" y="0" />
      <Sign type="SPEED_LIMIT" x="100" y="0" />
    </Scenario>
  )";

  auto scenarios = XmlScenarioParser::parseString(xml);
  ASSERT_EQ(scenarios.size(), 1U);
  ASSERT_EQ(scenarios[0].signs.size(), 2U);
  EXPECT_EQ(scenarios[0].signs[0].getType(), TrafficSignType::Stop);
  EXPECT_EQ(scenarios[0].signs[1].getType(), TrafficSignType::SpeedLimit);
}

TEST(XmlParserErrorTest, MultipleScenariosParsing)
{
  const std::string xml = R"(
    <Scenarios>
      <Scenario name="S1"><Car x="0" y="0"/></Scenario>
      <Scenario name="S2"><Car x="10" y="10"/></Scenario>
    </Scenarios>
  )";

  auto scenarios = XmlScenarioParser::parseString(xml);
  EXPECT_EQ(scenarios.size(), 2U);
  EXPECT_EQ(scenarios[0].name, "S1");
  EXPECT_EQ(scenarios[1].name, "S2");
}

TEST(XmlParserErrorTest, UnnamedScenarioGetsDefaultName)
{
  const std::string xml = R"(
    <Scenarios>
      <Scenario><Car x="0" y="0"/></Scenario>
    </Scenarios>
  )";
  auto scenarios = XmlScenarioParser::parseString(xml);
  ASSERT_EQ(scenarios.size(), 1U);
  EXPECT_EQ(scenarios[0].name, "Scenario1");
}
