#include "event_list.h"

namespace nano_edr {

EventList::~EventList() {
    Clear();
}
void EventList::PushBack(const Event& event) {
    EventNode* event_node_ptr = new EventNode(event);
    if (tail_ == nullptr) {
        head_ = event_node_ptr;
        tail_ = event_node_ptr;
    } else {
        tail_->next = event_node_ptr;
        tail_ = event_node_ptr;
    }
    size_++;
    if (capacity_ > 0 && size_ > capacity_) {
        PopFront();
    }
}

void EventList::PopFront() {
    if (head_ == nullptr) {
        return;
    }
    EventNode* new_head_ptr = head_->next;
    delete head_;
    head_ = new_head_ptr;
    if (head_ == nullptr) {
        tail_ = nullptr;
    }
    size_--;
}

void EventList::Clear() {
    while (size_ > 0) {
        PopFront();
    }
}
}