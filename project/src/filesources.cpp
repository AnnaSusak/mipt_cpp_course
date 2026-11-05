#include "agent.h"
#include "filesources.h"
#include <fstream>
#include <parse.h>
#include <event.h>
#include <print>


namespace nano_edr{
  FileSource::FileSource(const std::string& path) : log_(path) {
    if (!log_) {
        throw std::runtime_error("не удалось открыть журнал: " + path);
    }
  }
  void FileSource::Run(IEventHandler* handler) {
    std::string line;
    while (std::getline(log_, line)) {
        try {
            if (IsBlankOrComment(line)) {
                continue;
            }
            EventParts event_parts;
            if (!ParseEventParts(line, &event_parts)) {
                continue;
            }
            Event event(event_parts);
            handler->HandleEvent(event);
        } catch (const std::exception& e) {
            std::print(stderr, "пропущено событие: {}\n", e.what());
            continue;
        } catch (...) {
            std::print(stderr, "пропущено событие: неизвестная ошибка\n");
            continue;
        }
    }
  }
}