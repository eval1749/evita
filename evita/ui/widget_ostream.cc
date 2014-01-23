// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/widget.h"

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget) {
  const auto& rect = widget.rect();
  out << "{" << widget.class_name() << "@" << std::hex <<
    reinterpret_cast<uintptr_t>(&widget) << std::dec << " (" << rect.left <<
    "," << rect.top << ")+(" << rect.width() << "x" << rect.height() << ")}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const ui::Widget* widget) {
  if (widget)
    return out << *widget;
  return out << "null";
}
