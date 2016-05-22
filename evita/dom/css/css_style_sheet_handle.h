// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_CSS_CSS_STYLE_SHEET_HANDLE_H_
#define EVITA_DOM_CSS_CSS_STYLE_SHEET_HANDLE_H_

#include <memory>

#include "evita/ginx/scriptable.h"

namespace css {
class StyleSheet;
}

namespace dom {

namespace bindings {
class CSSStyleSheetHandleClass;
}

class ExceptionState;

//////////////////////////////////////////////////////////////////////
//
// CSSStyleSheetHandle
//
class CSSStyleSheetHandle final : public ginx::Scriptable<CSSStyleSheetHandle> {
  DECLARE_SCRIPTABLE_OBJECT(CSSStyleSheetHandle);

 public:
  ~CSSStyleSheetHandle() final;

  css::StyleSheet* value() const { return object_.get(); }

 private:
  friend class bindings::CSSStyleSheetHandleClass;

  // Bindings implementations
  CSSStyleSheetHandle();

  static void AppendStyleRule(CSSStyleSheetHandle* handle,
                              const base::string16& selector,
                              v8::Local<v8::Map> raw_style);
  static int CountRules(CSSStyleSheetHandle* handle);
  static void DeleteRule(CSSStyleSheetHandle* handle, int index);
  static void InsertStyleRule(CSSStyleSheetHandle* handle,
                              const base::string16& selector,
                              v8::Local<v8::Map> raw_style,
                              int index);
  static v8::Local<v8::Map> RuleAt(CSSStyleSheetHandle* handle,
                                   int index,
                                   ExceptionState* exception_state);

  std::unique_ptr<css::StyleSheet> object_;

  DISALLOW_COPY_AND_ASSIGN(CSSStyleSheetHandle);
};

}  // namespace dom

#endif  // EVITA_DOM_CSS_CSS_STYLE_SHEET_HANDLE_H_
