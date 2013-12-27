// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_gc_visitor_h)
#define INCLUDE_evita_gc_visitor_h

namespace gc {

class Visitable;

namespace internal {
class AbstractCollectable;
}

class Visitor {
  public: typedef internal::AbstractCollectable Collectable;
  public: Visitor() = default;
  public: virtual ~Visitor() = default;
  public: virtual void Visit(Collectable* collectable) = 0;
  public: virtual void Visit(Visitable* visitable) = 0;
  DISALLOW_COPY_AND_ASSIGN(Visitor);
};

}  // namespace gc

#endif //!defined(INCLUDE_evita_gc_visitor_h)
