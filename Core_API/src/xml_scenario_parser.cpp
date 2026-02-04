/**
 * @file xml_scenario_parser.cpp
 * @brief XML scenario parser implementation using pugixml
 */

#include "xml_scenario_parser.hpp"
#include <algorithm>
#include <cctype>
#include <pugixml.hpp>
#include <stdexcept>

namespace adas
{
namespace tests
{

std::vector<TestScenario>
XmlScenarioParser::parseFile(const std::string& filename)
{
  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_file(filename.c_str());
  if (!result)
  {
    throw std::runtime_error("Failed to load XML file: " + filename + " - " +
                             result.description());
  }

  std::vector<TestScenario> scenarios;

  // Find root element
  auto root = doc.child("Scenarios");
  if (!root)
  {
    // Try single scenario format
    auto scenario_elem = doc.child("Scenario");
    if (scenario_elem)
    {
      TestScenario scenario;
      scenario.name = "Scenario1";

      // Parse Car element
      auto car_elem = scenario_elem.child("Car");
      if (car_elem)
      {
        scenario.car_position.x = car_elem.attribute("x").as_double(0.0);
        scenario.car_position.y = car_elem.attribute("y").as_double(0.0);
        scenario.car_position.theta =
            car_elem.attribute("theta").as_double(0.0);
      }

      // Parse Sign elements
      std::uint32_t sign_id = 1U;
      for (auto sign_elem = scenario_elem.child("Sign"); sign_elem;
           sign_elem = sign_elem.next_sibling("Sign"))
      {
        const char *type_str = sign_elem.attribute("type").as_string("");
        const auto type = parseSignType(type_str);

        if (type.has_value())
        {
          const double x = sign_elem.attribute("x").as_double(0.0);
          const double y = sign_elem.attribute("y").as_double(0.0);
          const core::LaneId lane = static_cast<core::LaneId>(
              sign_elem.attribute("lane").as_uint(1U));

          scenario.signs.push_back(core::TrafficSign::create(
              sign_id++, type.value(), core::Position{x, y}, lane));
        }
      }

      // Parse ExpectedDistance
      auto distance_elem = scenario_elem.child("ExpectedDistance");
      if (distance_elem)
      {
        scenario.expected_distance = distance_elem.text().as_double(0.0);
      }

      // Parse Tolerance
      auto tolerance_elem = scenario_elem.child("Tolerance");
      if (tolerance_elem)
      {
        scenario.tolerance = tolerance_elem.text().as_double(0.001);
      }

      scenarios.push_back(scenario);
      return scenarios;
    }
    return scenarios;
  }

  // Parse multiple scenarios
  std::uint32_t scenario_num = 1U;
  for (auto scenario_elem = root.child("Scenario"); scenario_elem;
       scenario_elem = scenario_elem.next_sibling("Scenario"))
  {
    TestScenario scenario;

    const char *name_attr = scenario_elem.attribute("name").as_string(nullptr);
    scenario.name =
        name_attr ? name_attr : ("Scenario" + std::to_string(scenario_num++));

    // Parse Car
    auto car_elem = scenario_elem.child("Car");
    if (car_elem)
    {
      scenario.car_position.x = car_elem.attribute("x").as_double(0.0);
      scenario.car_position.y = car_elem.attribute("y").as_double(0.0);
      scenario.car_position.theta = car_elem.attribute("theta").as_double(0.0);
    }

    // Parse Signs
    std::uint32_t sign_id = 1U;
    for (auto sign_elem = scenario_elem.child("Sign"); sign_elem;
         sign_elem = sign_elem.next_sibling("Sign"))
    {
      const char *type_str = sign_elem.attribute("type").as_string("");
      const auto type = parseSignType(type_str);

      if (type.has_value())
      {
        const double x = sign_elem.attribute("x").as_double(0.0);
        const double y = sign_elem.attribute("y").as_double(0.0);
        const core::LaneId lane =
            static_cast<core::LaneId>(sign_elem.attribute("lane").as_uint(1U));

        scenario.signs.push_back(core::TrafficSign::create(
            sign_id++, type.value(), core::Position{x, y}, lane));
      }
    }

    // Parse ExpectedDistance
    auto distance_elem = scenario_elem.child("ExpectedDistance");
    if (distance_elem)
    {
      scenario.expected_distance = distance_elem.text().as_double(0.0);
    }

    // Parse Tolerance
    auto tolerance_elem = scenario_elem.child("Tolerance");
    if (tolerance_elem)
    {
      scenario.tolerance = tolerance_elem.text().as_double(0.001);
    }

    scenarios.push_back(scenario);
  }

  return scenarios;
}

std::vector<TestScenario>
XmlScenarioParser::parseString(const std::string& xml_content)
{
  pugi::xml_document doc;

  pugi::xml_parse_result result = doc.load_string(xml_content.c_str());
  if (!result)
  {
    throw std::runtime_error("Failed to parse XML string: " +
                             std::string(result.description()));
  }

  std::vector<TestScenario> scenarios;

  // Find root element
  auto root = doc.child("Scenarios");
  if (!root)
  {
    // Try single scenario format
    auto scenario_elem = doc.child("Scenario");
    if (scenario_elem)
    {
      TestScenario scenario;
      scenario.name = "Scenario1";

      // Parse Car element
      auto car_elem = scenario_elem.child("Car");
      if (car_elem)
      {
        scenario.car_position.x = car_elem.attribute("x").as_double(0.0);
        scenario.car_position.y = car_elem.attribute("y").as_double(0.0);
        scenario.car_position.theta =
            car_elem.attribute("theta").as_double(0.0);
      }

      // Parse Sign elements
      std::uint32_t sign_id = 1U;
      for (auto sign_elem = scenario_elem.child("Sign"); sign_elem;
           sign_elem = sign_elem.next_sibling("Sign"))
      {
        const char *type_str = sign_elem.attribute("type").as_string("");
        const auto type = parseSignType(type_str);

        if (type.has_value())
        {
          const double x = sign_elem.attribute("x").as_double(0.0);
          const double y = sign_elem.attribute("y").as_double(0.0);
          const core::LaneId lane = static_cast<core::LaneId>(
              sign_elem.attribute("lane").as_uint(1U));

          scenario.signs.push_back(core::TrafficSign::create(
              sign_id++, type.value(), core::Position{x, y}, lane));
        }
      }

      // Parse ExpectedDistance
      auto distance_elem = scenario_elem.child("ExpectedDistance");
      if (distance_elem)
      {
        scenario.expected_distance = distance_elem.text().as_double(0.0);
      }

      // Parse Tolerance
      auto tolerance_elem = scenario_elem.child("Tolerance");
      if (tolerance_elem)
      {
        scenario.tolerance = tolerance_elem.text().as_double(0.001);
      }

      scenarios.push_back(scenario);
      return scenarios;
    }
    return scenarios;
  }

  // Parse multiple scenarios
  std::uint32_t scenario_num = 1U;
  for (auto scenario_elem = root.child("Scenario"); scenario_elem;
       scenario_elem = scenario_elem.next_sibling("Scenario"))
  {
    TestScenario scenario;

    const char *name_attr = scenario_elem.attribute("name").as_string(nullptr);
    scenario.name =
        name_attr ? name_attr : ("Scenario" + std::to_string(scenario_num++));

    // Parse Car
    auto car_elem = scenario_elem.child("Car");
    if (car_elem)
    {
      scenario.car_position.x = car_elem.attribute("x").as_double(0.0);
      scenario.car_position.y = car_elem.attribute("y").as_double(0.0);
      scenario.car_position.theta = car_elem.attribute("theta").as_double(0.0);
    }

    // Parse Signs
    std::uint32_t sign_id = 1U;
    for (auto sign_elem = scenario_elem.child("Sign"); sign_elem;
         sign_elem = sign_elem.next_sibling("Sign"))
    {
      const char *type_str = sign_elem.attribute("type").as_string("");
      const auto type = parseSignType(type_str);

      if (type.has_value())
      {
        const double x = sign_elem.attribute("x").as_double(0.0);
        const double y = sign_elem.attribute("y").as_double(0.0);
        const core::LaneId lane =
            static_cast<core::LaneId>(sign_elem.attribute("lane").as_uint(1U));

        scenario.signs.push_back(core::TrafficSign::create(
            sign_id++, type.value(), core::Position{x, y}, lane));
      }
    }

    // Parse ExpectedDistance
    auto distance_elem = scenario_elem.child("ExpectedDistance");
    if (distance_elem)
    {
      scenario.expected_distance = distance_elem.text().as_double(0.0);
    }

    // Parse Tolerance
    auto tolerance_elem = scenario_elem.child("Tolerance");
    if (tolerance_elem)
    {
      scenario.tolerance = tolerance_elem.text().as_double(0.001);
    }

    scenarios.push_back(scenario);
  }

  return scenarios;
}

std::optional<core::TrafficSignType>
XmlScenarioParser::parseSignType(const std::string& type_str)
{
  std::string lower = type_str;
  std::transform(lower.begin(),
                 lower.end(),
                 lower.begin(),
                 [](unsigned char c)
                 { return static_cast<char>(std::tolower(c)); });

  if (lower == "stop")
  {
    return core::TrafficSignType::Stop;
  }
  if (lower == "yield")
  {
    return core::TrafficSignType::Yield;
  }
  if (lower == "speedlimit" || lower == "speed_limit")
  {
    return core::TrafficSignType::SpeedLimit;
  }

  return std::nullopt;
}

} // namespace tests
} // namespace adas
