#include <string>
#include "text.h"

namespace nano_edr {
  std::string ToLower(const std::string& s) {
    std::string res = "";
    for (auto i : s) {
      res += std::tolower(i);
    }
    return res;
  }
  bool EqualsIgnoreCase(const std::string& a, const std::string& b) {
    return ToLower(a) == ToLower(b);
  }
  bool ContainsIgnoreCase(const std::string& s, const std::string& needle){
    return ToLower(s).find(ToLower(needle)) != std::string::npos;
  }
  bool EndsWithIgnoreCase(const std::string& s, const std::string& suffix){
    return ToLower(s).ends_with(ToLower(suffix));
  }
}