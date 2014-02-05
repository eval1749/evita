// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_init_dict_h)
#define INCLUDE_evita_dom_init_dict_h

#include "base/basictypes.h"
#include "evita/v8_glue/v8.h"

namespace dom {

class InitDict {
  protected: enum class HandleResult {
    NotFound,
    Succeeded,
    CanNotConvert,
  };

  protected: InitDict();
  public: virtual ~InitDict();

  protected: v8::Isolate* isolate() const;

  protected: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) = 0;
  public: bool Init(v8::Handle<v8::Object> dict);

  DISALLOW_COPY_AND_ASSIGN(InitDict);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_init_dict_h)
