// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_VISUALS_CSS_STYLE_SHEET_HANDLE_H_
#define EVITA_DOM_VISUALS_CSS_STYLE_SHEET_HANDLE_H_

#include <memory>

#include "evita/v8_glue/scriptable.h"

namespace visuals {
namespace css {
class StyleSheet;
}
}

namespace dom {

namespace bindings {
class CSSStyleSheetHandle;
}

//////////////////////////////////////////////////////////////////////
//
// CSSStyleSheetHandle
//
class CSSStyleSheetHandle final
    : public v8_glue::Scriptable<CSSStyleSheetHandle> {
  DECLARE_SCRIPTABLE_OBJECT(CSSStyleSheetHandle);

 public:
  ~CSSStyleSheetHandle() final;

 private:
  friend class bindings::CSSStyleSheetHandleClass;

  // Bindings implementations
  int CountRules();
  void DeleteRule(int index);
  void InsertStyleRule(const base::string16& selector,
                       v8::Local<v8::Map> raw_style,
                       int index);
  v8::Local<v8::Map> RuleAt(int index);

  std::unique_ptr<css::StyleSheet> object_;

  DISALLOW_COPY_AND_ASSIGN(CSSStyleSheetHandle);
};

}  // namespace dom

#endif  // EVITA_DOM_VISUALS_CSS_STYLE_SHEET_HANDLE_H_
