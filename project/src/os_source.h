#pragma once
#include <string>
#include "event_source.h"
#include "os_handle.h"
#include "os.h"
namespace nano_edr{
  class OsSource: public IEventSource{
    public:
    explicit OsSource(const std::string& path);
    void Run(IEventHandler* handler) override;
    private:
    static void Trampoline(const os_event* ev, void* ctx) noexcept;
    OsHandle handle_;
  };
}