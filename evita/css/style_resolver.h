// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_style_resolver_h)
#define INCLUDE_evita_css_style_resolver_h

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/strings/string16.h"
#include "evita/css/style_sheet_observer.h"

namespace css {

class Style;
class StyleSheet;

class StyleResolver : private StyleSheetObserver {
  private: typedef std::unordered_map<base::string16,
                                      std::unique_ptr<Style>> StyleCache;

  private: mutable StyleCache partial_style_cache_;
  private: std::vector<const StyleSheet*> style_sheets_;
  private: mutable StyleCache style_cache_;

  public: StyleResolver();
  public: ~StyleResolver();

  public: void AddStyleSheet(const StyleSheet* style_sheet);
  private: void ClearCache();
  private: void InvalidateCache(const StyleRule*rule);
  public: void RemoveStyleSheet(const StyleSheet* style_sheet);
  public: const Style& Resolve(const base::string16& selector) const;
  public: const Style& ResolveWithoutDefaults(
      const base::string16& selector) const;

  // css::StyleSheetObserver
  private: virtual void DidAddRule(const StyleRule* rule) override;
  private: virtual void DidRemoveRule(const StyleRule* rule) override;

  DISALLOW_COPY_AND_ASSIGN(StyleResolver);
};

}  // namespace css

#endif //!defined(INCLUDE_evita_css_style_resolver_h)
