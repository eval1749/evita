// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_optional_h)
#define INCLUDE_evita_v8_glue_optional_h

namespace v8_glue {
// The template Optional<T> is a placeholder in function parameter list.
// You can use this as:
//  * Use with default value
//  void MyJsMethod(int first, Optional<int> second) {
//    MyCxxMethod(first, second.get(-1));
//  }
//
//  * Call C++ function by arity
//  void MyJsMethod(int first, Optional<int> second) {
//    if (second.is_supplied)
//      MyCxxMethod(first, second.value);
//    else
//      MyCxxMethod(first);
//  }
//
// Note: Since Optional<T> is pseudo object, we don't have gin::Converter.
template<typename T>
struct Optional {
  public: T value;
  public: bool is_supplied;
  public: T get(const T& default_value) {
    return is_supplied ? value : default_value;
  }
};
}  // namespace v8_glue

namespace gin {
class Arguments;
namespace internal {
template<typename T>
bool GetNextArgument(Arguments* args, int, bool,
                     v8_glue::Optional<T>* result) {
  if (args->HasNext()) {
    result->is_supplied = true;
    return args->GetNext(&result->value);
  }
  result->is_supplied = false;
  return true;
}
} // internal
} // gin

#endif //!defined(INCLUDE_evita_v8_glue_optional_h)
