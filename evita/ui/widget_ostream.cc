// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/widget.h"
#include "evita/ui/widget_ostream.h"

std::ostream& operator<<(std::ostream& out, const ui::Widget& widget) {
  out << "{" << widget.class_name() << "@" << std::hex <<
    reinterpret_cast<uintptr_t>(&widget) << " " <<
    std::dec << widget.bounds() << "}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const ui::Widget* widget) {
  if (widget)
    return out << *widget;
  return out << "null";
}
