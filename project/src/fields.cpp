#include "fields.h"
#include <stdexcept>
#include <limits>
#include "text.h"
#include <cctype>


namespace nano_edr{
  const std::string* FindField(const Event& event, const std::string& key){
    if (key == "ts")   return &event.raw_ts();
    if (key == "type") return &event.type();
    if (key == "pid")  return &event.pid();
    for (const Field& field : event.fields()) {
        if (field.key == key) {
            return &field.value;
        }
    }
    return nullptr; // не нашли поле но оно не было обязательным
  }
  const std::string& GetRequiredField(const Event& event, const std::string& key){
    const std::string* value = FindField(event, key);
    if (value == nullptr) {
        throw std::invalid_argument("Missed requiered : " + key); // нарушение контракта (поле должны было быть а его нет)
    }
    return *value;
  }
  //строка в число с проверкой на отстутствие не цифр
  bool ParseUint64(const std::string& s, uint64_t* out) {
    if (s.empty()) return false;
    uint64_t res = 0;
    constexpr uint64_t max = std::numeric_limits<uint64_t>::max();   // UINT64_MAX
    for (char c : s) {
        if (c < '0' || c > '9') return false; // не распарсили потому что есть что то кроме цифр
        const uint64_t digit = static_cast<uint64_t>(c - '0');
        if (res > (max - digit) / 10) return false; // не распарсили потому что слишком большое для uint64
        res = res * 10 + digit;
    }
    *out = res;
    return true;
  }
  bool GetIntField(const Event& event, const std::string& key, uint64_t* out) {
    const std::string* value = FindField(event, key);
    if (value == nullptr) return false;
    return ParseUint64(*value, out); //вренем false если поле не uint64
  }

  uint64_t GetIntField(const Event& event, const std::string& key, uint64_t fallback) {
    uint64_t out = 0;
    if (!GetIntField(event, key, &out)) return fallback; // возвращаем значение по умолчанию потому что отсутствие значение или битое поле в данном случае ок
    return out;
  }

  bool IsProcessStart(const Event& event){
    return event.type() == "process_start"; //  нет исключений тип либо такой либо неважно какой
  }
  bool IsFileWrite(const Event& event){
//  нет исключений тип либо такой либо неважно какой
    return event.type() == "file_write" || event.type() == "file_create" || event.type() == "file_move";
  }
  bool IsNetConnect(const Event& event){
//  нет исключений тип либо такой либо неважно какой
    return event.type() == "net_connect";
  }
  bool PathEndsWith(const Event& event, const std::string& suffix){
    const std::string* path = FindField(event, "path");
    if (!path){
      return false; // поля может не быть и это ок, но тогда оно не закачивается на суффикс поэтому false
    }
    return EndsWithIgnoreCase(*path, suffix);
  }
  void ReplacePath(std::string&  str, const std::string&  basic_string, const std::string&  replacement) {
    size_t basic_pos = str.find(basic_string, 0);
    while(basic_pos != std::string::npos) {
      str.replace(basic_pos, basic_string.length(), replacement);
      basic_pos = str.find(basic_string, basic_pos+replacement.length());
    }
  }
  bool CommandLineContains(const Event& event, const std::string& needle){
    const std::string* cmdline_val = FindField(event, "cmdline");
    if (!cmdline_val) {
      return false; // поля может не быть и это ок, но тогда оно не закачивается на суффикс поэтому false
    }
    return ContainsIgnoreCase(*cmdline_val, needle);
  }
  // не падает если поля нет просто не итерируется по нему
  std::string NormalizePath(const std::string& path) {
    std::string res1="";
    for (std::size_t i = 0; i < path.length(); ++i) {
      const char c = path[i];
      if (c == '/' || c == '\\') {
        if (res1.empty() || res1.back() != '\\')  {
          res1 += '\\';
        }
      } else {
          res1 += static_cast<char>(std::tolower(c));
      }
    }
    //раскрытие %temp% и %tmp%
    const std::string temp = "%temp%";
    const std::string tmp  = "%tmp%";
    const std::string replacement = "\\appdata\\local\\temp";
    ReplacePath(res1, temp, replacement);
    ReplacePath(res1, tmp, replacement);
    return res1;
}

}

