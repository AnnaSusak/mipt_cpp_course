#include "field_traits.h"
#include <charconv>
#include "text.h"
namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string& text, std::string* out) {
    *out = text;
    return true;
}
bool FieldTraits<uint64_t>::Parse(const std::string& text, uint64_t* out) {
    if (text.empty()) return false;
    uint64_t value = 0;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ptr == text.data() + text.size() && ec == std::errc()) {
        *out = value;
        return true;
    }
    return false;
}
bool FieldTraits<bool>::Parse(const std::string& text, bool* out) {
    if (EqualsIgnoreCase(text, "1") || EqualsIgnoreCase(text, "true") || EqualsIgnoreCase(text, "yes")) {
        *out = true;
        return true;
    }
    if (EqualsIgnoreCase(text, "0") || EqualsIgnoreCase(text, "false") || EqualsIgnoreCase(text, "no")) {
        *out = false;
        return true;
    }
    return false;
}

}
