#include "parse.h"

namespace nano_edr{
  const char* ws = " \t";

  bool IsBlankOrComment(const std::string& line){
    if(line.empty()) {
      return true;
    }
    size_t pos= line.find_first_not_of(ws);
    if (pos == std::string::npos) {
      return true;
    }
    return line[pos] == '#' || line[pos] == ';';
  }

    bool ParseEventParts(const std::string& line, EventParts* out){
    if(!out || IsBlankOrComment(line)) {
      return false;
    }
    *out = EventParts{};
    std::size_t i = 0;
    std::size_t n = line.length();
    while (i < n) {
        while (i < n && (line[i] == ' ' || line[i] == '\t')) {
            i++;
        }
        if (i >= n) break;
        //  поиск ключа
        std::string key;
        while (i < n && line[i] != '=' && line[i] != ' ' && line[i] != '\t') {
            key += line[i];
            i++;
        }
        if (key.empty() || i >= n || line[i] != '=') {
            return false;
        }
        i++;
        // поиск значения
        std::string value;
        if (i < n && line[i] == '"') {
            i++;
            while (i < n && line[i] != '"') {
                value += line[i];
                i++;
            }
            if (i >= n || line[i] != '"') {
                return false;
            }
            i++; // Пропуск закрывающей кавычки
            if (i < n && line[i] != ' ' && line[i] != '\t') {
                return false;
            }
        } else {
            while (i < n && line[i] != ' ' && line[i] != '\t') {
                value += line[i];
                i++;
            }
        }
        // Сохранение пары в структуру
        if (key == "ts" && out->ts.empty()) {
            out->ts = value;
        } else if (key == "type" && out->type.empty()) {
            out->type = value;
        } else if (key == "pid" && out->pid.empty()) {
            out->pid = value;
        } else {
            out->fields.push_back({key, value});
        }
    }
    return true;
  }

}



