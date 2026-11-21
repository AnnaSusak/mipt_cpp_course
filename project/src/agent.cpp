#include "agent.h"
#include <print>
#include "agent_rules.h"
#include "dedup_sink.h"

namespace nano_edr {

Agent::Agent(bool quiet, uint64_t dedup_period_ms)
    : engine_(BuildAgentRules(&model_)),
      quiet_(quiet),
      dedup_period_ms_(dedup_period_ms) {
    engine_.SetModel(&model_);
}
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
    model_.Observe(event);
    ++all_events_;
    ++event_types_count_[event.type()];
    std::vector<const IRule*> fired;
    printed_context_ = false;
    engine_.ProcessEvent(event, &fired);
    window_.PushBack(WindowEntry{event.ts(), std::string(event.type()), std::string(event.pid())});
}

void Agent::PrintSummary() const {
    if (quiet_) return;
    std::print("[STAT] в модели процессов {}, файлов {}\n", model_.process_count(), model_.file_count());
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
    if (suppressed_ > 0) {
        std::print("[STAT] подавлено дубликатов {} (срок {} мс)\n",
                   suppressed_, dedup_period_ms_);
    }
}
void Agent::OnSkipped() { ++skipped_; }
void Agent::OnBroken()  { ++broken_; }
void Agent::OnTick(){}
void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    all.Add([this](const Detection&) { PrintContextOnce(); });
    all.Add(std::move(sink));
    engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));

}
void Agent::PrintContextOnce() {
    if (printed_context_ || quiet_) {
        return;
    }
    PrintContext();
    printed_context_ = true;
}
}