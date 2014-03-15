// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/point.h"

#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// PointClass
//
class PointClass : public v8_glue::WrapperInfo {
  public: PointClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~PointClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto const templ = v8_glue::CreateConstructorTemplate(isolate,
        &PointClass::NewPoint);
    return templ;
  }

  private: static Point* NewPoint(float x, float y) {
    return new Point(x, y);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("x", &Point::x, &Point::set_x)
        .SetProperty("y", &Point::y, &Point::set_y);
  }

  DISALLOW_COPY_AND_ASSIGN(PointClass);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Point
//
DEFINE_SCRIPTABLE_OBJECT(Point, PointClass)

Point::Point(float x, float y) : x_(x), y_(y) {
}

Point::Point() {
}

Point::~Point() {
}

}  // namespace dom
