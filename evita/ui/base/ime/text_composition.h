// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_BASE_IME_TEXT_COMPOSITION_H_
#define EVITA_UI_BASE_IME_TEXT_COMPOSITION_H_

#include <vector>

#include "base/strings/string16.h"

namespace ui {

class TextComposition final {
 public:
  struct Span final {
    // TODO(eval1749) We should use |enum class| for |Type|.
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

  TextComposition(const base::string16& text, const std::vector<Span> spans);
  TextComposition();
  ~TextComposition();

  int caret() const { return caret_; }
  void set_caret(int new_caret) { caret_ = new_caret; }
  const std::vector<Span>& spans() const { return spans_; }
  void set_spans(const std::vector<Span>& new_spans) { spans_ = new_spans; }
  const base::string16& text() const { return text_; }
  void set_text(const base::string16& new_text) { text_ = new_text; }

 private:
  int caret_;
  std::vector<Span> spans_;
  base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(TextComposition);
};

}  // namespace ui

#endif  // EVITA_UI_BASE_IME_TEXT_COMPOSITION_H_
