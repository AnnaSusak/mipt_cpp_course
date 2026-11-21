 #include "conditions.h"
#include <iterator>
#include "fields.h"
#include "text.h"
 namespace nano_edr{
bool FieldEquals::Matches(const Event& event) const{
  const std::string* v = FindField(event, key_);
  if (!v) return false;
  return EqualsIgnoreCase(*v, value_);
}
bool FieldContains::Matches(const Event& event) const{
  const std::string* v = FindField(event, key_);
  if (!v) return false;
  return ContainsIgnoreCase(*v, needle_);
}
bool FieldEndsWith::Matches(const Event& event) const{
  const std::string* v = FindField(event, key_);
  if (!v) return false;
  return EndsWithIgnoreCase(*v, suffix_);
}
bool FieldInList::Matches(const Event& event) const{
  const std::string* v = FindField(event, key_);
  if (!v) return false;
  for (const std::string& val : values_) {
    if (EqualsIgnoreCase(*v, val)) return true;
  }
  return false;
}
bool EventTypeIs::Matches(const Event& event) const{
  for (auto t: types_){
    if (event.event_type()==t) return true;
  }
  return false;
}
void AllOf::Add(std::unique_ptr<ICondition> condition){
  parts_.push_back(std::move(condition));
}
void AnyOf::Add(std::unique_ptr<ICondition> condition){
  parts_.push_back(std::move(condition));
}
bool AllOf::Matches(const Event& event) const{
  if(std::empty(parts_)) return true;
  for ( const auto& c : parts_){
    if (!c->Matches(event)) return false;
  }
  return true;
}
bool AnyOf::Matches(const Event& event) const{
  if(std::empty(parts_)) return false;
  for (const auto& c : parts_){
    if (c->Matches(event)) return true;
  }
  return false;
}
}

