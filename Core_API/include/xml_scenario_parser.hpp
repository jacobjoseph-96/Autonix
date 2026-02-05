/**
 * @file xml_scenario_parser.hpp
 * @brief XML parser for test scenarios
 */

#ifndef ADAS_TESTS_XML_SCENARIO_PARSER_HPP
#define ADAS_TESTS_XML_SCENARIO_PARSER_HPP

#include "traffic_sign.hpp"
#include "types.hpp"
#include <optional>
#include <string>
#include <vector>


namespace adas {
namespace tests {

/**
 * @brief Test scenario data
 */
struct TestScenario {
  std::string name;
  core::Coordinate car_position;
  std::vector<core::TrafficSign> signs;
  std::optional<double> expected_distance;
  double tolerance{0.001};
};

/**
 * @brief Parser for XML test scenarios
 */
class XmlScenarioParser {
public:
  /**
   * @brief Parse scenarios from XML file
   * @param filename Path to XML file
   * @return Vector of parsed scenarios
   */
  [[nodiscard]] static std::vector<TestScenario>
  parseFile(const std::string &filename);

  /**
   * @brief Parse scenarios from XML string
   * @param xml_content XML content string
   * @return Vector of parsed scenarios
   */
  [[nodiscard]] static std::vector<TestScenario>
  parseString(const std::string &xml_content);

private:
  /**
   * @brief Parse sign type from string
   */
  [[nodiscard]] static std::optional<core::TrafficSignType>
  parseSignType(const std::string &type_str);
};

} // namespace tests
} // namespace adas

#endif // ADAS_TESTS_XML_SCENARIO_PARSER_HPP
