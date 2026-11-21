#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H
#include <cstddef>
#include <map>
#include <string>
#include "entity_model.h"
#include "ring_buffer.h"
#include "event_list.h"
#include "event_source.h"
#include "rule_engine.h"
#include <event.h>
#include <alert_sink.h>

namespace nano_edr {
struct WindowEntry {
    Timestamp ts;
    std::string type;
    std::string pid;
};
class Agent: public IEventHandler {
 public:
    static constexpr std::size_t kWindowCapacity = 256;
    explicit Agent(bool quiet, uint64_t dedup_period_ms = 10000);
    void HandleEvent(const Event& event) override;
    void PrintSummary() const;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;
    void Subscribe(AlertSink sink);

 private:
    void PrintContext() const;
    RingBuffer<WindowEntry, kWindowCapacity> window_;
    EntityModel model_;
    RuleEngine engine_;
    bool quiet_ = false;
    long long all_events_ = 0;
    std::map<std::string, long long> event_types_count_;
    long long skipped_ = 0;
    long long broken_ = 0;
    bool printed_context_ = false;
    void PrintContextOnce();
    uint64_t dedup_period_ms_{};
    std::size_t suppressed_{};
};

}

#endif