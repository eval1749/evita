// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_COMPOSITION_EVENT_H_
#define EVITA_DOM_EVENTS_COMPOSITION_EVENT_H_

#include <vector>

#include "evita/dom/events/composition_span.h"
#include "evita/dom/events/ui_event.h"

namespace domapi {
struct TextCompositionEvent;
}

namespace dom {

class CompositionEventInit;
class Window;

namespace bindings {
class CompositionEventClass;
}

class CompositionEvent final
    : public ginx::Scriptable<CompositionEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(CompositionEvent)

 public:
  CompositionEvent(const base::string16& type,
                   const CompositionEventInit& init_dict);
  explicit CompositionEvent(const domapi::TextCompositionEvent& event);
  virtual ~CompositionEvent();

 private:
  friend class bindings::CompositionEventClass;

  explicit CompositionEvent(const base::string16& type);

  int caret() const { return caret_; }
  base::string16 data() const { return data_; }
  std::vector<CompositionSpan*> spans() const { return spans_; }

  int caret_;
  base::string16 data_;
  std::vector<CompositionSpan*> spans_;

  DISALLOW_COPY_AND_ASSIGN(CompositionEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_COMPOSITION_EVENT_H_
