#include "agent.h"
#include <print>
#include "agent_rules.h"
#include "rule.h"

namespace nano_edr {

Agent::Agent( bool quiet)
    : window_(), engine_(BuildAgentRules()), quiet_(quiet) {}
void Agent::PrintContext() const {
    if(window_.empty()) return;
    if (window_.size() > 1) {
        std::print("[CTX] -2: {} {} pid={}\n",
                   window_[window_.size() - 2].ts.ms,
                   window_[window_.size() - 2].type,
                   window_[window_.size() - 2].pid);
    }
    std::print("[CTX] -1: {} {} pid={}\n",
                   window_.back().ts.ms,
                   window_.back().type,
                   window_.back().pid);
    }

void Agent::HandleEvent(const Event& event) {
    ++all_events_;
    ++event_types_count_[event.type()];
    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);
    if (!fired.empty() && !quiet_) {
        PrintContext();
    }
    for (const IRule* rule : fired) {
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(rule->severity()),
                   rule->id(),
                   event.raw_ts(),
                   event.pid());
    }
    window_.PushBack(WindowEntry{event.ts(), std::string(event.type()), std::string(event.pid())});
}

void Agent::PrintSummary() const {
    if (quiet_) return;
    std::print("Всего событий {}\n", all_events_);
    for (const auto& [type, count] : event_types_count_) {
        std::print("Событий типа {}: {}\n", type, count);
    }
    std::print("пропущено строк: {}\n", skipped_);
    std::print("битых строк: {}\n", broken_);
    std::print("правила, детектов:\n");
    for (std::size_t i = 0; i < engine_.size(); ++i) {
        std::print("  {}  {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
}
void Agent::OnSkipped() { ++skipped_; }
void Agent::OnBroken()  { ++broken_; }
void Agent::OnTick(){}
}