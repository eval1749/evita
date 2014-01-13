// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_contents_content_h)
#define INCLUDE_evita_dom_contents_content_h

#include "base/strings/string16.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {
namespace contents {

//////////////////////////////////////////////////////////////////////
//
// Content
//
class Content : public v8_glue::Scriptable<Content> {
  DECLARE_SCRIPTABLE_OBJECT(Content);

  private: base::string16 name_;

  protected: explicit Content(const base::string16& name);
  public: virtual ~Content();

  public: const base::string16& name() const { return name_; }

  DISALLOW_COPY_AND_ASSIGN(Content);
};

}  // namespace contents
}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_contents_content_h)
