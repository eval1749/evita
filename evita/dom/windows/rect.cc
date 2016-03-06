// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/rect.h"

#include "evita/dom/public/float_point.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/float_size.h"
#include "evita/dom/v8_strings.h"

namespace gin {

bool Converter<gfx::FloatRect>::FromV8(v8::Isolate* isolate,
                                       v8::Local<v8::Value> val,
                                       gfx::FloatRect* out) {
  if (val.IsEmpty() || !val->IsObject())
    return false;
  auto const obj = val->ToObject();
  if (!obj->GetConstructorName()->Equals(dom::v8Strings::Rect.Get(isolate)))
    return false;
  float x;
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::x.Get(isolate)), &x))
    return false;
  float y;
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::y.Get(isolate)), &y))
    return false;
  float width;
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::width.Get(isolate)),
                     &width)) {
    return false;
  }
  float height;
  if (!ConvertFromV8(isolate, obj->Get(dom::v8Strings::height.Get(isolate)),
                     &height)) {
    return false;
  }
  *out = gfx::FloatRect(gfx::FloatPoint(x, y), gfx::FloatSize(width, height));
  return true;
}

v8::Local<v8::Value> Converter<gfx::FloatRect>::ToV8(
    v8::Isolate* isolate,
    const gfx::FloatRect& rect) {
  auto const context = isolate->GetCurrentContext();
  auto const rect_ctor =
      context->Global()->Get(dom::v8Strings::Rect.Get(isolate));
  v8::Local<v8::Value> argv[]{
      ConvertToV8(isolate, rect.x()), ConvertToV8(isolate, rect.y()),
      ConvertToV8(isolate, rect.width()), ConvertToV8(isolate, rect.height()),
  };
  return rect_ctor->ToObject()->CallAsConstructor(4, argv);
}

}  // namespace gin
