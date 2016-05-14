// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_MODELS_STYLE_RESOLVER_H_
#define EVITA_TEXT_STYLE_MODELS_STYLE_RESOLVER_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/base/strings/atomic_string.h"
#include "evita/text/style/models/style_sheet_observer.h"

namespace base {
class AtomicString;
}

namespace css {

class Style;
class StyleSheet;

class StyleResolver final : private StyleSheetObserver {
 public:
  StyleResolver();
  ~StyleResolver();

  void AddStyleSheet(const StyleSheet* style_sheet);
  void RemoveStyleSheet(const StyleSheet* style_sheet);

  const Style& Resolve(const base::string16& selector) const;
  const Style& Resolve(base::AtomicString selector) const;

  const Style& ResolveWithoutDefaults(const base::string16& selector) const;
  const Style& ResolveWithoutDefaults(base::AtomicString selector) const;

 private:
  using StyleCache =
      std::unordered_map<base::AtomicString, std::unique_ptr<Style>>;

  void ClearCache();
  void InvalidateCache(const StyleRule* rule);

  // css::StyleSheetObserver
  void DidAddRule(const StyleRule* rule) override;
  void DidRemoveRule(const StyleRule* rule) override;

  mutable StyleCache partial_style_cache_;
  std::vector<const StyleSheet*> style_sheets_;
  mutable StyleCache style_cache_;

  DISALLOW_COPY_AND_ASSIGN(StyleResolver);
};

}  // namespace css

#endif  // EVITA_TEXT_STYLE_MODELS_STYLE_RESOLVER_H_
