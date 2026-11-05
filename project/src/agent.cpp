#include "agent.h"
#include <print>
#include "agent_rules.h"
#include "rule.h"

namespace nano_edr {

Agent::Agent(std::size_t window_size, bool quiet)
    : window_(window_size), engine_(BuildAgentRules()), quiet_(quiet) {}

std::size_t Agent::CheckRulesAndPrint(const Event& event) {
    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);
    for (const IRule* rule : fired) {
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                   SeverityName(rule->severity()),
                   rule->id(),
                   event.raw_ts(),
                   event.pid());
    }
    return fired.size();
}
void Agent::PrintContext() const {
    if (window_.size() > 1) {
        std::print("[CTX] -2: ts={} type={} pid={}\n",
                   context_->event.raw_ts(),
                   context_->event.type(),
                   context_->event.pid());
        std::print("[CTX] -1: ts={} type={} pid={}\n",
                   window_.back()->event.raw_ts(),
                   window_.back()->event.type(),
                   window_.back()->event.pid());
    } else if (window_.size() == 1) {
        std::print("[CTX] -1: ts={} type={} pid={}\n",
                   window_.back()->event.raw_ts(),
                   window_.back()->event.type(),
                   window_.back()->event.pid());
    }
}

void Agent::HandleEvent(const Event& event) {
    ++all_events_;
    ++event_types_count_[event.type()];
    const std::size_t det_count = CheckRulesAndPrint(event);
    if (det_count > 0 && !quiet_) {
        PrintContext();
    }
    window_.PushBack(event);
    if (window_.size() <= 2) {
        context_ = window_.head();
    } else {
        context_ = context_->next;
    }
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