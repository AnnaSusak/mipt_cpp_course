#include "os_handle.h"
#include "os.h"
#include <stdexcept>
#include <string>


namespace nano_edr{
  OsHandle::OsHandle(const std::string& config_path) {
    os_status status = os_init(config_path.c_str(), &handle_);
    if (status != OS_OK) {
        throw std::runtime_error(
            "os_init failed for " + config_path + os_status_str(status));
    }
  }
  OsHandle::~OsHandle() {
    os_free(handle_);
  }
  void OsHandle::Subscribe(os_event_cb callback, void* context) {
    os_status status = os_event_subscribe(handle_, callback, context);
    if (status != OS_OK) {
      throw std::runtime_error("os_subscribe failed " + std::string(os_status_str(status)));
    }
  }
  void OsHandle::Start() {
    os_status status = os_start(handle_);
    if (status != OS_OK) {
      throw std::runtime_error("os_start failed " + std::string(os_status_str(status)));
    }
  }
  os_status OsHandle::Wait(uint32_t timeout_ms) {
    return os_wait(handle_, timeout_ms);
  }
  void OsHandle::Stop() {
    os_stop(handle_);
  }
}


