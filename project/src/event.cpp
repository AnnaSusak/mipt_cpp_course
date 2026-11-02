#include "event.h"
#include <fields.h>
#include <parse.h>
#include <stdexcept>
#include <charconv>


namespace nano_edr{
  Event::Event(const EventParts& parts) {
    if(parts.type.empty()) {
      throw std::invalid_argument("пустой тип события");
    }
    uint64_t parsed_ts = 0;
    const char* begin = parts.ts.data();
    const char* end   = begin + parts.ts.size();
    auto [ptr, ec] = std::from_chars(begin, end, parsed_ts);
    if (ec != std::errc{} || ptr != end) {
        throw std::invalid_argument("неверное время события: " + parts.ts);
    }
    ts_ = Timestamp{parsed_ts};
    raw_ts_ = parts.ts;
    type_ = parts.type;
    event_type_=EventTypeFromString(type_);
    pid_ = parts.pid;
    fields_ = parts.fields;
  }
  std::string ToString(const Event& event) {
    std::string res = "ts=" + event.raw_ts() + " type=" + event.type();
    if (!event.pid().empty()) {
        res += " pid=" + event.pid();
    }
    for (const auto& field : event.fields()) {
        res += " " + field.key + "=" + field.value;
    }
    return res;
  }
  std::ostream& operator<<(std::ostream& os, const Event& event) {
    return os << ToString(event);
  }
  EventType EventTypeFromString(const std::string& type){
    if (type == "process_start") return EventType::kProcessStart;
    if (type == "file_create")   return EventType::kFileCreate;
    if (type == "file_write")    return EventType::kFileWrite;
    if (type == "file_move")     return EventType::kFileMove;
    if (type == "net_connect")   return EventType::kNetConnect;
    if (type == "process_end")   return EventType::kProcessEnd;
    if (type == "file_delete")   return EventType::kFileDelete;
    return EventType::kOther;
  }
}