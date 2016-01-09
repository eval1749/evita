// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EVENTS_TEXT_DOCUMENT_EVENT_H_
#define EVITA_DOM_EVENTS_TEXT_DOCUMENT_EVENT_H_

#include "base/strings/string16.h"
#include "evita/dom/events/event.h"
#include "evita/gc/member.h"

namespace dom {

class TextDocumentEventInit;
class TextDocumentWindow;

namespace bindings {
class TextDocumentEventClass;
}

class TextDocumentEvent final
    : public v8_glue::Scriptable<TextDocumentEvent, Event> {
  DECLARE_SCRIPTABLE_OBJECT(TextDocumentEvent)

 public:
  // Expose for|TextDocumentWindow|.
  TextDocumentEvent(const base::string16& type,
                    const TextDocumentEventInit& init_dict);
  ~TextDocumentEvent() final;

 private:
  friend class bindings::TextDocumentEventClass;

  explicit TextDocumentEvent(const base::string16& type);

  TextDocumentWindow* view() const { return view_.get(); }

  gc::Member<TextDocumentWindow> view_;

  DISALLOW_COPY_AND_ASSIGN(TextDocumentEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_TEXT_DOCUMENT_EVENT_H_
