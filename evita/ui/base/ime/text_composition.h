// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_base_ime_text_composition_h)
#define INCLUDE_evita_ui_base_ime_text_composition_h

#include "base/strings/string16.h"

#include <vector>

namespace ui {

class TextComposition  {
  public: struct Span {
    enum Type {
      Input,
      SelectionConverted,
      Converted,
      SelectionNotConverted,
      InputError,
      FixedConverted,
    };
    int end;
    int start;
    Type type;

    Span();
  };

  private: int caret_;
  private: std::vector<Span> spans_;
  private: base::string16 text_;

  public: TextComposition(const base::string16& text,
                          const std::vector<Span> spans);
  public: TextComposition();
  public: ~TextComposition();

  public: int caret() const { return caret_; }
  public: void set_caret(int new_caret) { caret_ = new_caret; }
  public: const std::vector<Span>& spans() const { return spans_; }
  public: void set_spans(const std::vector<Span>& new_spans) {
      spans_ = new_spans;
  }
  public: const base::string16& text() const { return text_; }
  public: void set_text(const base::string16& new_text) { text_ = new_text; }
};

} // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_ime_text_composition_h)
