#include "rule_engine.h"
#include <cstddef>
#include "detection.h"

namespace nano_edr{
  void RuleEngine::AddRule(std::unique_ptr<IRule> rule){
    rules_.push_back(std::move(rule));
  }
  std::size_t RuleEngine::ProcessEvent(const Event& event,
                             std::vector<const IRule*>* fired){
    std::size_t res=0;
    for(const auto& r : rules_) {
      if(r->Check(event)) {
        ++res;
        if(fired != nullptr) fired->push_back(r.get());
        Detection d = MakeDetection(*r, event);
        for(const auto& s: sinks_) {
          s(d);
        }
      }
    }
    return res;
  }
  Detection RuleEngine::MakeDetection(const IRule& rule, const Event& event) const{
    Detection d;
    d.rule = rule.id();
    d.severity = rule.severity();
    d.ts = event.ts();
    d.pid = event.pid();
    d.chain = model_ != nullptr ? model_->DescribeChain(event.pid()) : "";
    d.actions = rule.actions();
    return d;
  }
  void RuleEngine::Subscribe(AlertSink sink) {sinks_.push_back(sink);}
}