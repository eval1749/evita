// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "evita/visuals/css/float_color.h"
#include "evita/visuals/display/public/display_item_visitor.h"
#include "evita/visuals/display/public/display_items.h"
#include "evita/visuals/geometry/float_rect.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// Printer
//
class Printer final : public DisplayItemVisitor {
 public:
  Printer() = default;
  ~Printer() final = default;

  void Print(std::ostream* ostream, const DisplayItem& item);

 private:
// DisplayItemVisitor
#define V(name) void Visit##name(name##DisplayItem* item) final;
  FOR_EACH_DISPLAY_ITEM(V)
#undef V
  std::ostream* ostream_;

  DISALLOW_COPY_AND_ASSIGN(Printer);
};

void Printer::Print(std::ostream* ostream, const DisplayItem& item) {
  *ostream << item.class_name() << '.' << item.id();
  ostream_ = ostream;
  Visit(item);
}

void Printer::VisitBeginClip(BeginClipDisplayItem* item) {
  *ostream_ << item->bounds();
}

void Printer::VisitBeginTransform(BeginTransformDisplayItem* item) {
  *ostream_ << item->matrix();
}

void Printer::VisitClear(ClearDisplayItem* item) {
  *ostream_ << ' ' << item->color();
}

void Printer::VisitDrawLine(DrawLineDisplayItem* item) {
  *ostream_ << item->point1() << '-' << item->point2() << ' ' << item->color()
            << " thickness=" << item->thickness();
}

void Printer::VisitDrawRect(DrawRectDisplayItem* item) {
  *ostream_ << item->bounds() << ' ' << item->color()
            << " thickness=" << item->thickness();
}

void Printer::VisitDrawText(DrawTextDisplayItem* item) {
  *ostream_ << item->bounds() << ' ' << item->color()
            << " baseline=" << item->baseline() << " \"";
  for (const auto& char_code : item->text()) {
    if (char_code < 0x20 || char_code >= 0x7F) {
      *ostream_ << base::StringPrintf("\\u%04X", char_code);
      continue;
    }
    if (char_code == '"' || char_code == '\\')
      *ostream_ << '\\';
    *ostream_ << static_cast<char>(char_code);
  }
  *ostream_ << '"';
}

void Printer::VisitEndClip(EndClipDisplayItem* item) {}
void Printer::VisitEndTransform(EndTransformDisplayItem* item) {}

void Printer::VisitFillRect(FillRectDisplayItem* item) {
  *ostream_ << item->bounds() << ' ' << item->color();
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const DisplayItem& item) {
  Printer().Print(&ostream, item);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const DisplayItem* item) {
  if (!item)
    return ostream << "nullptr";
  return ostream << *item;
}

}  // namespace visuals
