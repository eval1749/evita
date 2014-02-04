// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_event_init_dict_h)
#define INCLUDE_evita_dom_events_event_init_dict_h

#include "evita/dom/init_dict.h"

namespace dom {

class EventInitDict : public InitDict {
  private: bool bubbles_;
  private: bool cancelable_;

  public: EventInitDict();
  public: virtual ~EventInitDict();

  public: bool bubbles() const { return bubbles_; }
  public: void set_bubbles(bool bubbles) { bubbles_ = bubbles; }
  public: bool cancelable() const { return cancelable_; }
  public: void set_cancelable(bool cancelable) { cancelable_ = cancelable; }

  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::String> key, v8::Handle<v8::Value> value) override;

  DISALLOW_COPY_AND_ASSIGN(EventInitDict);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_event_init_dict_h)
