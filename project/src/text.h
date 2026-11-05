#pragma once
#include <string>

namespace nano_edr{
  bool EqualsIgnoreCase(const std::string& a, const std::string& b);
  bool ContainsIgnoreCase(const std::string& s, const std::string& needle);
  bool EndsWithIgnoreCase(const std::string& s, const std::string& suffix);
}