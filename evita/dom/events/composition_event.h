// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_composition_event_h)
#define INCLUDE_evita_dom_events_composition_event_h

#include "evita/dom/events/ui_event.h"

#include <vector>

namespace domapi {
struct TextCompositionEvent;
}

namespace dom {

class CompositionEventInit;
class Window;

namespace bindings {
class CompositionEventClass;
}

class CompositionEvent : public v8_glue::Scriptable<CompositionEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(CompositionEvent)
  friend class bindings::CompositionEventClass;

  private: std::vector<uint8_t> attributes_;
  private: int caret_;
  private: base::string16 data_;

  public: CompositionEvent(const base::string16& type,
                           const CompositionEventInit& init_dict);
  private: explicit CompositionEvent(const base::string16& type);
  public: explicit CompositionEvent(const domapi::TextCompositionEvent& event);
  public: virtual ~CompositionEvent();

  private: std::vector<uint8_t> attributes() const { return attributes_; }
  private: int caret() const { return caret_; }
  private: base::string16 data() const { return data_; }

  DISALLOW_COPY_AND_ASSIGN(CompositionEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_composition_event_h)
