// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_either_h)
#define INCLUDE_evita_v8_glue_either_h

namespace v8_glue {
// The template Either<Left, Right> is inspired by Haskell's Either data type.
// You can use this as:
//  void MyJsMethod(Either<int, float> either) {
//    if (either.is_left)
//      MyCxxMethod(either.left;
//    else
//      MyCxxMethod(either.right);
//  }
//
// Note: Since Either<T> is pseudo object, we don't have gin::Converter.
template<typename Left, typename Right>
struct Either {
  public: Left left;
  public: Right right;
  public: bool is_left;
  explicit Either(Left left) : is_left(true), left(left) {
  }
  explicit Either(Right right) : is_left(false), right(right) {
  }
  Either() : is_left(false) {
  }
};
}  // namespace v8_glue

namespace gin {
template<typename Left, typename Right>
struct Converter<v8_glue::Either<Left, Right>> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
      const v8_glue::Either<Left, Right>& either) {
    return either.is_left ? ConvertToV8(isolate, either.left) :
                            ConvertToV8(isolate, either.right);
  }
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     v8_glue::Either<Left, Right>* out) {
   out->is_left = Converter<Left>::FromV8(isolate, val, &out->left);
   return out->is_left || Converter<Right>::FromV8(isolate, val, &out->right);
}
};
} // gin

#endif //!defined(INCLUDE_evita_v8_glue_either_h)
