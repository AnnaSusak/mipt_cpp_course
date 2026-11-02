#include "os_source.h"
#include "os.h"
#include <stdexcept>
#include <string>
#include "os_handle.h"
#include <print>
namespace nano_edr{
  OsSource::OsSource(const std::string& path): handle_(path) {}
  void OsSource::Run(IEventHandler* handler){
    handle_.Subscribe(&OsSource::Trampoline, handler);
    handle_.Start();
    while(true) {
      os_status status = handle_.Wait(100);
      if (status == OS_TIMEOUT){
         handler->OnTick();
         continue;
      }
      if (status == OS_OK) break;
      throw std::runtime_error("os_wait failed: " + std::string(os_status_str(status)));
    }
  }
  void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept {
    auto* handler = static_cast<IEventHandler*>(ctx);
    try {
        EventParts parts;
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type;
        if (ev->pid == 0) parts.pid = "";
        else parts.pid = std::to_string(ev->pid);
        for (size_t i = 0; i < ev->field_count; ++i) {
            parts.fields.push_back({ev->fields[i].key, ev->fields[i].value});
        }
        Event e(parts);
        handler->HandleEvent(e);
    } catch (const std::exception& ex) {
        handler->OnBroken();
        std::print(stderr, "пропущено событие (os.h): {}\n", ex.what());
    } catch (...) {
      handler->OnBroken();
        std::print(stderr, "пропущено событие (os.h): неизвестная ошибка\n");
    }
}
}