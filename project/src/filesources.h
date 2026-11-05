#pragma once
#include <fstream>
#include "event_source.h"
namespace nano_edr{
  class FileSource : public IEventSource{
    public:
      explicit FileSource(const std::string& path);
      void Run(IEventHandler* handler) override;
    private:
      std::ifstream log_;
  };
}