#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H
#include <cstddef>
#include <map>
#include <string>
#include "event.h"
#include "event_list.h"
#include "event_source.h"
#include "rule_engine.h"

namespace nano_edr {

class Agent: public IEventHandler {
 public:
    Agent(std::size_t window_size, bool quiet);
    void HandleEvent(const Event& event) override;
    void PrintSummary() const;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;

 private:
    std::size_t CheckRulesAndPrint(const Event& event);
    void PrintContext() const;

    EventList window_;
    RuleEngine engine_;
    bool quiet_ = false;
    long long all_events_ = 0;
    std::map<std::string, long long> event_types_count_;
    const EventNode* context_ = nullptr;
    long long skipped_ = 0;
    long long broken_ = 0;
};

}

#endif