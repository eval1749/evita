// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_visitable_h)
#define INCLUDE_evita_gc_visitable_h

namespace gc {

class Visitor;

class Visitable {
  public: Visitable() = default;
  public: ~Visitable() = default;

  public: virtual const char* visitable_class_name() const = 0;

  public: virtual void Accept(Visitor* visitor);

  DISALLOW_COPY_AND_ASSIGN(Visitable);
};

}  // namespace gc

#endif //!defined(INCLUDE_evita_gc_visitable_h)
