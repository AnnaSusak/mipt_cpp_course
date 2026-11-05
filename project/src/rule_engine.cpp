#include "rule_engine.h"
#include <cstddef>

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
      }
    }
    return res;
  }
}